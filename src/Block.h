#ifndef BLOCK_H
#define BLOCK_H

#include <unordered_map>
#include <vector>
#include <utility> // for std::pair

enum BlockType {
    Default,
    Grass,
    Sand,
    Dirt,
    Water,
    Stone,
    Wood,
    NumTypes,
};

// create map of texture coordinates for each size and type
// stores coordinates as x,y int, so top left 0, 0. to the right is 1, 0, below is 0, 1 and bottom right is 15, 15
// input top left corner, will get 0,0 to 1,1
// NOTE: Maximum textures in map is 63x63 icons due to 5 bit coordinates.
// front, back, left, right, top, bottom
std::unordered_map<int, std::vector<std::pair<int, int>>> textureCoordMap = {
    {BlockType::Grass, {{3, 0}, {3, 0}, {3, 0}, {3, 0}, {0, 0}, {2, 0}}},
    {BlockType::Sand, {{0, 11}, {0, 11}, {0, 11}, {0, 11}, {0, 11}, {0, 11}}},
    {BlockType::Dirt, {{2, 0}, {2, 0}, {2, 0}, {2, 0}, {2, 0}, {2, 0}}},
    {BlockType::Water, {{13, 12}, {13, 12}, {13, 12}, {13, 12}, {13, 12}, {13, 12}}},
    {BlockType::Stone, {{1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}, {1, 0}}},
    {BlockType::Wood, {{4, 0}, {4, 0}, {4, 0}, {4, 0}, {4, 0}, {4, 0}}}
};



struct Block {
    static constexpr int BLOCK_RENDER_SIZE = 2;
    // TODO: do we keep this in CPU or in GPU ?
    bool isActive;    Block(){};
    ~Block(){};
    BlockType blockType;
};

#endif // BLOCK_H