cmake_minimum_required(VERSION 3.16)

project(inferencer-cpp VERSION 1.0.0 LANGUAGES CXX)

if(NOT DEFINED ENV{QT_CMAKE_PATH})
    message(FATAL_ERROR "QT_CMAKE_PATH environment variable is not set.")
else()
    set(CMAKE_PREFIX_PATH $ENV{QT_CMAKE_PATH})
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

find_package(onnxruntime REQUIRED)
find_library(ONNX_RUNTIME_LIB onnxruntime HINTS /usr/local/lib)

find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})

find_path(HIREDIS_HEADER hiredis)
include_directories(${HIREDIS_HEADER})
find_library(HIREDIS_LIB hiredis)

find_path(REDIS_PLUS_PLUS_HEADER sw)
include_directories(${REDIS_PLUS_PLUS_HEADER})
find_library(REDIS_PLUS_PLUS_LIB redis++)

add_subdirectory(libs/byte-track-cpp)
include_directories(${CMAKE_SOURCE_DIR}/libs/byte-track-cpp/include)

find_package(Eigen3 REQUIRED)
include_directories(${EIGEN3_INCLUDE_DIR})

qt_standard_project_setup()

qt_add_executable(inferencer-cpp
  src/mainwindow.ui
  src/mainwindow.cpp
  src/Worker.cpp
  src/ONNXInferencer.cpp
  src/ObjectDetectionDrawer.cpp
  src/MyTimer.cpp
  src/globals.cpp
  src/Redis.cpp
  src/main.cpp
)

target_link_libraries(inferencer-cpp PRIVATE Qt6::Widgets ${OpenCV_LIBS} ${ONNX_RUNTIME_LIB} ${HIREDIS_LIB} ${REDIS_PLUS_PLUS_LIB} ByteTrackEigen)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
  add_definitions(-DDEBUG_MODE)
  add_definitions(-DBUILD_DIR="${CMAKE_SOURCE_DIR}/buildDebug/")
endif()

# Deploy
install(TARGETS ${PROJECT_NAME}
  BUNDLE DESTINATION .
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
qt_generate_deploy_app_script(
  TARGET ${PROJECT_NAME}
  OUTPUT_SCRIPT deploy_script
  NO_UNSUPPORTED_PLATFORM_ERROR
  NO_TRANSLATIONS
  NO_COMPILER_RUNTIME
)
install(SCRIPT ${deploy_script})

# Uninstall; Qt library files are not being tracked in the install_manifest.txt
add_custom_target("uninstall" COMMENT "Uninstall installed files")
add_custom_command(
  TARGET "uninstall"
  POST_BUILD
  COMMENT "Uninstall files with install_manifest.txt"
  COMMAND xargs rm -vf < install_manifest.txt || echo Nothing in install_manifest.txt to be uninstalled!
)