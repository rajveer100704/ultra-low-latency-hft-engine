cmake_minimum_required(VERSION 3.20)
project(HFT_System LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_options(/W3 /Ot /O2 /GL /EHa- /GR-)
    add_link_options(/LTCG)
else()
    add_compile_options(-O3 -march=native -flto -fno-exceptions -fno-rtti)
endif()

include(FetchContent)

# Fetch simdjson
FetchContent_Declare(
    simdjson
    GIT_REPOSITORY https://github.com/simdjson/simdjson.git
    GIT_TAG v3.6.0
)
FetchContent_MakeAvailable(simdjson)

# Fetch spdlog
FetchContent_Declare(
    spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog.git
    GIT_TAG v1.13.0
)
FetchContent_MakeAvailable(spdlog)

# Find Boost (assuming it might be installed)
set(Boost_USE_STATIC_LIBS ON)
find_package(Boost QUIET COMPONENTS system thread)

if(NOT Boost_FOUND)
    message(STATUS "Boost not found, skipping Boost features.")
endif()

# Find Curses/ncurses
find_package(Curses QUIET)
if(CURSES_FOUND)
    set(NCURSES_FOUND ON)
    message(STATUS "Curses found: ${CURSES_LIBRARIES}")
endif()


# Common include directories
# Using project-relative paths everywhere
include_directories(${CMAKE_CURRENT_SOURCE_DIR})


# Add subdirectories
add_subdirectory(core)
add_subdirectory(feed)
add_subdirectory(parser)
add_subdirectory(orderbook)

add_subdirectory(strategy)
add_subdirectory(risk)
add_subdirectory(execution)
add_subdirectory(pnl)
add_subdirectory(ui)
add_subdirectory(replay)


# OMS Module
add_library(oms oms/order_manager.cpp)
target_link_libraries(oms PRIVATE core risk spdlog::spdlog)

# Exchange Module
add_library(exchange exchange/matching_engine.cpp)
target_link_libraries(exchange PRIVATE core orderbook spdlog::spdlog)

# Main executable
add_executable(hft_main main.cpp)
target_link_libraries(hft_main PRIVATE 
    core 
    feed 
    parser 
    orderbook 
    strategy 
    risk 
    execution 
    pnl 
    ui
    oms
    exchange
    replay
    simdjson::simdjson 
    spdlog::spdlog
)

if(Boost_FOUND)
    target_link_libraries(hft_main PRIVATE Boost::system Boost::thread)
    target_compile_definitions(hft_main PRIVATE USE_BOOST)
endif()

add_executable(benchmarks tests/benchmarks.cpp)
target_link_libraries(benchmarks PRIVATE orderbook core spdlog::spdlog)

if(NCURSES_FOUND)
    target_link_libraries(hft_main PRIVATE ncurses)
endif()



