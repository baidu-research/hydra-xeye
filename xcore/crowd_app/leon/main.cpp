
/// @file
/// @copyright All code copyright Movidius Ltd 2014, all rights reserved.
///            For License Warranty see: common/license.txt
///
/// @brief     Variation of "IpipePPTest09, IpipePPTest09_a
///            PP inputs 960x128 and outputs 480x64 RGB data for VideoSipp
///            LOS starts LRT which executes the PP test.
///

// 1: Includes
// ----------------------------------------------------------------------------
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <rtems.h>
#include <pthread.h>
#include <sched.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mv_types.h>
#include <mvMacros.h>
#include <rtems/cpuuse.h>
#include <rtems/stackchk.h>
#include "bsp.h"
#include <DrvCpr.h>
#include <DrvLeon.h>
#include "deep.h"
#include "fathom_init.h"
#include "debug.h"
#include "telnet.h"

#include "time.h"
#include <rtems/score/todimpl.h>
#include "OsDrvTimer.h"
#include "LeonIPCApi.h"
#include "mvLog.h"
#include "mvTensorTimer.h"
#include "facemsg.h"
#include "los_sys_config.h"
#include "temperature.h"

#include "ntp_client.h"
#include <chrono>
#include <cJSON.h>
#include <fstream>
#include "params/params.hpp"
#include "utils/json_utils.hpp"
#include "utils/timer.hpp"
#include "http/http.hpp"
#include "pubdef.h"
#include "watchdog/watchdog.hpp"
#include "Log.h"
#include "Appenders/ConsoleAppender.h"
#include "Appenders/ColorConsoleAppender.h"
#include <cassert>
#include <atomic>
#include <sstream>
#include <memory>
#include <rtems.h>
#include "net.h"
#include "spiflash/spiflash.hpp"
#include "los_plat_common.h"
#include "ringbuf.hpp"
#include "platform_pubdef.h"
#include "videodemo_device.h"
#include "utils/utils.hpp"
#include "http/tracking_data_manager.hpp"
#include "http/online_update_manager.hpp"
#include "graph.hpp"
#include "http/heartbeat_manager.hpp"
#include <fstream>
extern float lrt_g_temp_css;
extern float lrt_g_temp_mss;
extern float lrt_g_temp_upa0;
extern float lrt_g_temp_upa1;

#ifdef __cplusplus
extern "C" {
#endif
extern void* os_Init(void* args);
extern void SNTP_TIME_SET(void *arg);
#ifdef __cplusplus
}
#endif

extern int lrt_modelInitDone;
extern char  lrt_g_server_addr[128];
extern RunningMode lrt_g_running_mode;
extern leonIPCChannel_t lrt_lrt2los_channel;
extern XeyePcbType __attribute__((section(".ddr_direct.data"))) lrt_board_pcb_type;
extern char* lrt_g_xeye_version;
extern std::vector<Graph_t> g_graphs;

std::atomic_int total_frames;
std::atomic_int cached_frames;
std::atomic_bool enable_watchdog_feeding;

std::shared_ptr<xeye::WatchDog> g_watchdog_ptr;
std::shared_ptr<utils::PeroidTimer> g_peroid_timer_ptr;
std::shared_ptr<TrackingDataManager> g_tracking_data_manager_ptr;
std::shared_ptr<OnlineUpdateManager> g_online_update_manager_ptr;
std::shared_ptr<HeartbeatManager> g_heartbeat_manager_ptr;

rtems_id task_deep_id = 0;
rtems_id task_deep_id2 = 0;
rtems_id task_consumer_id = 0;
rtems_id task_ipc_handler_id = 0;

rtems_task multyProc1(rtems_task_argument unused) {
    UNUSED(unused);
    deepLearning();
}
rtems_task multyProc2(rtems_task_argument unused) {
    UNUSED(unused);
    deepLearning_multi();
}

