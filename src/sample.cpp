#include <event2/event.h>
#include <event2/http.h>
#include <event2/buffer.h>
#include <event2/util.h>
#include <event2/keyvalq_struct.h>
#include <fcntl.h>    // open() 文件操作
#include <unistd.h>   // read()/close() 文件操作
#include <sys/stat.h> // 获取文件状态
#include <iostream>
#include <string>
#include <fstream>
#include <cerrno>     // 支持 errno
#include <cstring>    // 支持 strerror(errno)
#include <sys/stat.h>
struct fileListEntry{
    std::string name;
    std::string path;
    std::string type;
    std::string comment;
};
// std::string ReadFile(const std::string &path)
// {
//     std::ifstream is(path.c_str(), std::ifstream::in);

//     // 寻找文件末端
//     is.seekg(0, is.end);

//     // 获取长度
//     int flength = is.tellg();

//     // 重新定位
//     is.seekg(0, is.beg);
//     char *buffer = new char[flength];

//     // 读取文件
//     is.read(buffer, flength);
//     std::string msg(buffer, flength);
//     return msg;
// }

void request_handler(evhttp_request *req, void *arg)
{
    const char *uri = evhttp_request_get_uri(req);
    std::cout << "Request for " << uri << std::endl;
    
    std::string body;
    const char *content_type = "text/html; charset=UTF-8"; // 默认为 HTML
    struct stat st;
    int fd;
    evbuffer *buf = evbuffer_new();
    if (std::string(uri) == "/") {
        fd = ::open("/home/huang/prog/libeventWeb/html/file_management.html", O_RDONLY);
        // body = ReadFile("/home/huang/prog/libeventWeb/html/file_management.html");
    } else if (std::string(uri) == "/file_management.css") {
        fd = ::open("/home/huang/prog/libeventWeb/html/file_management.css", O_RDONLY);
        // body = ReadFile("/home/huang/prog/libeventWeb/html/file_management.css");
        content_type = "text/css; charset=UTF-8";
    } else if (std::string(uri) == "/file_management.js") {
        fd = ::open("/home/huang/prog/libeventWeb/html/file_management.js", O_RDONLY);
        // body = ReadFile("/home/huang/prog/libeventWeb/html/file_management.js");
        content_type = "application/javascript; charset=UTF-8";
    }
    else if(std::string(uri) == "/favicon.ico"){
        fd = ::open("/home/huang/prog/libeventWeb/html/kicat.jpg", O_RDONLY);
        // body = ReadFile("/home/huang/prog/libeventWeb/html/琪猫猫.jpg");
        content_type = "image/jpeg";
    }
    std::cout<<fd<<std::endl;
    if(fd==-1){
        evhttp_send_reply(req, 404, "Not Found", NULL);
    }
    else {
        fstat(fd, &st);
        evbuffer_add_file(buf, fd, 0, st.st_size);
        ::close(fd);
        // evbuffer_add(buf, body.c_str(), body.size());
        
        // 创建 HTTP 响应并设置 Content-Type
        struct evkeyvalq *headers = evhttp_request_get_output_headers(req);
        evhttp_add_header(headers, "Content-Type", content_type);
        
        evhttp_send_reply(req, 200, "OK", buf);
    }
    evbuffer_free(buf);
}

void filelsit_handler(evhttp_request *req, void *arg)
{
    const char *uri = evhttp_request_get_uri(req);
    std::cout << "Request for " << uri << std::endl;
    std::string content_type = "application/json; charset=UTF-8";
    std:: string body = "{\"files\":[{\"name\":\"a.txt\",\"path\":\"/home/huang/prog/libeventWeb/file/a.txt\",\"type\":\"txt\",\"comment\":\"test\"}]}";
    evbuffer *buf = evbuffer_new();
    evbuffer_add(buf, body.c_str(), body.size());
    struct evkeyvalq *headers = evhttp_request_get_output_headers(req);
    evhttp_add_header(headers, "Content-Type", content_type.c_str());
    evhttp_send_reply(req, 200, "OK", buf);
    evbuffer_free(buf);
}

void download_handler(evhttp_request *req, void *arg)
{
    const char *uri=evhttp_request_get_uri(req);
    std::string path(uri);
    auto pos = path.begin();
    while (pos!=path.end()){
        if(*pos=='=')
        break;
        ++pos;
    }
    path.assign(++pos, path.end());
    int i = 0;
    while ((i= path.find("%2F", i)) !=std::string::npos)
    {
        path.replace(i, 3, "/");
        ++i;
    }
    std::string file_name(path.begin() + path.rfind("/", path.size() - 1)+1, path.end());
    std::cout << path << std::endl;
    int filefd = ::open(path.c_str(), O_RDONLY);
    struct stat file_stat;
    fstat(filefd, &file_stat);
    evbuffer *buf = evbuffer_new();
    evbuffer_add_file(buf, filefd, 0, file_stat.st_size);
    struct evkeyvalq* headers=evhttp_request_get_output_headers(req);
    evhttp_add_header(headers, "Content-Type", "application/octet-stream");
    file_name = "attachment; filename=" + file_name;
    evhttp_add_header(headers, "Content-Disposition", file_name.c_str());
    evhttp_send_reply(req, 200, "OK", buf);
    evbuffer_free(buf);
    ::close(filefd);
}
int main()
{
    fileListEntry fle = {"file1", "/home/file1", "file", "file1 comment"};
    struct event_base *base;
    struct evhttp *http;
    base = event_base_new();
    if (base == nullptr) {
        std::cerr << "Failed to create event_base: " << strerror(errno) << std::endl;
        return 1;
    }

    http = evhttp_new(base);
    if (http == nullptr) {
        std::cerr << "Failed to create HTTP server: " << strerror(errno) << std::endl;
        event_base_free(base);
        return 1;
    }

    if (evhttp_bind_socket(http, "0.0.0.0", 12345) != 0) {
        std::cerr << "Failed to bind socket: " << strerror(errno) << std::endl;
        evhttp_free(http);
        event_base_free(base);
        return 1;
    }

    evhttp_set_cb(http, "/", request_handler, nullptr);
    evhttp_set_cb(http, "/file_management.css", request_handler, nullptr);
    evhttp_set_cb(http, "/file_management.js", request_handler, nullptr);
    evhttp_set_cb(http, "/api/files", filelsit_handler, nullptr);
    evhttp_set_cb(http, "/api/files/download", download_handler, nullptr);

    std::cout << "Starting server on port 12345..." << std::endl;

    event_base_dispatch(base);
    evhttp_free(http);
    event_base_free(base);
    return 0;
}
