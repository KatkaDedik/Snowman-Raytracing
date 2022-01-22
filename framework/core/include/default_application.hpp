#pragma once

#include "capsule.hpp"
#include "cube.hpp"
#include "cylinder.hpp"
#include "iapplication.hpp"
#include "phong_material_ubo.hpp"
#include "sphere.hpp"
#include "teapot.hpp"
#include "torus.hpp"

class DefaultApplication : public IApplication {

    // ----------------------------------------------------------------------------
    // Variables
    // ----------------------------------------------------------------------------
protected:

    /** The default lit program. */
    ShaderProgram default_lit_program;

    /** The default unlit program. */
    ShaderProgram default_unlit_program;

    /** The elapsed time from the beginning (in milliseconds). */
    long double elapsed_time = 0;

    /** The OpenGL query object that we use to get render time of one frame. */
    GLuint render_time_query;

    /** The current FPS on GPU. */
    float fps_gpu;

    /** The current FPS on CPU. */
    float fps_cpu;

    // ----------------------------------------------------------------------------
    // Variables (Geometry)
    // ----------------------------------------------------------------------------
protected:
    /** The default pre-initialized cube geometry. */
    Cube cube;

    /** The default pre-initialized sphere geometry. */
    Sphere sphere;

    /** The default pre-initialized torus geometry. */
    Torus torus;

    /** The default pre-initialized cylinder geometry. */
    Cylinder cylinder;

    /** The default pre-initialized capsule geometry. */
    Capsule capsule;

    /** The default pre-initialized teapot geometry. */
    Teapot teapot;

    /** Empty VAO. */
    GLuint empty_vao;

    // ----------------------------------------------------------------------------
    // Variables (Materials)
    // ----------------------------------------------------------------------------
protected:
    /** The UBO defining the red material. */
    PhongMaterialUBO red_material_ubo;
    /** The UBO defining the green material. */
    PhongMaterialUBO green_material_ubo;
    /** The UBO defining the blue material. */
    PhongMaterialUBO blue_material_ubo;
    /** The UBO defining the cyan material. */
    PhongMaterialUBO cyan_material_ubo;
    /** The UBO defining the magenta material. */
    PhongMaterialUBO magenta_material_ubo;
    /** The UBO defining the yellow material. */
    PhongMaterialUBO yellow_material_ubo;
    /** The UBO defining the white material. */
    PhongMaterialUBO white_material_ubo;
    /** The UBO defining the black material. */
    PhongMaterialUBO black_material_ubo;
    /** The UBO defining the gray material. */
    PhongMaterialUBO gray_material_ubo;

    PhongMaterialUBO orange_material_ubo;

    // ----------------------------------------------------------------------------
    // Constructors
    // ----------------------------------------------------------------------------
public:
    /**
     * Constructs a new @link DefaultApplication with a custom width and height.
     * @param 	initial_width 	The initial width of the window.
     * @param 	initial_height	The initial height of the window.
     * @param 	arguments	  	The arguments used to obtain the application directory.
     */
    DefaultApplication(int initial_width, int initial_height, std::vector<std::string> arguments);

    /** The virtual destructor. */
    virtual ~DefaultApplication() {
    }

    // ----------------------------------------------------------------------------
    // Shaders
    // ----------------------------------------------------------------------------

    /**
     * {@copydoc IApplication::compile_shaders}
     */
    void compile_shaders() override;

    // ----------------------------------------------------------------------------
    // Update
    // ----------------------------------------------------------------------------
public:
    /**
     * {@copydoc IApplication::update}
     */
    void update(float delta) override;

    // ----------------------------------------------------------------------------
    // Render
    // ----------------------------------------------------------------------------
public:
    /**
     *  {@copydoc IApplication::render}
     */
    void render() override;

    // ----------------------------------------------------------------------------
    // GUI
    // ----------------------------------------------------------------------------
public:
    /**
     * {@copydoc IApplication::render_ui}
     */
    void render_ui() override {
    }

    // ----------------------------------------------------------------------------
    // Input Events
    // ----------------------------------------------------------------------------
public:
 
};
