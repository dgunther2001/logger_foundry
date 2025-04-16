project(logger_daemon)

add_library(log_writer_lib
    log_writer.cpp
)

target_include_directories(log_writer_lib 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
        $<INSTALL_INTERFACE:include>
)

install(TARGETS log_writer_lib
    EXPORT logger_foundryTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)