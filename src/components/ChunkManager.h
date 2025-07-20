#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H

#include "Chunk.h"

#include <learnopengl/shader_m.h>
#include <unordered_map>
#include <vector>
#include <future>

/*
    TODO LIST:
    - feat: async chunk loading?
    - feat: chunk unloading?
    - fix: some chunks (def. first one) has weird alpha rendering bug - need to
   investigate the cause of this later.
*/

class TPoint3D {
  public:
    TPoint3D(float x, float y, float z) : x(x), y(y), z(z){};

    float x, y, z;
};

struct hashFunc {
    size_t operator()(const TPoint3D &k) const {
        size_t h1 = std::hash<float>()(k.x);
        size_t h2 = std::hash<float>()(k.y);
        size_t h3 = std::hash<float>()(k.z);
        return (h1 ^ (h2 << 1)) ^ h3;
    }
};

struct equalsFunc {
    bool operator()(const TPoint3D &lhs, const TPoint3D &rhs) const {
        return (lhs.x == rhs.x) && (lhs.y == rhs.y) && (lhs.z == rhs.z);
    }
};

typedef std::vector<Chunk *> ChunkList;
typedef std::unordered_map<TPoint3D, Chunk *, hashFunc, equalsFunc> ChunkMap;

struct ChunkManager {
    static int const ASYNC_NUM_CHUNKS_PER_FRAME = 12;
    static constexpr int WORLD_SIZE = 16; // world size in chunks
    static constexpr int WORLD_SIZE_CUBED =
        WORLD_SIZE * WORLD_SIZE * WORLD_SIZE;

    // Chunk *chunks[WORLD_SIZE_CUBED]; // array of chunks
    Chunk *chunks[WORLD_SIZE_CUBED] = {nullptr};

    inline int getChunkIndex(int x, int y, int z) const {
        return x + y * WORLD_SIZE + z * WORLD_SIZE * WORLD_SIZE;
    }

    inline int chunkIndexFromChunkPos(int x, int y, int z) const {
        int halfWorldSize =
            (WORLD_SIZE * (Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE)) / 2;
        int result = ((x + halfWorldSize) /
                      (Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE)) +
                     ((y + halfWorldSize) /
                      (Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE)) *
                         WORLD_SIZE +
                     ((z + halfWorldSize) /
                      (Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE)) *
                         WORLD_SIZE * WORLD_SIZE;

        return result;
    }

    std::shared_ptr<std::mutex> chunkMutex;
    std::shared_ptr<std::mutex> visibilityMutex;
    ChunkManager();
    ChunkManager(unsigned int _chunkGenDistance,
                 unsigned int _chunkRenderDistance, Shader *_terrainShader, 
                TerrainGenerator * terrainGenerator);
    ~ChunkManager();
    void update(float dt, Camera newCamera);
    void updateAsyncChunker(Camera newCamera);
    void updateLoadList();
    void updateSetupList();
    void updateRebuildList();
    void updateFlagsList();
    void updateUnloadList(glm::vec3 newCameraPosition);
    void updateVisibilityList(glm::vec3 newCameraPosition);
    void updateRenderList(glm::vec3 newCameraPosition, Frustum frustum);

    void pregenerateChunks();

    void QueueChunkToRebuild(Chunk *chunk);
    std::pair<glm::vec3, glm::vec3>
    GetChunkGenRange(glm::vec3 newCameraPosition);
    std::pair<glm::vec3, glm::vec3>
    GetChunkRenderRange(glm::vec3 newCameraPosition);
    void render(Camera newCamera);

    Shader *terrainShader;

    ChunkList chunkLoadList;
    ChunkList chunkSetupList;
    ChunkList chunkRebuildList;
    ChunkList chunkRenderList;
    ChunkList chunkUnloadList;
    ChunkList chunkVisibilityList;

    bool genChunk;
    bool forceVisibilityupdate;
    Camera camera;

