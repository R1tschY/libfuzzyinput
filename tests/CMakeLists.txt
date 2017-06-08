# Boost Test
find_package(Boost COMPONENTS unit_test_framework REQUIRED)
add_definitions(-DBOOST_TEST_DYN_LINK)

# c++17
if(CMAKE_VERSION VERSION_LESS 3.0)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++1z")
endif()

# Warnings
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Werror -Wshadow -Wnon-virtual-dtor -Wcast-align -Wunused -Woverloaded-virtual -Wcast-qual -Wconversion -Wold-style-cast -Wno-unused-parameter")

# Sources
file(GLOB_RECURSE test_fuzzyinput_SOURCES "*.cpp")

# test-fuzzyinput
add_executable(test-fuzzyinput ${test_fuzzyinput_SOURCES})
target_link_libraries(test-fuzzyinput fuzzyinput strtox-regex ${Boost_LIBRARIES})

if(CMAKE_VERSION VERSION_GREATER 3.0)
  set_property(TARGET test-fuzzyinput PROPERTY CXX_STANDARD 14)
  set_property(TARGET test-fuzzyinput PROPERTY CXX_STANDARD_REQUIRED 17)
endif()

# make check
#if (TARGET check)
#else()
#    add_custom_target(check)
#endif()
#add_custom_command(TARGET test-fuzzyinput
#                   POST_BUILD
#                   COMMAND test-fuzzyinput -i -s --detect_fp_exceptions=y -l message
#                   WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
#                   COMMENT "Running test-fuzzyinput ..." VERBATIM)
#add_dependencies(check test-fuzzyinput)


