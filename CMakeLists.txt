cmake_minimum_required(VERSION 3.8)
project(floorplan_annotator)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(OpenMP)
find_package(OpenSSL REQUIRED)
find_package(OpenCV REQUIRED)
find_package(CURL REQUIRED)
find_package(cpprestsdk REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(ortools_vendor REQUIRED)
find_package(Qt5 COMPONENTS Widgets Concurrent Test Network REQUIRED)
# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)

message(${ortools_vendor_DIR})

add_library(${PROJECT_NAME}
  SHARED
  src/utils/rest_communicator.cpp
  src/utils/curl_communicator.cpp
  src/feature_detection.cpp
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include/${PROJECT_NAME}>"
  ${OpenCV_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME}
  OpenSSL::SSL
  OpenMP::OpenMP_CXX
  cpprestsdk::cpprest
  nlohmann_json::nlohmann_json
  CURL::libcurl
  ortools::ortools
  Qt5::Widgets
  Qt5::Concurrent
  Qt5::Network
  ${OpenCV_LIBS})

install (TARGETS
  ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin)

install(
  DIRECTORY include/
  DESTINATION include/
)

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  set(ament_cmake_cpplint_FOUND TRUE)
  find_package(ament_cmake_gtest REQUIRED)
  ament_lint_auto_find_test_dependencies()
  add_subdirectory(test)
endif()

ament_export_include_directories(
  include
  ${OpenCV_INCLUDE_DIRS} 
)

ament_export_libraries(
  ${PROJECT_NAME}
)

ament_export_dependencies(${dependencies})

ament_package()
