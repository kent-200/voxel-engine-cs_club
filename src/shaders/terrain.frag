#version 330 core
out vec4 FragColor;

in vec2 texCoord;
in vec3 ourColor;
flat in int _useInColor;

uniform sampler2D texture1;

void main()
{
    if(_useInColor == 1){
        FragColor = vec4(ourColor, 1.0);
    } else {
        FragColor = texture(texture1, texCoord);
    }
}