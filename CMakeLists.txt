cmake_minimum_required( VERSION 3.24 )

project( iRecHS2s LANGUAGES C CXX )
set(MYPROJECT iRecHS2sUvc)

set(CMAKE_CXX_STANDARD 20)

if(WIN32)
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /MT")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /MTd")
endif()

# boost require
find_package(boost_format REQUIRED CONFIG)
find_package(boost_foreach REQUIRED CONFIG)
find_package(boost_filesystem REQUIRED CONFIG)
find_package(boost_serialization REQUIRED CONFIG)
find_package(boost_wserialization REQUIRED CONFIG)
find_package(boost_asio REQUIRED CONFIG)
# boost require

# libjpeg-turbo require
find_package(libjpeg-turbo CONFIG REQUIRED)
# libjpeg-turbo require

# libuvc require
find_package(PThreads4W REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(libusb REQUIRED IMPORTED_TARGET libusb-1.0)

set(LIBUSB_WIN_INCLUDE_DIR ${libusb_INCLUDE_DIRS} CACHE PATH "Path to the libusb include dir")
set(LIBUSB_WIN_IMPORT_LIB_PATH ${pkgcfg_lib_libusb_libusb-1.0} CACHE FILEPATH "Path to the libusb import library")
set(PTHREADS_WIN_INCLUDE_DIR ${PThreads4W_INCLUDE_DIR} CACHE PATH "Path to the pthreads-win include dir")
set(PTHREADS_WIN_IMPORT_LIB_PATH ${PThreads4W_LIBRARY_RELEASE} CACHE FILEPATH "Path to the pthreads-win import library")


include(FetchContent)
FetchContent_Declare(
  libuvc
  GIT_REPOSITORY    https://github.com/pupil-labs/libuvc
  GIT_TAG           master
  FIND_PACKAGE_ARGS
)

FetchContent_MakeAvailable(libuvc)
# libuvc require

add_executable(${MYPROJECT}
    DlgCtrls.h
    eyedata.h
    framework.h
    imgproc.h
    iRecClass.h
    iRecHS2s.h
    Resource.h
    targetver.h
    UvcCamera.h
    WinCtrls.h
    calibration.cpp
    CameraView.cpp
    dac.cpp
    DlgCtrls.cpp
    eyedata.cpp
    fileio.cpp
    imgproc.cpp
    iRecClass.cpp
    iRecHS2s.cpp
    net_io.cpp
    objectmap.cpp
    options.cpp
    pos2gaze.cpp
    timeline.cpp
    UvcCamera.cpp
    WinCtrls.cpp
    XYGraph.cpp
    iRecHS2s.rc
    )

add_definitions(-D_UNICODE -DUNICODE)
set_target_properties(${MYPROJECT} PROPERTIES WIN32_EXECUTABLE TRUE)

target_link_libraries(${MYPROJECT} PRIVATE 
    Boost::format
    Boost::foreach
    Boost::filesystem
    Boost::serialization
    Boost::wserialization
    Boost::asio
    )

target_link_libraries(${MYPROJECT} PRIVATE
    user32
    gdi32 
    shlwapi 
    comctl32
    )

target_link_libraries(${MYPROJECT} PRIVATE PLLibUVC::uvc)
target_link_libraries(${MYPROJECT} PRIVATE $<IF:$<TARGET_EXISTS:libjpeg-turbo::turbojpeg>,libjpeg-turbo::turbojpeg,libjpeg-turbo::turbojpeg-static>)