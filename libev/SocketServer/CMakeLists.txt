# CMake 最低版本号
cmake_minimum_required(VERSION 2.8)

#项目信息
project(ServerApp)

#查找当前目录下所有源文件
#并将名称保存到 DIR_SRCS

aux_Source_directory(./src DIR_SRCS)

#指定生成目标
add_executable(ServerApp ./src/LibevSockt_Server.c)
add_executable(TestClient ./src/client.c)

#添加链接库
target_link_libraries(ServerApp ev)