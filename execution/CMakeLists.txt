add_library(execution STATIC
    execution_engine.cpp
)

target_link_libraries(execution PUBLIC 
    core 
    spdlog::spdlog
)

target_include_directories(execution PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
