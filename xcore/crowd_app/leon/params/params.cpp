#include "params.hpp"
#include <Log.h>
#ifndef MVLOG_UNIT_NAME
#define MVLOG_UNIT_NAME params
#endif
#include <mvLog.h>
#include "utils/utils.hpp"

char* Params::merge_conf_json(const char* conf_json) {
    cJSON* root = cJSON_Parse(conf_json);
    if (root == NULL) {
        LOGE << "invalid json string";
        return NULL;
    }

    // merge xeye_id
    cJSON_ReplaceItemInObject(root, "xeye_id", cJSON_CreateString(m_xeye_id.c_str()));

    // merge confidence
    cJSON_ReplaceItemInObject(root, "confidence", cJSON_CreateNumber(m_confidence));

    // merge sensor ccm index
    cJSON_ReplaceItemInObject(root, "sensor_ccm_index", cJSON_CreateNumber(m_sensor_ccm_index));

    // merge data server url
    cJSON* data_server = cJSON_GetObjectItem(root, "data_server");
    if (data_server != NULL) {
        cJSON_ReplaceItemInObject(data_server, "url", \
                cJSON_CreateString(data_server_url().c_str()));
    } else {
        LOGE << "Can not merge data server url";
    }

    // merge heartbeat server url
    cJSON* heartbeat_server = cJSON_GetObjectItem(root, "heartbeat_server");
    if (heartbeat_server != NULL) {
        cJSON_ReplaceItemInObject(heartbeat_server, "url", \
                cJSON_CreateString(heartbeat_server_url().c_str()));
    } else {
        LOGE << "Can not merge heartbeat server url";
    }

    // merge local network info
    cJSON* local_network_info = cJSON_GetObjectItem(root, "local_network_info");
    if (local_network_info != NULL) {
        cJSON_ReplaceItemInObject(local_network_info, "ip_address", \
                cJSON_CreateString(local_ipaddr().c_str()));
        cJSON_ReplaceItemInObject(local_network_info, "net_mask", \
                cJSON_CreateString(local_netmask().c_str()));
        cJSON_ReplaceItemInObject(local_network_info, "gate_way", \
                cJSON_CreateString(local_gateway().c_str()));
    } else {
        LOGE << "Can not merge local network info";
    }
    char* new_json = cJSON_Print(root);
    cJSON_Delete(root);
    return new_json;
}



