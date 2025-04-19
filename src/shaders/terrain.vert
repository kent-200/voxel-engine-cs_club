#version 330 core
layout (location = 0) in int vertexPosition;

out vec2 TexCoord;
out vec3 ourColor;
flat out int _useInColor;

uniform vec3 inColor;
uniform bool useInColor;
uniform vec3 worldPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

const vec3 colors[3] = vec3[](vec3(0.0, 0.0, 0.0), vec3(0.0, 0.5, 0.0), vec3(0.5, 0.5, 0.0));

const float TX_WIDTH = 1.0f/16.0f;
const float TX_HEIGHT = 1.0f/16.0f;

void main()
{
    int offset = 16; // Offset to revert the signed range

    // Decode the 32-bit integer into x, y, z positions
    float x = ((vertexPosition & 0x3F) - offset);         // 6 bits for x
    float y = (((vertexPosition >> 6) & 0x3F) - offset);  // 6 bits for y
    float z = (((vertexPosition >> 12) & 0x3F) - offset); // 6 bits for z

    // int position = (((vertexPosition >> 21) & 0x3F)); // 6 bits for texture

    // convert to texture coordinate - expand to 4 bits each for 16x16 range
    int texX = (vertexPosition >> 21) & 0xF; // bits 21–24
    int texY = (vertexPosition >> 25) & 0xF; // bits 25–28
    float tex_x = float(texX) * TX_WIDTH;
    float tex_y = float(texY) * TX_HEIGHT;


    // 32 bits
    //[start].ttttttttfffzzzzzzyyyyyyxxxxxx[end]

    // expand t to 8 bit later, 256 options to match file

    // No normal or type used in this example for movement
    vec3 decodedPos = vec3(x, y, z);

    gl_Position = projection * view * model * vec4(decodedPos + worldPos, 1.0);

    _useInColor = useInColor ? 1 : 0;       // change to int, frag shader doesn't support bool

    if(_useInColor == 1){
        ourColor = inColor;
        TexCoord = vec2(0, 0);
    } else {
        ourColor = vec3(1.0, 1.0, 1.0);
        TexCoord = vec2(tex_x, tex_y);
    }
}
