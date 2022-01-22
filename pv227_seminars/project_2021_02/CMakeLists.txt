# ----------------------------------------------------------------------------
# This is the configuration file for 'Project #2 (2021)'.
# ----------------------------------------------------------------------------

# Sets the name of the project, and stores it in the variable PROJECT_NAME.
project(pv227_project_2021_02 CXX)

# Adds a new executable target.
add_executable(
    ${PROJECT_NAME}
    application.hpp application.cpp main.cpp
)

# Sets the build properties.
# CXX_STANDARD - the C++ standard whose features are requested.
# CXX_EXTENSIONS - no compiler specific extensions are requested.
set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES CXX_STANDARD 20
               CXX_EXTENSIONS OFF
)

# Specifies libraries or flags to use when linking a given target and/or its dependents.
target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE FRAMEWORK_CORE
)

# Generates the configuration file.
file(
    GENERATE
    OUTPUT "$<TARGET_FILE_DIR:${PROJECT_NAME}>/configuration.toml"
    CONTENT "
framework_shaders = \"${CMAKE_SOURCE_DIR}/framework/core/shaders\"
framework_textures = \"${CMAKE_SOURCE_DIR}/framework/core/textures\"
shaders = \"${CMAKE_CURRENT_SOURCE_DIR}/shaders\"
textures = \"${CMAKE_CURRENT_SOURCE_DIR}/textures\"
"
)
