# ----------------------------------------------------------------------------
# This is the configuration file for the FRAMEWORK_CORE module. 
# The FRAMEWORK_CORE module provides the basic classes and method used in all courses.
# ----------------------------------------------------------------------------

# Sets the minimum required version of cmake.
cmake_minimum_required(VERSION 3.14)

# Sets the name of the project, and stores it in the variable PROJECT_NAME.
project(FRAMEWORK_CORE CXX)

# Sets up a new target if it was not already registered.
if(NOT TARGET ${PROJECT_NAME})
    # Adds a new library target.
    add_library(${PROJECT_NAME} STATIC)

    # Sets the build properties. CXX_STANDARD - the C++ standard whose features are requested. CXX_EXTENSIONS - no compiler specific
    # extensions are requested.
    set_target_properties(
        ${PROJECT_NAME}
        PROPERTIES CXX_STANDARD 20
                   CXX_EXTENSIONS OFF
    )

    if(MSVC)
        # force MSVC to define correct values for __cplusplus macro
        target_compile_options(
            ${PROJECT_NAME}
            PUBLIC "/Zc:__cplusplus"
        )
    endif()

    # Finds the external libraries and load their settings.
    find_package(glfw3 CONFIG REQUIRED)
    find_package(glad CONFIG REQUIRED)
    find_package(imgui CONFIG REQUIRED)
    find_package(glm CONFIG REQUIRED)
    find_package(toml11 CONFIG REQUIRED)
    find_package(tinyobjloader CONFIG REQUIRED)
    find_package(GTest CONFIG REQUIRED)
    find_path(STB_INCLUDE_DIRS "stb.h")

    # Specifies libraries to use when linking the library target defined above.
    if (TARGET glm)
        target_link_libraries(
            ${PROJECT_NAME}
            PUBLIC glad::glad glfw imgui::imgui glm toml11::toml11 tinyobjloader::tinyobjloader GTest::gtest 
        )
    endif()

    if (TARGET glm::glm)
        target_link_libraries(
            ${PROJECT_NAME}
            PUBLIC glad::glad glfw imgui::imgui glm::glm toml11::toml11 tinyobjloader::tinyobjloader GTest::gtest
        )
    endif()

    # Specifies the include directories to use when compiling the library target.
    target_include_directories(
        ${PROJECT_NAME}
        PUBLIC include include/opengl include/scene include/geometry geometries ${STB_INCLUDE_DIRS}
    )

    # Collects the source files and specifis them as target sources.
    target_sources(
        ${PROJECT_NAME}
        PRIVATE include/camera.hpp
                include/color.hpp
                include/default_application.hpp
                include/iapplication.hpp
                include/manager.hpp
                include/template.hpp
                include/opengl/opengl_object.hpp
                include/opengl/program.hpp
                include/opengl/program_map.hpp
                include/opengl/shader.hpp
                include/opengl/texture.hpp
                include/opengl/ubo.hpp
                include/scene/advanced_scene_object.hpp
                include/scene/camera_ubo.hpp
                include/scene/light_ubo.hpp
                include/scene/material_ubo.hpp
                include/scene/model_ubo.hpp
                include/scene/pbr_material_ubo.hpp
                include/scene/phong_material_ubo.hpp
                include/scene/scene_object.hpp
                include/utils/configuration.hpp
                include/utils/utils.hpp
                include/geometry/capsule.hpp
                include/geometry/cube.hpp
                include/geometry/cylinder.hpp
                include/geometry/geometry.hpp
                include/geometry/geometry_base.hpp
                include/geometry/sphere.hpp
                include/geometry/teapot.hpp
                include/geometry/torus.hpp
                src/camera.cpp
                src/color.cpp
                src/default_application.cpp
                src/iapplication.cpp
                src/manager.cpp
                src/geometry/geometry.cpp
                src/geometry/geometry_base.cpp
                src/opengl/program.cpp
                src/opengl/program_map.cpp
                src/opengl/shader.cpp
                src/opengl/texture.cpp
                src/utils/utils.cpp)
endif()