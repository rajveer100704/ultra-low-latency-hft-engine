target_include_directories("${target}" ${public} [==[$<BUILD_INTERFACE:C:/Users/BIT/hft_system/build/_deps/simdjson-src/include>]==] ${private} [==[$<BUILD_INTERFACE:C:/Users/BIT/hft_system/build/_deps/simdjson-src/src>]==])
target_compile_features("${target}" ${public} [==[cxx_std_11]==])
target_link_libraries("${target}" ${public} [==[Threads::Threads]==])
target_compile_definitions("${target}" ${public} [==[SIMDJSON_THREADS_ENABLED=1]==])
