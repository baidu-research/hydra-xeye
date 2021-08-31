cmake_minimum_required(VERSION 2.8.11)
project(test_xeye_sdk)

include_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/../
)

link_directories(
    ${PROJECT_SOURCE_DIR}
    ${PROJECT_SOURCE_DIR}/../
)

#test cjson
set(SOURCE_TEST_CJSON
  test_cjson.cpp
)

add_executable(test_cjson
  ${SOURCE_TEST_CJSON}
)

target_link_libraries(test_cjson
  xeye_sdk
  pthread
  ${OpenCV_LIBS}
  ${LIBUSB_1_LIBRARIES}
)

#test xeye sdk
set(SOURCE_TEST_XEYE_SDK
  image_handler.cpp
  ssd_output_handler.cpp
  watchdog_handler.cpp
  test_xeye_sdk.cpp
)

add_executable(${PROJECT_NAME}
  ${SOURCE_TEST_XEYE_SDK}
)

target_link_libraries(${PROJECT_NAME}
  xeye_sdk
  pthread
  ${OpenCV_LIBS}
  ${LIBUSB_1_LIBRARIES}
)

#test xeye computation sdk
set(COMPUTATION_PROJECT_NAME test_xeye_computation_sdk)
set(SOURCE_TEST_XEYE_COMPUTATION_SDK
  image_handler.cpp
  ssd_output_handler.cpp
  watchdog_handler.cpp
  test_xeye_computation_sdk.cpp
)

add_executable(${COMPUTATION_PROJECT_NAME}
  ${SOURCE_TEST_XEYE_COMPUTATION_SDK}
)

target_link_libraries(${COMPUTATION_PROJECT_NAME}
  xeye_sdk
  pthread
  ${OpenCV_LIBS}
  ${LIBUSB_1_LIBRARIES}
)

#test body detector
set (BODY_DETECTOR_PROJECT_NAME test_body_detector)
set(SOURCE_TEST_BODY_DETECTOR
  image_handler.cpp
  body_detector_handler.cpp
  watchdog_handler.cpp
  test_body_detector.cpp
)

add_executable(${BODY_DETECTOR_PROJECT_NAME}
  ${SOURCE_TEST_BODY_DETECTOR}
)

target_link_libraries(${BODY_DETECTOR_PROJECT_NAME}
  xeye_sdk
  pthread
  ${OpenCV_LIBS}
  ${LIBUSB_1_LIBRARIES}
)


#test xeye computation sdk without preprocess
set(COMPUTATION_NO_PREPROCESS_PROJECT_NAME test_no_preprocess)
set(SOURCE_TEST_XEYE_COMPUTATION_NO_PREPROCESS_SDK
  image_handler.cpp
  ssd_output_handler.cpp
  watchdog_handler.cpp
  test_no_preprocess.cpp
)

add_executable(${COMPUTATION_NO_PREPROCESS_PROJECT_NAME}
    ${SOURCE_TEST_XEYE_COMPUTATION_NO_PREPROCESS_SDK}
)

target_link_libraries(${COMPUTATION_NO_PREPROCESS_PROJECT_NAME}
  xeye_sdk
  pthread
  ${OpenCV_LIBS}
  ${LIBUSB_1_LIBRARIES}
)

#test style transfer
set(STYLE_TRANSFER_PROJECT_NAME test_style_transfer)
set(SOURCE_TEST_STYLE_TRANSFER_SDK
  style_transfer_handler.cpp
  watchdog_handler.cpp
  test_style_transfer.cpp
)

add_executable(${STYLE_TRANSFER_PROJECT_NAME}
    ${SOURCE_TEST_STYLE_TRANSFER_SDK}
)

target_link_libraries(${STYLE_TRANSFER_PROJECT_NAME}
  xeye_sdk
  pthread
  ${OpenCV_LIBS}
  ${LIBUSB_1_LIBRARIES}
)
