#ifndef HTTP_HPP
#define HTTP_HPP
#include <string>
#include <memory>
#include <mvLog.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <mvTensorTimer.h>
#include <Log.h>
#include <cJSON.h>
#include "pubdef.h"
#include "ringbuf.hpp"
#include "facemsg.h"
#include <vector>
#include <string>
#include <memory>
#include <zbase64.h>
#include <netdb.h>
#include <map>
#include <utils/utils.hpp>
#include <netinet/tcp.h>  // for TCP_NODELAY
enum HTTP_STATUS {
    HTTP_STATUS_200_OK = 0,
    HTTP_STATUS_404_NOT_FOUND,
    HTTP_STATUS_400_BAD_REQUEST,
    HTTP_STATUS_408_REQUEST_TIMEOUT,
    HTTP_STATUS_502_BAD_GATEWAY,
    HTTP_STATUS_403_FORBIDDEN,
    HTTP_STATUS_UNSUPPORTED_CODE
};

static const std::vector<std::string> http_status_code_str = {
    "200 OK",
    "404 Not Found",
    "400 Bad Request",
    "408 Request Timeout",
    "502 Bad Gateway",
    "403 Forbidden"
};

class HTTPClient {
public:
    // target has two different type of values
    // one for target ip address, this type is used by local server
    // another is target domain name, the ip address of target is
    // parsed by domain name.
    HTTPClient(std::string target, int port = 80) : m_socket(-1) {
        // identify the target
        unsigned long addr = inet_addr(target.c_str());
        m_port = port;
        if (addr == INADDR_NONE) {
            // invalid ip address, probably is domain name
            m_domain_name = std::move(target);
            is_given_by_domain_name = true;
        } else {
            // target is given by ip address
            m_domain_name = target + ":" + std::to_string(m_port);
            m_server_addr.push_back(target);
            is_given_by_domain_name = false;
        }
    }


    bool resolve_domain_name_to_address() {
        if (!is_given_by_domain_name) {
            return true;
        }
        struct in_addr **addr_list;
        std::string domain_name = m_domain_name;
        struct hostent* host = gethostbyname(domain_name.c_str());
        if (host == NULL) {
            return false;
        }
        addr_list = (struct in_addr**)host->h_addr_list;
        for (; *addr_list != NULL; addr_list++) {
            std::string current_ip = std::string(inet_ntoa(**addr_list));
            m_server_addr.push_back(current_ip);
        }
        LOGI << "Resolved " << m_server_addr.size() << " ip addresses: ";
        for (const std::string& ip : m_server_addr) {
            LOGI << ip;
        }
        return m_server_addr.size() > 0;
    }

    std::string get_domain_name() const {
        return m_domain_name;
    }


    bool init() {
        // resolve ip address by domain name.
        if (!resolve_domain_name_to_address()) {
            LOGE << "Resovle domain name: " << m_domain_name << " to ip address failed";
            return false;
        }
        return true;
    }
    ~HTTPClient() {
        close_socket();
    }
    bool connect_to_server(int timeout_ms) {
        if (m_server_addr.size() == 0) {
            LOGI << "No ip address provided, should not hanpened here.";
            return false;
        }
        int flags = 0;
        bool connected = false;
        for (auto server_addr : m_server_addr) {
            if (server_addr.empty()) {
                continue;
            }
            // TODO(yanghongtian): Figure out the difference of PF_INET and AF_INET
            m_socket = socket(AF_INET, SOCK_STREAM, 0);
            if (m_socket < 0) {
                LOGE << "Create socket FAILED, fd: " << m_socket;
                break;
            }

            enable_reuse_addr(m_socket, true);
            enable_reuse_port(m_socket, true);

            // config socket to non-blocking mode
            flags = fcntl(m_socket, F_GETFL, 0);
            if (flags < 0) {
                close_socket();
                break;
            }

            flags |= O_NONBLOCK;
            if (fcntl(m_socket, F_SETFL, flags) < 0) {
                close_socket();
                break;
            }
            LOGD << "Start to connect to ip address: " << server_addr;
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = inet_addr(server_addr.c_str());
            addr.sin_port = htons(m_port);
            int retry_internal = 2;
            if (connect(m_socket, (struct sockaddr*)&addr, sizeof(addr)) != 0) {
                if (errno == EINPROGRESS) {
                    fd_set fdw;
                    struct timeval tv;
                    tv.tv_sec = timeout_ms / 1000;
                    tv.tv_usec = (timeout_ms % 1000) * 1000;
                    FD_ZERO(&fdw);
                    FD_SET(m_socket, &fdw);
                    int res = 0;
                    do {
                        mv::tensor::Timer t;
                        res = select(m_socket + 1, NULL, &fdw, NULL, &tv);
                        if (res <= 0) {
                            LOGW << "Connection to server timeout or error occurs, error string: "
                                << strerror(errno) << ", time takes: " << t.elapsed()
                                << ", socket: " << m_socket;
                            // timeout or error occurs
                            if (errno == EINTR || errno == EINPROGRESS) {
                                if (retry_internal-- == 0) {
                                    break;
                                }
                                continue;
                            } else {
                                break;
                            }
                        } else {
                            // Socket selected for write
                            socklen_t lon = sizeof(int);
                            int valopt = 0;
                            if (getsockopt(m_socket, SOL_SOCKET, SO_ERROR, (void*)(&valopt), &lon) < 0) {
                                LOGE << "Error in getsockopt() " << strerror(errno);
                                break;
                            }
                            // Check the value returned...
                            if (valopt || !FD_ISSET(m_socket, &fdw)) {
                                LOGE << "Error in delayed connection() " << strerror(valopt);
                            }
                            // successfully connect to server.
                            connected = true;
                            break;
                        }
                    } while(1);
                } else if (errno == EISCONN) {
                    LOGW << "Socket is connected";
                    connected = true;
                } else {
                    LOGE << "Error connecting: " << strerror(errno) << ", fd: " << m_socket;
                    connected = false;
                }
            } else {
                connected = true;
            }
            // if successfully connect to server, break out the for loop, or continue to retry other ip.
            if (connected) {
                LOGD << "Successfully connect to ip address: " << server_addr;
                break;
            }
            close_socket();
            LOGW << "Can not connect to ip address: " << server_addr;
        }
        if (!connected) {
            return false;
        }
        flags &= ~O_NONBLOCK;
        if (fcntl(m_socket, F_SETFL, flags) < 0) {
			LOGE << "connection to server successfully, but config socket to blocking mode failed";
            close_socket();
            return false;
        }
		return true;
    }

