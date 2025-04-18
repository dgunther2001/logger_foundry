add_library(input_socket_lib
    input_socket.cpp
)

target_include_directories(input_socket_lib
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../../include>
        $<INSTALL_INTERFACE:include>
)

install(TARGETS input_socket_lib
    EXPORT logger_foundryTargets
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)