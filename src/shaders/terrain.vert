#version 330 core
layout (location = 0) in int vertexPosition;

out vec2 texCoord;
out vec3 ourColor;
flat out int _useInColor;

// colour 
uniform vec3 inColor;
uniform bool useInColor;

// chunk position and 3d
uniform vec3 worldPos;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

// texture size 
uniform float texWidth;
uniform float texHeight;

// const vec3 colors[3] = vec3[](vec3(0.0, 0.0, 0.0), vec3(0.0, 0.5, 0.0), vec3(0.5, 0.5, 0.0));


void main()
{
    int offset = 16; // Offset to revert the signed range

    // Decode the 32-bit integer into x, y, z positions
    float x = ((vertexPosition & 0x3F) - offset);         // 6 bits for x
    float y = (((vertexPosition >> 6) & 0x3F) - offset);  // 6 bits for y
    float z = (((vertexPosition >> 12) & 0x3F) - offset); // 6 bits for z

    // int position = (((vertexPosition >> 21) & 0x3F)); // 6 bits for texture

    // decode texture coordinate - 4 bits each for 16x16 range
    float texX = (vertexPosition >> 21) & 0xF; // bits 21–24
    float texY = (vertexPosition >> 25) & 0xF; // bits 25–28
    texX = texX * texWidth;
    texY = texY * texHeight;


    // 32 bits
    //        texture coord(4) normal(3) block coord(6) 
    //[start]...yyyyxxxxfffzzzzzzyyyyyyxxxxxx[end]


    // No normal or type used in this example for movement
    vec3 decodedPos = vec3(x, y, z);

    gl_Position = projection * view * model * vec4(decodedPos + worldPos, 1.0);

    _useInColor = useInColor ? 1 : 0;       // change to int, frag shader doesn't support bool

    if(_useInColor == 1){
        ourColor = inColor;
        texCoord = vec2(0, 0);
    } else {
        ourColor = vec3(1.0, 1.0, 1.0);
        texCoord = vec2(texX, texY);
    }
}
