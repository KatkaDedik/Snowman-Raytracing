#pragma once

#include "glad/glad.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <cstring> // <cstring> instead of string as workaround for unix-based systems

/**
 * The class providing a collection of static utility methods.
 *
 * @author	<a href="mailto:jan.byska@gmail.com">Jan Byška</a>
 */
class Utils {
    public:
    /**
     * Checks if a specified OpenGL extension exists.
     *
     * @param 	extension	The extension to test.
     *
     * @return	@p true if OpenGL exists, @p false if not.
     */
    static bool is_open_gl_extension_present(const char* extension) {
        // Gets the number of extensions.
        GLint num_extension;
        glGetIntegerv(GL_NUM_EXTENSIONS, &num_extension);

        // Gos through all extensions and check whether the given extension is present.
        for (int i = 0; i < num_extension; i++) {
            if (strcmp((const char*)glGetStringi(GL_EXTENSIONS, i), extension) == 0) {
                // We found the extension.
                return true;
            }
        }
        // We did not found the extension.
        return false;
    }
};

/**
 * The class providing a collection of static utility methods for working with shaders.
 *
 * @author	<a href="mailto:jan.byska@gmail.com">Jan Byška</a>
 */
class ShaderUtils {

  public:
    /**
     * The sole purpose of this class is to load a file and extract the text that is in it. In theory, this class could be used
     * for a variety of text- processing purposes, but it was initially designed to be used to load shader source code. Since the
     * entire class is a static class, you only have to add this single line of code to your project:
     * --------------------------------------------------------------------------------------
     * std::string shaderSource = Utils::load("./path/to/shader.extension");
     * --------------------------------------------------------------------------------------
     * This will (recursively) extract the source code from the first shader file. Now, you might be wondering, what is the point
     * of using your code for something so trivial as to loading a file and calling the "std::getline()" function on it? Well,
     * besides loading the shader source code from a single file, the loader also supports custom keywords that allow you to
     * include external files inside your shader source code!
     *
     * The source code was shared under MIT License.
     *
     * @author	Tahar Meijs (https://github.com/tntmeijs/GLSL-Shader-Includes)
     * @param 	path			The path to the "main" shader file.
     * @param 	include_indentifier	The include indentifier.
     * @return	The loaded string.
     */
    static std::string load_shader(const std::filesystem::path& input_path, std::string include_indentifier = "#pragma include") {
        std::filesystem::path path = input_path;
        path.make_preferred();
        std::filesystem::path directory = path;
        directory.remove_filename();

        include_indentifier += ' ';
        static bool is_recursive_call = false;

        std::string full_source_code = "";
        std::ifstream file(path);

        if (!file.is_open()) {
            std::cerr << "ERROR: could not open the shader at: " << path << "\n" << std::endl;
            return full_source_code;
        }

        std::string line_buffer;
        while (std::getline(file, line_buffer)) {
            // Looks for the new shader include identifier.
            if (line_buffer.find(include_indentifier) != line_buffer.npos) {
                // Removes the include identifier, this will cause the path to remain.
                line_buffer.erase(0, include_indentifier.size());

                // The include path is relative to the current shader file path.
                const std::filesystem::path include_path = directory / line_buffer;

                // By using recursion, the new include file can be extracted
                // and inserted at this location in the shader source code.
                is_recursive_call = true;
                full_source_code += load_shader(include_path);

                // Do not add this line to the shader source code, as the include
                // path would generate a compilation issue in the final source code.
                continue;
            }

            full_source_code += line_buffer + '\n';
        }

        // Only add the null terminator at the end of the complete file,
        // essentially skipping recursive function calls this way.
        if (!is_recursive_call)
            full_source_code += '\0';

        file.close();

        return full_source_code;
    }
};

/**
 * The class providing a collection of static utility methods for working with frame buffers.
 *
 * @author	<a href="mailto:jan.byska@gmail.com">Jan Byška</a>
 * @author	<a href="mailto:cejka.honza@gmail.com ">Jan Čejka</a>
 */
class FBOUtils {

  public:
    /**
     * Checks the status of a currently bind framebuffer and prints an error if the framebuffer is not complete.
     *
     * @param 	framebuffer	The framebuffer to check.
     * @param 	buffer_name	The name of the framebuffer that will be used in the error message.
     *
     * @return	True if it succeeds, false if it fails.
     */
    static bool check_framebuffer_status(GLuint framebuffer, std::string buffer_name) {
        GLenum status = glCheckNamedFramebufferStatus(framebuffer, GL_FRAMEBUFFER);
        if (status != GL_FRAMEBUFFER_COMPLETE) {
            std::cout << "Failed to create a complete framebuffer (" << buffer_name << ")." << std::endl;
            return false;
        }
        return true;
    }

