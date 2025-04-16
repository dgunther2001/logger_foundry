project(logger_daemon)

add_library(buffer_parsing_lib
    buffer_parser.cpp
)

target_include_directories(buffer_parsing_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
        $<INSTALL_INTERFACE:include>
)

install(TARGETS buffer_parsing_lib
    EXPORT logger_foundryTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)