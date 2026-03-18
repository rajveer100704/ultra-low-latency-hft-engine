add_library(orderbook STATIC
    order_book.cpp
)

target_link_libraries(orderbook PUBLIC 
    core
)

target_include_directories(orderbook PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
