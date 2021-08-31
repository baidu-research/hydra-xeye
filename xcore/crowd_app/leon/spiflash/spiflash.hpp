/******************************************************************************
 *     __    __  _____  __    __  _____
 *     \ \  / / | ____| \ \  / / | ____|
 *      \ \/ /  | |__    \ \/ /  | |__
 *       }  {   |  __|    \  /   |  __|
 *      / /\ \  | |___    / /    | |___
 *     /_/  \_\ |_____|  /_/     |_____|
 *
 *****************************************************************************/

#ifndef LEON_SPIFLASH_HPP
#define LEON_SPIFLASH_HPP
#include <spiflash.h>
#include <memory>
#include <string>
#include <Log.h>
#include <mvLog.h>
#include "utils/utils.hpp"
#include <functional>
#include <mvTensorTimer.h>
typedef std::function<bool(int)> TaskControlCallback;

static const std::string reboot_reason_str[] = {
    "power off or system crash",
    "unknown network issue",
    "deep learning blocked",
    "system upgrade"
};

enum {
    TASK_OP_SUSPEND = 0,
    TASK_OP_RESUME = 1
};

enum {
    POWER_OFF = 1000,
    NETWORK_ISSUE = 1001,
    DEEP_BLOCKED = 1002,
    SYSTEM_UPGRADE = 1003
};



class SpiFlash {
static const std::size_t flash_offset = 0x700000;
static const uint32_t const_valid_flag = 0x55aacc33;
    typedef struct _flash_layout {
        // this field is used to identify the current flash layout is valid or not.
        uint32_t const_valid_flag;
        // this field records the mannual reboot count, power off and system crash excluded.
        uint32_t reboot_count;
        // this field records the latest reboot reason.
        uint32_t reboot_reason;
    } FlashLayout;
private:
    SpiFlash() : m_initialized(false) {
    }
public:
    ~SpiFlash() {
    }
    static std::shared_ptr<SpiFlash> getInstance() {
        static std::shared_ptr<SpiFlash> _spi_flash(new SpiFlash);
        assert(_spi_flash != nullptr);
        return _spi_flash;
    }

    bool init() {
        if (!m_initialized) {
            if (0 == flash_init()) {
                m_initialized = true;
            } else {
                return false;
            }
        }
        FlashLayout curr_layout;
        read_layout_from_flash(&curr_layout);

        if (curr_layout.const_valid_flag != const_valid_flag) {
            mvLog(MVLOG_INFO, "invalid flash layout, start to initialized");
            // means the current layout in flash is not valid, need to initialize.
            m_flash_layout.const_valid_flag = const_valid_flag;
            m_flash_layout.reboot_count = 0;
            m_flash_layout.reboot_reason = POWER_OFF;  // this is the default reboot reason.
            write_layout_to_flash(&m_flash_layout);
        } else {
            // read back the latest reboot_count and reboot_reason;
            m_flash_layout.const_valid_flag = curr_layout.const_valid_flag;
            m_flash_layout.reboot_count = curr_layout.reboot_count;
            m_flash_layout.reboot_reason = curr_layout.reboot_reason;
            if (curr_layout.reboot_reason >= POWER_OFF && \
                curr_layout.reboot_reason <= SYSTEM_UPGRADE) {
                LOGI << "current reboot count: " << curr_layout.reboot_count
                     << ", latest reboot reason: " << reboot_reason_str[curr_layout.reboot_reason - POWER_OFF];
            } else {
                LOGE << "invalid reboot reason index: " << curr_layout.reboot_reason;
            }
            // reset to default reboot state.
            curr_layout.reboot_reason = POWER_OFF;
            write_layout_to_flash(&curr_layout);
        }
        return true;
    }

