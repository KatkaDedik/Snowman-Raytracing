# Sets the minimum required version of cmake.
cmake_minimum_required(VERSION 3.14)

# Sets the name of the project, and stores it in the variable PROJECT_NAME.
# CXX specified which programming language (i.e., C++) will be used.
project(main CXX)

# Adds all required subdirectories to the build.
add_subdirectory(pv227_seminars)

# Below is some code that can be used later if you need your own libraries outside vcpgk.
# include(FetchContent)
# FetchContent_Declare(range-v3
#   GIT_REPOSITORY https://github.com/ericniebler/range-v3.git
#   GIT_TAG 4989f3e9ff2efee1852942bb9328ef121369ba02
# )
# FetchContent_MakeAvailable(Range-v3)