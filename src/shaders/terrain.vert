#version 330 core
layout (location = 0) in int vertexPosition;

out vec2 texCoord;
out vec3 ourColor;
out float brightness;       // constant all colours for now 
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


// fast shadow for depth - no lighting yet
const float brightnessArr[6] = float[6](
    0.86,  // front
    0.86,  // back
    0.80,   // left
    0.80,  // right
    1.00,  // top
    0.60  // bottom
);


void main()
{
    int offset = 16; // Offset to revert the signed range

    // Decode the 32-bit integer into x, y, z positions
    float x = ((vertexPosition & 0x3F) - offset);         // 6 bits for x
    float y = (((vertexPosition >> 6) & 0x3F) - offset);  // 6 bits for y
    float z = (((vertexPosition >> 12) & 0x3F) - offset); // 6 bits for z
    int normalIndex = ((vertexPosition >> 18) & 0x7); // 3 bits for normal, assigned to face 1-6, see brightnessArr

    // int position = (((vertexPosition >> 21) & 0x3F)); // 6 bits for texture

    // decode texture coordinate - 5 bits each for 63x63 range
    float u = (vertexPosition >> 21) & 0x1F; // bits 21–25
    float v = (vertexPosition >> 26) & 0x1F; // bits 26–30
    u = u * texWidth;
    v = v * texHeight;


    // 32 bits
    //[start].vvvvvuuuuufffzzzzzzyyyyyyxxxxxx[end]


    // No normal or type used in this example for movement
    vec3 decodedPos = vec3(x, y, z);

    gl_Position = projection * view * model * vec4(decodedPos + worldPos, 1.0);

    _useInColor = useInColor ? 1 : 0;       // change to int, frag shader doesn't support bool

    if(_useInColor == 1){
        ourColor = inColor;
        texCoord = vec2(0, 0);
        brightness = 1.0f; // default brightness
    } else {
        ourColor = vec3(1.0, 1.0, 1.0);
        texCoord = vec2(u, v);
        brightness = brightnessArr[normalIndex]; 
    }
}
