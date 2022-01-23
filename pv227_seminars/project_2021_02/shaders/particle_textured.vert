#version 450 core

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
// The particle position.
layout (location = 0) in vec4 start_position;

uniform float particle_size_vs;
uniform float time;

// The UBO with camera data.	
layout (std140, binding = 0) uniform CameraData
{
	mat4 projection;		// The projection matrix.
	mat4 projection_inv;	// The inverse of the projection matrix.
	mat4 view;				// The view matrix
	mat4 view_inv;			// The inverse of the view matrix.
	mat3 view_it;			// The inverse of the transpose of the top-left part 3x3 of the view matrix
	vec3 eye_position;		// The position of the eye in world space.
};

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
out VertexData
{
	vec4 position_vs;  // The particle position in view space.
	float particle_size_vs;
	float particle_rotation_vs;
} out_data;

const float PI = 3.14159265359f;

float hash13(vec3 p3)
{
	p3  = fract(p3 * .1031);
    p3 += dot(p3, p3.zyx + 31.32);
    return fract((p3.x + p3.y) * p3.z);
}

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	float size_factor = hash13(start_position.xyz) * 0.5f + 0.5f;
	out_data.particle_size_vs = particle_size_vs * size_factor;
	out_data.particle_rotation_vs = hash13(start_position.xyz) * 2 * PI;

	vec4 new_position = start_position;
	new_position.y -= time * 0.001f;
	new_position.y = mod(new_position.y, 60.0f) - 1.0f;
	out_data.position_vs = view * new_position;
}