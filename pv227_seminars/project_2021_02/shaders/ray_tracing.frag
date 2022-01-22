#version 450 core

// ----------------------------------------------------------------------------
// Input Variables
// ----------------------------------------------------------------------------
in VertexData
{
	vec2 tex_coord;
} in_data;

// The UBO with camera data.	
layout (std140, binding = 0) uniform CameraBuffer
{
	mat4 projection;	  // The projection matrix.
	mat4 projection_inv;  // The inverse of the projection matrix.
	mat4 view;			  // The view matrix
	mat4 view_inv;		  // The inverse of the view matrix.
	mat3 view_it;		  // The inverse of the transpose of the top-left part 3x3 of the view matrix
	vec3 eye_position;	  // The position of the eye in world space.
};

// The structure holding the information about a single Phong light.
struct PhongLight
{
	vec4 position;                   // The position of the light. Note that position.w should be one for point lights and spot lights, and zero for directional lights.
	vec3 ambient;                    // The ambient part of the color of the light.
	vec3 diffuse;                    // The diffuse part of the color of the light.
	vec3 specular;                   // The specular part of the color of the light. 
	vec3 spot_direction;             // The direction of the spot light, irrelevant for point lights and directional lights.
	float spot_exponent;             // The spot exponent of the spot light, irrelevant for point lights and directional lights.
	float spot_cos_cutoff;           // The cosine of the spot light's cutoff angle, -1 point lights, irrelevant for directional lights.
	float atten_constant;            // The constant attenuation of spot lights and point lights, irrelevant for directional lights. For no attenuation, set this to 1.
	float atten_linear;              // The linear attenuation of spot lights and point lights, irrelevant for directional lights.  For no attenuation, set this to 0.
	float atten_quadratic;           // The quadratic attenuation of spot lights and point lights, irrelevant for directional lights. For no attenuation, set this to 0.
};

// The UBO with light data.
layout (std140, binding = 2) uniform PhongLightsBuffer
{
	vec3 global_ambient_color;		// The global ambient color.
	int lights_count;				// The number of lights in the buffer.
	PhongLight lights[8];			// The array with actual lights.
};

// The material data.
struct PBRMaterialData
{
   vec3 diffuse;     // The diffuse color of the material.
   float roughness;  // The roughness of the material.
   vec3 f0;          // The Fresnel reflection at 0�.
};

 // The buffer with individual materials for each sphere.
layout (std140, binding = 3) uniform PBRMaterialBuffer
{
   PBRMaterialData materials[3];
};

// The buffer with spheres.
layout (std140, binding = 4) uniform SpheresBuffer
{
	vec4 spheres[3];
};

// The windows size.
uniform vec2 resolution;
// The number of spheres to render.
uniform int spheres_count;
// The number of iterations.
uniform int iterations;

// The skybox texture.
layout (binding = 0) uniform samplerCube skybox_tex; 

/** The flag determining if a tone mapping should be used. */
uniform bool use_tone_mapping;
/** The exposure value. */
uniform float exposure;
/** The gamma value. */
uniform float gamma;

// ----------------------------------------------------------------------------
// Output Variables
// ----------------------------------------------------------------------------
// The final output color.
layout (location = 0) out vec4 final_color;

// ----------------------------------------------------------------------------
// Ray Tracing Structures
// ----------------------------------------------------------------------------
// The definition of a ray.
struct Ray {
    vec3 origin;     // The ray origin.
    vec3 direction;  // The ray direction.
};
// The definition of an intersection.
struct Hit {
    float t;				  // The distance between the ray origin and the intersection points along the ray. 
	vec3 intersection;        // The intersection point.
    vec3 normal;              // The surface normal at the interesection point.
	PBRMaterialData material; // The material of the object at the intersection point.
};
const Hit miss = Hit(1e20, vec3(0.0), vec3(0.0), PBRMaterialData(vec3(0),0,vec3(0)));

// ----------------------------------------------------------------------------
// Local Methods
// ----------------------------------------------------------------------------

// The FresnelSchlick approximation of the reflection.
vec3 FresnelSchlick(in vec3 f0, in vec3 V, in vec3 H)
{
	float VdotH = clamp(dot(V, H), 0.0, 1.0);
	return f0 + (1.0 - f0) * pow(1.0 - VdotH, 5.0);
}

