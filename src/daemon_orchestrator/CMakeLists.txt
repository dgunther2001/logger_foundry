add_library(daemon_orchestrator_lib
    daemon_orchestrator.cpp
)

target_include_directories(daemon_orchestrator_lib 
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../buffer_parsing>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../input_socket>
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../log_writer>
        #$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
        $<INSTALL_INTERFACE:include>
)

target_link_libraries(daemon_orchestrator_lib
    PUBLIC log_writer_lib
    PUBLIC buffer_parsing_lib
    PUBLIC input_socket_lib
)

install(TARGETS daemon_orchestrator_lib
    EXPORT logger_foundryTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)