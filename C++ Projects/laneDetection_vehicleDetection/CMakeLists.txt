cmake_minimum_required(VERSION 3.0)
project( laneDetection_vehicleDetection )

find_package( OpenCV 3.2.0 REQUIRED )
#find_package( OpenCV 2.4.13 REQUIRED )

#file(COPY "video" DESTINATION ${CMAKE_BINARY_DIR})
file(COPY ${CMAKE_SOURCE_DIR}/res/cars_Yan.xml DESTINATION ${CMAKE_BINARY_DIR})
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR})
add_subdirectory(src)


