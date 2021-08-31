#ifndef TRACKING_DATA_MANAGER_HPP
#define TRACKING_DATA_MANAGER_HPP
#include "http.hpp"
#include "ringbuf.hpp"
#include "pubdef.h"
#include "facemsg.h"
#include "cJSON.h"
#include <rtems.h>
#include <map>
#include <vector>
#include <functional>
#include <mutex>
extern int lrt_image_width;
extern int lrt_image_height;
typedef std::function<void(int)> WatchdogControlCallback;
class TrackingDataManager {
const int watchdog_triggered_frames = 1024;
const std::string file_name = "/mnt/sdcard/tracking_data.txt";
public:
    TrackingDataManager(std::string target, int port, \
            std::string xeye_id, std::string url, \
            size_t msg_queue_size, size_t rb_for_tracks_capacity, \
            size_t rb_for_images_capacity) {
        msg_queue_size_ = msg_queue_size;
        http_client_ptr_.reset(new HTTPClient(target, port));
        ringbuf_for_bboxes_ptr_.reset(new Ringbuf_t<Track_t>(rb_for_tracks_capacity));
        ringbuf_for_images_ptr_.reset(new Ringbuf_t<char>(rb_for_images_capacity));
        url_ = std::move(url);
        xeye_id_ = std::move(xeye_id);
    }
    ~TrackingDataManager() {
        // TODO(yanghongtian): close message queue, waiting for thread joined
        rtems_message_queue_delete(msg_queue_id_);
        rtems_task_delete(thread_id_);
    }
    bool init(WatchdogControlCallback cb) {
        if (http_client_ptr_ == nullptr) {
            LOGE << "can not create http client object";
            return false;
        }
        http_header_ = std::move(generate_http_header(url_, http_client_ptr_->get_domain_name()));

        if (ringbuf_for_bboxes_ptr_ == nullptr) {
            LOGE << "Can not create ringbuf for bounding boxes";
            return false;
        }

        if (ringbuf_for_images_ptr_ == nullptr) {
            LOGE << "Can not create ringbuf for images";
            return false;
        }

        int status = rtems_message_queue_create(\
            rtems_build_name('M','S','D','M'), \
            msg_queue_size_, sizeof(HttpMsg), RTEMS_FIFO | RTEMS_LOCAL, \
            &msg_queue_id_);
        if (status != RTEMS_SUCCESSFUL) {
            mvLog(MVLOG_ERROR, "can not create message queue, status = %d", status);
            return false;
        }

        status = rtems_task_create(rtems_build_name('T','R','C','M'), 120, \
                RTEMS_MINIMUM_STACK_SIZE * 4, RTEMS_DEFAULT_MODES, \
                RTEMS_FLOATING_POINT | RTEMS_LOCAL, &thread_id_);
        if (status != RTEMS_SUCCESSFUL) {
            LOGE << "Create task FAILED";
            return false;
        }

        http_msgs_cached_for_tracking_.reserve(watchdog_triggered_frames);
        http_msgs_cached_for_images_.reserve(8);
        read_tracking_data_from_local_file(&http_msgs_cached_for_tracking_);
        status = rtems_task_start(thread_id_, \
                rtems_task_entry(&TrackingDataManager::thread_consumer), \
                rtems_task_argument(this));
        if (status != RTEMS_SUCCESSFUL) {
            LOGE << "Start task FAILED";
            return false;
        }
        register_watchdog_callback(cb);

        if (!http_client_ptr_->init()) {
            LOGE << "initalized http client FAILED";
            return false;
        }

        return true;
    }

    bool save_tracking_data_to_local_file() {
        if (http_msgs_cached_for_tracking_.size() == 0) {
            return true;
        }
        std::ofstream of(file_name);
        if (!of.is_open()) {
            return false;
        }
        std::vector<Track_t> bboxes;
        std::vector<HttpMsg>& tracking_data = http_msgs_cached_for_tracking_;
        for (size_t i = 0; i < tracking_data.size(); ++i) {
            of << tracking_data[i].ts << " " << tracking_data[i].num_boxes << " ";
            if (tracking_data[i].num_boxes > 0) {
                bboxes.resize(tracking_data[i].num_boxes);
                ringbuf_for_bboxes_ptr_->get(bboxes.data(), tracking_data[i].num_boxes, NULL);
                for (Track_t& ind : bboxes) {
                    of << ind.id << " " << ind.left << " " << ind.top << " " << ind.width << " "
                       << ind.height << " " << ind.confirmed << " " << ind.time_since_update << " ";
                }
            }
            of << "\n";
        }
        LOGI << "Saving " << tracking_data.size() << " tracking data to local disk";
        tracking_data.resize(0);
        return true;
    }

