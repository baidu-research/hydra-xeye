#ifndef _PARAM_CONF_HPP_
#define _PARAM_CONF_HPP_

#include <string>
#include <iostream>
#include <utility>  // for pair and make_pair
#include <fstream>
#include <memory>
#include <functional>
#include <vector>
#include <cassert>
#include <tuple>
#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include "pubdef.h"
#include "arpa/inet.h"
#include <netinet/in.h>
#ifndef MVLOG_UNIT_NAME
#define MVLOG_UNIT_NAME params
#endif
#include <mvLog.h>
#include <Log.h>

class Params {
    private:
        // the purpose of making the constructor private here is to force
        // the client to use Params class only by the getInstance interface.
        Params() : m_xeye_id("invalid"), m_conf_filename("conf.json") {
            m_mode.second = "normal";

        }
    public:
        ~Params() {
        }
        static std::shared_ptr<Params> getInstance() {
            static std::shared_ptr<Params> crowd_conf_ptr(new Params());
            return crowd_conf_ptr;
        }
        typedef std::function<void(const Params*)> ParamsCallback;

        bool is_domain_name(std::string host) const {
            unsigned long addr = inet_addr(host.c_str());
            return addr == INADDR_NONE;
        }

        char* merge_conf_json(const char* conf_json);

        // initialize Params class with a given json-file.
        bool load_from_file(const std::string& filename);
        // save Params status to a json-file.
        bool save_to_file(const std::string& filename) const;
        const std::string& xeye_id() const {
            return m_xeye_id;
        }
        void set_xeye_id(const std::string& xeye_id) {
            m_xeye_id = xeye_id;
            // xeye_id changed, call the callback function
            apply_update();
        }

        // server address getters
        std::string data_server_addr() const {
            return std::get<0>(m_data_server);
        }
        std::string ntp_server_addr() const {
            return m_ntp_server.first;
        }
        std::string heartbeat_server_addr() const {
            return std::get<0>(m_hb_server);
        }
        std::string playback_server_addr() const {
            return m_playback_server.first;
        }
        std::string data_server_url() const {
            return std::get<2>(m_data_server);
        }
        std::string heartbeat_server_url() const {
            return std::get<2>(m_hb_server);
        }
        std::string local_ipaddr() const {
            return std::get<0>(m_local_address);
        }
        std::string local_netmask() const {
            return std::get<1>(m_local_address);
        }
        std::string local_gateway() const {
            return std::get<2>(m_local_address);
        }
        std::string online_update_host() const {
            return std::get<0>(m_online_update);
        }
        std::string online_update_url_prefix() const {
            return std::get<1>(m_online_update);
        }
        int ccm_index() const {
            return m_sensor_ccm_index;
        }

        void set_ccm_index(int index) {
            if (index >= 0) {
                m_sensor_ccm_index = index;
            }
        }

        // server port getters
        int data_server_port() const {
            return std::get<1>(m_data_server);
        }

        int ntp_server_port() const {
            return m_ntp_server.second;
        }

        int heartbeat_server_port() const {
            return std::get<1>(m_hb_server);
        }

        int playback_server_port() const {
            return m_playback_server.second;
        }

        bool is_online_image_grab_enable() const {
            return std::get<0>(m_online_img_grab);
        }
        std::string online_image_grab_strategy() const {
            return std::get<1>(m_online_img_grab);
        }
        int online_image_flow_control_value() const {
            return std::get<2>(m_online_img_grab);
        }

        // server address setters
        void set_data_server_addr(const std::string& addr) {
            std::get<0>(m_data_server) = addr;
            apply_update();
        }
        void set_ntp_server_addr(const std::string& addr) {
            m_ntp_server.first = addr;
            apply_update();
        }
        void set_heartbeat_server_addr(const std::string& addr) {
            std::get<0>(m_hb_server) = addr;
            apply_update();
        }
        void set_playback_server_addr(const std::string& addr) {
            m_playback_server.first = addr;
            apply_update();
        }

        void set_data_server_port(int port) {
            std::get<1>(m_data_server) = port;
            apply_update();
        }
        void set_data_server_url(const std::string& url) {
            std::get<2>(m_data_server) = url;
            apply_update();
        }
        void set_ntp_server_port(int port) {
            m_ntp_server.second = port;
            // update the conf file on disk
            apply_update();
        }
        void set_heartbeat_server_port(int port) {
            std::get<1>(m_hb_server) = port;
            // update the conf file on disk
            apply_update();
        }
        void set_heartbeat_server_url(const std::string& url) {
            std::get<2>(m_hb_server) = url;
            // update the conf file on disk
            apply_update();
        }
        void set_playback_server_port(int port) {
            m_playback_server.second = port;
            // update the conf file on disk
            apply_update();
        }

