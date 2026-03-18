add_library(replay STATIC
    market_data_recorder.cpp
    replay_engine.cpp
)

target_link_libraries(replay PRIVATE core spdlog::spdlog)
target_include_directories(replay PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
