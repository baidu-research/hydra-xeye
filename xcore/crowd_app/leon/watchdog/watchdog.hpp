#ifndef WATCHDOG_HPP
#define WATCHDOG_HPP
#include "OsDrvTimer.h"
#include "mvLog.h"

namespace xeye {
class WatchDog {
    typedef enum {
        NOT_INITIALIZED = 0,
        DISABLED,
        ENABLED
    } WatchdogStatus;
    // only can be configured to 7158ms at 600Mhz frequency
    static const int max_system_reset_ms = 7158;
    static const int default_callback_priority = 12;
    // set this flag to true to enable watchdog's auto feeding, it works as the following:
    // first, config the countdown_system_reset_ms and countdown_threshold_isr_ms,
    // where, countdown_threshold_isr_ms must be less than countdown_system_reset_ms
    // second, install a isr callback function, this function will be called,
    // when countdown_system_reset_ms reach to countdown_threshold_isr_ms,
    // if auto_feed_enable is set to true, feeding watchdog is done in this callback function,
    // which means, the watchdog hardware doesn't need a mannual feeding
    static bool auto_feed_enabled;
    static void auto_feed_cb() {
        // 0 means restore the value which the configuration stage configured.
        if (auto_feed_enabled) {
            OsDrvTimerWatchdogUpdate(0, NULL);
        }
    }
    public:
        explicit WatchDog(uint32_t sys_reset_ms = max_system_reset_ms) : \
        m_status(NOT_INITIALIZED) {
            m_system_reset_ms = (sys_reset_ms <= max_system_reset_ms) \
                                ?  sys_reset_ms : max_system_reset_ms;
            auto_feed_enabled = false;
        }
        ~WatchDog() {
            disable();
        }
        bool init(void) {
            int status = OsDrvTimerInit();
            if ((status == OS_MYR_DRV_SUCCESS) || \
                (status == OS_MYR_DRV_ALREADY_OPENED) || \
                (status == OS_MYR_DRV_ALREADY_INITIALIZED)){
                    OsDrvTimerWatchdogEnable(WDOG_DISABLE);
                    m_status = DISABLED;
                    return true;
            }
            mvLog(MVLOG_ERROR, "Initialized timer module FAILED");
            return false;
        }
        // can only called in the same thread or task where watchdog was configured
        bool enable() {
            if (m_status == NOT_INITIALIZED) {
                mvLog(MVLOG_ERROR, "Watchdog has not been initialized");
                return false;
            }
            // system will reset when system_reset_timer reach to 0
            // callback function will be called when system_reset_timer reach to threshhold
            OsDrvWatchDogCfg_t cfg = {
                auto_feed_cb,
                default_callback_priority,
                m_system_reset_ms,
                1000
            };
            int ret = OsDrvTimerWatchdogConfigure(&cfg);
            if (ret != OS_MYR_DRV_SUCCESS) {
                if (ret == OS_MYR_DRV_RESOURCE_BUSY) {
                    mvLog(MVLOG_INFO, "the watchdog is enabled and cann not be configured");
                } else if (ret == OS_MYR_DRV_NOTOPENED) {
                    mvLog(MVLOG_ERROR, "the watchdog driver was not initialized");
                }
                return false;
            }

            ret = OsDrvTimerWatchdogEnable(WDOG_ENABLE);
            if (ret != OS_MYR_DRV_SUCCESS) {
                mvLog(MVLOG_ERROR, "the watchdog driver was not initialized");
                return false;
            }
            mvLog(MVLOG_INFO, "Enble watchdog successfully, system will reset after %d ms,"
                    " if no feed happens", m_system_reset_ms);
            m_status = ENABLED;
            return true;
        }

        // can only call in the same thread or task where watchdog was configured
        bool disable() {
            if (m_status == NOT_INITIALIZED || \
                m_status == DISABLED) {
                mvLog(MVLOG_ERROR, "watchdog has not been initailized, or enabled");
                return false;
            }
            m_status = DISABLED;
            return (OS_MYR_DRV_SUCCESS == OsDrvTimerWatchdogEnable(WDOG_DISABLE));
        }
        // this interface can be freely called.
        bool feed() const {
            if (m_status != ENABLED) {
                // mvLog(MVLOG_ERROR, "watchdog is not enabled or not initialized");
                return false;
            }

            uint32_t remaining_ms = 0;
            int ret = OsDrvTimerWatchdogUpdate(m_system_reset_ms, &remaining_ms);
            if (ret != OS_MYR_DRV_SUCCESS) {
                // mvLog(MVLOG_ERROR, "feed watchdog FAILED");
                return false;
            }
            // mvLog(MVLOG_INFO, "feed watchdog successfully, remaining time in ms: %d", remaining_ms);
            return true;
        }

        void enable_auto_feed() {
            auto_feed_enabled = true;
        }

        void disable_auto_feed() {
            auto_feed_enabled = false;
        }

    private:
        WatchdogStatus m_status;
        uint32_t m_system_reset_ms;
};  // class WatchDog
// definitions
bool WatchDog::auto_feed_enabled;
}  // namespace xeye
#endif  // WATCHDOG_HPP