bool Params::load_from_file(const std::string& filename) {
    // [NOTE]: before calling load_from_file function,
    // this callback function should be initialized first
    // or the xeye_id could not pass to LRT properly.
    assert(m_params_callback != nullptr);
    // try to figure out why the c++ way do not work.
    FILE *f = NULL;
    long len = 0;

    /* open in read binary mode */
    f = fopen(filename.c_str(), "rb");
    if (NULL == f) {
        LOGE << "Can not open file: " << filename;
        return false;
    }
    /* get the length */
    fseek(f, 0, SEEK_END);
    len = ftell(f);
    fseek(f, 0, SEEK_SET);
    std::shared_ptr<char> data(new char[len + 1]);

    fread(data.get(), 1, len, f);
    data.get()[len] = '\0';
    if (0 != fclose(f)) {
        // non fatal error, only apply logging
        LOGE << "close json file: " << filename << " FAILED";
    }

    cJSON* root = cJSON_Parse(data.get());

    if (nullptr == root) {
        LOGE << "Invalid cjson format maybe";
        return false;
    }

    bool succeed = true;
    cJSON* psub_node = NULL;

    psub_node = cJSON_GetObjectItem(root, "xeye_id");
    if (psub_node != NULL) {
        m_xeye_id = std::string(psub_node->valuestring);
    } else {
        succeed = false;
        LOGE << "no xeye_id nodes in config file";
    }
    mvLog(MVLOG_INFO, "xeye_id: %s", m_xeye_id.c_str());

    psub_node = cJSON_GetObjectItem(root, "confidence");
    if (psub_node != NULL) {
        m_confidence = psub_node->valueint;
    } else {
        succeed = false;
        LOGE << "no confidence sub node in config file";
    }

    mvLog(MVLOG_INFO, "Confidence: %d", m_confidence);

    psub_node = cJSON_GetObjectItem(root, "sensor_ccm_index");
    if (psub_node != NULL) {
        m_sensor_ccm_index = psub_node->valueint;
    } else {
        succeed = false;
        LOGE << "No sensor_ccm_index nodes in config file";
    }

    cJSON* running_mode = cJSON_GetObjectItem(root, "running_mode");
    if (running_mode != NULL) {
        psub_node = cJSON_GetObjectItem(running_mode, "mode");
        if (psub_node != NULL) {
            set_mode(std::string(psub_node->valuestring));
        } else {
            succeed = false;
            LOGE << "No mode nodes in running_mode";
        }
        mvLog(MVLOG_INFO, "Running mode: %s", get_mode_str().c_str());
    } else {
        succeed = false;
        LOGE << "No running_mode nodes in config file";
    }

    cJSON* online_grab = cJSON_GetObjectItem(root, "online_grab");

    if (online_grab != NULL) {
        psub_node = cJSON_GetObjectItem(online_grab, "enable");
        if (psub_node != NULL) {
            std::get<0>(m_online_img_grab) = (psub_node->valueint != 0);
        } else {
            succeed = false;
            LOGE << "No enable node in online_grab";
        }
        psub_node = cJSON_GetObjectItem(online_grab, "strategy");
        if (psub_node != NULL) {
            std::get<1>(m_online_img_grab) = std::string(psub_node->valuestring);
            if (online_image_grab_strategy() != "interval" && \
                online_image_grab_strategy() != "ondemand") {
                std::get<0>(m_online_img_grab) = false;
                LOGE << "Invalid image grabbing strategy, this feature is forced to be disabled.";
            } else {
                psub_node = cJSON_GetObjectItem(online_grab, "value");
                if (psub_node != NULL && psub_node->valueint > 0) {
                    std::get<2>(m_online_img_grab) = psub_node->valueint;
                } else {
                    // if value is not valie, still forced this feature to be disabled;
                    std::get<0>(m_online_img_grab) = false;
                    LOGE << "Invalid image grab value setting. value should be > 0";
                }
            }
        } else {
            succeed = false;
            LOGE << "No strategy node in online_grab";
        }

        mvLog(MVLOG_INFO, "online grab enable  : %d", is_online_image_grab_enable());
        mvLog(MVLOG_INFO, "online grab strategy: %s", online_image_grab_strategy().c_str());
        mvLog(MVLOG_INFO, "online grab value   : %d", online_image_flow_control_value());
    } else {
        succeed = false;
        mvLog(MVLOG_ERROR, "No online_grab node in config file");
    }
    cJSON* online_update = cJSON_GetObjectItem(root, "online_update");
    if (online_update) {
        psub_node = cJSON_GetObjectItem(online_update, "host");
        if (psub_node != NULL) {
            std::get<0>(m_online_update) = std::string(psub_node->valuestring);
        }
        psub_node = cJSON_GetObjectItem(online_update, "url_prefix");
        if (psub_node != NULL) {
            std::get<1>(m_online_update) = std::string(psub_node->valuestring);
        }
    }
    if (!load_servers_config_from_json(root)) {
        succeed = false;
    }

    if (!load_models_config_from_json(root)) {
        succeed = false;
    }

    cJSON_Delete(root);

    // save the path of the conf file
    m_conf_filename = filename;

    // call the callback function
    if (m_params_callback) {
        m_params_callback(this);
    }
    return succeed;
}

bool Params::save_to_file(const std::string& filename) const {
    cJSON* root = NULL;
    root = cJSON_CreateObject();
    cJSON_AddItemToObject(root, "xeye_id", \
            cJSON_CreateString(m_xeye_id.c_str()));
    cJSON_AddItemToObject(root, "confidence", \
            cJSON_CreateNumber(get_confidence()));
    cJSON_AddItemToObject(root, "sensor_ccm_index", \
                          cJSON_CreateNumber(m_sensor_ccm_index));
    cJSON* mode = cJSON_CreateObject();
    cJSON_AddItemToObject(mode, "comment", \
            cJSON_CreateString("running mode, can only be configured as record, normal, live mode"));
    cJSON_AddItemToObject(mode, "mode", cJSON_CreateString(get_mode_str().c_str()));
    cJSON_AddItemToObject(root, "running_mode", mode);

    // online grab image configuration
    cJSON* online_grab = cJSON_CreateObject();
    cJSON_AddItemToObject(online_grab, "enable", cJSON_CreateBool(is_online_image_grab_enable()));
    cJSON_AddItemToObject(online_grab, "strategy", cJSON_CreateString(online_image_grab_strategy().c_str()));
    cJSON_AddItemToObject(online_grab, "value", cJSON_CreateNumber(online_image_flow_control_value()));
    cJSON_AddItemToObject(root, "online_grab", online_grab);
    // online update configuration
    cJSON* online_update = cJSON_CreateObject();
    cJSON_AddItemToObject(online_update, "host", cJSON_CreateString(online_update_host().c_str()));
    cJSON_AddItemToObject(online_update, "url_prefix", cJSON_CreateString(online_update_url_prefix().c_str()));
    cJSON_AddItemToObject(root, "online_update", online_update);

    save_servers_config_to_json(root);
    // models configuration
    save_models_config_to_json(root);
    // must free out_str here, or will cause a memory leak
    char* out_str = cJSON_Print(root);
    std::ofstream of(filename, std::ofstream::binary);
    of.write(out_str, strlen(out_str));
    of.close();
    cJSON_Delete(root);
    free(out_str);
    return true;
}

