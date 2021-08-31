#ifndef ONLINE_UPGRADE_HPP
#define ONLINE_UPGRADE_HPP
#include "params/params.hpp"
#include "spiflash/spiflash.hpp"
#include "utils/utils.hpp"
#include "http/http.hpp"
#include "cJSON.h"
#include <string>
#include <vector>
#include <map>
#include <cctype>
#include <rtems.h>
#include <zbase64.h>
#include <fstream>
#include <params/params.hpp>
#include <spiflash/spiflash.hpp>
#include <mvTensorTimer.h>
#include <microtar.h>
#include <cassert>
typedef std::map<std::string, std::string> FileTypeVerMap;
typedef std::map<std::string, std::string> UrlHeaderMap;
typedef std::function<void(int)> WatchdogCallback;
struct FileDesc {
    std::string file_type_;
    std::string md5_;
    std::string file_content_;
};

struct ResponseRes {
    int code_;
    std::string request_id_;
    std::string error_message_;
    FileDesc file_desc_;
    void print(void) {
        std::cout << "request_id: " << request_id_ << std::endl;
        std::cout << "code: " << code_ << std::endl;
        std::cout << "message: " << error_message_ << std::endl;
        std::cout << "\t" << "file_type: " << file_desc_.file_type_ << std::endl;
        std::cout << "\t" << "version: " << file_desc_.md5_ << std::endl;
    }
    std::string get_file_name_from_version(const std::string& firm_ver) {
        std::string file_name = firm_ver;
        file_name.erase(std::remove(file_name.begin(), file_name.end(), '!'), file_name.end());
        return "/mnt/sdcard/" + file_name + ".tar";
    }
    std::string save_to_file(void) {
        std::string file_name = get_file_name_from_version(file_desc_.md5_);
        LOGI << "save current upgrade data to file, file name: " << file_name;
        std::ofstream ofs(file_name);
        ofs << file_desc_.file_content_;
        ofs.close();
        return file_name;
    };
};


