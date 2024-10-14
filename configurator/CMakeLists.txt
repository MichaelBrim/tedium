cmake_minimum_required(VERSION 3.20 FATAL_ERROR)

project(Configurator)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_C_STANDARD 99)

# Download all the necessary third parties
include(FetchContent)

FetchContent_Declare(
        inih
        GIT_REPOSITORY https://github.com/benhoyt/inih.git
        GIT_TAG        r58
)

FetchContent_Declare(
        tinyexpr
        GIT_REPOSITORY https://github.com/codeplea/tinyexpr.git
        GIT_TAG        master
)
FetchContent_Declare(
        nanojsonc
        GIT_REPOSITORY https://github.com/open-source-patterns/nanojsonc.git
        GIT_TAG        1.0.0
)

FetchContent_MakeAvailable(tinyexpr inih nanojsonc)

# Combine sources for easier access
set(configurator_sources configurator.h configurator.c)

# Build all third party libraries
add_library(inih_lib STATIC ${inih_SOURCE_DIR}/ini.c)
target_include_directories(inih_lib PUBLIC ${inih_SOURCE_DIR})

add_library(tinyexpr_lib STATIC ${tinyexpr_SOURCE_DIR}/tinyexpr.c)
target_include_directories(tinyexpr_lib PUBLIC ${tinyexpr_SOURCE_DIR})

file(GLOB nanojsonc_SOURCES ${nanojsonc_SOURCE_DIR}/src/*)
add_library(nanojsonc_lib STATIC ${nanojsonc_SOURCES})
target_compile_features(nanojsonc PRIVATE c_std_11)
target_include_directories(nanojsonc_lib PUBLIC ${nanojsonc_SOURCE_DIR}/include)

# Combine third party libraries for easier access
set(NEEDED_LIBS inih_lib tinyexpr_lib nanojsonc_lib)

add_library(configurator STATIC ${configurator_sources})
target_link_libraries(configurator PRIVATE ${NEEDED_LIBS})

# Configurator_cpp target
add_executable(Configurator_cpp testpp.cpp)
target_include_directories(Configurator_cpp PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Configurator_cpp PRIVATE  configurator ${NEEDED_LIBS})

# Configurator_c target
add_executable(Configurator_c ${configurator_sources} test.c)
target_include_directories(Configurator_c PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(Configurator_c PRIVATE configurator ${NEEDED_LIBS} m)