bool Params::save_servers_config_to_json(cJSON* root) const {
    cJSON* data_server = NULL;
    cJSON* ntp_server = NULL;
    cJSON* heartbeat_server = NULL;
    cJSON* playback_server = NULL;
    cJSON* local_network_info = NULL;
    // data server
    cJSON_AddItemToObject(root, "data_server", \
            data_server = cJSON_CreateObject());
    cJSON_AddItemToObject(data_server, "comment", cJSON_CreateString( \
                "the IP address and port of server which xeye send data to"));
    cJSON_AddItemToObject(data_server, "addr", \
            cJSON_CreateString(data_server_addr().c_str()));
    cJSON_AddItemToObject(data_server, "port", \
            cJSON_CreateNumber(data_server_port()));
    cJSON_AddItemToObject(data_server, "url", \
            cJSON_CreateString(data_server_url().c_str()));

    // ntp server
    cJSON_AddItemToObject(root, "ntp_server", \
            ntp_server = cJSON_CreateObject());
    cJSON_AddItemToObject(ntp_server, "comment", cJSON_CreateString( \
                "the IP address and port of NTP time syncronization server"));
    cJSON_AddItemToObject(ntp_server, "addr", \
            cJSON_CreateString(ntp_server_addr().c_str()));
    cJSON_AddItemToObject(ntp_server, "port", \
            cJSON_CreateNumber(ntp_server_port()));

    // heartbeat server
    cJSON_AddItemToObject(root, "heartbeat_server", \
            heartbeat_server = cJSON_CreateObject());
    cJSON_AddItemToObject(heartbeat_server, "comment", cJSON_CreateString( \
                "the IP address and port of heartbeat server"));
    cJSON_AddItemToObject(heartbeat_server, "addr", \
            cJSON_CreateString(heartbeat_server_addr().c_str()));
    cJSON_AddItemToObject(heartbeat_server, "port", \
            cJSON_CreateNumber(heartbeat_server_port()));
    cJSON_AddItemToObject(heartbeat_server, "url", \
            cJSON_CreateString(heartbeat_server_url().c_str()));

    // playback server
    cJSON_AddItemToObject(root, "playback_server", \
            playback_server = cJSON_CreateObject());
    cJSON_AddItemToObject(playback_server, "comment", cJSON_CreateString( \
                "the IP address and port of playback server, \
                if addr section is empty, then playback mode is disabled"));
    cJSON_AddItemToObject(playback_server, "addr", \
            cJSON_CreateString(playback_server_addr().c_str()));
    cJSON_AddItemToObject(playback_server, "port", \
            cJSON_CreateNumber(playback_server_port()));
    // local network info
    cJSON_AddItemToObject(root, "local_network_info", \
            local_network_info = cJSON_CreateObject());
    cJSON_AddItemToObject(local_network_info, "comment", cJSON_CreateString( \
                "static ip address, netmask and gateway configuration"));
    cJSON_AddItemToObject(local_network_info, "ip_address", \
            cJSON_CreateString(local_ipaddr().c_str()));
    cJSON_AddItemToObject(local_network_info, "net_mask", \
            cJSON_CreateString(local_netmask().c_str()));
    cJSON_AddItemToObject(local_network_info, "gate_way", \
            cJSON_CreateString(local_gateway().c_str()));
    return true;
}