// Computes an intersection between a ray and a sphere defined by its center and radius.
// ray - the ray definition (contains ray.origin and ray.direction)
// center - the center of the sphere
// radius - the radius of the sphere
// i - the index of the sphere in the array, can be used to obtain the material from materials buffer
Hit RaySphereIntersection(Ray ray, vec3 center, float radius, int i) {
	// TASK 1: Compute the intersection between the ray and a sphere.
	//  Hints: Remember elementary school:
    //             det = b*b - 4*a*c
	//             solution = (-b +- sqrt(det)) / 2*a
	//         The normal can be computed as the direction between sphere center and the intersection point.
	//         Use materials[i] as the material in the returned hit (materials[0] is dedicated for the floor).
	//         Return 'miss' object defined above if there is no intersection.
	//  Optimalization: You can take out the multiplications and divisions with 2 and 4 -> think how to update the original equation.

	// Non-optimalized version.
	//	vec3 oc = ray.origin - center;
	//	float a = dot(ray.direction,ray.direction);
	//	float b = 2 * dot(ray.direction, oc);
	//	float c = dot(oc, oc) - (radius*radius);
	//
	//	float det = b*b - 4*a*c;
	//	if (det < 0.0) return miss;
	//
	//	float t = (-b - sqrt(det)) / 2*a;
	//	if (t < 0.0) t = (-b + sqrt(det)) / 2*a;
	//	if (t < 0.0) return miss;
	//
	//	vec3 intersection = ray.origin + t * ray.direction;
	//	vec3 normal = normalize(intersection - center);
	//    return Hit(t, intersection, normal, materials[i+1]);

	// Optimalized version.
	vec3 oc = ray.origin - center;
	float b = dot(ray.direction, oc);
	float c = dot(oc, oc) - (radius*radius);

	float det = b*b - c;
	if (det < 0.0) return miss;

	float t = -b - sqrt(det);
	if (t < 0.0) t = -b + sqrt(det);
	if (t < 0.0) return miss;

	vec3 intersection = ray.origin + t * ray.direction;
	vec3 normal = normalize(intersection - center);
    return Hit(t, intersection, normal, materials[i+1]);
}

// Computes an intersection between a ray and a plane defined by its normal and one point inside the plane.
// ray - the ray definition (contains ray.origin and ray.direction)
// normal - the plane normal
// point - a point laying in the plane
Hit RayPlaneIntersection(Ray ray, vec3 normal, vec3 point) {
	// TASK 1: Compute the intersection between the ray and a plane.
	//  Hints: Use materials[0] as the material in the returned hit.
	//         Return 'miss' object defined above if there is no intersection.
	float nd = dot(normal, ray.direction);
	vec3 sp = point - ray.origin;
	float t = dot(sp, normal) / nd;
    if (t < 0.0) return miss;

	vec3 intersection = ray.origin + t * ray.direction;
	
	// TASK 7 (Optional): Make the plane extend only to a limited distance (i.e., make it square or circle).
	// circle
	// if(length(intersection) > 30) return miss;
	// square
	if(intersection.x > 40 || intersection.x < -40 || intersection.z > 40 || intersection.z < -40) return miss;

    return Hit(t, intersection, normal, materials[0]);
}

// Evaluates the intersections of the ray with the scene objects and returns the closes hit.
Hit Evaluate(Ray ray){
	// Sets the closes hit either to miss or to an intersection with the plane representing the ground.
	Hit closest_hit = RayPlaneIntersection(ray, vec3(0, 1, 0), vec3(0));
	
	// TASK 1: Iterate over all spheres, compute the intersection of the ray with each sphere and return the intersection closest to the camera.  
	//  Hints: Use function 'RaySphereIntersection' (after you implement it).
	//         See the definition of the Hit structure above.
	//         'spheres_count' contains the number of spheres
	//         'spheres[i].xyz' contains the i-th sphere position 
	//         'spheres[i].w' contains the i-th sphere radius
	for(int i = 0; i < spheres_count; i++){
		vec3 center = spheres[i].xyz;
		Hit intersection = RaySphereIntersection(ray, center, spheres[i].w, i);
		if(intersection.t < closest_hit.t){
			closest_hit = intersection;
		}
	}
    return closest_hit;
}

