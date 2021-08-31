#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <rtems.h>
#include <rtems/bspIo.h>
#include <fcntl.h>
#include "rtems/rtems_bsdnet.h" //rtems_bsdnet_initialize_network()
#include <assert.h>
#include "sys/types.h"
#include <sys/socketvar.h>
#include "sys/socket.h"
#include <netinet/in.h>
#include "netdb.h" //gethostbyname
#include <sys/ioctl.h>
#include "net/if.h"
#include "net/if_var.h"
#include "arpa/inet.h"
#include "sys/proc.h"
#include <bsp/greth_gbit.h>
#include <bsp.h>
#include "DrvGpio.h"
#include "DrvI2cMaster.h"
#include "DrvI2c.h"
#include "OsDrvCpr.h"
#include "OsDrvTimer.h"
#include "rtems/dhcp.h"
#include <rtems/score/threadimpl.h>
#include <rtems/score/todimpl.h>
#include "time.h"
#include <mvLog.h>
// 2:  Source Specific #defines and types  (typedef, enum, struct)
// ----------------------------------------------------------------------------
#define EPOCHOFFSET										2208988800U

#define NTPRETRIES										10

// 3: Gloval Variables
// ----------------------------------------------------------------------------

// 4: Static Local Data
static void fromTime2RTEMSTOD(struct tm	*time,  rtems_time_of_day* daytime)
{
    // Transalte year and month as the format is different
    daytime->year       = time->tm_year + 1900;
    daytime->month      = time->tm_mon + 1;
    daytime->day        = time->tm_mday;
    daytime->hour       = time->tm_hour;
    daytime->minute     = time->tm_min;
    daytime->second     = time->tm_sec;
    daytime->ticks      = 0;
}

// ----------------------------------------------------------------------------
void SNTP_TIME_SET(void *arg)
{
    UNUSED(arg);

    int i, s, j;
    char IP[20] = { "192.168.31.1" };
    struct sockaddr_in server_addr;
    struct in_addr **addr_list;
    struct hostent* host;
    socklen_t saddr_l;
    char msg[48] = {0xe3, 0};
    rtems_time_of_day daytime;
    struct tm result;
    time_t tmit;

    // Clear server address variable
    memset(&server_addr, 0, sizeof(server_addr));

    s = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (s < 0)
    {
        printf("\nERROR opening socket\n");
        pthread_exit(0);
    }
    Timestamp_Control      uptime;
    _TOD_Get(&uptime);
    uint32_t secs = _Timestamp_Get_seconds( &uptime );
    //uint32_t usecs = _Timestamp_Get_nanoseconds( &uptime ) / TOD_NANOSECONDS_PER_MICROSECOND;
    uint32_t usecs = _Timestamp_Get_nanoseconds( &uptime );
    printf("\nFirst......%d,%d.................\n", secs, usecs);
    server_addr.sin_family	= AF_INET;
    server_addr.sin_addr.s_addr = inet_addr(IP);
    server_addr.sin_port	= htons(123);
    j = 0;
        // send data
    do{
        i = sendto(s, msg, sizeof(msg), 0, (struct sockaddr*)&server_addr, sizeof(server_addr));
        if(i < 0){
            printf("\nSNTP request to %s failed\n", inet_ntoa(server_addr.sin_addr));
        }
        else
        {
            printf("\nSNTP request to %s:  %d bytes sent\n", inet_ntoa(server_addr.sin_addr), i);
            sleep(5);
            // get the data back
            saddr_l = sizeof(server_addr);
            i = recvfrom(s, msg, sizeof(msg), MSG_DONTWAIT, (struct sockaddr * __restrict)&server_addr, &saddr_l);
            if (i != sizeof(msg)){
                printf("\nSNTP response from %s failed\n", inet_ntoa(server_addr.sin_addr));
            } else {
                printf("\nSNTP response from %s : %d bytes received\n", inet_ntoa(server_addr.sin_addr), i);
            }
        }
        j ++;
    }  while((j < NTPRETRIES) && (i != sizeof(msg)));

    if (j < NTPRETRIES) {

        tmit = ntohl(((int*)msg)[8]); //read recv time
        // time_t is in seconds from Jan 1, 1900, convert it to Jan 1, 1970
        tmit -= EPOCHOFFSET;
        // Convert time from 'time_t' (00:00:00 UTC, January 1, 1970) to local time, struct 'tm'
        if (localtime_r((const time_t*)&tmit, &result)) {
            fromTime2RTEMSTOD(&result, &daytime);
            i = rtems_clock_set(&daytime);
        }
    }
    else
    {
        printf("\nNTP Server retries %d exceeded\n", NTPRETRIES);
    }

    return NULL; // just so the compiler thinks we returned something
}

