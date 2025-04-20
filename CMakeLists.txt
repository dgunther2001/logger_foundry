cmake_minimum_required(VERSION 3.15)
project(logger_foundry VERSION 1.0.0)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    message(FATAL_ERROR "CMAKE_INSTALL_PREFIX not set. Specify with -DCMAKE_INSTALL_PREFIX")
endif()

add_subdirectory(src)

install(TARGETS logger_foundry
    EXPORT logger_foundryTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

install(EXPORT logger_foundryTargets
    FILE logger_foundryTargets.cmake
    NAMESPACE logger_foundry::
    DESTINATION lib
)

install(#DIRECTORY include/logger_foundry
    FILES ${CMAKE_CURRENT_SOURCE_DIR}/config_headers/socket_config_structs.h
    #$<INSTALL_INTERFACE:include>
    DESTINATION include
    #FILES_MATCHING PATTERN "*.h"
)

install(DIRECTORY include/logger_foundry
    DESTINATION include
)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/logger_foundryConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/logger_foundryConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/logger_foundryConfig.cmake"
    INSTALL_DESTINATION lib
)

install(FILES
    "${CMAKE_CURRENT_BINARY_DIR}/logger_foundryConfig.cmake"
    "${CMAKE_CURRENT_BINARY_DIR}/logger_foundryConfigVersion.cmake"
    DESTINATION lib
)

message(STATUS "Libraries found in ${CMAKE_INSTALL_PREFIX}/lib: ${ALL_LIBS}")
