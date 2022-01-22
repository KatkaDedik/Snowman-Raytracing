#pragma once
#include "camera_ubo.hpp"
#include "default_application.hpp"
#include "light_ubo.hpp"
#include "pbr_material_ubo.hpp"

/** The number of spheres forming the snowman. */
const int snowman_size = 10;

/** The structure defining the snowman. */
struct Snowman {
    glm::vec4 spheres[snowman_size];         // The spheres defining the snowman.
    PBRMaterialData materials[snowman_size]; // The respective materials for each sphere.
};

/** The definition of a snowman ubo. */
class SnowmanUBO : public UBO<Snowman> {
    using UBO<Snowman>::UBO; // copies constructors from the parent class
};

class Application : public DefaultApplication {
    // ----------------------------------------------------------------------------
    // Variables (Geometry)
    // ----------------------------------------------------------------------------
protected:
    /** The definition of the snowman. */
    Snowman snowman;
    /** The buffer with the snowman. */
    SnowmanUBO snowman_ubo;
    /** The positions of all particles (on GPU).*/
    GLuint particle_positions_bo;
    /**	The positions of all particles (on CPU).*/
    std::vector<glm::vec4> particle_positions;
    /** VAOs for rendering particles */
    GLuint particle_vao;

    // ----------------------------------------------------------------------------
    // Variables (Textures)
    // ----------------------------------------------------------------------------
    GLuint particle_tex;

protected:
    // ----------------------------------------------------------------------------
    // Variables (Light)
    // ----------------------------------------------------------------------------
protected:
    /** The UBO storing the data about lights - positions, colors, etc. */
    PhongLightsUBO phong_lights_ubo;
    // ----------------------------------------------------------------------------
    // Variables (Camera)
    // ----------------------------------------------------------------------------
protected:
    /** The UBO storing the information about camera. */
    CameraUBO camera_ubo;
    // ----------------------------------------------------------------------------
    // Variables (Shaders)
    // ----------------------------------------------------------------------------

    ShaderProgram particle_textured_program;

protected:
    // ----------------------------------------------------------------------------
    // Variables (Frame Buffers)
    // ----------------------------------------------------------------------------
protected:
    // ----------------------------------------------------------------------------
    // Variables (GUI)
    // ----------------------------------------------------------------------------
protected:
    /** The number of iterations. */
    int reflections = 3;

    const int max_snow_count = 131072;

    /** The desired snow particle count. */
    int desired_snow_count = 4096;

    /** The current snow particle count. */
    int current_snow_count = 256;

    /** The flag determining if a snow should be visible. */
    bool show_snow = true;

    /** The flag determining if the ambient occlusion should be used. */
    bool use_ambient_occlusion = true;

    /** The number of shadow samples. */
    int shadow_samples = 16;

    /** The flag determining if an area light should be present. */
    float sphere_light_radius = 0.5f;

    /** The flag determining if a blue noise should be used when sampling the spherical lights. */
    bool corrective_smooth_shadows = false;

    /** The flag determining if an rectangular area light should be used. */
    bool corrective_rectangular_area_light = false;

    // ----------------------------------------------------------------------------
    // Constructors
    // ----------------------------------------------------------------------------
public:
    Application(int initial_width, int initial_height, std::vector<std::string> arguments = {});

    /** Destroys the {@link Application} and releases the allocated resources. */
    virtual ~Application();

    // ----------------------------------------------------------------------------
    // Shaders
    // ----------------------------------------------------------------------------
    /**
     * {@copydoc DefaultApplication::compile_shaders}
     */
    void compile_shaders() override;

    // ----------------------------------------------------------------------------
    // Initialize Scene
    // ----------------------------------------------------------------------------
public:
    /** Prepares the required cameras. */
    void prepare_cameras();

    /** Prepares the required materials. */
    void prepare_materials();

    /** Prepares the required textures. */
    void prepare_textures();

    /** Prepares the lights. */
    void prepare_lights();

    /** Builds a snowman from individual parts. */
    void prepare_snowman();

    /** Prepares the scene objects. */
    void prepare_scene();

    /** Prepares the frame buffer objects. */
    void prepare_framebuffers();

    /** Resizes the full screen textures match the window. */
    void resize_fullscreen_textures();

    void reset_particles();

    // ----------------------------------------------------------------------------
    // Update
    // ----------------------------------------------------------------------------
    /**
     * {@copydoc DefaultApplication::update}
     */
    void update(float delta) override;

    // ----------------------------------------------------------------------------
    // Render
    // ----------------------------------------------------------------------------
public:
    /** @copydoc DefaultApplication::render */
    void render() override;

    void render_snow();

    /** Renders the snowman using rasterization. */
    void raster_snowman();
    // ----------------------------------------------------------------------------
    // GUI
    // ----------------------------------------------------------------------------
public:
    /** @copydoc DefaultApplication::render_ui */
    void render_ui() override;

    // ----------------------------------------------------------------------------
    // Input Events
    // ----------------------------------------------------------------------------
public:
    /** @copydoc DefaultApplication::on_resize */
    void on_resize(int width, int height) override;
};
