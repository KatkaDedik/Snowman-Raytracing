#include "geometry.hpp"
#include "glm/vec3.hpp"
#include <iostream>
#include <limits>
#include <tiny_obj_loader.h>
#include <glm/gtx/component_wise.hpp>

// ----------------------------------------------------------------------------
// Constructors
// ----------------------------------------------------------------------------
Geometry::Geometry(GLenum mode, int elements_per_vertex, int vertices_count, const float* vertices, int indices_count,
                   const unsigned int* indices, GLint position_loc, GLint normal_loc, GLint tex_coord_loc, GLint tangent_loc,
                   GLint bitangent_loc)
    : Geometry_Base(mode, elements_per_vertex, vertices_count, indices_count, position_loc, normal_loc, tex_coord_loc,
                    tangent_loc, bitangent_loc) {

    // Creates a single buffer for vertex data.
    glCreateBuffers(1, &vertex_buffer);
    glNamedBufferStorage(vertex_buffer, vertex_buffer_size, vertices, GL_DYNAMIC_STORAGE_BIT);

    init_vao();

    if (indices && indices_count > 0) {
        // Creates a buffer for indices.
        glCreateBuffers(1, &index_buffer);
        glNamedBufferStorage(index_buffer, indices_count * sizeof(unsigned int), indices, GL_DYNAMIC_STORAGE_BIT);
        glVertexArrayElementBuffer(vao, index_buffer);
    }
}

Geometry::Geometry(GLenum mode, int elements_per_vertex, std::vector<float> interleaved_vertices, std::vector<uint32_t> indices,
                   GLint position_loc, GLint normal_loc, GLint tex_coord_loc, GLint tangent_loc, GLint bitangent_loc)
    : Geometry(mode, elements_per_vertex, static_cast<int>(interleaved_vertices.size()), interleaved_vertices.data(),
               static_cast<int>(indices.size()), indices.data(), position_loc, normal_loc, tex_coord_loc, tangent_loc,
               bitangent_loc) {
}

Geometry::Geometry(
    GLenum mode,
    std::vector<float> positions,
    std::vector<uint32_t> indices,
    std::vector<float> normals,
    std::vector<float> colors,
    std::vector<float> tex_coords,
    std::vector<float> tangents,
    std::vector<float> bitangents,
    GLint position_loc,
    GLint normal_loc,
    GLint tex_coord_loc,
    GLint tangent_loc,
    GLint bitangent_loc)
    : Geometry_Base(mode, positions, indices, normals, colors, tex_coords, tangents, bitangents, position_loc, normal_loc,
                    tex_coord_loc, tangent_loc, bitangent_loc) {

    // Creates a single buffer for vertex data.
    glCreateBuffers(1, &vertex_buffer);
    glNamedBufferStorage(vertex_buffer, vertex_buffer_size, interleaved_vertices.data(), GL_DYNAMIC_STORAGE_BIT);

    init_vao();

    if (!indices.empty()) {
        // Creates a buffer for indices.
        glCreateBuffers(1, &index_buffer);
        glNamedBufferStorage(index_buffer, indices.size() * sizeof(uint32_t), indices.data(), GL_DYNAMIC_STORAGE_BIT);
        glVertexArrayElementBuffer(vao, index_buffer);
    }
}

Geometry::Geometry(const Geometry& other)
    : Geometry_Base(other) {
    // Creates a single buffer for vertex data.
    glCreateBuffers(1, &vertex_buffer);
    glNamedBufferStorage(vertex_buffer, vertex_buffer_size, nullptr, GL_DYNAMIC_STORAGE_BIT);
    glCopyNamedBufferSubData(other.vertex_buffer, vertex_buffer, 0, 0, vertex_buffer_size);

    // Creates a buffer for indices.
    glCreateBuffers(1, &index_buffer);
    glNamedBufferStorage(index_buffer, draw_elements_count * sizeof(unsigned int), nullptr, GL_DYNAMIC_STORAGE_BIT);
    glCopyNamedBufferSubData(other.index_buffer, index_buffer, 0, 0, draw_elements_count * sizeof(unsigned int));

    init_vao();

    // Sets the mode and the number of vertices to draw.
    if (mode != GL_PATCHES) {
        glVertexArrayElementBuffer(vao, index_buffer);
    }
}

Geometry::~Geometry() {
    glDeleteBuffers(1, &vertex_buffer);
    glDeleteBuffers(1, &index_buffer);
    glDeleteVertexArrays(1, &vao);
}

