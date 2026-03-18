add_library(strategy STATIC
    strategy_engine.cpp
)

target_link_libraries(strategy PUBLIC 
    core 
    orderbook
    spdlog::spdlog
)

target_include_directories(strategy PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
