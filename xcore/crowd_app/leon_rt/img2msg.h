#include <string>
int get_img_jpg(int cols, int rows, const char *img);

int get_img_str(void);

int make_http_msg(const std::string& url, char* dst_buf, int dst_buf_len, const char* xeye_id);

void make_heartbeat_msg(const std::string& url, float temp, \
        const std::string& xeye_id, int frames, char* dst_buf, int dst_buf_len);
