add_library(pnl STATIC
    pnl_engine.cpp
)

target_link_libraries(pnl PUBLIC 
    core
)

target_include_directories(pnl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
