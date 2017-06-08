cmake_minimum_required(VERSION 2.8.11)

if(CMAKE_VERSION VERSION_GREATER 3.2)
  cmake_policy(VERSION 3.2)
  project(fuzzyinput VERSION 0.1 LANGUAGES CXX)
else()
  project(fuzzyinput CXX)
endif()

if(CMAKE_VERSION VERSION_LESS 3.0)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++1z")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)

# Boost
find_package(Boost REQUIRED)

# sources
set(fuzzyinput_SOURCES libfuzzyinput/regexrandomgenerator.cpp)

# libfuzzyinput
add_library(fuzzyinput STATIC ${fuzzyinput_SOURCES})
target_link_libraries(fuzzyinput strtox-regex)
target_include_directories(fuzzyinput INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}")
target_include_directories(fuzzyinput SYSTEM PUBLIC ${Boost_INCLUDE_DIRS})

if(CMAKE_VERSION VERSION_GREATER 3.0)
  set_property(TARGET fuzzyinput PROPERTY CXX_STANDARD 14)
  set_property(TARGET fuzzyinput PROPERTY CXX_STANDARD_REQUIRED 17)
endif()

# Testing
add_subdirectory(tests)

