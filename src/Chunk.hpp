#pragma once
#include "raylib.h"
#include <cinttypes>
#include "string"
#include <vector>
const int c_chunkSize = 64;
const int c_sleepTime = 30;
const int c_screenWidth = 1920;
const int c_screenHeight = 1080;

struct Tile {
    std::string name;
    int weight;
    bool goesBothWays;
    bool falls;
    Color color;
    int dissolves;
    bool fluid;
};
struct Cell {
    uint8_t type;
    uint8_t direction;
};
struct Chunk {
	Cell blocks[c_chunkSize][c_chunkSize];

    Cell moveDown[c_chunkSize];
    Cell moveUp[c_chunkSize];
    Cell moveLeft[c_chunkSize];
    Cell moveRight[c_chunkSize];

    Cell topChunkDataCopy[c_chunkSize]; //Y+ Copy
    Cell bottomChunkDataCopy[c_chunkSize]; //Y- Copy
    Cell leftChunkDataCopy[c_chunkSize]; //X- copy
    Cell rightChunkDataCopy[c_chunkSize]; //X+ copy

    Image image;
	Texture texture;
	bool toBeUpdated;
	bool containsData;
    int lastUpdate;
	bool updated[c_chunkSize][c_chunkSize] = {false};

    void Draw(int x, int y) {
		DrawTexture(texture, x * c_chunkSize, y * c_chunkSize, WHITE);
	}
	void UpdateDisplayBuffer(std::vector<Tile> &tiles) {
		containsData = false;
        Color* pixels = (Color*)image.data;
		for (int x = 0; x < c_chunkSize; x++) {
			for (int y = 0; y < c_chunkSize; y++) {
				if (blocks[x][y].type != 0) {
					containsData = true;
				}
                if (blocks[x][y].type==0) {
                    pixels[y * c_chunkSize + x] = SKYBLUE;
                }
                else if (blocks[x][y].type<tiles.size()) {
                    pixels[y * c_chunkSize + x] = tiles[blocks[x][y].type].color;
                }
			}
		}
		UpdateTexture(texture, image.data);
		toBeUpdated = false;
	}
    bool MoveDown(int x, int y) {
        if (y + 1 < c_chunkSize) {
            if (blocks[x][y + 1].type == 0 && !updated[x][y + 1]) {
                blocks[x][y + 1] = blocks[x][y];
                blocks[x][y].type = 0;
                toBeUpdated = true;
                updated[x][y + 1] = true;
                updated[x][y] = true;
                lastUpdate = 0;
                return true;
            }
        }
        else {
            if (bottomChunkDataCopy[x].type == 0 && moveDown[x].type == 0) {
                moveDown[x] = blocks[x][y];
                blocks[x][y].type = 0;
                toBeUpdated = true;
                lastUpdate = 0;

                updated[x][y] = true;
                return true;
            }
        }
        return false;
    }
    bool MoveUp(int x, int y) {
        if (y - 1 >= 0) {
            if (blocks[x][y - 1].type == 0 && !updated[x][y - 1]) {
                
                blocks[x][y - 1] = blocks[x][y];
                blocks[x][y].type = 0;
                toBeUpdated = true;
                updated[x][y - 1] = true;
                updated[x][y] = true;
                lastUpdate = 0;
                return true;
            }
        }
        else {
            if (topChunkDataCopy[x].type == 0 && moveUp[x].type == 0) {
                moveUp[x] = blocks[x][y];
                blocks[x][y].type = 0;
                toBeUpdated = true;
                lastUpdate = 0;
                updated[x][y] = true;
                return true;
            }
        }
        return false;
    }

    bool MoveLeft(int x, int y) {
        if (x - 1 >= 0) {
            if (blocks[x - 1][y].type == 0 && !updated[x - 1][y]) {
                blocks[x - 1][y] = blocks[x][y];
                blocks[x][y].type = 0;
                toBeUpdated = true;
                updated[x - 1][y] = true;
                updated[x][y] = true;
                lastUpdate = 0;
                return true;
            }
        }
        else {
            if (leftChunkDataCopy[y].type == 0 && moveLeft[y].type == 0) {
                moveLeft[y] = blocks[x][y];
                blocks[x][y].type = 0;
                updated[x][y] = true;
                toBeUpdated = true;
                lastUpdate = 0;
                return true;
            }
        }
        return false;
    }