    /**
     * When setting a framebuffer, we need to "connect" fragment output data with color attachments. We do this by
     * calling glDrawBuffers with {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, ...} as the second parameter. The
     * following constant just saves repeating the definition of the array again and again.
     */
    static const GLenum draw_buffers_constants[];
};

/**
 * The class providing a collection of static utility methods for working with textures.
 *
 * @author	<a href="mailto:jan.byska@gmail.com">Jan Byška</a>
 * @author	<a href="mailto:cejka.honza@gmail.com ">Jan Čejka</a>
 */
class TextureUtils {

  public:
    /**
     * Sets the texture parameters by calling glTextureParameteri.
     *
     * @param 	texture   	The OpenGL texture name.
     * @param 	wrap_s	  	The wrap parameter value for GL_TEXTURE_WRAP_S.
     * @param 	wrap_t	  	The wrap parameter value for GL_TEXTURE_WRAP_T.
     * @param 	wrap_t	  	The wrap parameter value for GL_TEXTURE_WRAP_R.
     * @param 	min_filter	A filter parameter value for GL_TEXTURE_MIN_FILTER.
     * @param 	mag_filter	A filter parameter value for GL_TEXTURE_MAG_FILTER.
     */
    static void set_texture_3d_parameters(GLuint texture, GLint wrap_s, GLint wrap_t, GLint wrap_r, GLint min_filter, GLint mag_filter) {
        if (texture != 0) {
            glTextureParameteri(texture, GL_TEXTURE_WRAP_S, wrap_s);
            glTextureParameteri(texture, GL_TEXTURE_WRAP_T, wrap_t);
            glTextureParameteri(texture, GL_TEXTURE_WRAP_R, wrap_r);
            glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, min_filter);
            glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, mag_filter);
        } else {
            // fail silently
        }
    }

     /**
     * Sets the texture parameters by calling glTextureParameteri.
     *
     * @param 	texture   	The OpenGL texture name.
     * @param 	wrap_s	  	The wrap parameter value for GL_TEXTURE_WRAP_S.
     * @param 	wrap_t	  	The wrap parameter value for GL_TEXTURE_WRAP_T.
     * @param 	min_filter	A filter parameter value for GL_TEXTURE_MIN_FILTER.
     * @param 	mag_filter	A filter parameter value for GL_TEXTURE_MAG_FILTER.
     */
    static void set_texture_2d_parameters(GLuint texture, GLint wrap_s, GLint wrap_t, GLint min_filter, GLint mag_filter) {
        if (texture != 0) {
            glTextureParameteri(texture, GL_TEXTURE_WRAP_S, wrap_s);
            glTextureParameteri(texture, GL_TEXTURE_WRAP_T, wrap_t);
            glTextureParameteri(texture, GL_TEXTURE_MIN_FILTER, min_filter);
            glTextureParameteri(texture, GL_TEXTURE_MAG_FILTER, mag_filter);
        } else {
            // fail silently
        }
    }


    static unsigned char* load_image(const std::filesystem::path filename, bool flip_y, int &width, int &height);

    /**
     * Loads an image and creates OpenGL texture from it.
     *
     * @param 	filename	The filename of an image to load.
     * @param 	flip_y  	If @p true the texture will be flipped along the Y-axis (by default should be true for OpenGL).
     *
     * @return	The created OpenGL texture.
     */
    static GLuint load_texture_2d(const std::filesystem::path filename, bool flip_y = true);

    /**
     * Loads and 6 images and creates OpenGL cube texture.
     * @param 	width	   	the expected width of the loaded cube map texture.
     * @param 	height	   	the expected height of the loaded cube map texture.
     * @param 	filename_px	The filename of an image to load.
     * @param 	filename_nx	The filename of an image to load.
     * @param 	filename_py	The filename of an image to load.
     * @param 	filename_ny	The filename of an image to load.
     * @param 	filename_pz	The filename of an image to load.
     * @param 	filename_nz	The filename of an image to load.
     * @return	The created OpenGL texture.
     */
    static GLuint load_texture_cube(
        const int width, 
        const int height,
        const std::filesystem::path filename_px,
        const std::filesystem::path filename_nx,
        const std::filesystem::path filename_py,
        const std::filesystem::path filename_ny,
        const std::filesystem::path filename_pz,
        const std::filesystem::path filename_nz);
};
