add_library(parser STATIC
    trade_parser.cpp
)

target_link_libraries(parser PUBLIC 
    core 
    simdjson::simdjson
    spdlog::spdlog
)

target_include_directories(parser PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