    unsigned int chunkGenDistance;
    unsigned int chunkRenderDistance;
    TerrainGenerator *terrainGenerator = nullptr;
};
ChunkManager::ChunkManager() {
    chunkMutex = std::make_shared<std::mutex>();
    visibilityMutex = std::make_shared<std::mutex>();
    terrainGenerator = new TerrainGenerator(Chunk::CHUNK_SIZE, 0);
}

ChunkManager::ChunkManager(unsigned int _chunkGenDistance,
                           unsigned int _chunkRenderDistance,
                           Shader *_terrainShader, 
                           TerrainGenerator *terrainGenerator) {
    chunkGenDistance = _chunkGenDistance;
    chunkRenderDistance = _chunkRenderDistance;
    terrainShader = _terrainShader;
    genChunk = true;
    bool forceVisibilityupdate = true;
    this->terrainGenerator = terrainGenerator;

    chunkMutex = std::make_shared<std::mutex>();
    visibilityMutex = std::make_shared<std::mutex>();
}

ChunkManager::~ChunkManager() {}

// TODO: surely we can just pass the camera right?
void ChunkManager::update(float dt, Camera newCamera) {
    // if (genChunk) {
    //     updateAsyncChunker(newCameraPosition);
    //     // asyncChunkFuture = std::async(&ChunkManager::updateAsyncChunker,
    //     // this,
    //     //    newCameraPosition);
    // }
    updateLoadList();
    // std::async(std::launch::async, &ChunkManager::updateLoadList, this);
    updateSetupList();
    // std::async(std::launch::async, &ChunkManager::updateSetupList, this);
    updateRebuildList();
    // updateFlagsList();
    // updateUnloadList(newCameraPosition);
    updateVisibilityList(newCamera.cameraPos);
    updateRenderList(newCamera.cameraPos, newCamera.frustum);
    camera = newCamera;
    // cameraPosition = camera.cameraPos;
    // cameraLookAt = newCameraLookAt;
}

float roundUp(float number, float fixedBase) {
    if (fixedBase != 0 && number != 0) {
        number = ceil(number / fixedBase) * fixedBase;
    }
    return number;
}

std::pair<glm::vec3, glm::vec3>
ChunkManager::GetChunkGenRange(glm::vec3 newCameraPosition) {
    int startX = (int)roundUp(
        newCameraPosition.x -
            (chunkGenDistance * Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE),
        Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE);
    int endX = (int)roundUp(
        newCameraPosition.x +
            (chunkGenDistance * Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE),
        Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE);
    int startY = (int)roundUp(
        newCameraPosition.y -
            (chunkGenDistance * Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE),
        Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE);
    int endY = (int)roundUp(
        newCameraPosition.y +
            (chunkGenDistance * Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE),
        Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE);
    int startZ = (int)roundUp(
        newCameraPosition.z -
            (chunkGenDistance * Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE),
        Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE);
    int endZ = (int)roundUp(
        newCameraPosition.z +
            (chunkGenDistance * Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE),
        Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE);

    return std::pair<glm::vec3, glm::vec3>({startX, startY, startZ},
                                           {endX, endY, endZ});
}

std::pair<glm::vec3, glm::vec3>
ChunkManager::GetChunkRenderRange(glm::vec3 newCameraPosition) {
    int startX = (int)roundUp(newCameraPosition.x -
                                  (chunkRenderDistance * Chunk::CHUNK_SIZE *
                                   Block::BLOCK_RENDER_SIZE),
                              Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE);
    int endX = (int)roundUp(newCameraPosition.x +
                                (chunkRenderDistance * Chunk::CHUNK_SIZE *
                                 Block::BLOCK_RENDER_SIZE),
                            Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE);
    int startY = (int)roundUp(newCameraPosition.y -
                                  (chunkRenderDistance * Chunk::CHUNK_SIZE *
                                   Block::BLOCK_RENDER_SIZE),
                              Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE);
    int endY = (int)roundUp(newCameraPosition.y +
                                (chunkRenderDistance * Chunk::CHUNK_SIZE *
                                 Block::BLOCK_RENDER_SIZE),
                            Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE);
    int startZ = (int)roundUp(newCameraPosition.z -
                                  (chunkRenderDistance * Chunk::CHUNK_SIZE *
                                   Block::BLOCK_RENDER_SIZE),
                              Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE);
    int endZ = (int)roundUp(newCameraPosition.z +
                                (chunkRenderDistance * Chunk::CHUNK_SIZE *
                                 Block::BLOCK_RENDER_SIZE),
                            Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE);

    return std::pair<glm::vec3, glm::vec3>({startX, startY, startZ},
                                           {endX, endY, endZ});
}

