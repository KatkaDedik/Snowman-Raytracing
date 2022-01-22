#include "application.hpp"
#include "glm/gtx/color_space.inl"
#include "utils/utils.hpp"
#include "model_ubo.hpp"

Application::Application(int initial_width, int initial_height, std::vector<std::string> arguments)
    : DefaultApplication(initial_width, initial_height, arguments) {
    Application::compile_shaders();
    prepare_cameras();
    prepare_materials();
    prepare_textures();
    prepare_snowman();
    prepare_lights();
    prepare_scene();
    prepare_framebuffers();
}

Application::~Application() {
}

// ----------------------------------------------------------------------------
// Shaderes
// ----------------------------------------------------------------------------
void Application::compile_shaders() {
    default_unlit_program = ShaderProgram(shaders_path / "object.vert", shaders_path / "unlit.frag");
    default_lit_program = ShaderProgram(shaders_path / "object.vert", shaders_path / "lit.frag");
    
    particle_textured_program = ShaderProgram();
    particle_textured_program.add_vertex_shader(shaders_path / "particle_textured.vert");
    particle_textured_program.add_fragment_shader(shaders_path / "particle_textured.frag");
    particle_textured_program.add_geometry_shader(shaders_path / "particle_textured.geom");
    particle_textured_program.link();

    ray_tracing_program = ShaderProgram(shaders_path / "full_screen_quad.vert", shaders_path / "ray_tracing.frag");
    
    std::cout << "Shaders are reloaded." << std::endl;
}

// ----------------------------------------------------------------------------
// Initialize Scene
// ----------------------------------------------------------------------------
void Application::prepare_cameras() {
    // Sets the default camera position.
    camera.set_eye_position(glm::radians(-45.f), glm::radians(20.f), 25.f);
    // Computes the projection matrix.
    camera_ubo.set_projection(glm::perspective(glm::radians(45.f), static_cast<float>(this->width) / static_cast<float>(this->height), 1.0f, 1000.0f));
    camera_ubo.update_opengl_data();
}

void Application::prepare_materials() {
    const PBRMaterialData snow_material = PBRMaterialData(glm::vec3(1), glm::vec3(0.04f), 1.0f);
    const PBRMaterialData coal_material = PBRMaterialData(glm::vec3(0.1), glm::vec3(0.004f), 1.0f);
    const PBRMaterialData carrot_material = PBRMaterialData(glm::vec3(235.0f / 255.0f, 137.0f / 255.0f, 33.0f / 255.0f), glm::vec3(0.04f), 1.0f);
    snowman.materials[0] = snow_material;
    snowman.materials[1] = snow_material;
    snowman.materials[2] = snow_material;
    snowman.materials[3] = snow_material;
    snowman.materials[4] = snow_material;
    snowman.materials[5] = coal_material;
    snowman.materials[6] = coal_material;
    snowman.materials[7] = coal_material;
    snowman.materials[8] = coal_material;
    snowman.materials[9] = coal_material;
    snowman.materials[10] = carrot_material;
    snowman.materials[11] = carrot_material;
    snowman.materials[12] = carrot_material;
}