    bool MoveRight(int x, int y) {
        if (x + 1 < c_chunkSize) {
            if (blocks[x + 1][y].type == 0 && !updated[x + 1][y]) {
                blocks[x + 1][y] = blocks[x][y];
                blocks[x][y].type = 0;
                toBeUpdated = true;
                updated[x + 1][y] = true;
                updated[x][y] = true;
                lastUpdate = 0;
                return true;
            }
        }
        else {
            if (rightChunkDataCopy[y].type == 0 && moveRight[y].type == 0) {
                moveRight[y] = blocks[x][y];
                blocks[x][y].type = 0;
                toBeUpdated = true;
                lastUpdate = 0;
                updated[x][y] = true;
                return true;
            }
        }
        return false;
    }
    bool MoveDownLeft(int x, int y)
    {
        if (x > 0 && y + 1 < c_chunkSize)
        {
            if (blocks[x - 1][y + 1].type == 0 && !updated[x - 1][y + 1])
            {
                blocks[x - 1][y + 1] = blocks[x][y];
                blocks[x][y].type = 0;

                updated[x - 1][y + 1] = true;
                updated[x][y] = true;

                toBeUpdated = true;
                lastUpdate = 0;
                return true;
            }
        }
        return false;
    }

    bool MoveDownRight(int x, int y)
    {
        if (x + 1 < c_chunkSize && y + 1 < c_chunkSize)
        {
            if (blocks[x + 1][y + 1].type == 0 && !updated[x + 1][y + 1])
            {
                blocks[x + 1][y + 1] = blocks[x][y];
                blocks[x][y].type = 0;

                updated[x + 1][y + 1] = true;
                updated[x][y] = true;

                toBeUpdated = true;
                lastUpdate = 0;
                return true;
            }
        }
        return false;
    }
    void UpdatePhysics() {
        lastUpdate++;

        for (int x = 0; x < c_chunkSize; x++)
            for (int y = 0; y < c_chunkSize; y++)
                updated[x][y] = false;

        for (int y = c_chunkSize - 1; y >= 0; y--) {
            for (int x = 0; x < c_chunkSize; x++) {

                if (updated[x][y] || blocks[x][y].type == 0)
                    continue;

                switch (blocks[x][y].type) {

                // Sand
                case 1:
                {
                    if (!MoveDown(x, y)) {
                        if (GetRandomValue(0,1))
                            MoveLeft(x,y);
                        else
                            MoveRight(x,y);
                    }
                    break;
                }

                case 3:
                {
                    if (MoveDown(x, y))
                        break;

                    if (blocks[x][y].direction==0)
                    {
                        if (!MoveLeft(x, y)) 
                        {
                            blocks[x][y].direction = 1;
                            break;
                        }   
                    }
                    else
                    {
                        if (!MoveRight(x, y))
                        {
                            blocks[x][y].direction = 0;
                            break;
                        }
                    }


                    break;
                }
                }
            }
        }
    }
};

Chunk GenCleanChunk() {
	Chunk chunk;
    chunk.toBeUpdated = false;
    chunk.containsData = false;
    chunk.lastUpdate = 100;
	for (int x = 0; x < c_chunkSize; x++) {
        chunk.moveDown[x].type = 0;
        chunk.moveUp[x].type = 0;
        chunk.moveLeft[x].type = 0;
        chunk.moveRight[x].type = 0;
        chunk.bottomChunkDataCopy[x].type = 0;
        chunk.leftChunkDataCopy[x].type = 0;
        chunk.rightChunkDataCopy[x].type = 0;
        chunk.topChunkDataCopy[x].type = 0;
        for (int y = 0; y < c_chunkSize; y++) {
            chunk.blocks[x][y].type = 0;
            chunk.blocks[x][y].direction = 0;
        }
    }
	chunk.image = GenImageColor(c_chunkSize, c_chunkSize, SKYBLUE);
	chunk.texture = LoadTextureFromImage(chunk.image);
	return chunk;
}