        void set_local_ipaddr(const std::string& ip_addr) {
            std::get<0>(m_local_address) = ip_addr;
        }

        void set_local_netmask(const std::string& net_mask) {
            std::get<1>(m_local_address) = net_mask;
        }
        void set_local_gateway(const std::string& gate_way) {
            std::get<2>(m_local_address) = gate_way;
        }

        int models_count() const {
            return static_cast<int>(m_models.size());
        }

        std::string get_model_name_by_index(int index) const {
            std::string model_name = "";
            if (index < models_count()) {
                model_name = m_models[index].first;
            }
            return model_name;
        }

        std::string get_model_path_by_index(int index) const {
            std::string model_path = "";
            if (index < models_count()) {
                model_path = m_models[index].second;
            }
            return model_path;
        }

        std::string get_model_path_by_name(const std::string& model_name) const {
            std::string model_path = "";
            for (auto& item : m_models) {
                if (item.first == model_name) {
                    model_path = item.second;
                    break;
                }
            }
            return model_path;
        }

        void set_params_callback(ParamsCallback callback) {
            m_params_callback = callback;
        }

        RunningMode get_mode_index() const {
            std::string mode = std::get<1>(m_mode);
            if (mode == "record") {
                return RECORD;
            } else if (mode == "normal") {
                return NORMAL;
            } else if (mode == "live") {
                return LIVE;
            } else {
                LOGE << "invalid mode: " << mode;
                return INVALID_MODE;
            }
        }

        std::string get_mode_str() const {
            return std::get<1>(m_mode);
        }

        void set_mode(const std::string& mode) {
            std::get<1>(m_mode) = mode;
        }

        std::string get_mode_comment() const {
            return std::get<0>(m_mode);
        }

        int get_confidence() const {
            return m_confidence;
        }

        void set_confidence(int confidence) {
            m_confidence = confidence;
        }

    private:
        // default inline
        void apply_update() {
            if (m_params_callback) {
                m_params_callback(this);
            }
            // update the conf file on disk
            save_to_file(m_conf_filename);
        }
        bool save_servers_config_to_json(cJSON* root) const;
        bool load_servers_config_from_json(const cJSON* root);
        bool save_models_config_to_json(cJSON* root) const;
        bool load_models_config_from_json(cJSON* root);

    private:
        // the name of the config file
        std::string m_conf_filename;
        // the id of the xeye
        std::string m_xeye_id;
        // running mode, can only be configured to the following:
        // record: only grap frames and send to server
        // normal: deep learning
        // live  : video mode, view the live image by USB cable
        std::pair<std::string, std::string> m_mode;

        // sensor ccm index
        int m_sensor_ccm_index;
        // first -> data server address, second -> port
        // if port is 0, means not USED
        // third -> unl
        std::tuple<std::string, int, std::string> m_data_server;
        // first -> ntp server address, second -> port
        // if port is 0, means NOT USED
        std::pair<std::string, int> m_ntp_server;
        // first -> heartbeat server address, second -> port
        // if port is 0, means NOT USED
        // third -> url
        std::tuple<std::string, int, std::string> m_hb_server;

        // first  -> ip address
        // second -> network mask
        // third  -> gate way
        std::tuple<std::string, std::string, std::string> m_local_address;

        // first -> playback server address, second -> port
        // if address is empty, then the playback feature is disabled
        // so xeye is running in runtime mode, or run in playback mode.
        std::pair<std::string, int> m_playback_server;

        // models configuration, <model_name, model_path>
        std::vector<std::pair<std::string, std::string>> m_models;

        // online image grabbing configuration
        // first  -> true for enable, false for disable
        // second -> strategy: interval, ondemand
        // third  -> flow control setting.
        std::tuple<bool, std::string, int> m_online_img_grab;

        // online update configuration
        // first  -> host domain name or ip address
        // second -> url prefix
        std::pair<std::string, std::string> m_online_update;

        // callback function, the only purpose of this callback function is:
        // pass the xeye_id parameters from LOS ouside to LRT
        ParamsCallback m_params_callback;

        // detection confidence threshold, real confidence = m_confidence / 100
        int m_confidence;

        std::string m_conf_md5;
};
#endif  // _PARAM_CONF_HPP_
