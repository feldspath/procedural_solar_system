#version 330 core

layout (location = 0) in vec3 position;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 color;
layout (location = 3) in vec2 uv;

out vec2 tex_uv;

uniform mat4 view;
uniform mat4 projection;

void main()
{
  tex_uv = uv;
	gl_Position = projection * view * vec4(position, 1.0);
  gl_Position = vec4(gl_Position.x, gl_Position.y, gl_Position.w*0.999999, gl_Position.w);
}
