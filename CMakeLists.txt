cmake_minimum_required(VERSION 3.2)

project("ADVi3pp")

include(CMakeListsPrivate.txt)

add_custom_target(
    ADVi3++_5.1 ALL
    COMMAND ${PLATFORMIO_CMD} -f -c clion run $<$<CONFIG:Verbose>:--verbose> -e advi3pp-51
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    ADVi3++_5.1_BLTouch ALL
    COMMAND ${PLATFORMIO_CMD} -f -c clion run $<$<CONFIG:Verbose>:--verbose> -e advi3pp-51-bltouch
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    ADVi3++_5.1_BLTouch3 ALL
    COMMAND ${PLATFORMIO_CMD} -f -c clion run $<$<CONFIG:Verbose>:--verbose> -e advi3pp-51-bltouch3
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    ADVi3++_5.2C ALL
    COMMAND ${PLATFORMIO_CMD} -f -c clion run $<$<CONFIG:Verbose>:--verbose> -e advi3pp-52c
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    ADVi3++_5.2C_BLTouch ALL
    COMMAND ${PLATFORMIO_CMD} -f -c clion run $<$<CONFIG:Verbose>:--verbose> -e advi3pp-52c-bltouch
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    ADVi3++_5.2C_BLTouch3 ALL
    COMMAND ${PLATFORMIO_CMD} -f -c clion run $<$<CONFIG:Verbose>:--verbose> -e advi3pp-52c-bltouch3
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_custom_target(
    ADVi3++_5.4 ALL
    COMMAND ${PLATFORMIO_CMD} -f -c clion run $<$<CONFIG:Verbose>:--verbose> -e advi3pp-54
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

add_executable(${PROJECT_NAME} ${SRC_LIST})