    bool read_tracking_data_from_local_file(std::vector<HttpMsg>* tracking_ptr) {
        std::ifstream ifs(file_name);
        if (!ifs.is_open()) {
            return false;
        }
        std::string line;
        uint64_t ts;
        while (std::getline(ifs, line)) {
            std::istringstream iss(line);
            // tracking data
            size_t num;
            iss >> ts >> num;
            std::vector<Track_t> tracks(num);
            for (size_t i = 0; i < num; ++i) {
                iss >> tracks[i].id >> tracks[i].left >> tracks[i].top \
                    >> tracks[i].width >> tracks[i].height \
                    >> tracks[i].confirmed >> tracks[i].time_since_update;
            }
            Track_t* addr = ringbuf_for_bboxes_ptr_->put(tracks.data(), num, NULL);
            HttpMsg tracking_msg = {
                HttpMsg::TRACKING_MSG,
                num,
                ts,
                reinterpret_cast<char*>(addr),
                NULL,
                0
            };
            tracking_ptr->push_back(tracking_msg);
        }
        ifs.close();
        remove(file_name.c_str());
        LOGI << "Reading " << tracking_ptr->size() << " tracking data from disk";
        return true;
    }

    void thread_consumer(rtems_task_argument unused) {
        UNUSED(unused);
        int status = 0;
        http_msgs_cached_for_tracking_.reserve(watchdog_triggered_frames);
        http_msgs_cached_for_images_.reserve(8);
        rtems_interval ticks_one_second = rtems_clock_get_ticks_per_second();
        size_t tracking_thres = 16;
        for (;;) {
            HttpMsg http_msg;
            size_t msg_size = 0;
            status = rtems_message_queue_receive(msg_queue_id_, &http_msg, \
                &msg_size, RTEMS_FIFO, ticks_one_second);
            if (status != RTEMS_SUCCESSFUL && status != RTEMS_TIMEOUT) {
                LOGE << "error occured when receiving message from queue, status = " << status;
                break;
            }

            if (status == RTEMS_SUCCESSFUL) {
                if (http_msg.type == HttpMsg::TRACKING_MSG) {
                    http_msgs_cached_for_tracking_.push_back(http_msg);
                } else if (http_msg.type == HttpMsg::IMAGE_MSG) {
                    http_msgs_cached_for_images_.push_back(http_msg);
                }

                uint32_t num_pending = 0;
                status = rtems_message_queue_get_number_pending(msg_queue_id_, &num_pending);
                if (status == RTEMS_SUCCESSFUL) {
                    status = rtems_message_queue_receive(msg_queue_id_, &http_msg, \
                        &msg_size, RTEMS_FIFO, RTEMS_WAIT);
                    if (status == RTEMS_SUCCESSFUL) {
                        if (http_msg.type == HttpMsg::TRACKING_MSG) {
                            http_msgs_cached_for_tracking_.push_back(http_msg);
                        } else if (http_msg.type == HttpMsg::IMAGE_MSG) {
                            http_msgs_cached_for_images_.push_back(http_msg);
                        }
                    }
                }
                tracking_thres = 16;
            } else {
                tracking_thres = 1;
            }

            if (http_msgs_cached_for_tracking_.size() >= tracking_thres) {
                if (post_tracking_msg(http_msgs_cached_for_tracking_, 1, 4000)) {
                    http_msgs_cached_for_tracking_.resize(0);
                } else {
                    LOGE << "post tracking message failed. current cached size: "
                         << http_msgs_cached_for_tracking_.size();
                    if (http_msgs_cached_for_tracking_.size() > watchdog_triggered_frames) {
                        // disable feeding watchdog to enable system reboot
                        if (wd_cb_) {
                            save_tracking_data_to_local_file();
                            wd_cb_(NETWORK_ISSUE);
                        }
                    }
                }
            }
            if (http_msgs_cached_for_images_.size() > 0) {
                if (post_tracking_msg(http_msgs_cached_for_images_, 0, 2000)) {
                    http_msgs_cached_for_images_.resize(0);
                } else {
                    LOGE << "post image message failed.";
                }
            }
            rtems_task_wake_after(10);
        }
    }
    bool register_watchdog_callback(WatchdogControlCallback cb) {
        wd_cb_ = cb;
        return true;
    }