void Application::prepare_textures() {
    particle_tex = TextureUtils::load_texture_2d(textures_path / "snowflake.png");
    TextureUtils::set_texture_2d_parameters(particle_tex, GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

}

void Application::prepare_lights() {
    phong_lights_ubo = PhongLightsUBO(3, GL_UNIFORM_BUFFER);
    phong_lights_ubo.set_global_ambient(glm::vec3(0.2f));
}

void Application::prepare_snowman() {
    // Body
    snowman.spheres[0] = glm::vec4(0.0f, 1.2f, 0.0f, 1.5f);
    snowman.spheres[1] = glm::vec4(0.0f, 3.5f, 0.0f, 1.0f);
    snowman.spheres[2] = glm::vec4(0.0f, 4.9f, 0.0f, 0.7f);
    // Hands
    snowman.spheres[3] = glm::vec4(1.0f, 3.6f, 0.0f, 0.5f);
    snowman.spheres[4] = glm::vec4(-1.0f, 3.6f, 0.0f, 0.5f);
    // Eyes
    snowman.spheres[5] = glm::vec4(0.25f, 5.2f, 0.55f, 0.1f);
    snowman.spheres[6] = glm::vec4(-0.25f, 5.2f, 0.55f, 0.1f);
    // Coal
    snowman.spheres[7] = glm::vec4(0.0f, 3.9f, 0.9f, 0.1f);
    snowman.spheres[8] = glm::vec4(0.0f, 3.5f, 1.0f, 0.1f);
    snowman.spheres[9] = glm::vec4(0.0f, 3.1f, 0.9f, 0.1f);
    //carrot
    snowman.spheres[10] = glm::vec4(0.0f, 5.0f, 0.7f, 0.15f);
    snowman.spheres[11] = glm::vec4(0.0f, 5.0f, 0.85f, 0.12f);
    snowman.spheres[12] = glm::vec4(0.0f, 5.0f, 1.0f, 0.08f);
}

void Application::prepare_scene() {
    snowman_ubo = SnowmanUBO(snowman, GL_DYNAMIC_STORAGE_BIT);

    // Allocates GPU buffers.
    glCreateBuffers(1, &particle_positions_bo);
    glNamedBufferStorage(particle_positions_bo, sizeof(float) * 4 * max_snow_count, nullptr, GL_DYNAMIC_STORAGE_BIT);
   
    // Initialize positions and velocities, and uploads them into OpenGL buffers.
    reset_particles();

    glCreateVertexArrays(1, &particle_vao);

    glVertexArrayVertexBuffer(particle_vao, 0, particle_positions_bo, 0, 4 * sizeof(float));

    glEnableVertexArrayAttrib(particle_vao, 0);
    glVertexArrayAttribFormat(particle_vao, 0, 4, GL_FLOAT, GL_FALSE, 0);
    glVertexArrayAttribBinding(particle_vao, 0, 0);
}

void Application::prepare_framebuffers() {
}

void Application::resize_fullscreen_textures() {
}

// ----------------------------------------------------------------------------
// Update
// ----------------------------------------------------------------------------
void Application::update(float delta) {
    DefaultApplication::update(delta);

    // Updates the main camera.
    const glm::vec3 eye_position = camera.get_eye_position();
    camera_ubo.set_view(lookAt(eye_position, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)));
    camera_ubo.update_opengl_data();

    const float app_time_s = elapsed_time * 0.001;

    // Updates lights
    phong_lights_ubo.clear();
    std::vector<glm::vec3> positions(3);
    positions[0] = glm::vec3(4, 6, 4) * glm::vec3(cosf(app_time_s + 3.14), 1, sinf(app_time_s + 3.14));
    positions[1] = glm::vec3(4, 4, 4) * glm::vec3(cosf(app_time_s - 3.14 / 2.0), 1, sinf(app_time_s + 3.14 / 2.0));
    positions[2] = glm::vec3(5, 2, 5) * glm::vec3(cosf(app_time_s), 1, sinf(app_time_s));

    for (glm::vec3 light_position : positions) {
        PhongLightData light = PhongLightData::CreatePointLight(light_position, glm::vec3(0.0f), glm::vec3(1.0f), glm::vec3(0.1f), 1.0f, 0.0f, 0.0f);
        phong_lights_ubo.add(light);
    }
    phong_lights_ubo.update_opengl_data();

    if (desired_snow_count != current_snow_count) {
        current_snow_count = desired_snow_count;
        reset_particles();
    }
}

void Application::reset_particles() {
    // Sets the seed. We set it here to ensure that though all particles are at random places, they are always at the same places when they are reset.
    srand(69769);
    particle_positions.resize(desired_snow_count, glm::vec4(0.0f));
    // The points are uniformly distributed on the surface of a unit sphere, and their initial velocity is zero.
    for (int i = 0; i < current_snow_count; i++) {
        float x = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 60.0f - 30.0f;
        float y = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 60.0f;
        float z = static_cast<float>(rand()) / static_cast<float>(RAND_MAX) * 60.0f - 30.0f;

        particle_positions[i] = glm::vec4(x,y,z, 1.0f);
    }

    // Updates the OpenGL buffers.
    glNamedBufferSubData(particle_positions_bo, 0, sizeof(glm::vec4) * current_snow_count, particle_positions.data());
}


