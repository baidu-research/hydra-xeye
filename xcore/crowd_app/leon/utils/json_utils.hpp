#ifndef _JSON_UTILS_HPP
#define _JSON_UTILS_HPP
#include "pubdef.h"
#include <cJSON.h>
#include <string>
#include <vector>
#include <memory>
#include <cassert>
#include <mvLog.h>
#include <mvTensorTimer.h>
#include <Log.h>
extern int lrt_image_width;
extern int lrt_image_height;

namespace utils {
char* convert_tracking_info_to_json(const std::string& xeye_id, \
        const std::vector<std::vector<Track_t>>& bboxes, \
        const std::vector<char*>& imgs, \
        const std::vector<uint64_t>& ts) {
    LOGE_IF(ts.size() != bboxes.size()) << "invalid input vector size: "
        << ts.size() << ", " << bboxes.size();
    LOGE_IF(ts.size() != imgs.size()) << "Invalid input imgs size: "
        << ts.size() << ", " << imgs.size();
    if (ts.size() != bboxes.size()) {
        return NULL;
    }

    size_t number_of_frames = bboxes.size();
    cJSON* root = cJSON_CreateObject();
    cJSON* data = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "xeye_id", cJSON_CreateString(xeye_id.c_str()));
    cJSON_AddItemToObject(root, "data", data);
    bool img_activated = false;
    for (size_t i = 0; i < number_of_frames; ++i) {
        const std::vector<Track_t>& current_bboxes = bboxes[i];
        cJSON* one_frame = cJSON_CreateObject();

        cJSON_AddItemToObject(one_frame, "ts", cJSON_CreateNumber(ts[i]));
        cJSON* ppl = cJSON_CreateArray();
        cJSON_AddItemToObject(one_frame, "ppl", ppl);
        size_t number_of_bboxes = current_bboxes.size();
        for (size_t j = 0; j < number_of_bboxes; ++j) {
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
            cJSON* one_bbox = cJSON_CreateIntArray(reinterpret_cast<const int*>(&current_bboxes[j].id), \
                    sizeof(Track_t) / sizeof(int));
            cJSON_AddItemToArray(ppl, one_bbox);
        }
        cJSON_AddItemToArray(data, one_frame);
        if (imgs[i]) {
            img_activated = true;
            cJSON_AddItemToObject(one_frame, "img", cJSON_CreateString(imgs[i]));
        } else {
            cJSON_AddItemToObject(one_frame, "img", cJSON_CreateString(""));
        }
    }
    mv::tensor::Timer convert_timer;
    const int do_format = 0;  // 0 gives unformatted, 1 gives formatted
    char* json_str = cJSON_PrintBuffered(root, img_activated ? 128 * 1024 : 4 * 1024, do_format);
    mvLog(MVLOG_DEBUG, "convert cjson object to string takes %f ms\n%s", \
            convert_timer.elapsed(), json_str);
    cJSON_Delete(root);
    return json_str;
}

#if 0
char* convert_tracking_info_to_json(const std::string& xeye_id, \
        const std::vector<std::vector<Track_t>>& bboxes, \
        const std::vector<char*>& imgs, \
        const std::vector<uint64_t>& ts) {
    LOGE_IF(ts.size() != bboxes.size()) << "invalid input vector size: "
        << ts.size() << ", " << bboxes.size();
    LOGE_IF(ts.size() != imgs.size()) << "Invalid input imgs size: "
        << ts.size() << ", " << imgs.size();
    if (ts.size() != bboxes.size()) {
        return NULL;
    }

    size_t number_of_frames = bboxes.size();
    cJSON* root = cJSON_CreateObject();
    cJSON* data = cJSON_CreateArray();
    cJSON_AddItemToObject(root, "xeye_id", cJSON_CreateString(xeye_id.c_str()));
    cJSON_AddItemToObject(root, "data", data);
    bool img_activated = false;
    for (size_t i = 0; i < number_of_frames; ++i) {
        const std::vector<Track_t>& current_bboxes = bboxes[i];
        cJSON* one_frame = cJSON_CreateObject();

        cJSON_AddItemToObject(one_frame, "ts", cJSON_CreateNumber(ts[i]));
        cJSON* ppl = cJSON_CreateArray();
        cJSON_AddItemToObject(one_frame, "ppl", ppl);
        size_t number_of_bboxes = current_bboxes.size();
        for (size_t j = 0; j < number_of_bboxes; ++j) {
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
            cJSON* one_bbox = cJSON_CreateIntArray(reinterpret_cast<const int*>(&current_bboxes[j].id), \
                    sizeof(Track_t) / sizeof(int));
            cJSON_AddItemToArray(ppl, one_bbox);
        }
        cJSON_AddItemToArray(data, one_frame);
        if (imgs[i]) {
            img_activated = true;
            cJSON_AddItemToObject(one_frame, "img", cJSON_CreateString(imgs[i]));
        } else {
            cJSON_AddItemToObject(one_frame, "img", cJSON_CreateString(""));
        }
    }
    mv::tensor::Timer convert_timer;
    const int do_format = 0;  // 0 gives unformatted, 1 gives formatted
    char* json_str = cJSON_PrintBuffered(root, img_activated ? 128 * 1024 : 4 * 1024, do_format);
    mvLog(MVLOG_DEBUG, "convert cjson object to string takes %f ms\n%s", \
            convert_timer.elapsed(), json_str);
    cJSON_Delete(root);
    return json_str;
}
#endif
char* convert_heartbeat_info_to_json(const std::string& xeye_id, \
        const uint64_t& timestamp, float temp, \
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
    cJSON_AddItemToObject(temperature, "value", cJSON_CreateNumber(int(temp * 1000)));
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
}  // namespace utils
#endif  // _JSON_UTILS_HPP
