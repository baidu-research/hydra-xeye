#ifndef HEARTBEAT_MANAGER
#define HEARTBEAT_MANAGER
#include "http/http.hpp"
#include "utils/timer.hpp"
#include "temperature.h"
#include "spiflash/spiflash.hpp"
#include <memory>

typedef std::function<void(const std::vector<float>& temps)> TemperatureCallback;

class HeartbeatManager {
const std::string file_name = "/mnt/sdcard/heartbeat_data.txt";
public:
    HeartbeatManager(std::string host, int port, \
            std::string xeye_id, std::string url, \
            size_t retry = 1, size_t timeout_ms = 4000) {
        retry_ = retry;
        timeout_ = timeout_ms;
        http_client_ptr_.reset(new HTTPClient(host, port));
        xeye_id_ = std::move(xeye_id);
        http_header_ = std::move(generate_http_header(url, http_client_ptr_->get_domain_name()));
    }
    ~HeartbeatManager() {
        rtems_semaphore_delete(heartbeat_sem_);
        rtems_task_delete(heartbeat_task_id_);
        for (size_t i = 0; i < heartbeat_msg_cached_.size(); ++i) {
            if (heartbeat_msg_cached_[i]) {
                free(heartbeat_msg_cached_[i]);
            }
        }
    }

    bool init(TemperatureCallback cb) {
        int ret = rtems_semaphore_create(rtems_build_name('S','M','R','P'), \
            0, RTEMS_SIMPLE_BINARY_SEMAPHORE | RTEMS_FIFO | RTEMS_LOCAL | \
            RTEMS_NO_INHERIT_PRIORITY | RTEMS_NO_PRIORITY_CEILING | \
            RTEMS_NO_MULTIPROCESSOR_RESOURCE_SHARING, RTEMS_NO_PRIORITY, &heartbeat_sem_);
        if (ret != RTEMS_SUCCESSFUL) {
            LOGE << "Can not create heartbeat semaphore, status code: " << ret;
            return false;
        }

        ret = rtems_task_create(rtems_build_name('H','R','B','T'), 128, RTEMS_MINIMUM_STACK_SIZE * 4, \
                RTEMS_DEFAULT_MODES, RTEMS_LOCAL | RTEMS_FLOATING_POINT, \
                &heartbeat_task_id_);
        if (ret != RTEMS_SUCCESSFUL) {
            LOGE << "Can not create heartbeat task, status code: " << ret;
            return false;
        }
        ret = rtems_task_start(heartbeat_task_id_, \
                rtems_task_entry(&HeartbeatManager::thread_heartbeat), \
                rtems_task_argument(this));
        if (ret != RTEMS_SUCCESSFUL) {
            LOGE << "Can not start heartbeat task, status code: " << ret;
            return false;
        }
        if (http_client_ptr_ == nullptr) {
            LOGE << "can not create http client object";
            return false;
        }

        temp_cb_ = cb;
        return http_client_ptr_->init();
    }
    void trigger_heartbeat(void) {
        if (RTEMS_SUCCESSFUL != rtems_semaphore_release(heartbeat_sem_)) {
            LOGE << "Trigger heartbeat FAILED";
        }
    }
    bool save_heartbeat_data_to_local_file() {
        if (heartbeat_msg_cached_.size() == 0) {
            return true;
        }
        std::ofstream of(file_name);
        if (!of.is_open()) {
            return false;
        }
        for (size_t i = 0; i < heartbeat_msg_cached_.size(); ++i) {
            if (heartbeat_msg_cached_[i] == NULL) {
                continue;
            }
            of << heartbeat_msg_cached_[i] << "\n";
        }
        LOGI << "Saving " << heartbeat_msg_cached_.size() << " heartbeat data to local disk";
        heartbeat_msg_cached_.resize(0);
        return true;
    }
private:
    void thread_heartbeat(rtems_task_argument unused) {
        UNUSED(unused);
        for (;;) {
            int status = rtems_semaphore_obtain(heartbeat_sem_, RTEMS_WAIT, RTEMS_NO_TIMEOUT);
            if (status != RTEMS_SUCCESSFUL) {
                break;
            }
            float temp_css = GetCssTemp();
            float temp_mss = GetMssTemp();
            float temp_upa0 = GetUpa0Temp();
            float temp_upa1 = GetUpa1Temp();
            // pass out the temperatures
            if (temp_cb_) {
                std::vector<float> temperatures {temp_css, temp_mss, temp_upa0, temp_upa1};
                temp_cb_(temperatures);
            }
            float t1 = std::max(temp_css, temp_mss);
            float t2 = std::max(temp_upa0, temp_upa1);
            float t = std::max(t1, t2);
            uint64_t ts = utils::unix_timestamp();
            char* json_str = convert_heartbeat_info_to_json(xeye_id_, ts, static_cast<int>(t * 1000), \
                    SpiFlash::getInstance()->get_reboot_count(), \
                    SpiFlash::getInstance()->get_latest_reboot_reason());
            heartbeat_msg_cached_.push_back(json_str);
            if (post_heartbeat_msg(heartbeat_msg_cached_)) {
                heartbeat_msg_cached_.resize(0);
            }
        }
    }