    bool add_tracking_data(const IpcMsg& msg, const uint64_t& ts) {
        uint32_t num_pending = 0;
        rtems_message_queue_get_number_pending(msg_queue_id_, &num_pending);
        // make sure we send the current message queue success.
        if (msg_queue_size_ - num_pending < 2) {
            return false;
        }
        // if tracking data is not empty, push to ring buffer
        Track_t* tracking_addr = NULL;
        if (msg.msg_len > 0) {
            tracking_addr = add_tracking_data(reinterpret_cast<Track_t*>(\
                msg.msg_data), msg.msg_len);
            // no space for current tracking data, drop it.
            if (tracking_addr == NULL) {
                mvLog(MVLOG_ERROR, "add tracking data failed");
                return false;
            }
        }
        char* img_addr = NULL;
        if (msg.jpg_buf != NULL) {
            img_addr = add_jpeg_image_data(reinterpret_cast<char*>(msg.jpg_buf), msg.jpg_len);
        }
        HttpMsg http_msg = {
            HttpMsg::TRACKING_MSG,
            msg.msg_len,
            ts,
            reinterpret_cast<char*>(tracking_addr),
            img_addr,
            (img_addr != NULL) ? msg.jpg_len : 0
        };

        int status = rtems_message_queue_send(msg_queue_id_, \
            &http_msg, sizeof(http_msg));
        return (status == RTEMS_SUCCESSFUL);
    }

    bool add_jpeg_image_data(const IpcMsg& msg, const uint64_t& ts) {
        if (msg.jpg_len <= 0 || msg.jpg_buf == NULL) {
            return false;
        }

        uint32_t num_pending = 0;
        rtems_message_queue_get_number_pending(msg_queue_id_, &num_pending);
        // make sure we send the current message queue success.
        if (msg_queue_size_ - num_pending < 2) {
            return false;
        }
        char* img_addr = add_jpeg_image_data(reinterpret_cast<char*>(msg.jpg_buf), msg.jpg_len);
        // no enough space for jpeg image.
        if (img_addr == NULL) {
            return false;
        }

        HttpMsg http_msg = {
            HttpMsg::IMAGE_MSG,
            0,
            ts,
            NULL,
            img_addr,
            msg.jpg_len
        };
        int status = rtems_message_queue_send(msg_queue_id_, \
            &http_msg, sizeof(http_msg));
        return (status == RTEMS_SUCCESSFUL);
    }
private:
    bool post_tracking_msg(const std::vector<HttpMsg>& msg, size_t retry, int timeout) {
        // if the current cached msgs are too many, maybe server ip are changed
        // resolve ip address again
        if (msg.size() > 256) {
            if (!http_client_ptr_->resolve_domain_name_to_address()) {
                LOGE << "resolve ip address by domain name failed";
            }
        }
        size_t retry_times = retry;
        bool connected = false;
        do {
            connected = http_client_ptr_->connect_to_server(timeout);
            if (connected) {
                break;
            }
            rtems_task_wake_after(100);
        } while (retry_times--);

        if (!connected) {
            LOGE << "connect to server in post tracking message failed";
            return false;
        }
        char* json_body = convert_tracking_info_to_json(msg, xeye_id_);
        // post interface close socket internal
        bool succeed = http_client_ptr_->post(http_header_, \
                json_body, strlen(json_body));
        free(json_body);
        return succeed;
    }