class MicroTar {
public:
    MicroTar(const char* tar_buf, size_t buf_size) {
        int status = mtar_open(&tar_, tar_buf, buf_size);
        if (status == MTAR_ESUCCESS) {
            mtar_header_t file_header;
            while (mtar_read_header(&tar_, &file_header) != MTAR_ENULLRECORD) {
                LOGD << "file name: " << file_header.name << ", file size: " << file_header.size;
                file_headers_.push_back(file_header);
                mtar_next(&tar_);
            }
            // parsing readme.txt
            for (const mtar_header_t& h : file_headers_) {
                if (std::string(h.name) == "readme.txt") {
                    status = mtar_find(&tar_, "readme.txt", &h);
                    if (status != MTAR_ESUCCESS) {
                        continue;
                    }
                    std::string file_content(h.size + 1, '\0');
                    status = mtar_read_data(&tar_, file_content.data(), h.size);
                    if (status != MTAR_ESUCCESS) {
                        continue;
                    }
                    size_t pos = 0;
                    std::string one_line;
                    while ((pos = file_content.find('\n')) != std::string::npos) {
                        one_line = file_content.substr(0, pos);
                        file_content.erase(0, pos + 1);
                        std::string md5 = get_md5_from_string_line(one_line);
                        std::string file_type = get_file_type_from_string_line(one_line);
                        type_ver_map_[file_type] = md5;
                    }
                }
            }
            initialized_ = type_ver_map_.size() > 0;
        } else {
            initialized_ = false;
        }
    }
    MicroTar(const std::string& tar_file) {
        int status = mtar_open(&tar_, tar_file.c_str(), "r");
        if (status == MTAR_ESUCCESS) {
            mtar_header_t file_header;
            while (mtar_read_header(&tar_, &file_header) != MTAR_ENULLRECORD) {
                mvLog(MVLOG_INFO, "file name: %s, file size: %llu", file_header.name, file_header.size);
                file_headers_.push_back(file_header);
                mtar_next(&tar_);
            }
            // parsing readme.txt
            for (const mtar_header_t& h : file_headers_) {
                if (std::string(h.name) == "readme.txt") {
                    status = mtar_find(&tar_, "readme.txt", &h);
                    if (status != MTAR_ESUCCESS) {
                        continue;
                    }
                    std::string file_content(h.size + 1, '\0');
                    status = mtar_read_data(&tar_, file_content.data(), h.size);
                    if (status != MTAR_ESUCCESS) {
                        continue;
                    }
                    size_t pos = 0;
                    std::string one_line;
                    while ((pos = file_content.find('\n')) != std::string::npos) {
                        one_line = file_content.substr(0, pos);
                        file_content.erase(0, pos + 1);
                        std::string md5 = get_md5_from_string_line(one_line);
                        std::string file_type = get_file_type_from_string_line(one_line);
                        type_ver_map_[file_type] = md5;
                    }
                }
            }
            initialized_ = type_ver_map_.size() > 0;
        } else {
            LOGI << "can not open tar file, file name: " << tar_file;
            initialized_ = false;
        }
    }
    ~MicroTar() {
        if (initialized_) {
            mtar_close(&tar_);
        }
    }
    std::string get_ver_from_key(const std::string& key) const {
        return type_ver_map_[key];
    }
    // output the files that needed to be updated
    bool untar(const std::string& curr_firm_ver, std::vector<FileDesc>* files) {
        if (!initialized_) {
            return false;
        }
        if (files == NULL) {
            LOGE << "unmatch file type size";
            return false;
        }
        std::string upgrade_version = get_ver_from_key("version");
        if (upgrade_version.empty()) {
            return false;
        }

        if (curr_firm_ver == upgrade_version) {
            LOGI << "current firmware is identical, no need to update anything.";
            return false;
        }

        for (const FileTypeVerMap::iterator it = type_ver_map_.begin(); \
                it != type_ver_map_.end(); ++it) {
            if (it->first == "version") {
                continue;
            }

            FileDesc file;
            file.file_type_ = it->first;
            file.md5_ = it->second;
            file.file_content_ = get_file_from_tar(it->first);
            utils::MD5Calculator md5_calc;
            // if the received content's MD5 is not the same as readme.txt record, do not update.
            std::string md5_received = md5_calc(reinterpret_cast<unsigned char*>(\
                    const_cast<char*>(file.file_content_.data())), \
                    file.file_content_.length());
            // if md5 of the received content is not identical with the precompute one
            // DO NOT update anything
            if (md5_received != it->second) {
                LOGE << "the md5 of " << it->first << " is not identical of precompute md5, DO NOT UPDATE anything";
                LOGE << md5_received << " : " << it->second;
                return false;
            }
            files->push_back(file);
        }
        return files->size() > 0;
    }

private:
    std::string get_file_from_tar(const std::string& file_name) {
        int status = 0;
        if (file_name.empty()) {
            return std::string();
        }

        for (const mtar_header_t& h : file_headers_) {
            if (file_name == std::string(h.name)) {
                status = mtar_find(&tar_, file_name.c_str(), &h);
                if (status != MTAR_ESUCCESS) {
                    return std::string();
                }
                std::string file_content(h.size, '\0');
                status = mtar_read_data(&tar_, file_content.data(), h.size);
                if (status != MTAR_ESUCCESS) {
                    return std::string();
                } else {
                    return file_content;
                }
            }
        }
        return std::string();
    }
    std::string get_md5_from_string_line(const std::string& one_string_line) {
        size_t pos = one_string_line.find(':');
        if (pos == std::string::npos) {
            return std::string();
        }
        std::string md5 = one_string_line.substr(pos + 1, one_string_line.length());
        md5.erase(std::remove_if(md5.begin(), md5.end(), \
                    [](unsigned char x){return std::isspace(x);}), md5.end());
        return md5;
    }
    std::string get_file_type_from_string_line(const std::string& one_string_line) {
        size_t pos = one_string_line.find(':');
        if (pos == std::string::npos) {
            return std::string();
        }
        std::string file_type = one_string_line.substr(0, pos);
        file_type.erase(std::remove_if(file_type.begin(), file_type.end(), \
                    [](unsigned char x){return std::isspace(x);}), file_type.end());
        return file_type;
    }

private:
    bool initialized_;
    mtar_t tar_;
    std::vector<mtar_header_t> file_headers_;
    FileTypeVerMap type_ver_map_;
};



