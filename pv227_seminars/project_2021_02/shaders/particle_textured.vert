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
} out_data;

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	// TASK 4: Pass the color and the position in view space into the geometry shader.
	float tmp = time;
	out_data.position_vs = view * start_position;
}