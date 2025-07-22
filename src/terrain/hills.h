#ifndef HILL_TERRAIN_H
#define HILL_TERRAIN_H

#include <iostream>


#include "../components/TerrainGenerator.h"

#define STB_PERLIN_IMPLEMENTATION
#include "../../libs/stb_perlin.h"



class HillsTerrainGenerator : public TerrainGenerator {
    float frequency = 0.05f;
    float amplitude = 5.0f;
public:
    HillsTerrainGenerator(int CHUNK_SIZE, int seed) : TerrainGenerator(CHUNK_SIZE, seed) {}

    void generateChunk(glm::vec3 position, Block * blocks){
        // iterate x/z
        for(int x = 0; x < CHUNK_SIZE; x++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                float realx = (float) (position.x + x) * frequency;
                float realz = (float) (position.z + z) * frequency;

                float height = stb_perlin_noise3_seed(realx, 0.0f, realz, 0, 0, 0, seed);
                int blockHeight = CHUNK_SIZE - static_cast<int>((height + 1.0f) * amplitude);

                // limit block height to be within chunk bounds
                blockHeight = std::max(5, std::min(blockHeight, CHUNK_SIZE - 1)); 
                

                // fill blocks up to the height

                // fill grass
                int index = getIndex(x, blockHeight, z);
                blocks[index].isActive = true;
                blocks[index].blockType = BlockType::Grass;

                // fill dirt
                for(int y = blockHeight - 1; y >= blockHeight - 5; y--) {
                    if (y < 0) continue; // skip if below ground level
                    index = getIndex(x, y, z);
                    blocks[index].isActive = true;
                    blocks[index].blockType = BlockType::Dirt;
                }

                // fill stone
                for(int y = blockHeight - 5; y >= 0; y--) {
                    int index = getIndex(x, y, z);
                    blocks[index].isActive = true;
                    blocks[index].blockType = BlockType::Stone;
                }

            }
        }
    }

};


#endif 