class OnlineUpdateManager {
const std::string api_key = "xeye-hub";
const std::string secret_key = "3af64f74e89fecd02151a204022c5c34";
const std::string url_prefix = "/service/crowd/xeyehub";
const std::string get_file_url = "/api/v1/update/getFile";
const std::string report_ver_url = "/api/v1/update/postVersion";
const std::string device_type = "crowd";
const std::vector<std::string> file_types_supported = {
    "config",
    "firmware",
    "detect",
    "version"
};
public:
    // host can be domain name or ip address
    OnlineUpdateManager(std::string host, int port, \
            std::string xeye_id, std::string firm_ver,
            std::string url_prefix_in, size_t retry = 1, size_t timeout_ms = 4000) :
        xeye_id_(xeye_id), curr_firm_ver_(firm_ver) {
        retry_ = retry;
        timeout_ms_ = timeout_ms;
        http_client_ptr_.reset(new HTTPClient(host, port));
        std::vector<std::string> urls = {
            get_file_url,
            report_ver_url
        };
        if (!url_prefix_in.empty()) {
            url_prefix_ = url_prefix_in;
            LOGI << "Overwrite url prefix to " << url_prefix_;
        }  else {
            url_prefix_ = url_prefix;
        }

        add_url(urls);
    }
    ~OnlineUpdateManager() {
        rtems_task_delete(thread_id_);
    }

    bool init(WatchdogCallback cb) {
        int status = rtems_task_create(rtems_build_name('O','L','N','U'), \
                200, RTEMS_MINIMUM_STACK_SIZE * 8, RTEMS_DEFAULT_MODES, \
                RTEMS_DEFAULT_ATTRIBUTES, &thread_id_);
        if (status != RTEMS_SUCCESSFUL) {
            LOGE << "Create task in OnlineUpdateManager FAILED";
            return false;
        }

        if (!http_client_ptr_->init()) {
            LOGE << "Http initalize failed";
            return false;
        }

        status = rtems_task_start(thread_id_, \
                rtems_task_entry(&OnlineUpdateManager::thread_online_update), \
                rtems_task_argument(this));
        if (status != RTEMS_SUCCESSFUL) {
            LOGE << "Start task in OnlineUpdateManager FAILED";
            return false;
        }
        wd_cb_ = cb;
        return true;
    }

private:
    void add_url(const std::vector<std::string>& urls) {
        urls_ = urls;

        for (const std::string& item : urls) {
            url_specific_http_header_[item] = generate_http_header(item, \
                    http_client_ptr_->get_domain_name());
        }
    }
    std::string make_public_header(const std::string& url,
            const std::string& body, const uint64_t& ts) {
        std::string header = url_specific_http_header_[url] + \
            "request_id: " + xeye_id_ + std::to_string(ts) + "\r\n" + \
            "request_time: " + std::to_string(ts) + "\r\n" + \
            "signature: " + generate_signature(xeye_id_, ts, body) + "\r\n";
        return header;
    }
    std::string generate_signature(const std::string& xeye_id, \
            const uint64_t& ts, const std::string& request_body) {
        std::string signature_for_md5;
        std::string request_id = xeye_id + std::to_string(ts);
        signature_for_md5.reserve(api_key.length() + secret_key.length() + \
                request_body.length() + request_id.length());
        signature_for_md5 = api_key + secret_key + request_id + std::to_string(ts) + request_body;
        utils::MD5Calculator md5_calc;
        return md5_calc(reinterpret_cast<unsigned char*>(\
                    const_cast<char*>(signature_for_md5.c_str())), \
                signature_for_md5.length());
    }

    std::string generate_payload(const std::string& xeye_id) {
        cJSON* root = cJSON_CreateObject();
        if (root == NULL) {
            return std::string();
        }
        cJSON_AddItemToObject(root, "device_id", cJSON_CreateString(xeye_id.c_str()));
        cJSON_AddItemToObject(root, "device_type", cJSON_CreateString(device_type.c_str()));

        // version arrays
        cJSON* version;
        cJSON_AddItemToObject(root, "version", version = cJSON_CreateArray());

        cJSON* file_type = cJSON_CreateObject();
        cJSON_AddItemToObject(file_type, "file_type", cJSON_CreateString("crowd_firmware"));
        cJSON_AddItemToObject(file_type, "current_version", \
                cJSON_CreateString(curr_firm_ver_.c_str()));
        cJSON_AddItemToArray(version, file_type);
        const int do_format = 0;
        char* payload = cJSON_PrintBuffered(root, 256, do_format);

        cJSON_Delete(root);
        std::string ret = std::string(payload);
        free(payload);
        return ret;
    }

    bool update_model(const std::string& file_content) {
        std::string model_name = Params::getInstance()->get_model_name_by_index(0);
        if (model_name.empty()) {
            LOGE << "invalid model name";
            return false;
        }
        std::string model_path = Params::getInstance()->get_model_path_by_name(model_name);
        if (model_path.empty()) {
            LOGE << "invalid model path";
            return false;
        }
        std::ofstream ofs(model_path + model_name);
        ofs << file_content;
        ofs.close();
        LOGI << "Update detection model successfully";
        return true;
    }

