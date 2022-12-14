cmake_minimum_required(VERSION 3.0.2)
cmake_policy(SET CMP0074 NEW)
project(scan_republisher)



include(cmake/common.cmake)
include(cmake/set_rpath.cmake)
include(cmake/super_build.cmake)
include(cmake/find_opencv_helper.cmake)
set(CMAKE_CXX_STANDARD  14)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")




# 设置生成的安装包名字
set(CPACK_PACKAGE_NAME ${PROJECT_NAME})
# 设置支持指定安装目录的控制为 ON
set(CPACK_SET_DESTDIR ON)
# 设置安装到的目录路径
set(CPACK_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})
# 这是生成的安装的版本号信息
set(CPACK_PACKAGE_VERSION "1.0.0")
# 设置 group 名字
set(CPACK_RPM_PACKAGE_GROUP "vesoft")
# 设置 vendor 名字
set(CPACK_PACKAGE_VENDOR "vesoft")
# 设置 license 信息
set(CPACK_RPM_PACKAGE_LICENSE "Apache 2.0 + Common Clause 1.0")
set(CPACK_GENERATOR "DEB")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Yan QiDong")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION "This is a demo package from Yan QiDong.")
set(CPACK_DEBIAN_PACKAGE_VERSION ${PACKAGE_VERSION})
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)

## Compile as C++11, supported in ROS Kinetic and newer
# add_compile_options(-std=c++11)

## Find catkin macros and libraries
## if COMPONENTS list like find_package(catkin REQUIRED COMPONENTS xyz)
## is used, also find other catkin packages
find_package(catkin REQUIRED COMPONENTS
  nav_msgs
  roscpp
  sensor_msgs
  tf
  interactive_markers
)

## System dependencies are found with CMake's conventions
# find_package(Boost REQUIRED COMPONENTS system)


## Uncomment this if the package has a setup.py. This macro ensures
## modules and global scripts declared therein get installed
## See http://ros.org/doc/api/catkin/html/user_guide/setup_dot_py.html
# catkin_python_setup()

################################################
## Declare ROS messages, services and actions ##
################################################

## To declare and build messages, services or actions from within this
## package, follow these steps:
## * Let MSG_DEP_SET be the set of packages whose message types you use in
##   your messages/services/actions (e.g. std_msgs, actionlib_msgs, ...).
## * In the file package.xml:
##   * add a build_depend tag for "message_generation"
##   * add a build_depend and a exec_depend tag for each package in MSG_DEP_SET
##   * If MSG_DEP_SET isn't empty the following dependency has been pulled in
##     but can be declared for certainty nonetheless:
##     * add a exec_depend tag for "message_runtime"
## * In this file (CMakeLists.txt):
##   * add "message_generation" and every package in MSG_DEP_SET to
##     find_package(catkin REQUIRED COMPONENTS ...)
##   * add "message_runtime" and every package in MSG_DEP_SET to
##     catkin_package(CATKIN_DEPENDS ...)
##   * uncomment the add_*_files sections below as needed
##     and list every .msg/.srv/.action file to be processed
##   * uncomment the generate_messages entry below
##   * add every package in MSG_DEP_SET to generate_messages(DEPENDENCIES ...)

## Generate messages in the 'msg' folder
# add_message_files(
#   FILES
#   Message1.msg
#   Message2.msg
# )

## Generate services in the 'srv' folder
# add_service_files(
#   FILES
#   Service1.srv
#   Service2.srv
# )

## Generate actions in the 'action' folder
# add_action_files(
#   FILES
#   Action1.action
#   Action2.action
# )

## Generate added messages and services with any dependencies listed here
# generate_messages(
#   DEPENDENCIES
#   nav_msgs#   sensor_msgs
# )

################################################
## Declare ROS dynamic reconfigure parameters ##
################################################

## To declare and build dynamic reconfigure parameters within this
## package, follow these steps:
## * In the file package.xml:
##   * add a build_depend and a exec_depend tag for "dynamic_reconfigure"
## * In this file (CMakeLists.txt):
##   * add "dynamic_reconfigure" to
##     find_package(catkin REQUIRED COMPONENTS ...)
##   * uncomment the "generate_dynamic_reconfigure_options" section below
##     and list every .cfg file to be processed

## Generate dynamic reconfigure parameters in the 'cfg' folder
# generate_dynamic_reconfigure_options(
#   cfg/DynReconf1.cfg
#   cfg/DynReconf2.cfg
# )

