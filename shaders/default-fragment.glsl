#version 410

layout(location = 0) in vec3 vPosition;
layout(location = 1) in vec2 vTexCoords;
layout(location = 2) in vec3 vNormal;

layout(location = 0) out vec4 outColor;

//uniform vec3 diffuseColor;
uniform sampler2D diffuseTextureID;

/// Main function
void main()
{
	// Texturing
	vec2 flipped_texcoord = vec2(vTexCoords.x, 1.0 - vTexCoords.y);
	vec4 pixelColor = texture2D(diffuseTextureID, flipped_texcoord);

	// Lighting
	vec3 Lp = vec3(1, 0, 1);
	float Li = 1;
	vec3 Ka = vec3(0.2);
	vec3 Kd = vec3(1);
	vec3 Ks = vec3(0.6);
	float shininess = 1;
	vec3 n = normalize(vNormal);
	vec3 s = normalize(vec3(Lp) - vPosition);
	vec3 v = normalize(vec3(-vPosition));
	vec3 h = normalize(v + s);
	vec3 pixelLight = vec3(Li * (Ka * pixelColor.rgb) +
			 (Kd * pixelColor.rgb) * max(dot(s, n), 0.0) +
			 (Ks * pixelColor.rgb) * pow(max(dot(h, n), 0.0), shininess));
	
	// Final color
	outColor = vec4(pixelColor.rgb*pixelLight, pixelColor.a);
}