    bool update_firmware(const std::string& file_content) {
        if (file_content.empty()) {
            LOGE << "no valid firmware needed to be updated";
            return false;
        }

        SpiFlash::getInstance()->update_firmware(file_content.c_str(), \
                file_content.length());
        return true;
    }

    bool update_config(const std::string& file_content) {
        std::ofstream ofs("/mnt/sdacrd/conf.json");
        std::string conf_after_merge = Params::getInstance()->merge_conf_json(file_content.c_str());
        if (conf_after_merge.empty()) {
            LOGE << "invalid config file after merge";
            return false;
        }
        ofs << conf_after_merge;
        ofs.close();
        LOGI << "Update config file successfully.";
        return true;
    }

    bool post_version_report(std::shared_ptr<char> buf, size_t buf_len) {
        bool connected = false;
        int retry = retry_;
        do {
            connected = http_client_ptr_->connect_to_server(timeout_ms_);
            if (connected) {
                break;
            }
            rtems_task_wake_after(100);
        } while (retry--);

        if (!connected) {
            LOGE << "Can not connect to server";
            return false;
        }
        uint64_t ts = utils::unix_timestamp();
        std::string payload = generate_payload(xeye_id_);
        std::string http_header = make_public_header(report_ver_url, payload, ts);
        // socket are connect in http client internal
        return http_client_ptr_->post_with_response(http_header, \
                payload.c_str(), payload.length(), buf.get(), buf_len, 60);
    }


    bool post_get_file(std::shared_ptr<char> buf, size_t buf_len) {
        bool connected = false;
        int retry = retry_;
        do {
            connected = http_client_ptr_->connect_to_server(timeout_ms_);
            if (connected) {
                break;
            }
            rtems_task_wake_after(100);
        } while (retry--);

        if (!connected) {
            return false;
        }
        uint64_t ts = utils::unix_timestamp();
        std::string payload = generate_payload(xeye_id_);

        std::string http_header = make_public_header(get_file_url, payload, ts);

        return http_client_ptr_->post_with_response(http_header, \
                payload.c_str(), payload.length(), buf.get(), buf_len);
    }

private:
    void thread_online_update(rtems_task_argument unused) {
        UNUSED(unused);
        const int buf_len = 16 * 1024 * 1024;
        std::shared_ptr<char> online_update_buf(new char[buf_len]);
        assert(online_update_buf != nullptr);
        bool need_to_reboot = false;
        // only report once
        bool reported = false;
        for (;;) {
            // post version report at the very beginning.
            if (!reported) {
                LOGI << "Start to report current version to server";
                if (post_version_report(online_update_buf, buf_len)) {
                    LOGI << "report version successfully, current versin: " << curr_firm_ver_;
                    reported = true;
                } else {
                    LOGE << "report version failed";
                }
            }

            LOGI << "Start to send get_file request to server";
            // post a request for system upgrade.
            if (post_get_file(online_update_buf, buf_len)) {
                ResponseRes result;
                if (parse_json_key_from_response(online_update_buf, &result)) {
                    if (result.code_ == 0) {
                        if (!result.file_desc_.file_content_.empty()) {
                            LOGI << "Something needs to be update.";
                            need_to_reboot = apply_update(result.file_desc_.file_content_);
                        } else {
                            LOGI << "server send response with no file content";
                        }
                    } else {
                        LOGE << result.error_message_;
                    }
                } else {
                    LOGE << "Parsing json failed.";
                }
            } else {
                LOGE << "post get file return failed.";
            }

            if (need_to_reboot) {
                LOGI << "upgrade applying finished, try to reboot to take effect";
                need_to_reboot = false;
                // trigger system reboot
                if(wd_cb_) {
                    wd_cb_(SYSTEM_UPGRADE);
                }
            }

            rtems_task_wake_after(60000);
        }
    }
    bool apply_update(const std::string& tar_buffer) {
        bool untar_status = false;
        bool need_to_reboot = false;
        std::vector<FileDesc> files;
        MicroTar tar_tool(tar_buffer.c_str(), tar_buffer.length());

        untar_status = tar_tool.untar(curr_firm_ver_, &files);
        if (!untar_status || files.size() == 0) {
            LOGD << "The version of all files are identical, no need to upgrade";
            return false;
        }

        for (const FileDesc& file : files) {
            LOGI << "Start to update " << file.file_type_;
            if (file.file_type_ == "firmware") {
                update_firmware(file.file_content_);
                need_to_reboot = true;
            } else if (file.file_type_ == "config") {
                update_config(file.file_content_);
                need_to_reboot = true;
            } else if (file.file_type_ == "detect") {
                update_model(file.file_content_);
                need_to_reboot = true;
            } else {
                LOGE << "invalid file type";
                return false;
            }
        }
        return need_to_reboot;
    }
    std::string generate_http_header(const std::string& url, const std::string& domain_name) {
        if (url.empty() || domain_name.empty()) {
            return std::string();
        }

        std::string http_header;
        http_header.reserve(512);
        http_header = "POST " + url_prefix_ + url + " HTTP/1.1\r\n";
        http_header += "Host: " + domain_name + "\r\n";
        http_header += "Accept: application/json\r\n";
        http_header += "Content-Type: application/json;charset=UTF-8\r\n";
        return http_header;
    }

