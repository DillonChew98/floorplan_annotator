set(TEST_DIR ${CMAKE_CURRENT_SOURCE_DIR})

add_definitions( -DTEST_DIRECTORY=\"${CMAKE_CURRENT_SOURCE_DIR}/\")

include_directories(${GENERATED_DIR})

ament_add_gtest(test_${PROJECT_NAME}_rest
    ./rest_test.cpp
)
target_link_libraries(test_${PROJECT_NAME}_rest
  ${PROJECT_NAME}
)

ament_add_gtest(test_${PROJECT_NAME}_detection
    ./detection_test.cpp
)
target_link_libraries(test_${PROJECT_NAME}_detection
  ${PROJECT_NAME}
)