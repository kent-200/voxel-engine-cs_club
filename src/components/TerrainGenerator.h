#ifndef TERRAINGENERATOR_H
#define TERRAINGENERATOR_H

#include "Block.h"
#include <glm/glm.hpp>

/*
Base class for Terrain Generator:
    Inherit this class to create new terrain generators, override the generateChunk method.
    Put custom class in src/terrain/ folder.
    In main.cpp, include custom header file and then change TerrainGenerator pointer to use it.
    TerrainGenerator * terrainGenerator = new HillsTerrainGenerator(Chunk::CHUNK_SIZE, 1337);
*/

class TerrainGenerator {
protected:
    int CHUNK_SIZE;
    int seed;

    inline int getIndex(int x, int y, int z) const {
        return x + y * CHUNK_SIZE + z * CHUNK_SIZE * CHUNK_SIZE;
    }
public:
    // constructor
    TerrainGenerator(int CHUNK_SIZE, int seed);

    // contains default random
    virtual void generateChunk(glm::vec3 position, Block * blocks); 

};



TerrainGenerator::TerrainGenerator(int CHUNK_SIZE, int seed) {
    this->CHUNK_SIZE = CHUNK_SIZE;
    this->seed = seed;
}


/*
Generate chunk: 
    Use default random blocks
    override this method in custom terrain generators
*/
void TerrainGenerator::generateChunk(glm::vec3 position, Block * blocks) {
    // iterate blocks in chunk
    for (int x = 0; x < CHUNK_SIZE; x++) {
        for (int y = 0; y < CHUNK_SIZE; y++) {
            for (int z = 0; z < CHUNK_SIZE; z++) {
                int index = getIndex(x, y, z);
                // NOTE: there seems to be a "pattern" in some chunks - is the same seed be initted across threads?
                // seems like it does: https://en.cppreference.com/w/cpp/numeric/random/rand
                blocks[index].isActive = (std::rand() % 2 == 0) ? false : true;
                // make randint 1-7
                blocks[index].blockType = BlockType((std::rand() % 6) + 1);
                // blocks[index].isActive = true;
            }
        }
    }
}


#endif