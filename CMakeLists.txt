cmake_minimum_required(VERSION 2.8)
project( control )
find_package( OpenCV REQUIRED )
message(STATUS "OpenCV_FOUND = ${OpenCV_FOUND}")
find_package(X11 REQUIRED)
message(STATUS "X11_FOUND = ${X11_FOUND}")
message(STATUS "X11_INCLUDE_DIR = ${X11_INCLUDE_DIR}")
message(STATUS "X11_LIBRARIES = ${X11_LIBRARIES}")
message(STATUS "X11_FOUND = ${X11_FOUND}")

add_executable( control src/main.cpp
        src/bot.cpp src/bot.h
        src/network_manager.cpp src/network_manager.h
        src/state.cpp src/state.h
        src/utils.cpp src/utils.h
        src/camera_calibration.cpp src/camera_calibration.h
        src/agent.cpp src/agent.h
        src/ui.cpp src/ui.h
        src/behaviors.cpp src/behaviors.h
        src/d_star.cpp src/d_star.h src/behavior_base.h src/argparse.hpp src/behavior_base.cpp)
add_executable( mdns-mod src/dns-sd.c src/ClientCommon.c src/ClientCommon.h)
add_executable( test_suite test_suite.h tests.cpp cxxtest/cxxtest/ src/bot.cpp src/bot.h src/network_manager.cpp src/network_manager.h src/state.cpp src/state.h src/utils.cpp src/utils.h src/camera_calibration.cpp src/camera_calibration.h src/agent.cpp src/agent.h src/ui.cpp src/ui.h)
include_directories(${X11_INCLUDE_DIR})
link_directories(${X11_LIBRARIES})
target_link_libraries( test_suite ${OpenCV_LIBS} ${X11_LIBRARIES} )
target_link_libraries( control ${OpenCV_LIBS} ${X11_LIBRARIES} )
