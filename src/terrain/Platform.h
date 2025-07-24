#ifndef PLATFORM_TERRAIN_H
#define PLATFORM_TERRAIN_H

#include <iostream>


#include "../TerrainGenerator.h"



class PlatformTerrainGenerator : public TerrainGenerator {
public:

    PlatformTerrainGenerator(int CHUNK_SIZE, int seed) : TerrainGenerator(CHUNK_SIZE, seed) {}

    void generateChunk(glm::vec3 position, Block * blocks){
        int y = 0;
        int size_limit = 16;
        if(position.x > size_limit| position.x < -size_limit| position.z > size_limit || position.z < -size_limit){
            return;
        }
        for(int x = 0; x < CHUNK_SIZE; x++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                int index = getIndex(x, y, z);
                blocks[index].isActive = true;
                blocks[index].blockType = BlockType::Stone;
            }
        }
    }

};


#endif 

