add_subdirectory(buffer_parsing)
add_subdirectory(log_writer)
add_subdirectory(input_socket)
add_subdirectory(daemon_orchestrator)

add_library(logger_foundry
    logger_foundry.cpp
)

target_include_directories(logger_foundry
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../include>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../config_headers>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/daemon_orchestrator>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(logger_foundry
    PRIVATE
        buffer_parsing_lib
        log_writer_lib
        input_socket_lib
        daemon_orchestrator_lib
)

set_target_properties(logger_foundry PROPERTIES
    OUTPUT_NAME logger_foundry
)