// ----------------------------------------------------------------------------
// Methods
// ----------------------------------------------------------------------------
void Geometry::init_vao() {
    // Creates a new VAO.
    glCreateVertexArrays(1, &vao);

    // Binds the interleaved buffer to the VAO.
    glVertexArrayVertexBuffer(vao, 0, vertex_buffer, 0, vertex_buffer_stride);

    // Sets the vertex attributes and their parameters.
    int offset = 0;
    if (position_loc >= 0) {
        glEnableVertexArrayAttrib(vao, position_loc);
        glVertexArrayAttribFormat(vao, position_loc, 3, GL_FLOAT, GL_FALSE, 0);
        glVertexArrayAttribBinding(vao, position_loc, 0);
        offset += 3;
    }
    if (normal_loc >= 0) {
        glEnableVertexArrayAttrib(vao, normal_loc);
        glVertexArrayAttribFormat(vao, normal_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * offset);
        glVertexArrayAttribBinding(vao, normal_loc, 0);
          normal_offset = offset;
        offset += 3;
    }
    if (color_loc >= 0) {
        glEnableVertexArrayAttrib(vao, color_loc);
        glVertexArrayAttribFormat(vao, color_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * offset);
        glVertexArrayAttribBinding(vao, color_loc, 0);
    }
    if (tex_coord_loc >= 0) {
        glEnableVertexArrayAttrib(vao, tex_coord_loc);
        glVertexArrayAttribFormat(vao, tex_coord_loc, 2, GL_FLOAT, GL_FALSE, sizeof(float) * offset);
        glVertexArrayAttribBinding(vao, tex_coord_loc, 0);
         tex_coord_offset = offset;
        offset += 2;
    }
    if (tangent_loc >= 0) {
        glEnableVertexArrayAttrib(vao, tangent_loc);
        glVertexArrayAttribFormat(vao, tangent_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * offset);
        glVertexArrayAttribBinding(vao, tangent_loc, 0);
        offset += 3;
    }
    if (bitangent_loc >= 0) {
        glEnableVertexArrayAttrib(vao, bitangent_loc);
        glVertexArrayAttribFormat(vao, bitangent_loc, 3, GL_FLOAT, GL_FALSE, sizeof(float) * offset);
        glVertexArrayAttribBinding(vao, bitangent_loc, 0);
        offset += 3;
    }
}

Geometry Geometry::from_file(std::filesystem::path path) {
    const std::string extension = path.extension().generic_string();

    if (extension == ".obj") {
        tinyobj::ObjReader reader;

        if (!reader.ParseFromFile(path.generic_string())) {
            if (!reader.Error().empty()) {
                std::cerr << "TinyObjReader: " << reader.Error();
            }
        }

        if (!reader.Warning().empty()) {
            std::cout << "TinyObjReader: " << reader.Warning();
        }

        auto& attrib = reader.GetAttrib();
        auto& shapes = reader.GetShapes();
        auto& materials = reader.GetMaterials();

        // Take only the first shape found
        const tinyobj::shape_t& shape = shapes[0];

        std::vector<float> positions;
        std::vector<float> normals;
        std::vector<float> tex_coords;

        glm::vec3 min{std::numeric_limits<float>::max(), std::numeric_limits<float>::max(), std::numeric_limits<float>::max()};
        glm::vec3 max{std::numeric_limits<float>::min(), std::numeric_limits<float>::min(), std::numeric_limits<float>::min()};

        // Loop over faces(polygon)
        size_t index_offset = 0;
        for (size_t f = 0; f < shape.mesh.num_face_vertices.size(); f++) {
            // Loop over vertices in the face.
            for (size_t v = 0; v < 3; v++) {
                // Access to vertex
                tinyobj::index_t idx = shape.mesh.indices[index_offset + v];

                tinyobj::real_t vx = attrib.vertices[3 * idx.vertex_index + 0];
                tinyobj::real_t vy = attrib.vertices[3 * idx.vertex_index + 1];
                tinyobj::real_t vz = attrib.vertices[3 * idx.vertex_index + 2];

                tinyobj::real_t nx;
                tinyobj::real_t ny;
                tinyobj::real_t nz;
                if (!attrib.normals.empty()) {
                    nx = attrib.normals[3 * idx.normal_index + 0];
                    ny = attrib.normals[3 * idx.normal_index + 1];
                    nz = attrib.normals[3 * idx.normal_index + 2];
                } else {
                    nx = 0.0;
                    ny = 0.0;
                    nz = 0.0;
                }

                tinyobj::real_t tx;
                tinyobj::real_t ty;
                if (!attrib.texcoords.empty()) {
                    tx = attrib.texcoords[2 * idx.texcoord_index + 0];
                    ty = attrib.texcoords[2 * idx.texcoord_index + 1];
                } else {
                    tx = 0.0;
                    ty = 0.0;
                }

                min.x = std::min(min.x, vx);
                min.y = std::min(min.y, vy);
                min.z = std::min(min.z, vz);
                max.x = std::max(max.x, vx);
                max.y = std::max(max.y, vy);
                max.z = std::max(max.z, vz);

                positions.insert(positions.end(), {vx, vy, vz});
                normals.insert(normals.end(), {nx, ny, nz});
                tex_coords.insert(tex_coords.end(), {tx, ty});
            }
            index_offset += 3;
        }

        glm::vec3 diff = max - min;
        glm::vec3 center = min + 0.5f * diff;
        min -= center;
        max -= center;
        for (int i = 0; i < positions.size(); i += 3) {
            positions[i + 0] -= center.x;
            positions[i + 1] -= center.y;
            positions[i + 2] -= center.z;

            positions[i + 0] /= std::max(std::max(diff.x, diff.y), diff.z);
            positions[i + 1] /= std::max(std::max(diff.x, diff.y), diff.z);
            positions[i + 2] /= std::max(std::max(diff.x, diff.y), diff.z);
        }

        const int elements_per_vertex = 3 + (!normals.empty() ? 3 : 0) + (!tex_coords.empty() ? 2 : 0);

        return Geometry{GL_TRIANGLES, positions, {/*indices*/}, normals, {/*colors*/}, tex_coords};
    }
    std::cerr << "Extension " << extension << " not supported" << std::endl;

    return Geometry{};
}
