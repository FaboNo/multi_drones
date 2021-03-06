cmake_minimum_required(VERSION 2.4.6)
include($ENV{ROS_ROOT}/core/rosbuild/rosbuild.cmake)

# Set the build type.  Options are:
#  Coverage       : w/ debug symbols, w/o optimization, w/ code-coverage
#  Debug          : w/ debug symbols, w/o optimization
#  Release        : w/o debug symbols, w/ optimization
#  RelWithDebInfo : w/ debug symbols, w/ optimization
#  MinSizeRel     : w/o debug symbols, w/ optimization, stripped binaries
#set(ROS_BUILD_TYPE RelWithDebInfo)

rosbuild_init()

#set the default path for built executables to the "bin" directory
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
#set the default path for built libraries to the "lib" directory
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)

#uncomment if you have defined messages
rosbuild_genmsg()
#uncomment if you have defined services
#rosbuild_gensrv()

#common commands for building c++ executables and libraries
#rosbuild_add_library(${PROJECT_NAME} src/example.cpp)
#target_link_libraries(${PROJECT_NAME} another_library)
#rosbuild_add_boost_directories()
#rosbuild_link_boost(${PROJECT_NAME} thread)
#rosbuild_add_executable(example examples/example.cpp)
#target_link_libraries(example ${PROJECT_NAME})

find_package(Eigen REQUIRED)
include_directories(${Eigen_INCLUDE_DIRS})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR})
find_package(Ranav REQUIRED)
include_directories(${RANAV_INCLUDE_DIR})
link_directories(${RANAV_LIBRARY_DIR})

rosbuild_add_executable(multi_drone_control src/main_control.cpp src/ArdroneRosSync.cpp src/MultiAgent3dNavigation.cpp src/marker3dsensormodel.cpp src/rotor3dmotionmodel.cpp src/Camera.cpp)
target_link_libraries(multi_drone_control ${RANAV_LIBRARIES})

#rosbuild_add_executable(multi_drone_ekf src/main.cpp src/EKF.cpp src/Ardrone.cpp src/Camera.cpp src/motionmodel.cpp src/random.cpp src/rotor2Dmotionmodel.cpp src/sensormodel.cpp src/rotor3Dmotionmodel.cpp src/marker2dsensormodel.cpp src/marker3dsensormodel.cpp src/PIDcontroller.cpp src/Ardronecontroller.cpp)
