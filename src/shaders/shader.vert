#version 330 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec3 col;
layout(location = 2) in float _shadow;

out vec3 outcol;
out float shadow;

mat4 model = mat4(1.0); // define in vertex
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(position, 1.0);
    outcol = col; // Pass color to fragment shader
    shadow = _shadow; // Pass shadow value to fragment shader
}
