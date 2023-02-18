# CMakeList.txt: qqbot 的 CMake 项目，在此处包括源代码并定义
# 项目特定的逻辑。
#
cmake_minimum_required (VERSION 3.8)

project ("qqbot")

file(
	GLOB INCLUDEFILE
	"${CMAKE_SOURCE_DIR}/include/*.h"
	"${CMAKE_SOURCE_DIR}/include/*.cpp"
)

# 将源代码添加到此项目的可执行文件。
add_executable (qqbot "qqbot.cpp" "qqbot.h" "httplib.h" "init.h" "init.cpp"  "singleUser.h" "singleUser.cpp" "group.cpp" "group.h" "Json.cpp" "Json.h" "${INCLUDEFILE}" "permission.h" "permission.cpp" "register.h" "register.cpp" "cppPlugin.h")

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET qqbot PROPERTY CXX_STANDARD 20)
endif()

# TODO: 如有需要，请添加测试并安装目标。
