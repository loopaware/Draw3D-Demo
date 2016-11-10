#version 410

layout(location = 0) in vec3 vPosition;
//layout(location = 1) in vec2 vTexCoords;

layout(location = 0) out vec4 outColor;

//uniform vec3 diffuseColor;
//uniform sampler2D diffuseTextureID;

/// Main function
void main()
{
	//vec4 texel = texture2D(diffuseTextureID, vTexCoords);

	//outColor = texel;//vec4(vec3(texel.rgb), 1.0);
	outColor = vec4(1);
	//outColor = vec4(diffuseColor.rgb, 1);
	
	//outColor = vec4(vTexCoords.s, vTexCoords.t, 0, 1);
}