    void update_firmware(char* firm, size_t len) {
        // first, suspend the deep learning task
        if (m_cb) {
            m_cb(TASK_OP_SUSPEND);
        }
        rtems_task_wake_after(100);
        LOGI << "Going to do a firmware update...";
        struct tFlashParams my_flash;
        my_flash.offset = ALIGN_TO_SUBSECTOR(0);
        my_flash.size = ALIGN_TO_SUBSECTOR(len);

        float erase_time = 0.f;
        mv::tensor::Timer update_timer;
        // erasing.
        erase_spi_flash_Ex(&my_flash);
        erase_time = update_timer.elapsed();
        LOGI << "erase flash done, start to flash new firmware, erase time: " << erase_time << " ms";

        my_flash.inBuff = firm;

        write_spi_flash_Ex(&my_flash);

        LOGI << "write flash done!!!, writing time: " << update_timer.elapsed() - erase_time << " ms";

        // finally, resume the deep learning task
        if (m_cb) {
            m_cb(TASK_OP_RESUME);
        }
    }

    void update_reboot_reason_recording(int reboot_reason) {
        if (reboot_reason < POWER_OFF || \
            reboot_reason > SYSTEM_UPGRADE) {
            LOGE << "invalid reboot reason index: " << reboot_reason;
            return;
        }
        if (m_cb) {
            m_cb(TASK_OP_SUSPEND);
        }
        rtems_task_wake_after(100);
        FlashLayout curr;
        memcpy(&curr, &m_flash_layout, sizeof(FlashLayout));
        curr.reboot_count++;
        curr.reboot_reason = reboot_reason;
        LOGI << "reboot count " << curr.reboot_count << ", reboot reason: "
             << reboot_reason_str[curr.reboot_reason - POWER_OFF] << ", reboot reason index: "
             << curr.reboot_reason;
        write_layout_to_flash(&curr);
        if (m_cb) {
            m_cb(TASK_OP_RESUME);
        }
    }

    uint32_t get_latest_reboot_reason() const {
        return m_flash_layout.reboot_reason - POWER_OFF;
    }
    uint32_t get_reboot_count() const {
        return m_flash_layout.reboot_count;
    }
    void reset_reboot_reason_to_power_off() {
        FlashLayout curr_layout;
        memcpy(&curr_layout, &m_flash_layout, sizeof(FlashLayout));
        curr_layout.reboot_reason = POWER_OFF;
        write_layout_to_flash(&curr_layout);
    }
    void register_task_control_callback(TaskControlCallback cb) {
        m_cb = cb;
    }
private:
    void write_layout_to_flash(const FlashLayout* curr_layout) {
        tFlashParams flash_params = {
            NULL,
            NULL,
            NULL,
            ALIGN_TO_SUBSECTOR(flash_offset),
            ALIGN_TO_SUBSECTOR(sizeof(FlashLayout)),
            0,
            0
        };

        erase_spi_flash_Ex(&flash_params);

        std::shared_ptr<char> buf(new char[ALIGN_TO_SUBSECTOR(sizeof(FlashLayout))]);

        memcpy(buf.get(), curr_layout, sizeof(FlashLayout));
        flash_params.inBuff = buf.get();
        flash_params.size = ALIGN_TO_SUBSECTOR(sizeof(FlashLayout));
        write_spi_flash_Ex(&flash_params);
    }
    void read_layout_from_flash(FlashLayout* curr_layout) {
        std::shared_ptr<char> buf(new char[ALIGN_TO_SUBSECTOR(sizeof(FlashLayout))]);
        if (buf == nullptr) {
            LOGE << "can not allocate memory buffer";
            return;
        }
        memset(buf.get(), 0, sizeof(FlashLayout));
        tFlashParams flash_params = {
            NULL,
            NULL,
            buf.get(),
            ALIGN_TO_SUBSECTOR(flash_offset),
            ALIGN_TO_SUBSECTOR(sizeof(FlashLayout)),
            0,
            0
        };
        read_spi_flash_Ex(&flash_params);
        memcpy(curr_layout, buf.get(), sizeof(FlashLayout));
    }

    bool m_initialized;
    TaskControlCallback m_cb;
    FlashLayout m_flash_layout;
};
#endif  // LEON_SPIFLASH_HPP
