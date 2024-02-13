# CMakeList.txt: qqbot 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project ("qqbot")

file(GLOB SOURCEFILE
    "${CMAKE_SOURCE_DIR}/command/*.h"
    "${CMAKE_SOURCE_DIR}/command/*.cpp"

    "${CMAKE_SOURCE_DIR}/cppPlugin/*.h"
    "${CMAKE_SOURCE_DIR}/cppPlugin/*.cpp"

    "${CMAKE_SOURCE_DIR}/plugin/*.h"
    "${CMAKE_SOURCE_DIR}/plugin/*.cpp"

    "${CMAKE_SOURCE_DIR}/pluginLibrary/*.h"
    "${CMAKE_SOURCE_DIR}/pluginLibrary/*.cpp"

    "${CMAKE_SOURCE_DIR}/definition/*.h"
    "${CMAKE_SOURCE_DIR}/definition/*.cpp"

    "${CMAKE_SOURCE_DIR}/group/*.h"
    "${CMAKE_SOURCE_DIR}/group/*.cpp"

    "${CMAKE_SOURCE_DIR}/init/*.h"
    "${CMAKE_SOURCE_DIR}/init/*.cpp"

    "${CMAKE_SOURCE_DIR}/network/*.h"
    "${CMAKE_SOURCE_DIR}/network/*.cpp"

    "${CMAKE_SOURCE_DIR}/permission/*.h"
    "${CMAKE_SOURCE_DIR}/permission/*.cpp"

    "${CMAKE_SOURCE_DIR}/qqbot/*.h"
    "${CMAKE_SOURCE_DIR}/qqbot/*.cpp"

    "${CMAKE_SOURCE_DIR}/register/*.h"
    "${CMAKE_SOURCE_DIR}/register/*.cpp"

    "${CMAKE_SOURCE_DIR}/eventRegister/*.h"
    "${CMAKE_SOURCE_DIR}/eventRegister/*.cpp"

    "${CMAKE_SOURCE_DIR}/singleUser/*.h"
    "${CMAKE_SOURCE_DIR}/singleUser/*.cpp"
)

include_directories(
    "${CMAKE_SOURCE_DIR}/command/"
    "${CMAKE_SOURCE_DIR}/cppPlugin/"
    "${CMAKE_SOURCE_DIR}/definition/"
    "${CMAKE_SOURCE_DIR}/group/"
    "${CMAKE_SOURCE_DIR}/init/"
    "${CMAKE_SOURCE_DIR}/network/"
    "${CMAKE_SOURCE_DIR}/permission/"
    "${CMAKE_SOURCE_DIR}/qqbot/"
    "${CMAKE_SOURCE_DIR}/register/"
    "${CMAKE_SOURCE_DIR}/singleUser/"
    "${CMAKE_SOURCE_DIR}/plugin/"
    "${CMAKE_SOURCE_DIR}/pluginLibrary/"
    "${CMAKE_SOURCE_DIR}/eventRegister/"
    )

if (MSVC)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT /utf-8")

    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd /utf-8")
endif(MSVC)

# openssl支持
find_package(OpenSSL REQUIRED)

link_directories("${CMAKE_SOURCE_DIR}/lib")

# 将源代码添加到此项目的可执行文件。
add_executable (qqbot "${SOURCEFILE}")

#openssl
target_link_libraries(${PROJECT_NAME} PUBLIC
        OpenSSL::SSL
        OpenSSL::Crypto)

target_compile_definitions(${PROJECT_NAME} PUBLIC
        CPPHTTPLIB_OPENSSL_SUPPORT)

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET qqbot PROPERTY CXX_STANDARD 23)
endif()
