add_library(ui STATIC
    terminal_ui.cpp
)

target_link_libraries(ui PUBLIC 
    core 
    orderbook
)

if(NCURSES_FOUND)
    target_link_libraries(ui PUBLIC ${CURSES_LIBRARIES})
    target_include_directories(ui PUBLIC ${CURSES_INCLUDE_DIRS})
    target_compile_definitions(ui PUBLIC USE_NCURSES)
endif()

target_include_directories(ui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