// ----------------------------------------------------------------------------
// Render
// ----------------------------------------------------------------------------
void Application::render() {
    // Starts measuring the elapsed time.
    glBeginQuery(GL_TIME_ELAPSED, render_time_query);

    // Binds the main window framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);

    // Clears the framebuffer color.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    //ray_tracing_gpu();

    // Binds the the camera and the lights buffers.
    camera_ubo.bind_buffer_base(CameraUBO::DEFAULT_CAMERA_BINDING);
    phong_lights_ubo.bind_buffer_base(PhongLightsUBO::DEFAULT_LIGHTS_BINDING);

    if (use_raytracing) {
        raytrace_snowman();
    }
    else {
        raster_snowman();
    }
    if (show_snow) {
        render_snow();
    }

    // Resets the VAO and the program.
    glBindVertexArray(0);
    glUseProgram(0);

    // Stops measuring the elapsed time.
    glEndQuery(GL_TIME_ELAPSED);

    // Waits for OpenGL - don't forget OpenGL is asynchronous.
    glFinish();

    // Evaluates the query.
    GLuint64 render_time;
    glGetQueryObjectui64v(render_time_query, GL_QUERY_RESULT, &render_time);
    fps_gpu = 1000.f / (static_cast<float>(render_time) * 1e-6f);
}

void Application::raytrace_snowman() {
    // Binds the main window framebuffer.
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, width, height);

    // Clears the framebuffer color.
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // We do not need depth and depth test.
    glDisable(GL_DEPTH_TEST);

    // Uses the proper program.
    ray_tracing_program.use();
    ray_tracing_program.uniform("resolution", glm::vec2(width, height));
    ray_tracing_program.uniform("spheres_count", static_cast<int>(13));
    ray_tracing_program.uniform("use_ambient_occlusion", use_ambient_occlusion);
    ray_tracing_program.uniform("iterations", reflections);
    ray_tracing_program.uniform("sphere_light_radius", sphere_light_radius);

    // Binds the data with the camera and the lights.
    camera_ubo.bind_buffer_base(CameraUBO::DEFAULT_CAMERA_BINDING);
    phong_lights_ubo.bind_buffer_base(PhongLightsUBO::DEFAULT_LIGHTS_BINDING);

    // Binds the buffers containing the information about the spheres (positions + radii and materials).
    //glBindBufferBase(GL_UNIFORM_BUFFER, 4, snowman_ubo);
    snowman_ubo.bind_buffer_base(4);

    // Renders the full screen quad to evaluate every pixel.
    // Binds an empty VAO as we do not need any state.
    glBindVertexArray(empty_vao);
    // Calls a draw command with 3 vertices that are generated in vertex shader.
    glDrawArrays(GL_TRIANGLES, 0, 3);
}


void Application::render_snow() {
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);

    particle_textured_program.use();
    particle_textured_program.uniform("particle_size_vs", 0.2f);
    particle_textured_program.uniform("time", static_cast<float>(elapsed_time));
    glBindTextureUnit(0, particle_tex);

    // Binds the proper VAO (we use the VAO with the data we just wrote).
    glBindVertexArray(particle_vao);
    // Draws the particles as points.
    glPointSize(1.0f);
    glDrawArrays(GL_POINTS, 0, current_snow_count);

    glDisable(GL_BLEND);

}

