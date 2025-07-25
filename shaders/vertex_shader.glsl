#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
// add another one in vec2 (2d, uv), for textures
layout (location = 2) in vec2 aText;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vertexColor;
// here as well
out vec2 text;

void main() {
    vertexColor = aColor;
    text = aText;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