    char* convert_tracking_info_to_json(const std::vector<HttpMsg>& http_msgs, \
        const std::string& xeye_id) {
        const size_t max_bboxes = 32;
        bool img_activated = false;
        std::vector<Track_t> bboxes(max_bboxes);
        cJSON* root = cJSON_CreateObject();
        cJSON* data = cJSON_CreateArray();
        cJSON_AddItemToObject(root, "xeye_id", cJSON_CreateString(xeye_id.c_str()));
        cJSON_AddItemToObject(root, "data", data);

        std::vector<char*> tmp_buf;
        for (const HttpMsg& msg : http_msgs) {
            if (msg.num_boxes > 0) {
                bboxes.resize(msg.num_boxes);
                if (msg.data != reinterpret_cast<const char*>(\
                    ringbuf_for_bboxes_ptr_->get_read_addr())) {
                    LOGW << "provide data: " << msg.data
                         << ", is not equal to " << ringbuf_for_bboxes_ptr_->get_read_addr();
                    // remove one bbox to sync with objs
                    unsigned int offset = ringbuf_for_bboxes_ptr_->find(reinterpret_cast<Track_t*>(msg.data));
                    if (offset == 0) {
                        // msg.data object is not contained in ringbuffer, or ringbuffer is empty.
                        // drop this frame.
                        LOGW << "drop a frame of tracking data because of no available data found in ringbuffer";
                        continue;
                    } else {
                        // remove offset objects
                        ringbuf_for_bboxes_ptr_->get(NULL, offset, NULL);
                    }
                }
                size_t num_get = 0;
                Track_t* addr = ringbuf_for_bboxes_ptr_->get(bboxes.data(), msg.num_boxes, &num_get);
                assert(num_get == msg.num_boxes);
                assert(addr == reinterpret_cast<Track_t*>(msg.data));
            }
            const std::vector<Track_t>& current_bboxes = bboxes;
            cJSON* one_frame = cJSON_CreateObject();

            cJSON_AddItemToObject(one_frame, "ts", cJSON_CreateNumber(msg.ts));
            cJSON* ppl = cJSON_CreateArray();
            cJSON_AddItemToObject(one_frame, "ppl", ppl);
            size_t number_of_bboxes = current_bboxes.size();
            for (size_t j = 0; j < msg.num_boxes; ++j) {
                if (current_bboxes[j].width > lrt_image_width || \
                    current_bboxes[j].height > lrt_image_height || \
                    current_bboxes[j].left > lrt_image_width || \
                    current_bboxes[j].top > lrt_image_height || \
                    current_bboxes[j].width < 0 || \
                    current_bboxes[j].height < 0 || \
                    current_bboxes[j].top < 0 || \
                    current_bboxes[j].left < 0) {
                        mvLog(MVLOG_ERROR, "%d, %d, %d, %d, %d, %d, %d", current_bboxes[j].id, \
                        current_bboxes[j].left, current_bboxes[j].top, current_bboxes[j].width, \
                        current_bboxes[j].height, current_bboxes[j].confirmed, \
                        current_bboxes[j].time_since_update);
                }
                cJSON* one_bbox = cJSON_CreateIntArray(reinterpret_cast<const int*>(\
                            &current_bboxes[j].id), sizeof(Track_t) / sizeof(int));
                cJSON_AddItemToArray(ppl, one_bbox);
            }
            cJSON_AddItemToArray(data, one_frame);
            if (msg.jpg_img) {
                img_activated = true;
                assert(msg.jpg_img == ringbuf_for_images_ptr_->get_read_addr());
                if (!ringbuf_for_images_ptr_->is_memory_overlap(msg.jpg_img, msg.jpg_len)) {
                    cJSON_AddItemToObject(one_frame, "img", cJSON_CreateString(msg.jpg_img));
                    ringbuf_for_images_ptr_->get(NULL, msg.jpg_len, NULL);
                } else {
                    // handle the image memory overlap in ring buffer.
                    char* img_buf = new char[msg.jpg_len];
                    ringbuf_for_images_ptr_->get(img_buf, msg.jpg_len, NULL);
                    cJSON_AddItemToObject(one_frame, "img", cJSON_CreateString(img_buf));
                    tmp_buf.push_back(img_buf);
                }
            } else {
                cJSON_AddItemToObject(one_frame, "img", cJSON_CreateString(""));
            }
        }
        const int do_format = 0;  // 0 gives unformatted, 1 gives formatted
        char* json_str = cJSON_PrintBuffered(root, img_activated ? 128 * 1024 : 4 * 1024, do_format);
        cJSON_Delete(root);
        for (size_t i = 0; i < tmp_buf.size(); ++i) {
            if (tmp_buf[i]) {
                delete[] tmp_buf[i];
            }
        }
        return json_str;
    }
    /// data: the pointer of tracking data
    /// num: the number of tracking data, in Track_t
    Track_t* add_tracking_data(const Track_t* data, size_t num){
        uint32_t free_num = ringbuf_for_bboxes_ptr_->get_free_obj_number();
        // no enough space for tracking data of current frame.
        if (free_num < num) {
            return NULL;
        }
        return ringbuf_for_bboxes_ptr_->put(data, num, NULL);
    }
    /// data: the pointer of jpeg data
    /// len:  the length of jpeg image data, in bytes
    char* add_jpeg_image_data(const char* data, size_t len) {
        uint32_t free_num = ringbuf_for_images_ptr_->get_free_obj_number();
        // no enough space for this image frame
        if (free_num < len) {
            return NULL;
        }
        return ringbuf_for_images_ptr_->put(data, len, NULL);
    }
    std::string generate_http_header(const std::string& url, const std::string& domain_name) {
        if (url.empty()) {
            mvLog(MVLOG_ERROR, "init http header FAILED, empty URL");
            return std::string();
        }

        std::string http_header = "POST " + url + " HTTP/1.1\r\n" + \
                                  "Host: " + domain_name + "\r\n" + \
                                  "Accept: */*\r\n" + \
                                  "Content-Type: application/json;charset=UTF-8\r\n";
        return http_header;
    }
private:
    rtems_id msg_queue_id_;
    rtems_id thread_id_;
    WatchdogControlCallback wd_cb_;
    size_t msg_queue_size_;
    std::shared_ptr<Ringbuf_t<Track_t>> ringbuf_for_bboxes_ptr_;
    std::shared_ptr<Ringbuf_t<char>> ringbuf_for_images_ptr_;
    std::shared_ptr<HTTPClient> http_client_ptr_;
    std::string url_;
    std::string http_header_;
    std::string xeye_id_;
    std::vector<HttpMsg> http_msgs_cached_for_tracking_;
    std::vector<HttpMsg> http_msgs_cached_for_images_;
};
#endif  // TRACKING_DATA_MANAGER_HPP
