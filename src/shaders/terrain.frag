#version 330 core
out vec4 FragColor;

in vec2 texCoord;
in vec3 ourColor;
in float brightness;
flat in int _useInColor;

uniform sampler2D texture1;

void main()
{
    if(_useInColor == 1){
        FragColor = vec4(ourColor, 1.0);
    } else {
        vec4 textureColour = texture(texture1, texCoord);
        FragColor = vec4(textureColour.rgb * brightness, textureColour.a);
    }
}