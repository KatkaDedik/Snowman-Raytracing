#pragma once
#include "ubo.hpp"

struct A{int a;};

/** The templated base class for all materials. */
template <class T> class MaterialUBO : public UBO<T> {

    // ----------------------------------------------------------------------------
    // Constructors
    // ----------------------------------------------------------------------------
  public:

     /**
     * Constructs a new @link MaterialUBO with a default material. Note that the data are immediately copied to the
     * GPU.
     */
    MaterialUBO() : UBO<T>(GL_DYNAMIC_STORAGE_BIT) {
        set_material(T());  
        UBO<T>::update_opengl_data();
    }

    /**
     * Constructs a new @link MaterialUBO with a custom material. Note that the data are immediately copied to the
     * GPU.
     *
     * @param 	material	The material to upload.
     */
    MaterialUBO(const T& material) : UBO<T>(GL_DYNAMIC_STORAGE_BIT) {
        set_material(material);
        UBO<T>::update_opengl_data();
    }

    /**
     * Constructs a new @link MaterialUBO with a list of custom materials. Note that the data are immediately
     * copied to the GPU.
     *
     * @param 	materials	The list of materials to upload.
     */
    MaterialUBO(const std::vector<T>& materials) : UBO<T>(materials, GL_DYNAMIC_STORAGE_BIT) {
    }


    // ----------------------------------------------------------------------------
    // Getters & Setters
    // ----------------------------------------------------------------------------
  public:
    /**
     * Sets a new material to the first position in the buffer.
     *
     * @param 	material	The material to set.
     */
    void set_material(const T& material) { set_material(0, material); }

    /**
     * Sets a new material at a specified position in the buffer.
     *
     * @param 	idx			Zero-based index of position in the buffer that will be updated.
     * @param 	material	The material to set.
     */
    void set_material(int idx, const T& material) {
        UBO<T>::data[idx] = material;
    }
};