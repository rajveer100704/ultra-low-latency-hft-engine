add_library(feed STATIC
    websocket_client.cpp
)

target_link_libraries(feed PRIVATE 
    core 
    spdlog::spdlog
)

if(USE_BOOST)
    target_link_libraries(feed PRIVATE Boost::system Boost::thread)
endif()

target_include_directories(feed PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
