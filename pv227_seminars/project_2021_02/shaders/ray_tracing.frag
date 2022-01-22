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


struct PBRMaterialData{
	/** The diffuse color of the material. */
    vec3 diffuse;
    /** The roughness of the material. */
    float roughness;
    /** The Fresnel reflection at 0°. */
    vec3 f0;
};

const int snowman_sphere_count = 13;
layout (std140, binding = 4) uniform Snowman
{
	vec4 positions[snowman_sphere_count];
	PBRMaterialData materials[snowman_sphere_count];
} snowman;

// The windows size.
uniform vec2 resolution;
// The number of iterations.
uniform int iterations;

uniform bool use_ambient_occlusion;

uniform float sphere_light_radius;

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
	bool ignore_lights;
};
// The definition of an intersection.
struct Hit {
    float t;				  // The distance between the ray origin and the intersection points along the ray. 
	vec3 intersection;        // The intersection point.
    vec3 normal;              // The surface normal at the interesection point.
	PBRMaterialData material; // The material of the object at the intersection point.
	int light_index;
};
const Hit miss = Hit(1e20, vec3(0.0), vec3(0.0), PBRMaterialData(vec3(0),0,vec3(0)), -1);

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
Hit RaySphereIntersection(Ray ray, vec3 center, float radius, int i, bool is_snowman) {

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
	if (is_snowman) {
		return Hit(t, intersection, normal, snowman.materials[i], -1);
	} else {
		PBRMaterialData light_material = { lights[i].diffuse, 0.0f, vec3(0) };
		return Hit(t, intersection, normal, light_material, i);
	}
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
	
	if(intersection.x > 100 || intersection.x < -100 || intersection.z > 100 || intersection.z < -100)
		return miss;

    return Hit(t, intersection, normal, snowman.materials[0], -1);
}

// Evaluates the intersections of the ray with the scene objects and returns the closes hit.
Hit Evaluate(Ray ray){
	// Sets the closes hit either to miss or to an intersection with the plane representing the ground.
	Hit closest_hit = RayPlaneIntersection(ray, vec3(0, 1, 0), vec3(0));

	for(int i = 0; i < snowman_sphere_count; i++){
		vec3 center = snowman.positions[i].xyz;
		Hit intersection = RaySphereIntersection(ray, center, snowman.positions[i].w, i, true);
		if(intersection.t < closest_hit.t){
			closest_hit = intersection;
		}
	}

	if (!ray.ignore_lights) {
		for(int i = 0; i < lights_count; i++){
			vec3 center = lights[i].position.xyz / lights[i].position.w;
			Hit intersection = RaySphereIntersection(ray, center, sphere_light_radius, i, false);
			if(intersection.t < closest_hit.t){
				closest_hit = intersection;
			}
		}
	}

    return closest_hit;
}

// ----------------------------------------------------------------------------
// Ambient occlusion
// ----------------------------------------------------------------------------
float sphere_occlusion(vec3 position, vec3 normal, vec4 sphere)
{
	// taken from https://www.shadertoy.com/view/4djSDy
	vec3 di = sphere.xyz - position;
	float l  = length(di);
	float nl = dot(normal, di / l);
	float h  = l / sphere.w;
	float h2 = h * h;
	float k2 = 1.0 - h2 * nl * nl;

	// above or below the hemisphere
	float res = max(0.0, nl) / h2;

	// intersecting the hemisphere
	if(k2 > 0.0) {
		res = (nl * h + 1.0)/h2;
		res = 0.33 * res * res;
	}

	return res;
}

float occlude_ambient(vec3 position, vec3 normal)
{
	float occlusion = 0.0f;
	for (int i = 0; i < snowman_sphere_count; ++i) {
		occlusion += sphere_occlusion(position, normal, snowman.positions[i]);
	}
	return 1.0f - occlusion;
}

// Traces the ray trough the scene and accumulates the color.
vec3 Trace(Ray ray) {
    // The accumulated color and attenuation used when tracing the rays throug the scene.
	vec3 color = vec3(0.0);
    vec3 attenuation = vec3(1.0);

	// Due to floating-point precision errors, when a ray intersects geometry at a surface, the point of intersection could possibly be just below the surface.
	// The subsequent reflection and shadow rays would then bounce off the *inside* wall of the surface. This is known as self-intersection.
	// We, therefore, use a small epsilon to offset the subsequent rays.
	const float epsilon = 1e-2;

	float occluded_ambient = 1.0;
    for (int i = 0; i < iterations; ++i) {
        Hit hit = Evaluate(ray);
        if (hit != miss) {
			if (hit.light_index >= 0) {
				color += hit.material.diffuse * attenuation;
			}
			vec3 fresnel = FresnelSchlick(hit.material.f0, hit.normal, -ray.direction); 
			for (int i = 0; i < lights_count; ++i) {
				vec3 L = normalize(lights[i].position.xyz / lights[i].position.w - hit.intersection);
				Ray shadow_ray = Ray(hit.intersection + epsilon * L, L, true);
				Hit shadow_hit = Evaluate(shadow_ray);
				if (shadow_hit == miss) {
					color += max(dot(hit.normal, L), 0.0)
						* lights[i].diffuse
						* hit.material.diffuse
						* (1.0 - fresnel)
						* attenuation
						/ lights_count;
				}
			}
			attenuation *= fresnel;

			if (i == 0 && use_ambient_occlusion) {
				// ambient occlusion occurs in the first iteration only
				occluded_ambient = occlude_ambient(hit.intersection, hit.normal);
			}
			// TASK 3
            vec3 reflection = reflect(ray.direction, hit.normal);
            ray = Ray(hit.intersection + epsilon * reflection, reflection, false);
        } else {
			break;
        }
    }

    return color * occluded_ambient;
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
	Ray ray = Ray(eye_position, direction, false);

	// We pass the ray to the trace function.
	vec3 color = Trace(ray);

	final_color = vec4(color, 1.0);
}