#pragma once

// Prevents the GLFW header from including the OpenGL header.
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

// Includes the most common headers.
#include <filesystem>
#include <string>
#include <vector>

// Includes the most important GLM functions.
#include "imgui.h"
#include "utils/configuration.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

// Includes the mathematical definitions.
#define _USE_MATH_DEFINES
#include <math.h>

// Includes the custom headers.
#include "camera.hpp"
#include "program.hpp"

/**
 * The base class for all OpenGL windows.
 *
 * @author	<a href="mailto:jan.byska@gmail.com">Jan Byška</a>
 * @author	<a href="mailto:matus.talcik@gmail.com">Matúš Talčík</a>
 */
class IApplication {

  protected:
    /** The current window width. */
    GLsizei width;

    /** The current window height. */
    GLsizei height;

    /** Command-line arguments. */
    std::vector<std::string> arguments;

    /** The configuration loaded from a file configuration.toml. */
    Configuration configuration;

    /**
     * Path to framework's shaders. Loaded from {@link configuration} if a configuration file is available.
     * Otherwise working directory of a binary is used.
     */
    std::filesystem::path framework_shaders_path;

    /**
     * Path to applications shaders. Loaded from {@link configuration} if a configuration file is available.
     * Otherwise working directory of a binary is used.
     */
    std::filesystem::path shaders_path;

    /**
     * Path to framwork textures that is loaded from {@link configuration} if a configuration file is available.
     * Otherwise working directory of a binary is used.
     */
    std::filesystem::path framework_textures_path;

    /**
     * Path to application textures that is loaded from {@link configuration} if a configuration file is available.
     * Otherwise working directory of a binary is used.
     */
    std::filesystem::path textures_path;

    /** The application window. */
    GLFWwindow* window = nullptr;

    /** The default camera. */
    Camera camera;

  public:
    // ----------------------------------------------------------------------------
    // Constructors
    // ----------------------------------------------------------------------------

    /**
     * Constructs a new @link IApplication with a custom width and height.
     *
     * @param 	initial_width 	The initial width of the window.
     * @param 	initial_height	The initial height of the window.
     * @param 	arguments	  	The arguments used to obtain the application directory.
     */
    IApplication(int initial_width, int initial_height, std::vector<std::string> arguments);

    // ----------------------------------------------------------------------------
    // Input Events
    // ----------------------------------------------------------------------------
    /**
     * This method is invoked when the window size changes.
     *
     * The implementation is left to the child class.
     *
     * @param 	width 	The new width.
     * @param 	height	The new height.
     */
    virtual void on_resize(int width, int height);

    /**
     * This method is invoked when the mouse cursor is moved.
     *
     * The implementation is left to the child class.
     *
     * @param 	x	The X-coordinate of the cursor relative to the upper-left corner of the window.
     * @param 	y	The Y-coordinate of the cursor relative to the upper-left corner of the window.
     */
    virtual void on_mouse_move(double x, double y);

    /**
     * This method is invoked when a mouse button is pressed or released.
     *
     * The implementation is left to the child class.
     *
     * @param 	button	The mouse button that was pressed or released.
     * @param 	action	One of GLFW_PRESS or GLFW_RELEASE.
     * @param 	mods  	Bit field describing which modifier keys were held down.
     */
    virtual void on_mouse_button(int button, int action, int mods);

    /**
     * This method is invoked when a keyboard key is pressed, released, or repeated.
     * The default implementation passes the input to ImGui.
     *
     * @param 	key		    The keyboard key that was pressed or released.
     * @param 	scancode	The system-specific scancode of the key.
     * @param 	action  	One of GLFW_PRESS, GLFW_RELEASE or GLFW_REPEAT.
     * @param 	mods		Bit field describing which modifier keys were held down.
     */
    virtual void on_key_pressed(int key, int scancode, int action, int mods);

    // ----------------------------------------------------------------------------
    // Shaders
    // ----------------------------------------------------------------------------

    /**
     * Compiles shaders and deletes previous ones (if necessary).
     */
    virtual void compile_shaders() {}

    // ----------------------------------------------------------------------------
    // Update
    // ----------------------------------------------------------------------------

    /**
     * This method is invoked from within the infinite OpenGL loop. The purpose of this method is to update the
     * application specific data before rendering the content.
     *
     * @param 	delta	The elapsed time between this and the previous frame (in milliseconds).
     */
    virtual void update(float delta) = 0;

    // ----------------------------------------------------------------------------
    // Render
    // ----------------------------------------------------------------------------
    /**
     * This method is invoked from within the infinite OpenGL loop. The main purpose of this method is to
     * render the application specific content.
     */
    virtual void render() = 0;

