# CMake 最低版本号要求
cmake_minimum_required(VERSION 3.12)

# 项目信息
project(seaice)

#指定编译类型 DEBUG/RELEASE
SET(CMAKE_BUILD_TYPE "Debug")
SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")
set(CMAKE_CXX_FLAGS "-std=c++11 -pthread -rdynamic -lrt -Wl,--no-as-needed -ldl")

# Create a sources variable with a link to all cpp files to compile
#set(HELLO hello/helloworld.cpp)
#add_executable(hello1 ${HELLO})

#set(TEST_SRC test/test.cpp)
#add_executable(test ${TEST_SRC})

#add openssl header file
set(INCLUDE_DIR /usr/local/zlib-1.2.11/include
                /usr/local/ssl/include)
include_directories(${INCLUDE_DIR})

set(LINK_DIR /usr/local/ssl/lib
             /usr/local/lib/
            /usr/local/zlib-1.2.11/lib)
link_directories(${LINK_DIR})

set(LINK_LIB libyaml-cpp.so
             libssl.so
             libcrypto.so
             libz.so)

set(SEAICE_SRC  
                seaice/address.cpp
                seaice/application.cpp
                seaice/async_socket_stream.cpp
                seaice/bytearray.cpp
                seaice/config.cpp
                seaice/config2.cpp
                seaice/daemon.cpp
                seaice/ds/cache_status.cpp
                seaice/env.cpp
                seaice/fd_manager.cpp
                seaice/fiber.cpp
                seaice/hook.cpp
                seaice/http/http.cpp
                seaice/http/http11_parser.rl.cpp
                seaice/http/httpclient_parser.rl.cpp
                seaice/http/http_parser.cpp
                seaice/http/http_connection.cpp
                seaice/http/http_session.cpp
                seaice/http/http_server.cpp
                seaice/http/servlet.cpp
                seaice/http/ws_connection.cpp
                seaice/http/ws_servlet.cpp
                seaice/http/ws_server.cpp
                seaice/http/ws_session.cpp
                seaice/iomanager.cpp
                seaice/log.cpp
                seaice/library.cpp
                seaice/module.cpp
                seaice/mutex.cpp
                seaice/protocol.cpp
                seaice/rock/rock_connection.cpp
                seaice/rock/rock_session.cpp
                seaice/rock/rock_protocol.cpp
                seaice/rock/rockstream.cpp
                seaice/rock/rock_server.cpp
                seaice/scheduler.cpp
                seaice/socket.cpp
                seaice/socketstream.cpp
                seaice/stream.cpp
                seaice/tcpserver.cpp
                seaice/thread.cpp
                seaice/timer.cpp
                seaice/utils.cpp
                seaice/uri.cpp
                seaice/util/hash_util.cpp
                seaice/worker.cpp
                seaice/zlib_stream.cpp)

#set(TESTSRC  test/testLog.cpp)
#add_executable(testlog ${TESTSRC} ${SEAICE_SRC})
#target_link_libraries(testlog /usr/local/lib/libyaml-cpp.so)

#set(TESTSRC  test/testyaml.cpp)
#add_executable(testyaml ${TESTSRC} ${SEAICE_SRC})
#target_link_libraries(testyaml /usr/local/lib/libyaml-cpp.so)

#set(TESTSRC  test/testMutex.cpp)
#add_executable(testMutex ${TESTSRC} ${SEAICE_SRC})
#target_link_libraries(testMutex /usr/local/lib/libyaml-cpp.so)

#set(TESTSRC  test/testFilber.cpp)
#add_executable(testFilber ${TESTSRC} ${SEAICE_SRC})
#target_link_libraries(testFilber /usr/local/lib/libyaml-cpp.so)

#set(TESTSRC  test/testScheduler.cpp)
#add_executable(testScheduler ${TESTSRC} ${SEAICE_SRC})
#target_link_libraries(testScheduler /usr/local/lib/libyaml-cpp.so)

#set(TESTSRC  test/testIOM.cpp)
#add_executable(testIOM ${TESTSRC} ${SEAICE_SRC})
#target_link_libraries(testIOM /usr/local/lib/libyaml-cpp.so)

#set(TESTSRC  test/testTimer.cpp)
#add_executable(testTimer ${TESTSRC} ${SEAICE_SRC})
#target_link_libraries(testTimer /usr/local/lib/libyaml-cpp.so)

#set(TESTSRC  test/test.cpp)
#add_executable(test ${TESTSRC} ${SEAICE_SRC})
#target_link_libraries(test /usr/local/lib/libyaml-cpp.so)

#set(TESTSRC  test/testHOOK.cpp)
#add_executable(testHOOK ${TESTSRC} ${SEAICE_SRC})
#target_link_libraries(testHOOK /usr/local/lib/libyaml-cpp.so)

#set(TESTSRC  test/test_forward.cpp)
#add_executable(test_forward ${TESTSRC} ${SEAICE_SRC})
#target_link_libraries(test_forward /usr/local/lib/libyaml-cpp.so)

