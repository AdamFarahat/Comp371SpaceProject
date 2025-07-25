#version 330 core
in vec3 vertexColor;
// texture for frag here
in vec2 text;

out vec4 FragColor;
// sampler here
uniform sampler2D baseTexture;

void main() {
    vec4 tex = texture(baseTexture, text);
    FragColor = tex * vec4(vertexColor, 1.0);
}