void ChunkManager::pregenerateChunks() {
    int halfWorldSize =
        (WORLD_SIZE * (Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE)) / 2;

    std::vector<std::future<void>> futures; // Store futures to manage threads

    for (float i = -halfWorldSize; i < halfWorldSize;
         i += Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE) {
        for (float j = -halfWorldSize; j < halfWorldSize;
             j += Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE) {
            for (float k = -halfWorldSize; k < halfWorldSize;
                 k += Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE) {

                if (j > -Block::BLOCK_RENDER_SIZE) {
                    continue;
                }

                // Launch a new thread for each chunk generation
                futures.emplace_back(std::async(std::launch::async, [this, i, j,
                                                                     k]() {
                    std::lock_guard<std::mutex> lock(
                        *chunkMutex); // Ensure thread safety

                    size_t idx = chunkIndexFromChunkPos((int)i, (int)j, (int)k);
                    Chunk *currChunk = chunks[idx];
                    if (currChunk != nullptr) {
                        return;
                    }

                    // Create new chunk
                    Chunk *newChunk = new Chunk({i, j, k}, terrainShader);
                    chunks[idx] = newChunk;

                    std::lock_guard<std::mutex> visibilityLock(
                        *visibilityMutex);
                    chunkVisibilityList.push_back(newChunk);
                }));
            }
        }
    }

    // Wait for all threads to finish
    for (auto &fut : futures) {
        fut.get();
    }
}

void ChunkManager::updateAsyncChunker(Camera newCamera) {
    if (newCamera.cameraPos == camera.cameraPos) {
        return;
    }

    std::pair<glm::vec3, glm::vec3> chunkRange =
        GetChunkGenRange(camera.cameraPos);
    glm::vec3 start = chunkRange.first;
    glm::vec3 end = chunkRange.second;

    std::vector<std::future<void>> futures; // Store futures to manage threads

    for (float i = start.x; i < end.x;
         i += Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE) {
        for (float j = start.y; j < end.y;
             j += Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE) {
            for (float k = start.z; k < end.z;
                 k += Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE) {

                if (j > -Block::BLOCK_RENDER_SIZE) {
                    continue;
                }

                // Launch a new thread for each chunk generation
                futures.emplace_back(std::async(std::launch::async, [this, i, j,
                                                                     k]() {
                    std::lock_guard<std::mutex> lock(
                        *chunkMutex); // Ensure thread safety

                    size_t idx = chunkIndexFromChunkPos((int)i, (int)j, (int)k);
                    Chunk *currChunk = chunks[idx];
                    if (currChunk != nullptr) {
                        if (!currChunk->isLoaded()) {
                            std::lock_guard<std::mutex> visibilityLock(
                                *visibilityMutex);
                            chunkVisibilityList.push_back(currChunk);
                        }
                        return;
                    }

                    // Create new chunk
                    Chunk *newChunk = new Chunk({i, j, k}, terrainShader);
                    chunks[idx] = newChunk;

                    std::lock_guard<std::mutex> visibilityLock(
                        *visibilityMutex);
                    chunkVisibilityList.push_back(newChunk);
                }));
            }
        }
    }

    // Wait for all threads to finish
    for (auto &fut : futures) {
        fut.get();
    }
}