###################################
## catkin specific configuration ##
###################################
## The catkin_package macro generates cmake config files for your package
## Declare things to be passed to dependent projects
## INCLUDE_DIRS: uncomment this if your package contains header files
## LIBRARIES: libraries you create in this project that dependent projects also need
## CATKIN_DEPENDS: catkin_packages dependent projects also need
## DEPENDS: system dependencies of this project that dependent projects also need
catkin_package(
#  INCLUDE_DIRS include
#  LIBRARIES topic_serialization
#  CATKIN_DEPENDS nav_msgs roscpp sensor_msgs
#  DEPENDS system_lib
)

###########
## Build ##
###########

## Specify additional locations of header files
## Your package locations should be listed before other locations
include_directories(
# include
  ${catkin_INCLUDE_DIRS}
)

## Declare a C++ library
# add_library(${PROJECT_NAME}
#   src/${PROJECT_NAME}/topic_serialization.cpp
# )

## Add cmake target dependencies of the library
## as an example, code may need to be generated before libraries
## either from message generation or dynamic reconfigure
# add_dependencies(${PROJECT_NAME} ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})

## Declare a C++ executable
## With catkin_make all packages are built within a single CMake context
## The recommended prefix ensures that target names across packages don't collide
# add_executable(${PROJECT_NAME}_node src/topic_serialization_node.cpp)

## Rename C++ executable without prefix
## The above recommended prefix causes long target names, the following renames the
## target back to the shorter version for ease of user use
## e.g. "rosrun someones_pkg node" instead of "rosrun someones_pkg someones_pkg_node"
# set_target_properties(${PROJECT_NAME}_node PROPERTIES OUTPUT_NAME node PREFIX "")

## Add cmake target dependencies of the executable
## same as for the library above
# add_dependencies(${PROJECT_NAME}_node ${${PROJECT_NAME}_EXPORTED_TARGETS} ${catkin_EXPORTED_TARGETS})

## Specify libraries to link a library or executable target against
# target_link_libraries(${PROJECT_NAME}_node
#   ${catkin_LIBRARIES}
# )

#############
## Install ##
#############

# all install targets should use catkin DESTINATION variables
# See http://ros.org/doc/api/catkin/html/adv_user_guide/variables.html

## Mark executable scripts (Python etc.) for installation
## in contrast to setup.py, you can choose the destination
# catkin_install_python(PROGRAMS
#   scripts/my_python_script
#   DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
# )

## Mark executables for installation
## See http://docs.ros.org/melodic/api/catkin/html/howto/format1/building_executables.html
# install(TARGETS ${PROJECT_NAME}_node
#   RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
# )

## Mark libraries for installation
## See http://docs.ros.org/melodic/api/catkin/html/howto/format1/building_libraries.html
# install(TARGETS ${PROJECT_NAME}
#   ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
#   LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
#   RUNTIME DESTINATION ${CATKIN_GLOBAL_BIN_DESTINATION}
# )

## Mark cpp header files for installation
# install(DIRECTORY include/${PROJECT_NAME}/
#   DESTINATION ${CATKIN_PACKAGE_INCLUDE_DESTINATION}
#   FILES_MATCHING PATTERN "*.h"
#   PATTERN ".svn" EXCLUDE
# )

## Mark other files for installation (e.g. launch and bag files, etc.)
# install(FILES
#   # myfile1
#   # myfile2
#   DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
# )

#############
## Testing ##
#############

## Add gtest based cpp test target and link libraries
# catkin_add_gtest(${PROJECT_NAME}-test test/test_topic_serialization.cpp)
# if(TARGET ${PROJECT_NAME}-test)
#   target_link_libraries(${PROJECT_NAME}-test ${PROJECT_NAME})
# endif()

## Add folders to be run by python nosetests
# catkin_add_nosetests(test)


build_external_project(mosquitto lib ${CMAKE_CURRENT_SOURCE_DIR}/third_party/mosquitto-2.0.15.tar.gz -DCMAKE_BUILD_TYPE=Release -DcJSON_DIR=${cJSON_DIR} -DDOCUMENTATION=OFF)
set(mosquitto_LIBRARY ${mosquitto_ROOT}/lib/libmosquitto.so ${mosquitto_ROOT}/lib/libmosquittopp.so)
set(mosquitto_INCLUDE_DIR ${mosquitto_ROOT}/include)

#build_external_project(tinyxml2 lib ${CMAKE_CURRENT_SOURCE_DIR}/third_party/tinyxml2-9.0.0.tar.gz  -DCMAKE_BUILD_TYPE=Release  )
#find_package(tinyxml2 REQUIRED)
# tinyxml2::tinyxml2

