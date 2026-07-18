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
    bool MoveDown(int x, int y, std::vector<Tile> &tiles, bool moveByWeight=false) {
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
            else if (blocks[x][y+1].type!=0 && blocks[x][y+1].type!=255 && !updated[x][y+1] && moveByWeight && tiles[blocks[x][y+1].type].weight<tiles[blocks[x][y].type].weight) {
                Cell oldcell = blocks[x][y+1];
                blocks[x][y + 1] = blocks[x][y];
                blocks[x][y] = oldcell;
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
            else if (bottomChunkDataCopy[x].type != 0 && bottomChunkDataCopy[x].type != 255 && 
                    moveDown[x].type == 0 && moveByWeight && 
                    tiles[bottomChunkDataCopy[x].type].weight < tiles[blocks[x][y].type].weight) {
                Cell oldcell = bottomChunkDataCopy[x];
                bottomChunkDataCopy[x] = blocks[x][y];
                blocks[x][y] = oldcell;
                toBeUpdated = true;
                updated[x][y] = true;
                lastUpdate = 0;
                return true;
            }
        }
        return false;
    }
    bool MoveUp(int x, int y, std::vector<Tile> &tiles, bool moveByWeight=false) {
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
            else if (blocks[x][y-1].type!=0 && blocks[x][y-1].type!=255 && !updated[x][y-1] && moveByWeight && tiles[blocks[x][y-1].type].weight<tiles[blocks[x][y].type].weight) {
                Cell oldcell = blocks[x][y-1];
                blocks[x][y - 1] = blocks[x][y];
                blocks[x][y] = oldcell;
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

    bool MoveLeft(int x, int y, std::vector<Tile> &tiles, bool moveByWeight=false) {
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
            else if (blocks[x-1][y].type!=0 && blocks[x-1][y].type!=255 && !updated[x-1][y] && moveByWeight && tiles[blocks[x-1][y].type].weight<tiles[blocks[x][y].type].weight) {
                Cell oldcell = blocks[x-1][y];
                blocks[x - 1][y] = blocks[x][y];
                blocks[x][y] = oldcell;
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

    bool MoveRight(int x, int y,  std::vector<Tile> &tiles,bool moveByWeight=false) {
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
            else if (blocks[x+1][y].type!=0 && blocks[x+1][y].type!=255 && !updated[x+1][y] && moveByWeight && tiles[blocks[x+1][y].type].weight<tiles[blocks[x][y].type].weight) {
                Cell oldcell = blocks[x+1][y];
                blocks[x + 1][y] = blocks[x][y];
                blocks[x][y] = oldcell;
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
    void UpdatePhysics(std::vector<Tile>&tiles) {
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
                    if (!MoveDown(x, y,tiles,false)) {
                        if (GetRandomValue(0,1))
                            MoveLeft(x,y,tiles,false);
                        else
                            MoveRight(x,y,tiles,false);
                    }
                    break;
                }

                case 3:
                {
                    if (!MoveDown(x, y,tiles,true)) {
                        if (GetRandomValue(0,1)) {
                            MoveLeft(x,y,tiles,true);
                            break;
                        }
                        else {
                            MoveRight(x,y,tiles,true);
                            break;
                        }
                    }
                    if (blocks[x][y].direction==0)
                    {
                        if (!MoveLeft(x, y,tiles,false)) 
                        {
                            blocks[x][y].direction = 1;
                            break;
                        }   
                    }
                    else
                    {
                        if (!MoveRight(x, y,tiles,false))
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
            chunk.blocks[x][y].direction = GetRandomValue(0,1);
        }
    }
	chunk.image = GenImageColor(c_chunkSize, c_chunkSize, SKYBLUE);
	chunk.texture = LoadTextureFromImage(chunk.image);
	return chunk;
}

struct World {
    std::map<std::tuple<int, int>, Chunk> chunkMap;
	
    int chunksX = (c_screenWidth + c_chunkSize - 1) / c_chunkSize;
    int chunksY = (c_screenHeight + c_chunkSize - 1) / c_chunkSize;
    std::vector<Tile> materials;
    std::string trim(const std::string& s) {
        size_t f = s.find_first_not_of(" \t");
        size_t l = s.find_last_not_of(" \t");
        return (f == std::string::npos) ? "" : s.substr(f, (l - f + 1));
    }

    Color parseColor(std::string s) {
        s = s.substr(1, s.size() - 2); // Remove parens
        std::stringstream ss(s);
        std::string val;
        std::vector<int> c;
        while (std::getline(ss, val, ',')) c.push_back(std::stoi(val));
        return {c[0], c[1], c[2], c[3]};
    }
    void loadMaterials(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) return;

        std::string line;
        Tile* current = nullptr;

        while (std::getline(file, line)) {
            if (line.empty()) continue;

            if (line.back() == ':') {
                materials.emplace_back();
                current = &materials.back();
                current->name = line.substr(0, line.size() - 1);
                current->dissolves = -1; 
            } else if (current) {
                size_t sep = line.find('=');
                std::string k = trim(line.substr(0, sep));
                std::string v = trim(line.substr(sep + 1));

                if (k == "weight") current->weight = std::stoi(v);
                else if (k == "falls") current->falls = (v == "true");
                else if (k == "goes_to_sides") current->goesBothWays = (v == "true");
                else if (k == "color") current->color = parseColor(v);
                else if (k == "dissolve" && v != "None") current->dissolves = std::stoi(v);
                else if (k == "fluid" && v != "None") current->fluid = (v == "true");
            }
        }
    }
    void Draw() {
		for (int x = 0; x < chunksX; x++) {
			for (int y = 0; y < chunksY; y++) {
				if (chunkMap[{x,y}].toBeUpdated) {
					chunkMap[{x, y}].UpdateDisplayBuffer(materials);
				}
				if (chunkMap[{x, y}].containsData) {
					chunkMap[{x, y}].Draw(x, y);
                }

			}
		}
    }
    void DebugActivityDisplay() {
        
		for (int x = 0; x < chunksX; x++) {
			for (int y = 0; y < chunksY; y++) {
                if (chunkMap[{x,y}].containsData) {
                    DrawRectangleLines(x*c_chunkSize,y*c_chunkSize,c_chunkSize,c_chunkSize,BLACK);
                    DrawText(TextFormat("%d", chunkMap[{x,y}].lastUpdate),x*c_chunkSize,y*c_chunkSize,16,BLACK);
                }
            }
        }
    }
    void UpdatePhysics(std::vector<Tile> &tiles) {
        
        for (int x = 0; x < chunksX; x++) {
            for (int y = 0; y < chunksY; y++) {
                if (chunkMap[{x,y}].containsData ) {
                    for (int cx = 0; cx < c_chunkSize; cx++) {

                        if (y+1<chunksY) chunkMap[{x,y}].bottomChunkDataCopy[cx] = chunkMap[{x,y+1}].blocks[cx][0];
                        else chunkMap[{x,y}].bottomChunkDataCopy[cx].type = 255;
                        if (y-1>=0) chunkMap[{x,y}].topChunkDataCopy[cx] = chunkMap[{x,y-1}].blocks[cx][c_chunkSize-1];
                        else chunkMap[{x,y}].topChunkDataCopy[cx].type = 255;
                        if (x-1>=0) chunkMap[{x,y}].leftChunkDataCopy[cx] = chunkMap[{x-1,y}].blocks[c_chunkSize-1][cx];
                        else chunkMap[{x,y}].leftChunkDataCopy[cx].type = 255;
                        if (x+1<chunksX) chunkMap[{x,y}].rightChunkDataCopy[cx] = chunkMap[{x+1,y}].blocks[0][cx];
                        else chunkMap[{x,y}].rightChunkDataCopy[cx].type = 255;
                        if (chunkMap[{x,y}].moveDown[cx].type != 0 && y+1<chunksY) {
                            if (chunkMap[{x,y+1}].blocks[cx][0].type == 0) {
                                chunkMap[{x,y+1}].blocks[cx][0] = chunkMap[{x,y}].moveDown[cx];
                                chunkMap[{x,y}].moveDown[cx].type = 0;
                                chunkMap[{x,y}].toBeUpdated = true;
                                chunkMap[{x,y+1}].toBeUpdated = true;
                                chunkMap[{x,y+1}].containsData = true;
                                chunkMap[{x,y+1}].lastUpdate = 0;
                            }
                        }

                        if (chunkMap[{x,y}].moveUp[cx].type != 0 && y - 1 >= 0) {
                            if (chunkMap[{x,y-1}].blocks[cx][c_chunkSize-1].type == 0) {
                                chunkMap[{x,y-1}].blocks[cx][c_chunkSize-1] = chunkMap[{x,y}].moveUp[cx];
                                chunkMap[{x,y}].moveUp[cx].type = 0;
                                chunkMap[{x,y}].toBeUpdated = true;
                                chunkMap[{x,y-1}].toBeUpdated = true;
                                chunkMap[{x,y-1}].containsData = true;
                                chunkMap[{x,y-1}].lastUpdate = 0;
                            }
                        }

                        if (chunkMap[{x,y}].moveLeft[cx].type != 0 && x - 1 >= 0) {
                            if (chunkMap[{x-1,y}].blocks[c_chunkSize-1][cx].type == 0) {
                                chunkMap[{x-1,y}].blocks[c_chunkSize-1][cx] = chunkMap[{x,y}].moveLeft[cx];
                                chunkMap[{x,y}].moveLeft[cx].type = 0;
                                chunkMap[{x,y}].toBeUpdated = true;
                                chunkMap[{x-1,y}].toBeUpdated = true;
                                chunkMap[{x-1,y}].containsData = true;
                                chunkMap[{x-1,y}].lastUpdate = 0;
                            }
                        }

                        if (chunkMap[{x,y}].moveRight[cx].type != 0 && x + 1 < chunksX) {
                            if (chunkMap[{x+1,y}].blocks[0][cx].type == 0) {
                                chunkMap[{x+1,y}].blocks[0][cx] = chunkMap[{x,y}].moveRight[cx];
                                chunkMap[{x,y}].moveRight[cx].type = 0;
                                chunkMap[{x,y}].toBeUpdated = true;
                                chunkMap[{x+1,y}].toBeUpdated = true;
                                chunkMap[{x+1,y}].containsData = true;
                                chunkMap[{x+1,y}].lastUpdate = 0;
                            }
                        }
                    }

                    chunkMap[{x,y}].UpdatePhysics(tiles);
                }
            }
        }
    }
};