void ChunkManager::updateLoadList() {
    int lNumOfChunksLoaded = 0;
    ChunkList::iterator iterator;
    for (iterator = chunkLoadList.begin();
         iterator != chunkLoadList.end() &&
         (lNumOfChunksLoaded != ASYNC_NUM_CHUNKS_PER_FRAME);
         ++iterator) {
        Chunk *pChunk = (*iterator);
        if (pChunk->isLoaded() == false) {
            if (lNumOfChunksLoaded != ASYNC_NUM_CHUNKS_PER_FRAME) {
                pChunk->load();
                lNumOfChunksLoaded++;
                forceVisibilityupdate = true;
            }
        }
    } // Clear the load list (every frame)
    chunkLoadList.clear();
}

void ChunkManager::updateSetupList() { // Setup any chunks that have not
                                       // already been setup
    ChunkList::iterator iterator;
    for (iterator = chunkSetupList.begin(); iterator != chunkSetupList.end();
         ++iterator) {
        Chunk *pChunk = (*iterator);
        if (pChunk->isLoaded() && pChunk->isSetup() == false) {
            pChunk->setup(terrainGenerator);
            if (pChunk->isSetup()) { // Only force the visibility update if we
                                     // actually setup the chunk, some chunks
                                     // wait in the pre-setup stage...
                forceVisibilityupdate = true;
            }
        }
    } // Clear the setup list (every frame)
    chunkSetupList.clear();
}

void ChunkManager::QueueChunkToRebuild(Chunk *chunk) {
    chunkRebuildList.push_back(chunk);
}

void ChunkManager::updateRebuildList() {
    // Rebuild any chunks that are in the rebuild chunk list
    ChunkList::iterator iterator;
    int lNumRebuiltChunkThisFrame = 0;
    for (iterator = chunkRebuildList.begin();
         iterator != chunkRebuildList.end() &&
         (lNumRebuiltChunkThisFrame != ASYNC_NUM_CHUNKS_PER_FRAME);
         ++iterator) {
        Chunk *pChunk = (*iterator);
        if (pChunk->isLoaded() && pChunk->isSetup()) {
            if (lNumRebuiltChunkThisFrame != ASYNC_NUM_CHUNKS_PER_FRAME) {
                pChunk->rebuildMesh(); // If we rebuild a chunk, add it to the
                                       // list of chunks that need their render
                                       // flags updated
                // since we might now be empty or surrounded
                // m_vpChunkupdateFlagsList.push_back(pChunk); // Also add our
                // neighbours since they might now be surrounded too (If we have
                // neighbours) Chunk * pChunkXMinus = GetChunk(pChunk -> GetX()
                // - 1, pChunk -> GetY(), pChunk -> GetZ()); Chunk * pChunkXPlus
                // = GetChunk(pChunk -> GetX() + 1, pChunk -> GetY(), pChunk ->
                // GetZ()); Chunk * pChunkYMinus = GetChunk(pChunk -> GetX(),
                // pChunk -> GetY() - 1, pChunk -> GetZ()); Chunk * pChunkYPlus
                // = GetChunk(pChunk -> GetX(), pChunk -> GetY() + 1, pChunk ->
                // GetZ()); Chunk * pChunkZMinus = GetChunk(pChunk -> GetX(),
                // pChunk -> GetY(), pChunk -> GetZ() - 1); Chunk * pChunkZPlus
                // = GetChunk(pChunk -> GetX(), pChunk -> GetY(), pChunk ->
                // GetZ() + 1); if (pChunkXMinus != NULL)
                // m_vpChunkupdateFlagsList.push_back(pChunkXMinus); if
                // (pChunkXPlus != NULL)
                // m_vpChunkupdateFlagsList.push_back(pChunkXPlus); if
                // (pChunkYMinus != NULL)
                // m_vpChunkupdateFlagsList.push_back(pChunkYMinus); if
                // (pChunkYPlus != NULL)
                // m_vpChunkupdateFlagsList.push_back(pChunkYPlus); if
                // (pChunkZMinus != NULL)
                // m_vpChunkupdateFlagsList.push_back(pChunkZMinus); if
                // (pChunkZPlus != NULL)
                // m_vpChunkupdateFlagsList.push_back(pChunkZPlus); // Only
                // rebuild a certain number of chunks per frame
                lNumRebuiltChunkThisFrame++;
                forceVisibilityupdate = true;
            }
        }
    }
    // Clear the rebuild list
    chunkRebuildList.clear();
}

