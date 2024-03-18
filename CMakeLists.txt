cmake_minimum_required(VERSION 3.16)

project(inferencer-cpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_PREFIX_PATH "/home/rock/Qt/6.6.2/gcc_64/lib/cmake")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)
# find_package(OpenCV REQUIRED)
find_package(onnxruntime REQUIRED)
find_library(ONNX_RUNTIME_LIB onnxruntime HINTS /usr/local/lib)
# include_directories(${OpenCV_INCLUDE_DIRS})

# if(CMAKE_BUILD_TYPE MATCHES Debug)
#   set(OpenCV_DIR "/home/rock/builds/opencv-4.8.1/installDebug/lib/cmake/opencv4")
# endif()
find_package(OpenCV REQUIRED)
include_directories(${OpenCV_INCLUDE_DIRS})


qt_standard_project_setup()

qt_add_executable(inferencer-cpp
  src/mainwindow.ui
  src/mainwindow.cpp
  src/Worker.cpp
  src/ONNXInferencer.cpp
  src/ObjectDetectionDrawer.cpp
  src/MyTimer.cpp
  src/globals.cpp
  src/main.cpp
)

target_link_libraries(inferencer-cpp PRIVATE Qt6::Widgets ${OpenCV_LIBS} ${ONNX_RUNTIME_LIB})

set_target_properties(inferencer-cpp PROPERTIES
  WIN32_EXECUTABLE ON
  MACOSX_BUNDLE ON
)

if(CMAKE_BUILD_TYPE MATCHES Debug)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0")
  add_definitions(-DDEBUG_MODE)
  add_definitions(-DBUILD_DIR="${CMAKE_SOURCE_DIR}/buildDebug/")
endif()