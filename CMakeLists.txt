# 设置 CMake 最低版本要求
cmake_minimum_required(VERSION 3.10)

# 项目名称和语言
project(libeventWeb)

# 查找 LibEvent（自动查找系统中安装的 libevent）
# include(FindPkgConfig)
# pkg_check_modules(LIBEVENT REQUIRED libevent)
find_library(EVENT_CORE_LIB NAMES event_core PATHS /usr/lib /usr/local/lib)
find_library(EVENT_EXTRA_LIB NAMES event_extra PATHS /usr/lib /usr/local/lib)

# 包含头文件目录（通常不需要手动添加，find_package 已处理）
include_directories(${Libevent_INCLUDE_DIRS}/include)

# 添加可执行文件
add_executable(sample src/sample.cpp)

# 链接 libevent 库
target_link_libraries(sample ${Libevent_LIBRARIES} ${EVENT_CORE_LIB} ${EVENT_EXTRA_LIB})
# target_link_libraries(sample ${Libevent_LIBRARIES})