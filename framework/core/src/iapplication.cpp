#include "iapplication.hpp"
#include "utils/configuration.hpp"

// ----------------------------------------------------------------------------
// Constructors
// ----------------------------------------------------------------------------
IApplication::IApplication(int initial_width, int initial_height, std::vector<std::string> arguments)
    : width(initial_width), height(initial_height), arguments(arguments), configuration(Configuration(arguments[0])) {

    framework_shaders_path = configuration.get_path("framework_shaders", "/shaders");
    shaders_path = configuration.get_path("shaders", "/shaders");
    textures_path = configuration.get_path("textures", "/textures");
    framework_textures_path = configuration.get_path("framework_textures", "/textures");
}

// ----------------------------------------------------------------------------
// Input Events
// ----------------------------------------------------------------------------
void IApplication::on_resize(int width, int height) {
    this->width = std::max(width, 1);
    this->height = std::max(height, 1);

    glViewport(0, 0, width, height);
}

void IApplication::on_mouse_move(double x, double y) {
    // Consumes the event when interacting with GUI.
    if (!ImGui::GetIO().WantCaptureMouse) { 
        camera.on_mouse_move(x, y);
    }
}

void IApplication::on_mouse_button(int button, int action, int mods) {
    camera.on_mouse_button(button, action, mods);
}

void IApplication::on_key_pressed(int key, int scancode, int action, int mods) {
    ImGui::GetIO().KeysDown[key] = action;

    if (action == GLFW_PRESS) {
        switch (key) {
        case GLFW_KEY_R:
            compile_shaders();
            break;
        }
    }
}

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------
void IApplication::activate_render_buffer(GLint buffer, bool clear, bool depth_test) const {
    // Binds the frame buffer.
    glBindFramebuffer(GL_FRAMEBUFFER, buffer);

    // Clears the color and depth attachments (this is also fine if there is no color attachment).
    if (clear) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
    // Activates (or deactivates) the depth test.
    if (depth_test) {
        glEnable(GL_DEPTH_TEST);
    } else {
        glDisable(GL_DEPTH_TEST);
    }
}

void IApplication::activate_viewport(GLint x, GLint y, GLsizei width, GLsizei height) const {
    glViewport(x, y, width > 0 ? width : this->width, height > 0 ? height : this->height);
}

void IApplication::activate_viewport_grid(int x, int y, int grid_size_x, int grid_size_y) const {
    const int viewport_width = static_cast<int>(width) / grid_size_x;
    const int viewport_height = static_cast<int>(height) / grid_size_y;
    const int viewport_x = x * viewport_width;
    const int viewport_y = y * viewport_height;
    activate_viewport(viewport_x, viewport_y, viewport_width, viewport_height);
}

void IApplication::prepare_rendering(int x, int y, int width, int height, GLint buffer, bool clear, bool depth_test) const {
    activate_render_buffer(buffer, clear, depth_test);
    activate_viewport(x, y, width, height);
}

void IApplication::prepare_rendering_grid(int x, int y, int grid_size_x, int grid_size_y, GLint buffer, bool clear, bool depth_test) const {
    activate_render_buffer(buffer, clear, depth_test);
    activate_viewport_grid(x, y, grid_size_x, grid_size_y);
}

void IApplication::upload_projection_view(const ShaderProgram& program, glm::mat4 projection, glm::mat4 view) const {
    if (program.is_valid()) {
        program.use();
        program.uniform_matrix("projection", {projection});
        program.uniform_matrix("view", {view});
    }
}

// ----------------------------------------------------------------------------
// Getters & Setters
// ----------------------------------------------------------------------------
size_t IApplication::get_width() const { return this->width; }

size_t IApplication::get_height() const { return this->height; }

std::filesystem::path IApplication::get_framework_shaders_path() const { return this->framework_shaders_path; }

void IApplication::set_window(GLFWwindow* window) { this->window = window; }