// op, operation type, 0 for suspending, 1 for resuming
static bool deep_task_control(int op) {
    typedef std::function<rtems_status_code (rtems_id)> OperationType;
    static const OperationType ops[2] = {
        rtems_task_suspend,
        rtems_task_resume
    };
    assert((unsigned int)op < 2);
    // when writing flash or erasing flash, enable auto feed watchdog to prevent trigger rebooting.
    if (op == 0) {
        LOGI << "Suspend deep learning task and enable watchdog auto feed";
        g_watchdog_ptr->enable_auto_feed();
    } else {
        LOGI << "Resume deep learning task and disable watchdog auto feed";
        g_watchdog_ptr->disable_auto_feed();
    }
    if (task_deep_id > 0) {
        ops[op](task_deep_id);
    }
    if (task_deep_id2 > 0) {
        ops[op](task_deep_id2);
    }

    if (task_ipc_handler_id > 0) {
        ops[op](task_ipc_handler_id);
    }
    return true;
}


#ifdef TESTIMAGE
// msg should be a c-style null-terminator string
bool writeLogToFile(const char* filename, char* msg) {
    FILE* fp = NULL;
    static int creat_file = 0;
    if (creat_file == 0) {
        fp = fopen(filename, "w+");
        creat_file = 1;
    } else {
        fp = fopen(filename, "a");
    }
    if (fp == NULL || msg == NULL) {
        mvLog(MVLOG_ERROR, "can not open file: %s, or invalid message buffer", filename);
        if (fp) {
            fclose(fp);
        }
        return false;
    }
    size_t len = strlen(msg);
    fwrite(msg, 1, len, fp);
    return (0 == fclose(fp));
}


bool save_tracking_data_to_local_file(const Track_t* data, size_t len, const uint64_t& ts) {
    static const std::string file_name = "/mnt/sdcard/tracking_result.txt";
    std::ofstream of(file_name, std::ios::app);
    if (!of.is_open()) {
        return false;
    }
    of << ts << " " << len << " ";
    for (size_t i = 0; i < len; ++i) {
        const Track_t& ind = data[i];
        of << ind.id << " " << ind.left << " " << ind.top << " " << ind.width << " "
           << ind.height << " " << ind.confirmed << " " << ind.time_since_update << " ";
    }
    of << "\n";
    return true;
}
#endif

rtems_task thread_ipc_handler(rtems_task_argument unused) {
    UNUSED(unused);
    // TODO(yanghongtian): move this filename to configuration file.
    const char* filename = "/mnt/sdcard/xeye_human.txt";
    int status = IPC_SUCCESS;

    status = LeonIPCRxInit(&lrt_lrt2los_channel, NULL, IRQ_DYNAMIC_5, 5);
    if (status != IPC_SUCCESS) {
        mvLog(MVLOG_ERROR, "Init RX size of IPC failed\n");
        return;
    }
    rtems_interval ticks_per_second = rtems_clock_get_ticks_per_second();
    extern uint64_t lrt_g_timestamp;
    for(;;) {
        struct IpcMsg msg_received = {0, 0, 0, NULL, 0};

        // handle messages from RT
        status = LeonIPCWaitMessage(&lrt_lrt2los_channel, ticks_per_second * 60);
        if (status == IPC_TIMEOUT) {
            LOGE << "ipc timeout in thread_ipc_handler, mostly because of deep learning is blocked.";
            SpiFlash::getInstance()->update_reboot_reason_recording(DEEP_BLOCKED);
            plog::get()->setMaxSeverity(plog::none);
            rtems_task_wake_after(1000);
            system_reset();
        }
        uint32_t count = 0;
        status = LeonIPCNumberOfPendingMessages(&lrt_lrt2los_channel, &count);
        total_frames += count;
        if (status == IPC_SUCCESS && count > 0) {
            while (count-- > 0) {
                // timestamp hacking for now,
                // the final solution is acquire timestamp in the rt side when a image frame is received.
                if (count > 0) {
                    rtems_task_wake_after(5);
                }
                LeonIPCReadMessage(&lrt_lrt2los_channel, (uint32_t*)&msg_received);
                lrt_g_timestamp = utils::unix_timestamp();
                switch (msg_received.msg_type) {
#ifdef TESTIMAGE
                    case IpcMsgDetect: {
                        if (!writeLogToFile(filename, msg_received.msg_data)) {
                            mvLog(MVLOG_ERROR, "write log to file failed");
                        }
                        break;
                    }
#endif  // TESTIMAGE
                    case IpcMsgTracking:
                    {
#ifdef TESTIMAGE
                        save_tracking_data_to_local_file(reinterpret_cast<Track_t*>(msg_received.msg_data), \
                                msg_received.msg_len, lrt_g_timestamp);
#endif
                        if (!g_tracking_data_manager_ptr->add_tracking_data(\
                                    msg_received, lrt_g_timestamp)) {
                            LOGD << "Add tracking data to DataManager FAILED";
                        }
                        break;
                    }
                    case IpcMsgImage:
                    {
                        if (!g_tracking_data_manager_ptr->add_jpeg_image_data(\
                                    msg_received, lrt_g_timestamp)) {
                            LOGE << "Add Image data to DataManager FAILED";
                        }
                        break;
                    }
                    default: {
                        mvLog(MVLOG_ERROR, "invalid message type");
                        break;
                    }
                }
            }
        }
        rtems_task_wake_after(1);
    }
}