bool Params::load_servers_config_from_json(const cJSON* root) {
    bool succeed = true;
    cJSON* data_server = cJSON_GetObjectItem(root, "data_server");
    if (nullptr == data_server) {
        LOGE << "NO object named data_server in json file";
        succeed = false;
    } else {
        m_data_server = std::make_tuple( \
            std::string(cJSON_GetObjectItem(data_server, "addr")->valuestring), \
            cJSON_GetObjectItem(data_server, "port")->valueint, \
            cJSON_GetObjectItem(data_server, "url")->valuestring);
    }
    cJSON* ntp_server = cJSON_GetObjectItem(root, "ntp_server");
    if (nullptr == ntp_server) {
        LOGE << "NO object named ntp_server in json file";
        succeed = false;
    } else {
        m_ntp_server = std::make_pair( \
            std::string(cJSON_GetObjectItem(ntp_server, "addr")->valuestring), \
            cJSON_GetObjectItem(ntp_server, "port")->valueint);
    }

    cJSON* hb_server = cJSON_GetObjectItem(root, "heartbeat_server");
    if (nullptr == hb_server) {
        LOGE << "NO object named heartbeat_server in json file";
        succeed = false;
    } else {
        m_hb_server = std::make_tuple( \
            std::string(cJSON_GetObjectItem(hb_server, "addr")->valuestring), \
            cJSON_GetObjectItem(hb_server, "port")->valueint, \
            std::string(cJSON_GetObjectItem(hb_server, "url")->valuestring));
    }
    cJSON* local_network_info = cJSON_GetObjectItem(root, "local_network_info");
    if (nullptr == local_network_info) {
        LOGE << "NO object named local_network_info in json file";
        succeed = false;
    } else {
        m_local_address = std::make_tuple( \
            std::string(cJSON_GetObjectItem(local_network_info, "ip_address")->valuestring), \
            std::string(cJSON_GetObjectItem(local_network_info, "net_mask")->valuestring), \
            std::string(cJSON_GetObjectItem(local_network_info, "gate_way")->valuestring));
    }
    cJSON* playback_server = cJSON_GetObjectItem(root, "playback_server");
    if (nullptr == playback_server) {
        LOGE << "NO object named playback_server in json file";
        succeed = false;
    } else {
        m_playback_server = std::make_pair( \
            std::string(cJSON_GetObjectItem(playback_server, "addr")->valuestring), \
            cJSON_GetObjectItem(playback_server, "port")->valueint);
    }
    mvLog(MVLOG_INFO, "data_server:(%s, %d, %s)", data_server_addr().c_str(), \
            data_server_port(), data_server_url().c_str());
    mvLog(MVLOG_INFO, "playback_server:(%s, %d)", playback_server_addr().c_str(), \
            playback_server_port());
    mvLog(MVLOG_INFO, "ntp_server:(%s, %d)", ntp_server_addr().c_str(), \
            ntp_server_port());
    mvLog(MVLOG_INFO, "heartbeat_server:(%s, %d, %s)", heartbeat_server_addr().c_str(), \
            heartbeat_server_port(), heartbeat_server_url().c_str());
    if (std::get<0>(m_local_address) == "") {
        mvLog(MVLOG_INFO, "Use DHCP server");
    } else {
        mvLog(MVLOG_INFO, "Static ip address: %s, netmask: %s, gateway: %s", \
                std::get<0>(m_local_address).c_str(), \
                std::get<1>(m_local_address).c_str(), \
                std::get<2>(m_local_address).c_str());
    }
    return succeed;
}

bool Params::save_models_config_to_json(cJSON* root) const {
    assert(root != NULL);
    cJSON* models = NULL;
    // support unlimited models
    cJSON_AddItemToObject(root, "models", models = cJSON_CreateArray());

    for (size_t i = 0; i < m_models.size(); ++i) {
        cJSON* item = cJSON_CreateObject();
        cJSON_AddItemToObject(item, "comment", \
                cJSON_CreateString("model_name: the name of the model, \
            model_path: the abs path of the model file"));
        cJSON_AddItemToObject(item, "model_name", \
                cJSON_CreateString(m_models[i].first.c_str()));
        cJSON_AddItemToObject(item, "model_path", \
                cJSON_CreateString(m_models[i].second.c_str()));
        cJSON_AddItemToArray(models, item);
    }
    return true;
}

bool Params::load_models_config_from_json(cJSON* root) {
    assert(root != NULL);
    cJSON* models = cJSON_GetObjectItem(root, "models");
    if (models == nullptr) {
        return false;
    }
    int models_number = cJSON_GetArraySize(models);
    if (models_number <= 0) {
        mvLog(MVLOG_ERROR, "No models defined in conf.json");
        return false;
    }
    for (int i = 0; i < models_number; ++i) {
        cJSON* model = cJSON_GetArrayItem(models, i);
        if (model == nullptr) {
            LOGE << "No model nodes in models";
            return false;
        }
        m_models.push_back(std::make_pair( \
                    cJSON_GetObjectItem(model, "model_name")->valuestring,
                    cJSON_GetObjectItem(model, "model_path")->valuestring));
        mvLog(MVLOG_INFO, "model_name: %s, model_path: %s", \
                m_models[i].first.c_str(), m_models[i].second.c_str());
    }
    return (m_models.size() > 0);
}