// unload chunks
// void ChunkManager::updateUnloadList(glm::vec3 newCameraPosition) {
//     ChunkList::iterator iterator;
//     for (iterator = chunkUnloadList.begin(); iterator !=
//     chunkUnloadList.end();
//          iterator++) {
//         Chunk *pChunk = (*iterator);
//         if (pChunk->isLoaded()) {
//             // TODO: async here?
//             std::pair<glm::vec3, glm::vec3> chunkRange =
//                 GetChunkGenRange(newCameraPosition);
//             glm::vec3 start = chunkRange.first;
//             glm::vec3 end = chunkRange.second;
//             if (!((start.x <= pChunk->chunkPosition.x &&
//                    pChunk->chunkPosition.x <= end.x) &&
//                   (start.y <= pChunk->chunkPosition.y &&
//                    pChunk->chunkPosition.y <= end.y) &&
//                   (start.z <= pChunk->chunkPosition.z &&
//                    pChunk->chunkPosition.z <= end.z))) {
//                 pChunk->unload();
//                 chunks.erase(TPoint3D(pChunk->chunkPosition.x,
//                                       pChunk->chunkPosition.y,
//                                       pChunk->chunkPosition.z));
//                 // delete pChunk;
//             }
//         }
//     }
//     chunkUnloadList.clear();
// }

void ChunkManager::updateRenderList(glm::vec3 newCameraPosition,
                                    Frustum frustum) {
    // Clear the render list each frame BEFORE we do our tests to see what
    // chunks should be rendered
    chunkRenderList.clear();
    ChunkList::iterator iterator;
    for (iterator = chunkVisibilityList.begin();
         iterator != chunkVisibilityList.end(); ++iterator) {
        Chunk *pChunk = (*iterator);
        if (pChunk != NULL) {
            if (pChunk->isLoaded() && pChunk->isSetup()) {

                std::pair<glm::vec3, glm::vec3> chunkRange =
                    GetChunkRenderRange(newCameraPosition);
                glm::vec3 start = chunkRange.first;
                glm::vec3 end = chunkRange.second;
                if (((start.x <= pChunk->chunkPosition.x &&
                      pChunk->chunkPosition.x <= end.x) &&
                     (start.y <= pChunk->chunkPosition.y &&
                      pChunk->chunkPosition.y <= end.y) &&
                     (start.z <= pChunk->chunkPosition.z &&
                      pChunk->chunkPosition.z <= end.z))) {

                    constexpr glm::vec3 offset = glm::vec3(
                        (Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE) / 2,
                        (Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE) / 2,
                        (Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE) / 2);
                    glm::vec3 chunkCenter = pChunk->chunkPosition + offset;

                    if (!frustum.CubeInFrustum(
                            chunkCenter,
                            (Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE) / 2,
                            (Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE) / 2,
                            (Chunk::CHUNK_SIZE * Block::BLOCK_RENDER_SIZE) /
                                2)) {
                        continue;
                    }
                    chunkRenderList.push_back(pChunk);
                    // delete pChunk;
                }
                // chunkRenderList.push_back(pChunk);
            }
        }
    }
}

void ChunkManager::updateVisibilityList(glm::vec3 newCameraPosition) {
    for (Chunk *chunk : chunkVisibilityList) {
        chunkLoadList.push_back(chunk);
        // chunkUnloadList.push_back(chunk);
        chunkSetupList.push_back(chunk);
    }
}

void ChunkManager::render(Camera newCamera) {
    for (Chunk *chunk : chunkRenderList) {
        chunk->render(newCamera);
    }
}

#endif // CHUNK_MANAGER