    // ----------------------------------------------------------------------------
    // GUI
    // ----------------------------------------------------------------------------
    /**
     * This method is invoked from within the infinite OpenGL loop. The main purpose of this method is to
     * render the application specific GUI.
     */
    virtual void render_ui() = 0;

    // ----------------------------------------------------------------------------
    // Methods
    // ----------------------------------------------------------------------------
  public:
    /**
     * Activates a custom frame buffer. The methods also automatically sets the full screen size viewport and possibly
     * also clears the buffer and sets the depth test.
     *
     * @param 	buffer	  	The buffer to activate (0 is the main screen).
     * @param 	clear	  	If @p true the bounded frame buffer will be cleared.
     * @param 	depth_test	if @p true the depth test will be activated; if @p false the depth test will be deactivated.
     */
    void activate_render_buffer(GLint buffer = 0, bool clear = true, bool depth_test = true) const;

    /**
     * Activates a custom-sized viewport.
     *
     * @param 	x	   	The X-coordination of the lower left corner of the viewport rectangle (0 by default).
     * @param 	y	   	The Y-coordination of the lower left corner of the viewport rectangle (0 by default).
     * @param 	width	The width of the viewport (value<=0 is used to indicate full screen).
     * @param 	height	The height of the viewport (value<=0 is used to indicate full screen).
     */
    void activate_viewport(GLint x = 0, GLint y = 0, GLsizei width = 0, GLsizei height = 0) const;

    /**
     * Activates a viewport such that it is positioned within a grid.
     *
     * @param 	x	  	The grid x coordinate (position of the requested viewport in the grid).
     * @param 	y	  	The grid y coordinate (position of the requested viewport in the grid).
     * @param 	grid_size_x 	The grid maximum grid size (the assumed number of viewports along x axis).
     * @param 	grid_size_y	The grid maximum grid size (the assumed  number of viewports along y axis).
     */
    void activate_viewport_grid(int x, int y, int grid_size_x, int grid_size_y) const;

    /**
     * Prepares the OpenGL context for rendering. A convenience method for calling @link activate_render_buffer and
     * @link activate_viewport.
     *
     * @param 	x		  	The X-coordination of the lower left corner of the viewport rectangle (0 by default).
     * @param 	y		  	The Y-coordination of the lower left corner of the viewport rectangle (0 by default).
     * @param 	width	  	The width of the viewport (value<=0 is used to indicate full screen).
     * @param 	height	  	The height of the viewport (value<=0 is used to indicate full screen).
     * @param 	buffer	  	The buffer to activate (0 is the main screen).
     * @param 	clear	  	If @p true the bounded frame buffer will be cleared.
     * @param 	depth_test	if @p true the depth test will be activated; if @p false the depth test will be deactivated.
     */
    void prepare_rendering(int x = 0, int y = 0, int width = 0, int height = 0, GLint buffer = 0, bool clear = true, bool depth_test = true) const;

    /**
     * Prepares the OpenGL context for rendering withing a grid of viewports. A convenience method for calling @link
     * activate_render_buffer and
     * @link activate_viewport.
     *
     * @param 	x		   	The grid x coordinate (position of the requested viewport in the grid).
     * @param 	y		   	The grid y coordinate (position of the requested viewport in the grid).
     * @param 	grid_size_x	The grid maximum grid size (the assumed number of viewports along x axis).
     * @param 	grid_size_y	The grid maximum grid size (the assumed  number of viewports along y axis).
     * @param 	buffer	   	The buffer to activate (0 is the main screen).
     * @param 	clear	   	If @p true the bounded frame buffer will be cleared.
     * @param 	depth_test 	if @p true the depth test will be activated; if @p false the depth test will be deactivated.
     */
    void prepare_rendering_grid(int x, int y, int grid_size_x, int grid_size_y, GLint buffer = 0, bool clear = false, bool depth_test = true) const;

    /**
     * Uploads a projection and view matrices to a specified program. Note that the method also binds the program.
     *
     * @param 	program   	The program.
     * @param 	projection	The projection matrix.
     * @param 	view	  	The view matrix.
     */
    void upload_projection_view(const ShaderProgram& program, glm::mat4 projection, glm::mat4 view) const;

    // ----------------------------------------------------------------------------
    // Getters & Setters
    // ----------------------------------------------------------------------------
    /**
     * Returns the current window width.
     *
     * @return	the window width.
     */
    size_t get_width() const;

    /**
     * Returns the current window height
     *
     * @return	the window height.
     */
    size_t get_height() const;

    /** Returns the default path for the framework shaders. */
    std::filesystem::path get_framework_shaders_path() const;

    /**
     * Sets a new GLFW window corresponding to this application.
     *
     * @param 	window	The GLFW window to set.
     */
    void set_window(GLFWwindow* window);
};