    bool post_with_response(const std::string& header, const char* json_body, \
            size_t body_len, char* buf, size_t buf_len, size_t recv_timeout_sec = 5 * 60) {
        int flag = 1;
        setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
        if (!post_internal(header, json_body, body_len)) {
            LOGE << "post http request failed in " << __func__;
            close_socket();
            return false;
        }
        flag = 0;
        setsockopt(m_socket, IPPROTO_TCP, TCP_NODELAY, (char *) &flag, sizeof(int));
        LOGI << "Send request successfully...waiting for response...";

        int read_index = 0;
        // set up timeout for receival
        struct timeval tv;
        tv.tv_sec = recv_timeout_sec;
        tv.tv_usec = 0;
        setsockopt(m_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
        memset(buf, 0, buf_len);
        do {
            int ret = recv(m_socket, buf + read_index, 4 * 1024, 0);
            if (ret <= 0) {
                break;
            }
            read_index += ret;
            LOGD << "read index = " << read_index << ", ret = " << ret;
        } while (1);

        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // according to POSIX 1.1, this indicates a timeout.
            LOGE << "because of unreliable network condition, timeout hanppend";
            close_socket();
            return false;
        }
        LOGI << "Reveive " << read_index << " bytes from server, start to parse.";
        // making a hack here to remove "HTTP 1.1/ 408 Bad Request"
        size_t pos = std::string(buf + read_index - 32).find("HTTP/1.1 400 Bad Request");
        if (pos != std::string::npos) {
            mvLog(MVLOG_DEBUG, "Receive a HTTP/1.1 400 Bad Request at the end of response, manually remove it");
            *(buf + read_index - 32 + pos) = '\0';
        }
        close_socket();
        mvLog(MVLOG_DEBUG, "%s", buf);

        HTTP_STATUS status = parse_status_from_response(std::string(buf));
        if (status != HTTP_STATUS_200_OK) {
            if (status != HTTP_STATUS_UNSUPPORTED_CODE) {
                LOGE << "HTTP error code: " << http_status_code_str[status];
            }
            return false;
        }
        return true;
    }
public:
    HTTP_STATUS parse_status_from_response(const std::string& response) {
        HTTP_STATUS status = HTTP_STATUS_UNSUPPORTED_CODE;
        for (size_t i = 0; i < http_status_code_str.size(); ++i) {
            if (std::string::npos != response.find(http_status_code_str[i])) {
                status = HTTP_STATUS(i);
                break;
            }
        }
        return status;
    }

    bool post(const std::string& http_header, const char* json_body, size_t body_len) {
        bool status = post_internal(http_header, json_body, body_len);
        close_socket();
        return status;
    }
private:
    bool post_internal(const std::string& http_header, const char* json_body, size_t body_len) {
        std::string mid_content;
        mid_content.reserve(body_len + 64);
        mid_content = "Content-Length: " + std::to_string(body_len) + "\r\n\r\n";
        mid_content += json_body;
        mid_content += "\r\n\r\n";
        size_t total_size = http_header.length() + mid_content.length() + 1;
        m_buf_for_post.resize(total_size);
        memcpy(m_buf_for_post.data(), http_header.c_str(), http_header.length());
        memcpy(m_buf_for_post.data() + http_header.length(), \
                mid_content.c_str(), mid_content.length());
        m_buf_for_post[total_size - 1] = '\0';
        mvLog(MVLOG_DEBUG, "\n%s", m_buf_for_post.data());
        int need_to_send = m_buf_for_post.size();
        int bytes_sent = send(m_socket, m_buf_for_post.data(), need_to_send, 0);
        return (bytes_sent == need_to_send);
    }
    void close_socket() {
        if (m_socket >= 0) {
            close(m_socket);
            m_socket = -1;
        }
    }

    bool enable_sock_opt(int sock_fd, int op, bool enable) {
        int yes = (enable ? 1 : 0);
        int err = setsockopt(sock_fd, SOL_SOCKET, op, &yes, sizeof(yes));
        return !err;
    }
    bool enable_reuse_addr(int sock_fd, bool enable) {
        return enable_sock_opt(sock_fd, SO_REUSEADDR, enable);
    }
    bool enable_reuse_port(int sock_fd, bool enable) {
        return enable_sock_opt(sock_fd, SO_REUSEPORT, enable);
    }
private:
    int m_socket;
    // if domain name is given, server address is parsed by domain name
    std::vector<std::string> m_server_addr;
    std::string m_domain_name;
    int m_port;
    bool is_given_by_domain_name;
    std::vector<char> m_buf_for_post;
};
#endif // HTTP_HPP
