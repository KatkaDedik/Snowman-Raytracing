#include "utils/utils.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <vector>

const GLenum FBOUtils::draw_buffers_constants[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3,
                                                   GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5, GL_COLOR_ATTACHMENT6, GL_COLOR_ATTACHMENT7};

unsigned char* TextureUtils::load_image(const std::filesystem::path filename, bool flip_y, int& width, int& height) {
    int channels, required_channels = 4;
    unsigned char* loaded_data = stbi_load(filename.generic_string().data(), &width, &height, &channels, required_channels);

    // Flips the loaded pixels along the Y-axis if requested.
    if (flip_y && loaded_data != nullptr) {
        auto* flipped_data = new unsigned char[width * height * required_channels];
        for (int x = 0; x < width; x++) {
            for (int y = 0; y < height; y++) {
                const int index = (y * width + x) * required_channels;
                const int flipped_y = height - y - 1;
                const int flipped_index = (flipped_y * width + x) * required_channels;
                flipped_data[flipped_index] = loaded_data[index];
                flipped_data[flipped_index + 1] = loaded_data[index + 1];
                flipped_data[flipped_index + 2] = loaded_data[index + 2];
                if (channels > 3) {
                    flipped_data[flipped_index + 3] = loaded_data[index + 3];
                } else {
                    flipped_data[flipped_index + 3] = 255;
                }
            }
        }
        stbi_image_free(loaded_data); // release the other data
        loaded_data = flipped_data;
    }
    return loaded_data;
}

GLuint TextureUtils::load_texture_2d(const std::filesystem::path filename, bool flip_y) {
    GLuint texture;
    glCreateTextures(GL_TEXTURE_2D, 1, &texture);

    GLsizei width, height;
    unsigned char* loaded_data = load_image(filename, flip_y, width, height);

    glTextureStorage2D(texture, static_cast<GLsizei>(std::log2(width)), GL_RGBA8, width, height);
    glTextureSubImage2D(texture, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, loaded_data);

    // Frees the memory - if flip is true then the memory was allocated with new[].
    if (flip_y) {
        delete[] loaded_data;
    } else {
        stbi_image_free(loaded_data);
    }

    glGenerateTextureMipmap(texture);
    return texture;
}

GLuint TextureUtils::load_texture_cube(const int width, const int height, const std::filesystem::path filename_px, const std::filesystem::path filename_nx, const std::filesystem::path filename_py,
                                       const std::filesystem::path filename_ny, const std::filesystem::path filename_pz, const std::filesystem::path filename_nz) {

    const bool flip_y = true;
    GLuint texture;

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &texture);
    glTextureStorage2D(texture, static_cast<GLsizei>(std::log2(width)), GL_RGBA8, width, height);

    std::vector<std::filesystem::path> files = {filename_px, filename_nx, filename_py, filename_ny, filename_pz, filename_nz};
    for (int i = 0; i < 6; i++) {
        GLsizei w, h;
        unsigned char* loaded_data = load_image(files[i], flip_y, w, h);

        if (w != width || h != width) {
            std::cout << "The image (" << files[i] << ") does not have required resolution (" << width << "x" << height << ")." << std::endl;
        }
        glTextureSubImage3D(texture, 0, 0, 0, i, width, height, 1, GL_RGBA, GL_UNSIGNED_BYTE, loaded_data);
        // Frees the memory - if flip is true then the memory was allocated with new[].
        if (flip_y) {
            delete[] loaded_data;
        } else {
            stbi_image_free(loaded_data);
        }
    }

    glGenerateTextureMipmap(texture);
    return texture;
}