    bool parse_json_key_from_response(std::shared_ptr<char> response, \
            ResponseRes* res) {
        std::string response_str = std::string(response.get());
        size_t pos = response_str.find("{\"request_id\":");
        if (pos == std::string::npos) {
            LOGE << "invalid response payload: " << response_str;
            return false;
        }

        cJSON* root = cJSON_Parse(response.get() + pos);
        if (root == NULL) {
            LOGE << "http response's content is not a json string";
            return false;
        }

        // parsing request_id;
        cJSON* node = NULL;
        node = cJSON_GetObjectItem(root, "request_id");
        if (node == NULL) {
            LOGE << "no request id node in json string";
        } else {
            res->request_id_ = std::string(node->valuestring);
        }

        // operation code
        node = cJSON_GetObjectItem(root, "code");
        if (node == NULL) {
            LOGE << "no code node in json string";
        } else {
            res->code_ = node->valueint;
        }

        node = cJSON_GetObjectItem(root, "message");
        if (node == NULL) {
            LOGE << "no error message provide";
        } else {
            res->error_message_ = std::string(node->valuestring);
        }

        if (res->code_ != 0) {
            // operation is not valid
            cJSON_Delete(root);
            return true;
        }

        node = cJSON_GetObjectItem(root, "data");
        if (node == NULL) {
            LOGE << "No data node in json string";
            cJSON_Delete(root);
            return false;
        }

        int array_size = cJSON_GetArraySize(node);
        if (array_size <= 0) {
            LOGE << "invalid array size in json string";
            return false;
        }
        if (array_size != 1) {
            LOGE << "invalid array size";
            return false;
        }
        int out_byte = 0;
        for (int i = 0; i < array_size; ++i) {
            FileDesc& file_desc = res->file_desc_;
            cJSON* file_node = cJSON_GetArrayItem(node, i);
            if (file_node == NULL) {
                LOGE << "invalid file node";
                continue;
            }

            cJSON* sub_node = cJSON_GetObjectItem(file_node, "file_type");
            if (sub_node != NULL) {
                file_desc.file_type_ = std::string(sub_node->valuestring);
            }

            sub_node = cJSON_GetObjectItem(file_node, "version");
            if (sub_node != NULL) {
                file_desc.md5_ = std::string(sub_node->valuestring);
            }

            sub_node = cJSON_GetObjectItem(file_node, "file");
            if (sub_node != NULL) {
                // base64 decoding
                file_desc.file_content_ = decoder_.Decode(sub_node->valuestring, \
                    strlen(sub_node->valuestring), out_byte);
            }
        }
        cJSON_Delete(root);
        return true;
    }
    std::string get_file_name_from_version(const std::string& firm_ver) {
        std::string file_name = firm_ver;
        file_name.erase(std::remove(file_name.begin(), file_name.end(), '!'), file_name.end());
        return "/mnt/sdcard/" + file_name + ".tar";
    }

private:
    size_t retry_;
    size_t timeout_ms_;
    WatchdogCallback wd_cb_;
    rtems_id thread_id_;
    std::string xeye_id_;
    std::string curr_firm_ver_;
    std::string url_prefix_;
    std::shared_ptr<HTTPClient> http_client_ptr_;
    std::vector<std::string> urls_;
    UrlHeaderMap url_specific_http_header_;
    ZBase64 decoder_;
};
#endif  // ONLINE_UPGRADE_HPP
