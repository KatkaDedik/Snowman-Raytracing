# ----------------------------------------------------------------------------
# This is the main configuration file for the PV227 seminars.
# ----------------------------------------------------------------------------

# Adds the required parts of the framework.
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../framework/core ${CMAKE_CURRENT_BINARY_DIR}/framework/core)


# Adds projects.
add_subdirectory(project_2021_02)