// this callback function must call when xeye_id is change each time.
void params_callback(const Params* params) {
    extern char lrt_g_xeye_id[];
    extern char lrt_g_server_addr[];
    extern char lrt_g_hb_server_addr[];
    extern char lrt_g_server_url[];
    extern char lrt_g_hb_server_url[];
    extern int lrt_g_server_port;
    extern int lrt_g_hb_server_port;
    extern int lrt_g_score_line;
    mvLog(MVLOG_DEBUG, "change xeye_id from %s to %s", lrt_g_xeye_id, \
            params->xeye_id().c_str());
    memcpy(lrt_g_xeye_id, params->xeye_id().c_str(), params->xeye_id().length());
    lrt_g_xeye_id[params->xeye_id().length()] = '\0';

    lrt_g_running_mode = params->get_mode_index();

    memcpy(lrt_g_server_addr, params->data_server_addr().c_str(), \
            params->data_server_addr().length());
    lrt_g_server_addr[params->data_server_addr().length()] = '\0';
    lrt_g_server_port = params->data_server_port();
    if (!params->data_server_url().empty()) {
        memcpy(lrt_g_server_url, params->data_server_url().c_str(), \
            params->data_server_url().length());
        lrt_g_server_url[params->data_server_url().length()] = '\0';
    } else {
        memset(lrt_g_server_url, 0, 64);
    }
    if (!params->heartbeat_server_url().empty()) {
        memcpy(lrt_g_hb_server_url, params->heartbeat_server_url().c_str(), \
            params->heartbeat_server_url().length());
        lrt_g_hb_server_url[params->heartbeat_server_url().length()] = '\0';
    } else {
        memset(lrt_g_hb_server_url, 0, 64);
    }

    lrt_g_hb_server_port = params->heartbeat_server_port();
    memcpy(lrt_g_hb_server_addr, params->heartbeat_server_addr().c_str(), \
            params->heartbeat_server_addr().length());
    extern int lrt_g_ccm_index;
    lrt_g_ccm_index = params->ccm_index();
    extern OnlineGrabConf lrt_online_grab_conf;
    lrt_online_grab_conf.enabled = params->is_online_image_grab_enable();
    if (params->online_image_grab_strategy() == "interval") {
        lrt_online_grab_conf.strategy = OnlineGrabConf::INTERVAL;
    } else if (params->online_image_grab_strategy() == "ondemand") {
        lrt_online_grab_conf.strategy = OnlineGrabConf::ONDEMAND;
    } else {
        lrt_online_grab_conf.strategy = OnlineGrabConf::INVALID;
    }
    lrt_online_grab_conf.value = params->online_image_flow_control_value();
    lrt_g_score_line = params->get_confidence();
    if (lrt_g_score_line <= 0) {
        mvLog(MVLOG_ERROR, "invalid confidence: %d", lrt_g_score_line);
        LOGE << "invalid confidence: " << lrt_g_score_line;
    }
}