build_external_project(Ceres lib ${CMAKE_CURRENT_SOURCE_DIR}/third_party/ceres-solver-2.1.0.tar.gz  -DCMAKE_BUILD_TYPE=RelWithDebInfo -DMINIGLOG=ON -DGFLAGS=OFF -DSUITESPARSE=OFF -DBUILD_EXAMPLES=OFF)
find_package(Ceres  REQUIRED)
message(Ceres_ROOT : ${Ceres_ROOT} ,CERES_LIBRARIES : ${CERES_LIBRARIES} )


build_external_project(PCL lib ${CMAKE_CURRENT_SOURCE_DIR}/third_party/pcl-pcl-1.12.1.tar.gz  -DCMAKE_BUILD_TYPE=Release  -DBUILD_examples=OFF -DBUILD_apps=OFF -DBUILD_global_tests=OFF -DBUILD_simulation=OFF)
find_package(PCL REQUIRED)

#octomap-1.9.6.tar.gz
build_external_project(octomap lib ${CMAKE_CURRENT_SOURCE_DIR}/third_party/octomap-1.9.6.tar.gz  -DCMAKE_BUILD_TYPE=Release  -DBUILD_OCTOVIS_SUBPROJECT=OFF -DBUILD_DYNAMICETD3D_SUBPROJECT=OFF)
find_package(octomap REQUIRED)


add_executable(scan_republisher src/scan_republisher.cpp src/message/MqttClient.cpp)
target_link_libraries(scan_republisher PUBLIC  ${catkin_LIBRARIES} ${mosquitto_LIBRARY} ) 
target_include_directories(scan_republisher PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include  ${catkin_INCLUDE_DIRS} ${mosquitto_INCLUDE_DIR}   ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann)

add_executable(scan_filter_intensity src/scan_filter_intensity.cpp)
target_link_libraries(scan_filter_intensity PUBLIC  ${catkin_LIBRARIES}  ${CERES_LIBRARIES})
target_include_directories(scan_filter_intensity PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include  ${catkin_INCLUDE_DIRS}   ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann)


find_package (Eigen3 REQUIRED NO_MODULE)

add_library(pcl_norm_2d STATIC src/icp/PCA2D.cpp src/icp/Normal2dEstimation.cpp)
target_include_directories( pcl_norm_2d PUBLIC  ${EIGEN3_INCLUDE_DIR} ${CMAKE_SOURCE_DIR}/include  ${CMAKE_SOURCE_DIR}/third_party/nlohmann  ${PCL_INCLUDE_DIRS} ${OCTOMAP_INCLUDE_DIRS} )





add_executable(scan_matching src/scan_matching.cpp)
target_include_directories(scan_matching PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include  ${catkin_INCLUDE_DIRS}   ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann ${PCL_INCLUDE_DIRS} ${OCTOMAP_INCLUDE_DIRS} )

target_link_libraries(scan_matching PUBLIC pcl_norm_2d ${catkin_LIBRARIES}  ${CERES_LIBRARIES}  ${PCL_LIBRARIES} ${PCL_COMMON_LIBRARIES} ${PCL_IO_LIBRARIES} ${PCL_FILTERS_LIBRARIES} ${OCTOMAP_LIBRARIES} )
if(scan_matching_debug)
    set_asan(scan_matching)
endif()


add_executable(scan_circle src/scan_circle.cpp)
target_include_directories(scan_circle PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include  ${catkin_INCLUDE_DIRS}   ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nlohmann ${PCL_INCLUDE_DIRS} ${OCTOMAP_INCLUDE_DIRS} )

target_link_libraries(scan_circle PUBLIC pcl_norm_2d ${catkin_LIBRARIES}  ${CERES_LIBRARIES}  ${PCL_LIBRARIES} ${PCL_COMMON_LIBRARIES} ${PCL_IO_LIBRARIES} ${PCL_FILTERS_LIBRARIES} ${OCTOMAP_LIBRARIES} )


# ${Ceres_ROOT}/lib
set(shared_dirs ${mosquitto_ROOT}/lib ${PCL_ROOT}/lib )
foreach(dir ${shared_dirs})
    if(EXISTS "${dir}" AND IS_DIRECTORY "${dir}")
        message("EXISTS：${dir}")
        #Installing roslaunch Files or Other Resources
        install(DIRECTORY ${dir}/
                DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
                PATTERN ".svn" EXCLUDE)
    endif()
endforeach()


install(TARGETS scan_republisher
        ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
        )
install(TARGETS scan_matching
        ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
        )

install(TARGETS scan_filter_intensity
        ARCHIVE DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        LIBRARY DESTINATION ${CATKIN_PACKAGE_LIB_DESTINATION}
        RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
        )

install(DIRECTORY launch
        DESTINATION ${CATKIN_PACKAGE_SHARE_DESTINATION}
        PATTERN "launch/data" EXCLUDE
        )

set_target_runpath(scan_matching)
include(CPack)