#pragma once
#include "Chunk.hpp"
#include "raylib.h"

Chunk GenCleanChunk(int cx, int cy, bool genCleanImage=false) {
    Chunk chunk;
    chunk.toBeUpdated = false;
    chunk.containsData = false;
    chunk.lastUpdate = 100;
    for (int x = 0; x < c_chunkSize; x++) {
        for (int y = 0; y < c_chunkSize; y++) {
            chunk.blocks[x][y].type = 0;
            chunk.blocks[x][y].direction = GetRandomValue(0, 1);
            chunk.blocks[x][y].updated = false;
            
        }
    }
    for (int i = 0; i < c_chunkSize; i++) {
        chunk.moveDown[i].type = 0;
        chunk.moveDown[i].direction = 0;
        chunk.moveDown[i].updated = false;
        
        chunk.moveUp[i].type = 0;
        chunk.moveUp[i].lifeTime = -1;
        chunk.moveUp[i].direction = 0;
        chunk.moveUp[i].updated = false;
        
        chunk.moveLeft[i].type = 0;
        chunk.moveLeft[i].direction = 0;
        chunk.moveLeft[i].lifeTime = -1;
        chunk.moveLeft[i].updated = false;
        
        chunk.moveRight[i].type = 0;
        chunk.moveRight[i].direction = 0;
        chunk.moveRight[i].lifeTime = -1;
        chunk.moveRight[i].updated = false;
        
        chunk.topChunkDataCopy[i].type = 0;
        chunk.topChunkDataCopy[i].direction = 0;
        chunk.topChunkDataCopy[i].updated = false;
        
        chunk.bottomChunkDataCopy[i].type = 0;
        chunk.bottomChunkDataCopy[i].direction = 0;
        chunk.bottomChunkDataCopy[i].updated = false;
        
        chunk.leftChunkDataCopy[i].type = 0;
        chunk.leftChunkDataCopy[i].direction = 0;
        chunk.leftChunkDataCopy[i].updated = false;
        chunk.updatedYLine[i] = false;

        chunk.rightChunkDataCopy[i].type = 0;
        chunk.rightChunkDataCopy[i].direction = 0;
        chunk.rightChunkDataCopy[i].updated = false;
        
        chunk.swapDown[i].type = 0;
        chunk.swapDown[i].direction = 0;
        chunk.swapDown[i].updated = false;
        
        chunk.swapUp[i].type = 0;
        chunk.swapUp[i].direction = 0;
        chunk.swapUp[i].updated = false;
    }
    if (genCleanImage) {
        chunk.image = GenImageColor(c_chunkSize, c_chunkSize, SKYBLUE);
        chunk.texture = LoadTextureFromImage(chunk.image);
            
    }
    
    return chunk;
}



Chunk GenCleanChunkTerrain(int cx, int cy, bool genCleaImage= false) {
    Chunk chunk;
    chunk.toBeUpdated = true;
    chunk.containsData = true;
    chunk.lastUpdate = 0;
    for (int x = 0; x < c_chunkSize; x++) {
        for (int y = 0; y < c_chunkSize; y++) {
            chunk.blocks[x][y].type = 0;
            chunk.blocks[x][y].direction = GetRandomValue(0, 1);
            chunk.blocks[x][y].updated = false;
        }
    }
    for (int i = 0; i < c_chunkSize; i++) {
        chunk.moveDown[i].type = 0;
        chunk.moveDown[i].direction = 0;
        chunk.moveDown[i].updated = false;
        chunk.updatedYLine[i] = false;
        chunk.moveUp[i].type = 0;
        chunk.moveUp[i].lifeTime = -1;
        chunk.moveUp[i].direction = 0;
        chunk.moveUp[i].updated = false;
        
        chunk.moveLeft[i].type = 0;
        chunk.moveLeft[i].direction = 0;
        chunk.moveLeft[i].lifeTime = -1;
        chunk.moveLeft[i].updated = false;
        
        chunk.moveRight[i].type = 0;
        chunk.moveRight[i].direction = 0;
        chunk.moveRight[i].lifeTime = -1;
        chunk.moveRight[i].updated = false;
        
        chunk.topChunkDataCopy[i].type = 0;
        chunk.topChunkDataCopy[i].direction = 0;
        chunk.topChunkDataCopy[i].updated = false;
        
        chunk.bottomChunkDataCopy[i].type = 0;
        chunk.bottomChunkDataCopy[i].direction = 0;
        chunk.bottomChunkDataCopy[i].updated = false;
        
        chunk.leftChunkDataCopy[i].type = 0;
        chunk.leftChunkDataCopy[i].direction = 0;
        chunk.leftChunkDataCopy[i].updated = false;
        
        chunk.rightChunkDataCopy[i].type = 0;
        chunk.rightChunkDataCopy[i].direction = 0;
        chunk.rightChunkDataCopy[i].updated = false;
        
        chunk.swapDown[i].type = 0;
        chunk.swapDown[i].direction = 0;
        chunk.swapDown[i].updated = false;
        
        chunk.swapUp[i].type = 0;
        chunk.swapUp[i].direction = 0;
        chunk.swapUp[i].updated = false;
    }
    if (genCleaImage) {
        chunk.image = GenImageColor(c_chunkSize, c_chunkSize, SKYBLUE);
        chunk.texture = LoadTextureFromImage(chunk.image);
    
    }
    
    Image caveMap = GenImagePerlinNoise(c_chunkSize, c_chunkSize, 
                                        cx, cy, 0.08f);
    
    Image heightMap = GenImagePerlinNoise(c_chunkSize, 1, 
                                          cx,0, 0.02f);
    for (int x = 0; x < c_chunkSize; x++) {
        int worldX = cx + x;
        float heightNoise = GetImageColor(heightMap, x, 0).r / 255.0f;

        const int baseHeight = 300;
        const int amplitude = 50;

        const int grassDepth = 3;   // 3 pixels of grass
        const int dirtDepth = 6;    // 6 pixels of dirt below the grass

        int terrainHeight = baseHeight + (heightNoise - 0.5f) * amplitude;

        for (int y = 0; y < c_chunkSize; y++) {
            int worldY = cy + y;
            float caveValue = (float)GetImageColor(caveMap, x, y).r / 255.0f;
            bool isCave = caveValue > 0.35f ;
               
            if (worldY > terrainHeight) {
                 chunk.blocks[x][y].type = 2*isCave; // Stone
            }
            else if (worldY > terrainHeight - dirtDepth) {
                chunk.blocks[x][y].type = 8*isCave; // Grass
            }
            else if (worldY > terrainHeight - grassDepth - dirtDepth) {
               
                chunk.blocks[x][y].type = 9*isCave; // Dirt
            }
            else {
                chunk.blocks[x][y].type = 0; // Air
            }
        }
    }
    return chunk;
}