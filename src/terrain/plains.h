#ifndef PLAINS_TERRAIN_H
#define PLAINS_TERRAIN_H

#include <iostream>


#include "../TerrainGenerator.h"



class PlainsTerrainGenerator : public TerrainGenerator {
public:

    PlainsTerrainGenerator(int CHUNK_SIZE, int seed) : TerrainGenerator(CHUNK_SIZE, seed) {}

    void generateChunk(glm::vec3 position, Block * blocks){
        for(int y = 0; y < 10; y++){
            for(int x = 0; x < CHUNK_SIZE; x++) {
                for (int z = 0; z < CHUNK_SIZE; z++) {
                    int index = getIndex(x, y, z);
                    blocks[index].isActive = true;
                    blocks[index].blockType = BlockType::Stone;
                }
            }
        }
        for(int y = 10; y < 15; y++){
            for(int x = 0; x < CHUNK_SIZE; x++) {
                for (int z = 0; z < CHUNK_SIZE; z++) {
                    int index = getIndex(x, y, z);
                    blocks[index].isActive = true;
                    blocks[index].blockType = BlockType::Dirt;
                }
            }
        }
        for(int y = 15; y < 16; y++){
            for(int x = 0; x < CHUNK_SIZE; x++) {
                for (int z = 0; z < CHUNK_SIZE; z++) {
                    int index = getIndex(x, y, z);
                    blocks[index].isActive = true;
                    blocks[index].blockType = BlockType::Grass;
                }
            }
        }
    }

};


#endif 

