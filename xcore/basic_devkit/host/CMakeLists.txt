cmake_minimum_required(VERSION 2.8.11)

set(CMAKE_BUILD_TYPE Release)

project(xeye_sdk)

# NEON or SSE
if((CMAKE_SYSTEM_PROCESSOR MATCHES "^arm"))
    set(NEON_FLAGS "-D__ARM_NEON__ -DENABLE_NEON -mfloat-abi=hard -mfpu=neon -Ofast")
    message("Enabling neon for armv7 " ${NEON_FLAGS})
elseif (CMAKE_SYSTEM_PROCESSOR MATCHES "^aarch64")
    set(NEON_FLAGS "-D__ARM_NEON__ -DENABLE_NEON -Ofast")
    message("Enabling neon for armv8 " ${NEON_FLAGS})
else()
    add_definitions(-mssse3)
endif()

set(CMAKE_CXX_FLAGS "-std=c++11 -Wno-deprecated-declarations ${CMAKE_CXX_FLAGS} ${NEON_FLAGS}")
set(CMAKE_C_FLAGS "-std=gnu99 -Wno-deprecated-declarations ${CMAKE_C_FLAGS} ${NEON_FLAGS}")
set(CMAKE_CXX_FLAGS "-g ${CMAKE_CXX_FLAGS}")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${PROJECT_SOURCE_DIR}/cmake/")
message(STATUS "CMAKE_MODULE_PATH: ${CMAKE_MODULE_PATH}")

find_package(OpenCV REQUIRED)
find_package(libusb1.0 REQUIRED)

#find_package(Glog REQUIRED)
#find_package(GFlags REQUIRED)
#find_package(JPEGTURBO REQUIRED
#message(STATUS "JPEGTURBO_INCLUDE_DIR =  ${JPEGTURBO_INCLUDE_DIR}")
#message(STATUS "JPEGTURBO_LIBRARIES =  ${JPEGTURBO_LIBRARIES}")

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${OpenCV_INCLUDE_DIRS}
    ${LIBUSB_INCLUDE_DIRS}
)
message(STATUS "PROJECT_SOURCE_DIR: ${PROJECT_SOURCE_DIR}")

set(XEYE_SDK_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/cJSON.c
    ${CMAKE_CURRENT_SOURCE_DIR}/connector_message_thread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dl_input_thread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dl_output_thread.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dl_process.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/file_map.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/fp16_convert.c
    ${CMAKE_CURRENT_SOURCE_DIR}/usb_connector.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/xeye.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/xeye_computation.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/xeye_computation_interface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/xeye_config.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/xeye_interface.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/xeye_usb.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/xeye_util.cpp
)

set(STATIC_LIBRARY_NAME "${PROJECT_NAME}_static")
add_library(${STATIC_LIBRARY_NAME} STATIC ${XEYE_SDK_SOURCES})

add_library(${PROJECT_NAME} SHARED ${XEYE_SDK_SOURCES})

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC 
    ${OpenCV_LIBS} 
    ${LIBUSB_1_LIBRARIES})

add_subdirectory(tests)
