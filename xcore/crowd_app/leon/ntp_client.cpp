/* Stripped-down & simplified ntpclient by tofu
 *
 * Copyright (C) 1997-2010  Larry Doolittle <larry@doolittle.boa.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License (Version 2,
 * June 1991) as published by the Free Software Foundation.  At the
 * time of writing, that license was published by the FSF with the URL
 * http://www.gnu.org/copyleft/gpl.html, and is incorporated herein by
 * reference.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#include "ntp_client.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>
#include <rtems.h>
#include <mvLog.h>
#include <Log.h>
#define JAN_1970  0x83aa7e80      /* 2208988800 1970 - 1900 in seconds */
#define NTP_PORT  123

/* How to multiply by 4294.967296 quickly (and not quite exactly)
 * without using floating point or greater than 32-bit integers.
 * If you want to fix the last 12 microseconds of error, add in
 * (2911*(x))>>28)
 */
#define NTPFRAC(x) ( 4294*(x) + ( (1981*(x))>>11 ) )

/* The reverse of the above, needed if we want to set our microsecond
 * clock (via settimeofday) based on the incoming time in NTP format.
 * Basically exact.
 */
#define USEC(x) ( ( (x) >> 12 ) - 759 * ( ( ( (x) >> 10 ) + 32768 ) >> 16 ) )

/* Converts NTP delay and dispersion, apparently in seconds scaled
 * by 65536, to microseconds.  RFC1305 states this time is in seconds,
 * doesn't mention the scaling.
 * Should somehow be the same as 1000000 * x / 65536
 */
#define sec2u(x) ( (x) * 15.2587890625 )

#define LI 0
#define VN 3
#define MODE 3
#define STRATUM 0
#define POLL 4
#define PREC -6

struct ntptime {
	unsigned int coarse;
	unsigned int fine;
};

bool send_packet(int sd, struct sockaddr* addr) {
	uint32_t data[12];
	struct timeval now;

	memset(data, 0, sizeof(data));
	data[0] = htonl (
		( LI << 30 ) | ( VN << 27 ) | ( MODE << 24 ) |
		( STRATUM << 16) | ( POLL << 8 ) | ( PREC & 0xff ) );
	data[1] = htonl(1 << 16);  /* Root Delay (seconds) */
	data[2] = htonl(1 << 16);  /* Root Dispersion (seconds) */
	gettimeofday(&now, NULL);
	data[10] = htonl(now.tv_sec + JAN_1970); /* Transmit Timestamp coarse */
	data[11] = htonl(NTPFRAC(now.tv_usec));  /* Transmit Timestamp fine   */
	return !(sendto(sd, data, sizeof(data), 0, addr, sizeof(struct sockaddr)) < 0);
}

void time_to_daytime(struct timeval *tv, rtems_time_of_day *daytime) {
    mvLog(MVLOG_DEBUG, "time_to_daytime tv_sec:%d tv_usec:%d\n",
           (int)tv->tv_sec, (int)tv->tv_usec);
    time_t rawtime = (time_t)tv->tv_sec;
    struct tm *timeinfo = gmtime(&rawtime);
    mvLog(MVLOG_DEBUG, "tm_year:%d tm_mon:%d tm_mday:%d tm_hour:%d tm_min:%d tm_sec:%d\n",
           timeinfo->tm_year, timeinfo->tm_mon, timeinfo->tm_mday,
           timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
    mvLog(MVLOG_DEBUG, "asctime:%s\n", asctime(timeinfo));
    // Transalte year and month as the format is different
    daytime->year = timeinfo->tm_year + 1900;
    daytime->month = timeinfo->tm_mon + 1;
    daytime->day = timeinfo->tm_mday;
    daytime->hour = timeinfo->tm_hour;
    daytime->minute = timeinfo->tm_min;
    daytime->second = timeinfo->tm_sec;
    daytime->ticks = (tv->tv_usec * 1000) / rtems_configuration_get_nanoseconds_per_tick();
}

int set_time(const uint32_t* data) {
	struct timeval tv;

	tv.tv_sec  = ntohl(((uint32_t *)data)[10]) - JAN_1970;
	tv.tv_usec = USEC(ntohl(((uint32_t *)data)[11]));
    rtems_time_of_day daytime;
    time_to_daytime(&tv, &daytime);
    rtems_status_code status = rtems_clock_set(&daytime);
    if (0 != status) {
        mvLog(MVLOG_ERROR,"FAILED TO SET CLOCK.\n");
        return 1;
    }

	return 0;		/* All good, time set! */
}

bool ntp_update_time(const char *server_ip) {
    uint32_t packet[12] = {0};
    bool success = false;

    struct sockaddr_in saddr;
    memset(&saddr, 0, sizeof(struct sockaddr_in));
    saddr.sin_family = AF_INET;
    saddr.sin_port   = htons(NTP_PORT);
    inet_pton(AF_INET, server_ip, &saddr.sin_addr.s_addr);

    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        return false;
    }

	/* Send NTP query to server ... */
	if (!send_packet(sock_fd, (struct sockaddr*)&saddr)) {
        LOGD << "Can not send data to ntp server for syncronization";
        goto _ntp_exit;
    }
    fd_set rfd;
    FD_ZERO(&rfd);
    FD_SET(sock_fd, &rfd);
    struct timeval timeout;
    timeout.tv_sec = 2;
    timeout.tv_usec = 0;

    if (select(sock_fd + 1, &rfd, NULL, NULL, &timeout) > 0) {
        size_t count = 0;
        int retry = 2;
        socklen_t lon = sizeof(struct sockaddr);
        do {
            rtems_task_wake_after(10);
            int len = recvfrom(sock_fd, (char*)packet + count, \
                sizeof(packet) - count, 0, (struct sockaddr*)&saddr, &lon);
            if (len >= 0) {
                count += len;
            } else {
                goto _ntp_exit;
            }
        } while (retry-- && count < sizeof(packet));
        if (count < sizeof(packet)) {
            goto _ntp_exit;
        }
    } else {
        goto _ntp_exit;
    }

    if (!set_time(packet)) {
        mvLog(MVLOG_DEBUG, "time syncronizaiton process all done");
        success = true;
    }

_ntp_exit:
	close(sock_fd);
	return success;
}