    bool read_heartbeat_data_from_local_file() {
        std::ifstream ifs(file_name);
        if (!ifs.is_open()) {
            return false;
        }
        std::string line;
        while (std::getline(ifs, line)) {
            std::istringstream iss(line);
            std::string heartbeat;
            iss >> heartbeat;
            char* data = reinterpret_cast<char*>(malloc(heartbeat.length() + 1));
            memcpy(data, heartbeat.c_str(), heartbeat.length());
            data[heartbeat.length()] = '\0';
            heartbeat_msg_cached_.push_back(data);
        }
        ifs.close();
        remove(file_name.c_str());
        LOGI << "Loading " << heartbeat_msg_cached_.size() << " heartbeat data from local disk";
        return true;
    }


    bool post_heartbeat_msg(const std::vector<char*>& msgs) {
        size_t retry_times = retry_;
        bool connected = false;
        do {
            connected = http_client_ptr_->connect_to_server(timeout_);
            if (connected) {
                break;
            }
            rtems_task_wake_after(100);
        } while (retry_times--);
        if (!connected) {
            LOGE << "Can not connect to server";
            return false;
        }
        std::string json_str = "";
        for (size_t i = 0; i < msgs.size(); ++i) {
            json_str += msgs[i];
            // free memory
            if (msgs[i]) {
                free(msgs[i]);
            }
        }
        return http_client_ptr_->post(http_header_, json_str.c_str(), json_str.length());
    }

