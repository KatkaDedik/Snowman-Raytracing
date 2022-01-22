#include "geometry_base.hpp"

// ----------------------------------------------------------------------------
// Constructors
// ----------------------------------------------------------------------------

Geometry_Base::Geometry_Base(GLenum mode, int elements_per_vertex, int vertices_count, int indices_count, GLint position_loc, GLint normal_loc, GLint tex_coord_loc, GLint tangent_loc,
                             GLint bitangent_loc, GLint color_loc)
    : mode(mode), elements_per_vertex(elements_per_vertex), position_loc(position_loc), normal_loc(normal_loc),
      tex_coord_loc(tex_coord_loc), tangent_loc(tangent_loc), bitangent_loc(bitangent_loc), color_loc(color_loc) {

    vertex_buffer_stride = elements_per_vertex * sizeof(float);
    vertex_buffer_size = vertices_count * vertex_buffer_stride;

    init_patches_count();
    draw_elements_count = indices_count;
    draw_arrays_count = vertices_count;
}

Geometry_Base::Geometry_Base(GLenum mode, std::vector<float> positions, std::vector<uint32_t> indices, std::vector<float> normals, std::vector<float> colors, std::vector<float> tex_coords,
                             std::vector<float> tangents, std::vector<float> bitangents, GLint position_loc, GLint normal_loc, GLint tex_coord_loc, GLint tangent_loc, GLint bitangent_loc,
                             GLint color_loc)
    : mode(mode), position_loc(!positions.empty() ? position_loc : -1), normal_loc(!normals.empty() ? normal_loc : -1),
      tex_coord_loc(!tex_coords.empty() ? tex_coord_loc : -1), tangent_loc(!tangents.empty() ? tangent_loc : -1),
      bitangent_loc(!bitangents.empty() ? bitangent_loc : -1), color_loc(!colors.empty() ? color_loc : -1) {

    // Computes the number of elements per vertex.
    elements_per_vertex = 3 + (!normals.empty() ? 3 : 0) + (!colors.empty() ? 3 : 0) + (!tex_coords.empty() ? 2 : 0) +
                          (!tangents.empty() ? 3 : 0) + (!bitangents.empty() ? 3 : 0);

    // Computes the number of vertices.
    const int vertices_count = static_cast<int>(positions.size() / 3);

    // Builds the interleaved buffer from the input data.
    for (size_t i = 0; i < positions.size() / 3; i += 1) {
        interleaved_vertices.push_back(positions[i * 3 + 0]);
        interleaved_vertices.push_back(positions[i * 3 + 1]);
        interleaved_vertices.push_back(positions[i * 3 + 2]);

        if (!normals.empty()) {
            interleaved_vertices.push_back(normals[i * 3 + 0]);
            interleaved_vertices.push_back(normals[i * 3 + 1]);
            interleaved_vertices.push_back(normals[i * 3 + 2]);
        }

        if (!colors.empty()) {
            interleaved_vertices.push_back(colors[i * 3 + 0]);
            interleaved_vertices.push_back(colors[i * 3 + 1]);
            interleaved_vertices.push_back(colors[i * 3 + 2]);
        }

        if (!tex_coords.empty()) {
            interleaved_vertices.push_back(tex_coords[i * 2 + 0]);
            interleaved_vertices.push_back(tex_coords[i * 2 + 1]);
        }

        if (!tangents.empty()) {
            interleaved_vertices.push_back(tangents[i * 3 + 0]);
            interleaved_vertices.push_back(tangents[i * 3 + 1]);
            interleaved_vertices.push_back(tangents[i * 3 + 2]);
        }

        if (!bitangents.empty()) {
            interleaved_vertices.push_back(bitangents[i * 3 + 0]);
            interleaved_vertices.push_back(bitangents[i * 3 + 1]);
            interleaved_vertices.push_back(bitangents[i * 3 + 2]);
        }
    }

    vertex_buffer_stride = elements_per_vertex * sizeof(float);
    vertex_buffer_size = vertices_count * vertex_buffer_stride;

    init_patches_count();

    // Sets the number of elements according to the
    if (!indices.empty()) {
        draw_elements_count = static_cast<GLsizei>(indices.size());
    }

    draw_arrays_count = vertices_count;
}

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------
void Geometry_Base::swap_fields(Geometry_Base& first, Geometry_Base& second) const noexcept {
    using std::swap;

    swap(first.vertex_buffer, second.vertex_buffer);
    swap(first.vertex_buffer_size, second.vertex_buffer_size);
    swap(first.index_buffer, second.index_buffer);
    swap(first.vao, second.vao);
    swap(first.mode, second.mode);
    swap(first.draw_arrays_count, second.draw_arrays_count);
    swap(first.draw_elements_count, second.draw_elements_count);
    swap(first.patch_vertices, second.patch_vertices);
    swap(first.position_loc, second.position_loc);
    swap(first.normal_loc, second.normal_loc);
    swap(first.tex_coord_loc, second.tex_coord_loc);
    swap(first.tangent_loc, second.tangent_loc);
    swap(first.bitangent_loc, second.bitangent_loc);
    swap(first.color_loc, second.color_loc);
    swap(first.elements_per_vertex, second.elements_per_vertex);
    swap(first.vertex_buffer_stride, second.vertex_buffer_stride);
    swap(first.interleaved_vertices, second.interleaved_vertices);
    swap(first.position_offset, second.position_offset);
    swap(first.color_offset, second.color_offset);
    swap(first.normal_offset, second.normal_offset);
    swap(first.tex_coord_offset, second.tex_coord_offset);
}

void Geometry_Base::init_patches_count() {
    if (mode == GL_PATCHES) {
        patch_vertices = 16;
    }
}

void Geometry_Base::bind_vao() const {
    glBindVertexArray(vao);
}

void Geometry_Base::draw() const {
    bind_vao();

    if (mode == GL_PATCHES) {
        glPatchParameteri(GL_PATCH_VERTICES, patch_vertices);
    }

    if (draw_elements_count > 0) {
        glDrawElements(mode, draw_elements_count, GL_UNSIGNED_INT, nullptr);
    } else {
        glDrawArrays(mode, 0, draw_arrays_count);
    }
}

void Geometry_Base::draw_instanced(int count) const {
    bind_vao();

    if (mode == GL_PATCHES) {
        glPatchParameteri(GL_PATCH_VERTICES, patch_vertices);
    }

    if (draw_elements_count > 0) {
        glDrawElementsInstanced(mode, draw_elements_count, GL_UNSIGNED_INT, nullptr, count);
    } else {
        glDrawArraysInstanced(mode, 0, draw_arrays_count, count);
    }
}
