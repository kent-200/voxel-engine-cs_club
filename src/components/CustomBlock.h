#ifndef CUSTOM_BLOCK_H
#define CUSTOM_BLOCK_H

#include <iostream>
#include <vector>
#include <utility>
#include <glm/glm.hpp>
#include "Block.h"
#include "ChunkMesh.h"


struct CustomMesh {
    static constexpr bool DEBUG_TRIANGLES = false; // Display green triangles on blocks
    static constexpr int MESH_VERTEX_BUFFERS = 2;
    int vertexCount;   // Number of vertices stored in arrays
    int indiciesCount; // Number of triangles stored (indexed or not)

    float *vertices;
    /*
            Represents vertex data by packing them into a 32-bit float:
            [start]...ttttttfffzzzzzzyyyyyyxxxxxx[end]
            where:
            - x, y, z: represent bits occupied to represent vertex position
   within a chunk
            - f: bits occupied to represent the vertex's face's normal vector
            - t: block type ID
    */
    unsigned int *indices; // Vertex indices (in case vertex data comes indexed)

    // OpenGL identifiers
    unsigned int vaoId; // OpenGL Vertex Array Object id
    unsigned int
        *vboId; // OpenGL Vertex Buffer Objects id (default vertex data)
};


class CustomBlock {
    static void AddCubeFace(CustomMesh *mesh, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, 
        glm::vec3 col, float shadow);
        
public:

    static void CreateCube(CustomMesh *mesh, float blockX, float blockY, float blockZ,
        float size, glm::vec3 col);

};




void CustomBlock::AddCubeFace(CustomMesh *mesh, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, glm::vec3 col, float shadow) {
    int *vCount = &mesh->vertexCount;
    int *iCount = &mesh->indiciesCount;
    

    int v1 = *vCount;
    int index_offset = v1 / 7;

    // arr of points
    glm::vec3 points[4] = {p1, p2, p3, p4};


    // Add vertices
    for(int i = 0; i < 4; i++){
        int io = i * 7;
        // x y z for each point
        mesh->vertices[v1 + io] = points[i].x;
        mesh->vertices[v1 + io + 1] = points[i].y;
        mesh->vertices[v1 + io + 2] = points[i].z;
        
        mesh->vertices[v1 + io + 3] = col.x;
        mesh->vertices[v1 + io + 4] = col.y;
        mesh->vertices[v1 + io + 5] = col.z;

        mesh->vertices[v1 + io + 6] = shadow; // shadow value
    }

    // Add indices
    mesh->indices[*iCount] = index_offset;
    mesh->indices[*iCount + 1] = index_offset + 1;
    mesh->indices[*iCount + 2] = index_offset + 2;
    mesh->indices[*iCount + 3] = index_offset;
    mesh->indices[*iCount + 4] = index_offset + 2;
    mesh->indices[*iCount + 5] = index_offset + 3;

    *vCount += 28; // 7 floats per vertex, 4 vertices per face
    *iCount += 6;
}



void CustomBlock::CreateCube(CustomMesh *mesh, float blockX, float blockY, float blockZ,
                       float size, glm::vec3 col) {
    int hs = (int)(size / 2.0f);

    // TODO: casts here?
    // TODO: ignore normals for now


    // DEFINES THE 8 POINTS OF THE CUBE
    // NEED TO CHANGE BLOCKTYPE to x and y coords for map

    // packvertex(int x, int y, int z, int normal, int tex_x, int tex_y)
    // set texture coordinates to 0,0 for now, change according to face

    glm::vec3 p1 = {Block::BLOCK_RENDER_SIZE * blockX - hs,
                               Block::BLOCK_RENDER_SIZE * blockY - hs,
                               Block::BLOCK_RENDER_SIZE * blockZ + hs};
                               
    glm::vec3 p2 = {Block::BLOCK_RENDER_SIZE * blockX + hs,
                               Block::BLOCK_RENDER_SIZE * blockY - hs,
                               Block::BLOCK_RENDER_SIZE * blockZ + hs};
                               
    glm::vec3 p3 = {Block::BLOCK_RENDER_SIZE * blockX + hs,
                               Block::BLOCK_RENDER_SIZE * blockY + hs,
                               Block::BLOCK_RENDER_SIZE * blockZ + hs};
                               
    glm::vec3 p4 = {Block::BLOCK_RENDER_SIZE * blockX - hs,
                               Block::BLOCK_RENDER_SIZE * blockY + hs,
                               Block::BLOCK_RENDER_SIZE * blockZ + hs};
                               
    glm::vec3 p5 = {Block::BLOCK_RENDER_SIZE * blockX + hs,
                               Block::BLOCK_RENDER_SIZE * blockY - hs,
                               Block::BLOCK_RENDER_SIZE * blockZ - hs};
                               
    glm::vec3 p6 = {Block::BLOCK_RENDER_SIZE * blockX - hs,
                               Block::BLOCK_RENDER_SIZE * blockY - hs,
                               Block::BLOCK_RENDER_SIZE * blockZ - hs};
                               
    glm::vec3 p7 = {Block::BLOCK_RENDER_SIZE * blockX - hs,
                               Block::BLOCK_RENDER_SIZE * blockY + hs,
                               Block::BLOCK_RENDER_SIZE * blockZ - hs};
                               
    glm::vec3 p8 = {Block::BLOCK_RENDER_SIZE * blockX + hs,
                               Block::BLOCK_RENDER_SIZE * blockY + hs,
                               Block::BLOCK_RENDER_SIZE * blockZ - hs};
                               

    
    


    // front, back, left, right, top, bottom

    glm::vec3 n1;       // for normal??, not used. 
    // front face

        
        AddCubeFace(mesh, p1, p2, p3, p4, col, 0.86f);
    

    // back face
   

        AddCubeFace(mesh, p5, p6, p7, p8, col, 0.86f);
    

    // left face
   
      
        AddCubeFace(mesh, p2, p5, p8, p3, col, 0.8f);
    

    // right face

        AddCubeFace(mesh, p6, p1, p4, p7, col, 0.8f);
    

    // top face
    

        AddCubeFace(mesh, p4, p3, p8, p7, col, 1.0f);
    

    // bottom face
    

        AddCubeFace(mesh, p6, p5, p2, p1, col, 0.7f);
    
}



#endif // CUSTOM_BLOCK_H