cmake_minimum_required( VERSION 3.15 )

project( iRecHS2sUvc )

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_VERSION 10.0)

# add Windows SDK
find_package(WinSDK REQUIRED)

include_directories(${WinSDK_INCLUDE_DIRS})

# add libuvc
include(FetchContent)
FetchContent_Declare(
  libuvc
  GIT_REPOSITORY https://github.com/pupil-labs/libuvc
  GIT_TAG        master
)

FetchContent_MakeAvailable(libuvc)

add_executable(PROJECT_NAME main.cpp)

target_link_libraries(PROJECT_NAME ${WinSDK_LIBRARIES})
target_link_libraries(PROJECT_NAME libuvc)