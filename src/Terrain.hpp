#pragma once
#include "Chunk.hpp"
#include "raylib.h"

Chunk GenCleanChunk(int cx, int cy) {
    Chunk chunk;
    chunk.toBeUpdated = false;
    chunk.containsData = false;
    chunk.lastUpdate = 100;
    for (int x = 0; x < c_chunkSize; x++) {
        for (int y = 0; y < c_chunkSize; y++) {
            chunk.blocks[x][y].type = 0;
            chunk.blocks[x][y].direction = GetRandomValue(0, 1);
            chunk.blocks[x][y].updated = false;
            chunk.hash[x][y] = hash(x+cx*c_chunkSize,y+cy*c_chunkSize);
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
    chunk.image = GenImageColor(c_chunkSize, c_chunkSize, SKYBLUE);
    chunk.texture = LoadTextureFromImage(chunk.image);
    
    return chunk;
}



Chunk GenCleanChunkTerrain(int cx, int cy) {
    Chunk chunk;
    chunk.toBeUpdated = true;
    chunk.containsData = true;
    chunk.lastUpdate = 0;
    for (int x = 0; x < c_chunkSize; x++) {
        for (int y = 0; y < c_chunkSize; y++) {
            chunk.blocks[x][y].type = 0;
            chunk.blocks[x][y].direction = GetRandomValue(0, 1);
            chunk.blocks[x][y].updated = false;
            chunk.hash[x][y] = hash(x+cx*c_chunkSize,y+cy*c_chunkSize);
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
    chunk.image = GenImageColor(c_chunkSize, c_chunkSize, SKYBLUE);
    chunk.texture = LoadTextureFromImage(chunk.image);
    
    Image heightMap = GenImagePerlinNoise(c_chunkSize, 1, 
                                          cx * c_chunkSize,0, 0.02f);
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

            if (worldY > terrainHeight) {
                chunk.blocks[x][y].type = 2; // Grass
            }
            else if (worldY > terrainHeight - dirtDepth) {
                chunk.blocks[x][y].type = 8; // Dirt
            }
            else if (worldY > terrainHeight - grassDepth - dirtDepth) {
                chunk.blocks[x][y].type = 9; // Stone
            }
            else {
                chunk.blocks[x][y].type = 0; // Air
            }
        }
    }
    return chunk;
}