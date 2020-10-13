set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -Wall")
add_executable( laneDetection_vehicleDetection laneDetection_vehicleDetection.cpp)
target_link_libraries( laneDetection_vehicleDetection LINK_PUBLIC ${OpenCV_LIBS} )


