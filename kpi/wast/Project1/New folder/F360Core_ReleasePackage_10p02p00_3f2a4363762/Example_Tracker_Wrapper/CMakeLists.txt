cmake_minimum_required(VERSION 3.16.3)
project(Example_Wrapper_Test VERSION 0.1.1)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if (MSVC)
    add_compile_options(
        "$<$<CONFIG:DEBUG>:/MTd>" # taken from raw VS
        "$<$<CONFIG:RELEASE>:/MT>" # taken from raw VS
        "$<$<CONFIG:RELEASE>:/Gy>" # taken from raw VS
        "$<$<CONFIG:RELEASE>:/Oi>" # taken from raw VS
        "$<$<CONFIG:RELEASE>:/GL>" # taken from raw VS
        /sdl- # taken from raw VS
        /W4 # taken from raw VS
        #/WX # taken from raw VS
        /Gm- # taken from raw VS
        /MP # add parallell build
    )
    
    add_link_options(
        "$<$<CONFIG:RELEASE>:/LTCG>" # taken from raw VS
    )
    
    add_compile_definitions(
        _SCL_SECURE_NO_WARNINGS # taken from raw VS
        _CRT_SECURE_NO_WARNINGS # taken from raw VS
        $<$<CONFIG:DEBUG>:_DEBUG>
    )
endif()

add_executable(Example_Wrapper_Test wrapper_lib_test.cpp)

add_subdirectory(example_wrapper_lib)
 

target_link_libraries(Example_Wrapper_Test
                      PUBLIC
                      example_wrapper_lib)