    char* convert_heartbeat_info_to_json(const std::string& xeye_id, \
        const uint64_t& timestamp, int temp, \
        uint32_t curr_reboot_count, uint32_t latest_reboot_reason) {
        static uint64_t first_timestamp = timestamp;
        std::string tags = "camera_id=" + xeye_id + ",head_id=201";
        cJSON* root = cJSON_CreateArray();

        cJSON* heartbeat = cJSON_CreateObject();
        cJSON_AddItemToObject(heartbeat, "endpoint", cJSON_CreateString("xeye"));
        cJSON_AddItemToObject(heartbeat, "metric", cJSON_CreateString("heartbeat"));
        cJSON_AddItemToObject(heartbeat, "timestamp", cJSON_CreateNumber(timestamp / 1000));
        cJSON_AddItemToObject(heartbeat, "step", cJSON_CreateNumber(10));
        cJSON_AddItemToObject(heartbeat, "value", cJSON_CreateNumber(5));
        cJSON_AddItemToObject(heartbeat, "counterType", cJSON_CreateString("GAUGE"));
        cJSON_AddItemToObject(heartbeat, "tags", cJSON_CreateString(tags.c_str()));
        cJSON_AddItemToArray(root, heartbeat);

        cJSON* temperature = cJSON_CreateObject();
        cJSON_AddItemToObject(temperature, "endpoint", cJSON_CreateString("xeye"));
        cJSON_AddItemToObject(temperature, "metric", cJSON_CreateString("temperature"));
        cJSON_AddItemToObject(temperature, "timestamp", cJSON_CreateNumber(timestamp / 1000));
        cJSON_AddItemToObject(temperature, "step", cJSON_CreateNumber(10));
        cJSON_AddItemToObject(temperature, "value", cJSON_CreateNumber(temp));
        cJSON_AddItemToObject(temperature, "counterType", cJSON_CreateString("GAUGE"));
        cJSON_AddItemToObject(temperature, "tags", cJSON_CreateString(tags.c_str()));
        cJSON_AddItemToArray(root, temperature);

        cJSON* runtime = cJSON_CreateObject();
        cJSON_AddItemToObject(runtime, "endpoint", cJSON_CreateString("xeye"));
        cJSON_AddItemToObject(runtime, "metric", cJSON_CreateString("runtime"));
        cJSON_AddItemToObject(runtime, "timestamp", cJSON_CreateNumber(timestamp / 1000));
        cJSON_AddItemToObject(runtime, "step", cJSON_CreateNumber(10));
        // when trigger the first heartbeat, xeye is online 10 seconds already.
        cJSON_AddItemToObject(runtime, "value", cJSON_CreateNumber((timestamp - first_timestamp) / 1000 + 10));
        cJSON_AddItemToObject(runtime, "counterType", cJSON_CreateString("GAUGE"));
        cJSON_AddItemToObject(runtime, "tags", cJSON_CreateString(tags.c_str()));
        cJSON_AddItemToArray(root, runtime);

        cJSON* reboot_count = cJSON_CreateObject();
        cJSON_AddItemToObject(reboot_count, "endpoint", cJSON_CreateString("xeye"));
        cJSON_AddItemToObject(reboot_count, "metric", cJSON_CreateString("reboot_count"));
        cJSON_AddItemToObject(reboot_count, "timestamp", cJSON_CreateNumber(timestamp / 1000));
        cJSON_AddItemToObject(reboot_count, "step", cJSON_CreateNumber(10));
        cJSON_AddItemToObject(reboot_count, "value", cJSON_CreateNumber(curr_reboot_count));
        cJSON_AddItemToObject(reboot_count, "counterType", cJSON_CreateString("GAUGE"));
        cJSON_AddItemToObject(reboot_count, "tags", cJSON_CreateString(tags.c_str()));
        cJSON_AddItemToArray(root, reboot_count);

        cJSON* reboot_reason = cJSON_CreateObject();
        cJSON_AddItemToObject(reboot_reason, "endpoint", cJSON_CreateString("xeye"));
        cJSON_AddItemToObject(reboot_reason, "metric", cJSON_CreateString("reboot_reason"));
        cJSON_AddItemToObject(reboot_reason, "timestamp", cJSON_CreateNumber(timestamp / 1000));
        cJSON_AddItemToObject(reboot_reason, "step", cJSON_CreateNumber(10));
        cJSON_AddItemToObject(reboot_reason, "value", cJSON_CreateNumber(latest_reboot_reason));
        cJSON_AddItemToObject(reboot_reason, "counterType", cJSON_CreateString("GAUGE"));
        cJSON_AddItemToObject(reboot_reason, "tags", cJSON_CreateString(tags.c_str()));
        cJSON_AddItemToArray(root, reboot_reason);
        const int do_format = 0;  // 0 gives unformatted, 1, gives formatted
        char* msg = cJSON_PrintBuffered(root, 512, do_format);
        cJSON_Delete(root);
        return msg;
    }
    std::string generate_http_header(std::string url, std::string domain_name) {
        if (url.empty()) {
            mvLog(MVLOG_ERROR, "init http header FAILED, empty URL");
            return std::string();
        }
        // reserve 512 bytes for storing http header
        std::string http_header = "POST " + url + " HTTP/1.1\r\n" + \
                                  "Host: " + domain_name + "\r\n" + \
                                  "Accept: */*\r\n" + \
                                  "Content-Type: application/json;charset=UTF-8\r\n";
        return http_header;
    }
private:
    rtems_id heartbeat_sem_;
    rtems_id heartbeat_task_id_;
    size_t retry_;
    size_t timeout_;
    TemperatureCallback temp_cb_;
    std::string xeye_id_;
    std::string http_header_;
    std::shared_ptr<HTTPClient> http_client_ptr_;
    std::vector<char*> heartbeat_msg_cached_;
};


#endif  // HEARTBEAT_MANAGER
