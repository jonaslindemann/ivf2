#version 410 core

// Per-vertex inputs
layout (location=0) in vec4 position;
layout (location=1) in vec3 normal;
layout (location=2) in vec3 inColor;

uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;
uniform bool useLighting = true;

// Light and material properties
uniform vec3 light_pos = vec3(0.0, 100.0, 0.0);
uniform vec3 diffuse_albedo = vec3(1.0, 0.0, 0.0);
uniform vec3 specular_albedo = vec3(0.7);
uniform float specular_power = 128.0;
uniform vec3 ambient = vec3(0.1, 0.1, 0.1);

// Outputs to the fragment shader
out VS_OUT
{
    vec3 color;
} vs_out;

void main(void)
{
    // Calculate view-space coordinate

    vec4 P = modelViewMatrix * position;

	if (useLighting)
	{

		// Calculate normal in view space

		vec3 N = mat3(modelViewMatrix) * normal;

		// Calculate view-space light vector

		vec3 L = light_pos - P.xyz;

		// Calculate view vector (simply the negative of the view-space position)

		vec3 V = -P.xyz;

		// Normalize all three vectors

		N = normalize(N);
		L = normalize(L);
		V = normalize(V);

		// Calculate R by reflecting -L around the plane defined by N

		vec3 R = reflect(-L, N);

		//diffuse_albedo = inColor;

		// Calculate the diffuse and specular contributions

		vec3 diffuse = max(dot(N, L), 0.0) * diffuse_albedo; 
		vec3 specular = pow(max(dot(R, V), 0.0), specular_power) * specular_albedo;

		// Send the color output to the fragment shader

		vs_out.color = ambient + diffuse + specular;

	}
	else
	{
		vs_out.color = vec3(1.0, 1.0, 0.0);	
	}

    // Calculate the clip-space position of each vertex

    gl_Position = projectionMatrix * P;
}