#set(TESTSRC  test/testAddr.cpp)
#add_executable(testAddr ${TESTSRC} ${SEAICE_SRC})
#target_link_libraries(testAddr /usr/local/lib/libyaml-cpp.so)

#set(TESTSRC  test/testSocket.cpp)
#add_executable(testSocket ${TESTSRC} ${SEAICE_SRC})
#target_link_libraries(testSocket /usr/local/lib/libyaml-cpp.so)

#set(TESTSRC  test/test_ByteArray.cpp)
#add_executable(test_ByteArray ${TESTSRC} ${SEAICE_SRC})
#target_link_libraries(test_ByteArray /usr/local/lib/libyaml-cpp.so)

#set(TESTSRC  test/test_http.cpp)
#add_executable(test_http ${TESTSRC} ${SEAICE_SRC})
#target_link_libraries(test_http /usr/local/lib/libyaml-cpp.so)

#set(TESTSRC  test/test_tcp_server.cpp)
#add_executable(test_tcp_server ${TESTSRC} ${SEAICE_SRC})
#target_link_libraries(test_tcp_server /usr/local/lib/libyaml-cpp.so)

#set(TESTSRC  example/echo_server.cpp)
#add_executable(echo_server ${TESTSRC} ${SEAICE_SRC})
#target_link_libraries(echo_server /usr/local/lib/libyaml-cpp.so)

#set(TESTSRC  test/test_http_server.cpp)
#add_executable(test_http_server ${TESTSRC} ${SEAICE_SRC})
#target_link_libraries(test_http_server /usr/local/lib/libyaml-cpp.so)

#set(TESTSRC  test/test_http_connection.cpp)
#add_executable(test_http_connection ${TESTSRC} ${SEAICE_SRC})
#target_link_libraries(test_http_connection /usr/local/lib/libyaml-cpp.so)

#set(TESTSRC  test/test_daemon.cpp)
#add_executable(test_daemon ${TESTSRC} ${SEAICE_SRC})
#target_link_libraries(test_daemon /usr/local/lib/libyaml-cpp.so)

#set(TESTSRC  test/test_env.cpp)
#add_executable(test_env ${TESTSRC} ${SEAICE_SRC})
#target_link_libraries(test_env /usr/local/lib/libyaml-cpp.so)

#set(TESTSRC  test/test_config2.cpp)
#add_executable(test_config2 ${TESTSRC} ${SEAICE_SRC})
#target_link_libraries(test_config2 /usr/local/lib/libyaml-cpp.so)

if(flase)
set(TESTSRC  test/test_application.cpp)
add_executable(test_application ${TESTSRC} ${SEAICE_SRC})
target_link_libraries(test_application ${LINK_LIB})

set(TESTSRC  test/test_ws_client.cpp)
add_executable(test_ws_client ${TESTSRC} ${SEAICE_SRC})
target_link_libraries(test_ws_client ${LINK_LIB})

set(TESTSRC  test/test_ws_server.cpp)
add_executable(test_ws_server ${TESTSRC} ${SEAICE_SRC})
target_link_libraries(test_ws_server ${LINK_LIB})

set(TESTSRC  test/test.cpp)
add_executable(test ${TESTSRC} ${SEAICE_SRC})
target_link_libraries(test ${LINK_LIB})

set(TESTSRC  test/test_worker.cpp)
add_executable(test_worker ${TESTSRC} ${SEAICE_SRC})
target_link_libraries(test_worker ${LINK_LIB})

set(TESTSRC  test/test_timed_cache.cpp)
add_executable(test_timed_cache ${TESTSRC} ${SEAICE_SRC})
target_link_libraries(test_timed_cache ${LINK_LIB})

set(TESTSRC  test/test_timed_lru_cache.cpp)
add_executable(test_timed_lru_cache ${TESTSRC} ${SEAICE_SRC})
target_link_libraries(test_timed_lru_cache ${LINK_LIB})

endif()

set(TESTSRC  test/test_zlib_stream.cpp)
add_executable(test_zlib_stream ${TESTSRC} ${SEAICE_SRC})
target_link_libraries(test_zlib_stream ${LINK_LIB})

set(TESTSRC  test/test_rock_client.cpp)
add_executable(test_rock_client ${TESTSRC} ${SEAICE_SRC})
target_link_libraries(test_rock_client ${LINK_LIB})

set(TESTSRC  test/test_rock_server.cpp)
add_executable(test_rock_server ${TESTSRC} ${SEAICE_SRC})
target_link_libraries(test_rock_server ${LINK_LIB})

# 指定编译器
#set(cmake_c_flags_debug "-g -Wall)

# 查找当前目录下的所有源文件
# 并将名称保存到 DIR_SRCS 变量
#aux_source_directory(. DIR_SRCS)

# 添加 math 子目录
#add_subdirectory(test)
#add_subdirectory(hello)

# 指定生成目标 
#add_executable(hello helloworld.cpp)

# 添加链接库


# Set the directories that should be included in the build command for this target
# when running g++ these will be included as -I/directory/path/
#target_include_directories(hello_headers
#    PRIVATE 
#        ${PROJECT_SOURCE_DIR}/include
#)