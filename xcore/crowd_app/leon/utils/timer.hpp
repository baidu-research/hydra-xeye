#ifndef UTILS_TIMER_HPP
#define UTILS_TIMER_HPP
namespace utils {
#include <rtems.h>
#include <Log.h>
class PeroidTimer {
    public:
        typedef void (*TimerCallBack)();
        static void common_tsr(rtems_id id, PeroidTimer* instance) {
            if (instance == NULL) {
                return;
            }
            rtems_status_code status = rtems_timer_fire_after(id, \
                    instance->m_peroid_sec * rtems_clock_get_ticks_per_second(), \
                    rtems_timer_service_routine_entry(&PeroidTimer::common_tsr), instance);
            if (status != RTEMS_SUCCESSFUL) {
                LOGE << "fire peroid timer failed, status code: " << status;
            }
            // call user installed routine
            if (instance->m_cb_from_external != NULL) {
                (instance->m_cb_from_external)();
            }
        }
    public:
        PeroidTimer(int sec, rtems_name timer_name, TimerCallBack cb = NULL) : m_peroid_sec(sec) {
            rtems_status_code status = rtems_timer_create(timer_name, &m_timer_id);
            if (status != RTEMS_SUCCESSFUL) {
                LOGE << "create peroid timer failed, status code: " << status;
                m_is_timer_created = false;
            }
            m_cb_from_external = cb;
            m_is_timer_created = true;
            status = rtems_timer_fire_after(m_timer_id, \
                    sec * rtems_clock_get_ticks_per_second(), \
                    rtems_timer_service_routine_entry(&PeroidTimer::common_tsr), this);
            if (status != RTEMS_SUCCESSFUL) {
                LOGE << "fire peroid timer failed, status code: " << status;
            }
        }
        ~PeroidTimer() {
            if (m_is_timer_created) {
                rtems_status_code status = RTEMS_SUCCESSFUL;
                status = rtems_timer_cancel(m_timer_id);
                if (status != RTEMS_SUCCESSFUL) {
                    LOGE << "Cancel peroid timer failed, status code: " << status;
                }
                status = rtems_timer_delete(m_timer_id);
                if (status != RTEMS_SUCCESSFUL) {
                    LOGE << "delete peroid timer failed, status code: " << status;
                }
            }
        }
    private:
        int m_peroid_sec;
        int m_is_timer_created;
        rtems_id m_timer_id;
        TimerCallBack m_cb_from_external;
};
}  // namespace utils
#endif  // UTILS_TIMER_HPP
