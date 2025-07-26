#version 330 core
in vec3 outcol;
in float shadow;

out vec4 finalColor;


void main() {
    finalColor = vec4(outcol * shadow, 1.0f);
}