// Traces the ray trough the scene and accumulates the color.
vec3 Trace(Ray ray) {
    // The accumulated color and attenuation used when tracing the rays throug the scene.
	vec3 color = vec3(0.0);
    vec3 attenuation = vec3(1.0);
	// The direction towards the light.
	vec3 L = normalize(lights[0].position.xyz);
	// Due to floating-point precision errors, when a ray intersects geometry at a surface, the point of intersection could possibly be just below the surface.
	// The subsequent reflection and shadow rays would then bounce off the *inside* wall of the surface. This is known as self-intersection.
	// We, therefore, use a small epsilon to offset the subsequent rays.
	const float epsilon = 1e-2;

	// TASK 1: At this moment we only pass the ray to Evaluate method and return the unlit color of the closest object.
	//  Hints: Nothing to implement here.
	Hit hit = Evaluate(ray);
	color = hit.material.diffuse;
	
	// TASK 2: Simulate the perfect Lambertian surface (you can use the Blinn-Phong model).
	//	Hints: The material diffuse color is in hit.material.diffuse
	//         The surface normal is in the hit.normal
	//         The direction towards light is in the L vector.
	//         The light color is in the lights[0].diffuse
	color = max(dot(hit.normal, L), 0.0) * lights[0].diffuse * hit.material.diffuse;

	// TASK 3: Generate secondary rays (reflections only) and accumulate the color from each ray.
	//  Hints: Use the Evaluate() method you have implemented before to get the closest hit for each ray.
	//         See the definition of the Hit structure above.
	
	// TASK 4: Utilize the physical-based propagation of the light using FresnelSchlick approximation.
	//  Hints: Use FresnelSchlick method implemented above - use hit.material.f0 as the reflected light in 0�

	// TASK 5: Add shadows.
	//  Hints: To avoid numerical errors, offset the ray by a small epsilon towards the light or along the normal.
	//         To check if the ray did not hit anything you can use if(Evaluate(ray) == miss){...}

	// TASK 6: Add the sky.
	//  Hints: The skybox cubemap texture is stored in 'skybox_tex'.
	//         Do not forget to use attenuation based on the FresnelSchlick approximation
	//         Accumulate the skybox color only once - e.g., use 'break;' to finish the iterations early
	//         Also note that if you use 'break;' you are breaking the uniform flow, thus you have to use textureLod method.
	color = vec3(0,0,0);
    for (int i = 0; i < iterations; ++i) {
        Hit hit = Evaluate(ray);
        if (hit != miss) {
			// TASK 3
			//color += max(dot(hit.normal, L), 0.0) * lights[0].diffuse * hit.material.diffuse;

			// TASK 4 + 5
			Ray shadow_ray = Ray(hit.intersection + epsilon * L, L); 
			vec3 fresnel = FresnelSchlick(hit.material.f0, hit.normal, -ray.direction); 
			if(Evaluate(shadow_ray) == miss) {
				color += max(dot(hit.normal, L), 0.0) * lights[0].diffuse * hit.material.diffuse * (1.0 - fresnel) * attenuation; 
			}
			attenuation *= fresnel; 

			// TASK 3
            vec3 reflection = reflect(ray.direction, hit.normal);
            ray = Ray(hit.intersection + epsilon * reflection, reflection);
        } else {
			// TASK 6
			color += attenuation * textureLod(skybox_tex, ray.direction, 0).rgb;
			break;
        }
    }

    return color;
}

// ----------------------------------------------------------------------------
// Main Method
// ----------------------------------------------------------------------------
void main()
{
	// The aspect ratio.
	// It tells us how many times the window is wider (or narrower) w.r.t. its height.
	float aspect_ratio = resolution.x/resolution.y;

	// TASK 1: Generate the primary ray and pass it to the Trace function.
	//  Hints: You can use in_data.tex_coord or gl_FragCoord.xy to obtain the fragment position P on the screen (x,y coordinates).
	//         Think about the 'z' coordinate, where the point P should be positioned?
	//         Do not forget that you need to transform the fragment position to the range <(-1,-1),(1,1)> 
	//             in_data.tex_coord is in range <(0,0),(1,1)> while gl_FragCoord.xy is in range <(0,0),(widht,height)>
	//         Use the aspect_ratio to properly scale the fragment position to non-rectanglural window.
	//		   'S' from slides is in eye_position
	//         Do not forget to apply the view matrix (or rather its inversion) to the point P.
	//         Do not forget to normalize the direction vector.

	// We use the texture coordinates and the aspect ratio to map the coordinates to the screen space.
	vec2 uv = (2.0*in_data.tex_coord - 1.0) * vec2(aspect_ratio, 1.0);
	//vec2 uv = (2.0*gl_FragCoord.xy / resolution.xy - 1.0) * vec2(aspect_ratio, 1.0);

	// Computes the ray origin and ray direction using the view matrix.
	vec3 P = vec3(view_inv * vec4(uv, -1.0, 1.0));
	vec3 direction = normalize(P - eye_position);
	Ray ray = Ray(eye_position, direction);

	// We pass the ray to the trace function.
	vec3 color = Trace(ray);

	// TASK 8 (Optional): Add tone mapping.
	if(use_tone_mapping){
		vec3 mapped = vec3(1.0f) - exp2(-color * exposure);
		color = pow(mapped, vec3(1.0 / gamma));
	}

	final_color = vec4(color, 1.0);
}