void Application::raster_snowman() {
    int id = 0;
    // Renders the snowman
    for (glm::vec4 sph : snowman.spheres) {
        default_lit_program.use();

        ModelUBO model_ubo(translate(glm::mat4(1.0f), glm::vec3(sph)) * scale(glm::mat4(1.0f), glm::vec3(sph.w)));

        // Handles the textures.
        default_lit_program.uniform("has_texture", false);
        default_lit_program.uniform("use_ambient_occlusion", use_ambient_occlusion);
        snowman_ubo.bind_buffer_base(4);
        glBindTextureUnit(0, 0);

        // Note that the material are hard-coded here since the default lit shader works with PhongMaterial not PBRMaterial as defined in snowman.
        if (id < 5) {
            white_material_ubo.bind_buffer_base(PhongMaterialUBO::DEFAULT_MATERIAL_BINDING);
        } else if (id < 10){
            black_material_ubo.bind_buffer_base(PhongMaterialUBO::DEFAULT_MATERIAL_BINDING);
        }
        else {
            red_material_ubo.bind_buffer_base(PhongMaterialUBO::DEFAULT_MATERIAL_BINDING);
        }
        model_ubo.bind_buffer_base(ModelUBO::DEFAULT_MODEL_BINDING);
        sphere.bind_vao();
        sphere.draw();
        id++;
    }

    // Render the floor
    white_material_ubo.bind_buffer_base(PhongMaterialUBO::DEFAULT_MATERIAL_BINDING);
    ModelUBO model_ubo(translate(glm::mat4(1.0f), glm::vec3(0.0f, -0.1f, 0.0f)) * scale(glm::mat4(1.0f), glm::vec3(30.0f, 0.1f, 30.0f)));
    model_ubo.bind_buffer_base(ModelUBO::DEFAULT_MODEL_BINDING);
    cube.bind_vao();
    cube.draw();

    // Renders the lights.
    for (int i = 0; i < 3; i++) {
        default_unlit_program.use();

        ModelUBO model_ubo(translate(glm::mat4(1.0f), glm::vec3(phong_lights_ubo.get_light(i).position)) * scale(glm::mat4(1.0f), glm::vec3(0.1f)));

        // Note that the material are hard-coded here since the default lit shader works with PhongMaterial not PBRMaterial as defined in snowman.
        white_material_ubo.bind_buffer_base(PhongMaterialUBO::DEFAULT_MATERIAL_BINDING);
        model_ubo.bind_buffer_base(ModelUBO::DEFAULT_MODEL_BINDING);
        sphere.bind_vao();
        sphere.draw();
    }
}

// ----------------------------------------------------------------------------
// GUI
// ----------------------------------------------------------------------------
void Application::render_ui() {
    const float unit = ImGui::GetFontSize();

    ImGui::Begin("Settings", nullptr, ImGuiWindowFlags_NoDecoration);
    ImGui::SetWindowSize(ImVec2(20 * unit, 17 * unit));
    ImGui::SetWindowPos(ImVec2(2 * unit, 2 * unit));

    ImGui::PushItemWidth(150.f);

    std::string fps_cpu_string = "FPS (CPU): ";
    ImGui::Text(fps_cpu_string.append(std::to_string(fps_cpu)).c_str());

    std::string fps_string = "FPS (GPU): ";
    ImGui::Text(fps_string.append(std::to_string(fps_gpu)).c_str());

    ImGui::SliderInt("Reflections Quality", &reflections, 1, 100);

    const char* particle_labels[10] = {"256", "512", "1024", "2048", "4096", "8192", "16384", "32768", "65536", "131072"};
    int exponent = static_cast<int>(log2(current_snow_count) - 8); // -8 because we start at 256 = 2^8
    if (ImGui::Combo("Particle Count", &exponent, particle_labels, IM_ARRAYSIZE(particle_labels))) {
        desired_snow_count = static_cast<int>(glm::pow(2, exponent + 8)); // +8 because we start at 256 = 2^8
    }

    ImGui::Checkbox("Show Snow", &show_snow);

    ImGui::Checkbox("Ambient Occlusion", &use_ambient_occlusion);
    ImGui::Checkbox("Raytracing", &use_raytracing);

    ImGui::SliderFloat("Sphere Light Radius", &sphere_light_radius, 0, 1, "%.1f");
    ImGui::SliderInt("Shadow Quality", &shadow_samples, 1, 128);

    ImGui::Checkbox("Corrective: Smooth Shadow Edges", &corrective_smooth_shadows);
    ImGui::Checkbox("Corrective: Rectangular Area Light", &corrective_rectangular_area_light);

    ImGui::End();
}

// ----------------------------------------------------------------------------
// Input Events
// ----------------------------------------------------------------------------
void Application::on_resize(int width, int height) {
    DefaultApplication::on_resize(width, height);
    resize_fullscreen_textures();
}
