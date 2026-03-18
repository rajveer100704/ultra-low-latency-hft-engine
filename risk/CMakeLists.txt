add_library(risk STATIC
    risk_engine.cpp
)

target_link_libraries(risk PUBLIC 
    core 
    spdlog::spdlog
)

target_include_directories(risk PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