// Wanning: This callback function will be called in a timer service routine context,
// you can not execute float point operation in timer service routine
void heartbeat_peroidic_callback() {
    if (g_heartbeat_manager_ptr) {
        g_heartbeat_manager_ptr->trigger_heartbeat();
    }
}

void temperature_callback(const std::vector<float>& temperatures) {
    if (temperatures.size() != 4) {
        return;
    }
    lrt_g_temp_css = temperatures[0];
    lrt_g_temp_mss = temperatures[1];
    lrt_g_temp_upa0 = temperatures[2];
    lrt_g_temp_upa1 = temperatures[3];
}

// This Function will modify gk_UsbResourceData manually to support different
// resolution for uvc protocal. The Default resolution is 1080p(1920*1080).
// If you want to support a new resolution, just copy config/videodemo_device.urc
// and modify resultion value to generate a new videodemo_device_urc.c, compare
// the diff to the default one, then we can get the diff at gk_UsbResourceData,
// finally we need fill the diff at this function.
void modify_usb_urc(XeyePcbType pcb_type) {
    // Now we use pcb_type instead of resolution
    // xeyeV2 is 1152*768 for ar0330
    // xeyeface is 1920*1080 for isp
    extern unsigned char gk_UsbResourceData[];
    if (pcb_type == XEYE_20) {
        gk_UsbResourceData[150] = 4;
        gk_UsbResourceData[151] = 0;
        gk_UsbResourceData[152] = 3;

        gk_UsbResourceData[345] = 4;
        gk_UsbResourceData[346] = 0;
        gk_UsbResourceData[347] = 3;

        gk_UsbResourceData[540] = 4;
        gk_UsbResourceData[541] = 0;
        gk_UsbResourceData[542] = 3;
        LOGD << "success set urc to 1152*768";
    }
}

void watchdog_control_callback(int reboot_reason) {
    SpiFlash::getInstance()->update_reboot_reason_recording(reboot_reason);
    LOGI << "There are two reason for disable watchdog feeding to reboot system, " \
        "reboot reason index: " << reboot_reason;
    // disable logging to avoid crashing filesystem when watchdog is triggered.
    plog::get()->setMaxSeverity(plog::none);
    rtems_task_wake_after(1000);
    system_reset();
}

