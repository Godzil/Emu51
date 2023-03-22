cmake_minimum_required(VERSION 3.11)

# External cmake modules
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/external/cmake ${CMAKE_MODULE_PATH})
include(GetGitRevisionDescription)

set(CMAKE_CXX_STANDARD 11)

project(Emu51)

git_describe(VERSION --tags --dirty=-dirty)
message("-- Building version ${VERSION}")

add_compile_definitions(VERSION="${VERSION}")

file(GLOB EMU51_SOURCES *.cpp)
file(GLOB EMU51_HEADERS include/*.h)

add_executable(Emu51)

target_sources(Emu51 PRIVATE ${EMU51_SOURCES} ${EMU51_HEADERS})
target_include_directories(Emu51 PRIVATE include)
target_link_libraries(Emu51 alleg)
