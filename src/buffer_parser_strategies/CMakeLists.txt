project(logger_daemon)

add_library(buffer_parsing_strategies_lib
    default_pyroxene_strategy/default_pyroxene_strategy.cpp
)

target_include_directories(buffer_parsing_strategies_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})