extern "C" void* POSIX_Init(void* args) {
    UNUSED(args);
    rtems_status_code ret = RTEMS_SUCCESSFUL;
    rtems_name task_deep;
    rtems_name task_deep2;
    rtems_name task_consumer;
    rtems_name task_ipc_handler;
    rtems_name msg_queue_name_for_http;
    XeyePcbType xeye_pcb_type;
    LosSysConfig_t sys_config;

    sys_config.sys_clock_enable = true;
    sys_config.temperature_enable = true;
    sys_config.sys_clock_Hz = 600;
    sys_config.sys_memory_enable = true;
    sys_config.mem_policy = CROWD_MEM_POLICY;
    sys_config.uart_enable = true;
    leonOS_platform_init(sys_config);
    os_Init(args);
    xeye_pcb_type = lrt_board_pcb_type;
    modify_usb_urc(xeye_pcb_type);

    // configure each logging file up to 50M, keep up to 10 logging files.
    static plog::RollingFileAppender<plog::TxtFormatter> fileAppender(\
            "/mnt/sdcard/crowd.log", 10 * 1024 * 1024, 20);
    // static plog::ColorConsoleAppender<plog::TxtFormatter> colorConsoleAppender;
    // plog::init(plog::info, &fileAppender).addAppender(&colorConsoleAppender);
    plog::init(plog::info, &fileAppender);

    LOGI << "Start to init watchdog";
    enable_watchdog_feeding = true;
    g_watchdog_ptr.reset(new xeye::WatchDog());
    assert(g_watchdog_ptr != nullptr);
    if (!g_watchdog_ptr->init()) {
        LOGE << "Can not initialized watchdog module, will terminate application";
        rtems_task_wake_after(1000);
        system_reset();
    }
    LOGI << "Initialized watchdog successfully, start to loading configuration file";

    // must all this function first, or the xeye id parameter will not pass to LRT correctly.
    Params::getInstance()->set_params_callback(params_callback);
    if (!Params::getInstance()->load_from_file("/mnt/sdcard/conf.json")) {
        // incompatible configration file,
        // TODO(yanghongtian): under this condition, if no network configuration are valid,
        // load them from flash(the default one)
        LOGE << "incompatible configuration file";
    }

    NetInit(xeye_pcb_type, Params::getInstance()->local_ipaddr(), \
            Params::getInstance()->local_netmask(), \
            Params::getInstance()->local_gateway());
    LOGI << "Successfully acquire network ip address, now do the time syncronization with ntp server: "
         << Params::getInstance()->ntp_server_addr();

    telnet_init(deep_task_control);
    if (!(lrt_g_running_mode == RECORD || \
        lrt_g_running_mode == NORMAL || \
        lrt_g_running_mode == LIVE)) {
        LOGE << "Unsupported mode, please check the configuration file";
        // go into infinite loop, to allow network usage.
        while (1) {
            rtems_task_wake_after(1000);
        }
    }
    LOGI << "Loading configuration file successfully";

    // Create New Task
    task_deep = rtems_build_name('P', 'K', 'E', 'S');
    task_deep2 = rtems_build_name('P', 'K', 'T', 'S');
    task_ipc_handler = rtems_build_name('T', 'I', 'P', 'C');

    if (!SpiFlash::getInstance()->init()) {
        LOGE << "initialized spi flash failed, program exited";
        plog::get()->setMaxSeverity(plog::none);
        rtems_task_wake_after(1000);
        system_reset();
    }

    SpiFlash::getInstance()->register_task_control_callback(deep_task_control);
    if (lrt_g_running_mode != LIVE) {
        LOGI << "Start to loading graph from disk...";
        if (fathom_init() == 0) {
            lrt_modelInitDone = 1;
            LOGI << "Loading graph from disk, Done.";
        } else {
            LOGE << "Loading model file FAILED";
            plog::get()->setMaxSeverity(plog::none);
            rtems_task_wake_after(1000);
            system_reset();
        }
        XeyeClockControl(1, 0);
        // do the ntp time syncronization, this job must be finished successfully until we go further.
        while (1) {
            bool status = ntp_update_time(Params::getInstance()->ntp_server_addr().c_str());
            if (status) {
                LOGI << "Time syncronizaiton DONE";
                break;
            }
            rtems_task_wake_after(10);
            LOGW << "ntp time syncronization FAILED";
        }
        // initialize heartbeat manager
        g_heartbeat_manager_ptr.reset(new HeartbeatManager(\
                    Params::getInstance()->heartbeat_server_addr(),
                    Params::getInstance()->heartbeat_server_port(),
                    Params::getInstance()->xeye_id(),
                    Params::getInstance()->heartbeat_server_url()));

        if (!g_heartbeat_manager_ptr->init(temperature_callback)) {
            LOGE << "can not initialized heartbeat manager";
            plog::get()->setMaxSeverity(plog::none);
            rtems_task_wake_after(1000);
            system_reset();
        }

        // configured to 10 seconds peroidic timer
        g_peroid_timer_ptr.reset(new utils::PeroidTimer(10, \
            rtems_build_name('T', 'M', 'R', '1'), \
            heartbeat_peroidic_callback));
        if (!g_peroid_timer_ptr) {
            LOGE << "Can not create peroidic timer";
            plog::get()->setMaxSeverity(plog::none);
            rtems_task_wake_after(1000);
            system_reset();
        }

        // config max message queue size to 256, tracking ring buffer size for 4K * sizeof(Track_t)
        // image ring buffer size for 4M
        g_tracking_data_manager_ptr.reset(new TrackingDataManager(\
                    Params::getInstance()->data_server_addr(), \
                    Params::getInstance()->data_server_port(), \
                    Params::getInstance()->xeye_id(), \
                    Params::getInstance()->data_server_url(), \
                    256, 4 * 1024, 4 * 1024 * 1024));
        if (!g_tracking_data_manager_ptr->init(watchdog_control_callback)) {
            LOGE << "initialized data server failed";
            plog::get()->setMaxSeverity(plog::none);
            rtems_task_wake_after(1000);
            system_reset();
        }

        // only enable online update features when data server is given by domain name.
        if (Params::getInstance()->is_domain_name(\
                    Params::getInstance()->data_server_addr()) && \
                !Params::getInstance()->online_update_host().empty()) {
            g_online_update_manager_ptr.reset(\
                new OnlineUpdateManager(Params::getInstance()->online_update_host(), \
                    Params::getInstance()->data_server_port(), \
                    Params::getInstance()->xeye_id(), \
                    std::string(lrt_g_xeye_version), \
                    Params::getInstance()->online_update_url_prefix()));
            if (!g_online_update_manager_ptr->init(watchdog_control_callback)) {
                LOGE << "Initialize online update features failed.";
                plog::get()->setMaxSeverity(plog::none);
                rtems_task_wake_after(1000);
                system_reset();
            }
        }

        // enable watchdog before task creation
        if (!g_watchdog_ptr->enable()) {
            LOGE << "Can not enable watchdog, will terminate appplication";
            plog::get()->setMaxSeverity(plog::none);
            rtems_task_wake_after(1000);
            system_reset();
        }

        ret = rtems_task_create(task_deep, 128, RTEMS_MINIMUM_STACK_SIZE * 4,
                                RTEMS_DEFAULT_MODES, RTEMS_LOCAL | RTEMS_FLOATING_POINT, \
                                &task_deep_id);
        assert(ret == RTEMS_SUCCESSFUL);
        ret = rtems_task_create(task_deep2, 128, RTEMS_MINIMUM_STACK_SIZE * 4,
                                RTEMS_DEFAULT_MODES, RTEMS_DEFAULT_ATTRIBUTES | RTEMS_FLOATING_POINT, \
                                &task_deep_id2);
        assert(ret == RTEMS_SUCCESSFUL);
        ret = rtems_task_create(task_ipc_handler, 112, RTEMS_MINIMUM_STACK_SIZE * 2,
                                RTEMS_DEFAULT_MODES, RTEMS_LOCAL | RTEMS_FLOATING_POINT, \
                                &task_ipc_handler_id);
        assert(ret == RTEMS_SUCCESSFUL);


        ret = rtems_task_start(task_deep_id, multyProc1, 1);
        assert(ret == RTEMS_SUCCESSFUL);
        ret = rtems_task_start(task_deep_id2, multyProc2, 1);
        assert(ret == RTEMS_SUCCESSFUL);
        ret = rtems_task_start(task_ipc_handler_id, thread_ipc_handler, 1);
        assert(ret == RTEMS_SUCCESSFUL);
    }

    // if everything goes fine here, reset the reboot reason to power-off state here.
    SpiFlash::getInstance()->reset_reboot_reason_to_power_off();
    uint32_t heartbeat_msg_count = 0;

    for (;;) {
        g_watchdog_ptr->feed();
        // do the time syncronizationa
        if (lrt_g_running_mode != LIVE && (0 == ++heartbeat_msg_count % 128)) {
            if (!ntp_update_time(Params::getInstance()->ntp_server_addr().c_str())) {
                LOGW << "NTP time syncronization FAILED in runtime, " \
                        "but it's not a FATAL error, will try next time";
            }
        }
        rtems_task_wake_after(1000);
    }
    DrvLeonRTWaitExecution();
    rtems_task_delete(RTEMS_SELF);
    exit(0);
}
