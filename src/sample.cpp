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
#include <vector>
#include <fstream>
#include <cerrno>     // 支持 errno
#include <cstring>    // 支持 strerror(errno)
#include <sys/stat.h>
#include <cstdio> // 添加 std::remove 所需头文件
#include <sstream>
struct fileListEntry{
    std::string name;
    std::string path;
    std::string type;
    std::string comment;
};


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
        fd = ::open("../html/file_management.html", O_RDONLY);
    } else if (std::string(uri) == "/file_management.css") {
        fd = ::open("../html/file_management.css", O_RDONLY);
        content_type = "text/css; charset=UTF-8";
    } else if (std::string(uri) == "/file_management.js") {
        fd = ::open("../html/file_management.js", O_RDONLY);
        content_type = "application/javascript; charset=UTF-8";
    }
    else if(std::string(uri) == "/favicon.ico"){
        fd = ::open("../html/kicat.jpg", O_RDONLY);
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
    std::string body = "{\"files\":[{\"name\":\"a.txt\",\"location\":\"haung-ubuntu\",\"path\":\"../file/a.txt\",\"type\":\"txt\",\"comment\":\"test\"},{\"name\":\"b.txt\",\"location\":\"haung-ubuntu\",\"path\":\"../file/b.txt\",\"type\":\"txt\",\"comment\":\"test\"}]}";
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
    if(filefd < 0){
        std::cerr << "open file error" << std::endl;
        evhttp_send_error(req, HTTP_NOTFOUND,"file not found");
        return;
    }
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

void search_handler(evhttp_request *req, void *arg){
    const char* uri=evhttp_request_get_uri(req);
    std::cout<<"Request for:"<<uri<<std::endl;
    std::string struri(uri);
    int pos=struri.find("keywords=");
    if(pos==std::string::npos)
        return;
    int i = 0;
    while ((i = struri.find("%2C", i)) != std::string::npos)
    {
        struri.replace(i, 3, ",");
        ++i;
    }
    std::stringstream strkeywords(std::string(struri.begin() + pos+9, struri.end()));
    std::vector<std::string> keywords;
    std::string token;
    while (std::getline(strkeywords, token,',')){
        keywords.push_back(token);
        // std::cout<<"keyword:"<<token<<std::endl;
    }
    std::string content_type = "application/json; charset=UTF-8";
    std::string body = "{\"files\":[{\"name\":\"b.txt\",\"location\":\"haung-ubuntu\",\"path\":\"../file/b.txt\",\"type\":\"txt\",\"comment\":\"test\"}]}";
    evbuffer *buf = evbuffer_new();
    evhttp_add_header(evhttp_request_get_output_headers(req),"content-type",content_type.c_str());
    // evbuffer_add(buf,body.c_str(),body.size());
    evbuffer_add_printf(buf, "{\"status\": \"error\", \"message\": \"No such file or directory\"}");
    evhttp_send_reply(req, HTTP_INTERNAL, "Internal Server Error", buf);
    // evhttp_send_reply(req,500,"OK",buf);
    evbuffer_free(buf);
}
void delete_handler(evhttp_request *req, void *arg){
    const char *uri=evhttp_request_get_uri(req);
    // std::cout<<"Request for:"<<uri<<std::endl;
    std::string path(uri);
    auto pos = path.begin();
    while (pos != path.end())
    {
        if (*pos == '=')
            break;
        ++pos;
    }
    path.assign(++pos, path.end());
    int i = 0;
    while ((i = path.find("%2F", i)) != std::string::npos)
    {
        path.replace(i, 3, "/");
        ++i;
    }

    int ret = remove(path.c_str());
    if(ret!=0){
        evbuffer *buf=evbuffer_new();
        evhttp_add_header(evhttp_request_get_output_headers(req), "Content-Type", "application/json; charset=utf-8");
        evbuffer_add_printf(buf, "{\"status\": \"error\", \"message\": \"No such file or directory\"}");
        evhttp_send_reply(req, HTTP_INTERNAL, "Internal Server Error", buf);
        evbuffer_free(buf);
    }
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
    evhttp_set_cb(http, "/favicon.ico", request_handler, nullptr);
    evhttp_set_cb(http, "/api/files/search", search_handler, nullptr);
    evhttp_set_cb(http, "/api/files/delete", delete_handler, nullptr);
    

    std::cout << "Starting server on port 12345..." << std::endl;

    event_base_dispatch(base);
    evhttp_free(http);
    event_base_free(base);
    return 0;
}
