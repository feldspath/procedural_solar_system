#version 330 core

in vec2 tex_uv;

in vec3 localCoords;
in vec3 localNormal;

layout(location=0) out vec4 FragColor;

uniform sampler2D image_texture;


void main()
{
  FragColor = vec4(texture(image_texture, tex_uv).xyz, 1.0);
}
