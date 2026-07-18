#pragma once
#include "raylib.h"
#include <cinttypes>
#include "string"
#include <vector>
#include <tuple>
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
    bool updated;
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

    Cell swapDown[c_chunkSize];   // For weight-based swaps going down
    Cell swapUp[c_chunkSize];     // For weight-based swaps going up
    Image image;
	Texture texture;
	bool toBeUpdated;
	bool containsData;
    int lastUpdate;

    void Draw(int x, int y, Vector2 cameraPosition) {
		DrawTexture(texture, x * c_chunkSize-cameraPosition.x, y * c_chunkSize-cameraPosition.y, WHITE);
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
            if (blocks[x][y + 1].type == 0 && !blocks[x][y + 1].updated) {
                blocks[x][y + 1] = blocks[x][y];
                blocks[x][y].type = 0;
                toBeUpdated = true;
                blocks[x][y + 1].updated = true;
                blocks[x][y].updated = true;
                lastUpdate = 0;
                return true;
            }
            else if (blocks[x][y+1].type!=0 && blocks[x][y+1].type!=255 && !blocks[x][y+1].updated && moveByWeight && tiles[blocks[x][y+1].type].weight<tiles[blocks[x][y].type].weight) {
                Cell oldcell = blocks[x][y+1];
                blocks[x][y + 1] = blocks[x][y];
                blocks[x][y] = oldcell;
                toBeUpdated = true;
                blocks[x][y + 1].updated = true;
                blocks[x][y].updated = true;
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
                blocks[x][y].updated = true;
                return true;
            }
            else if (bottomChunkDataCopy[x].type != 0 && bottomChunkDataCopy[x].type != 255 && 
                    moveDown[x].type == 0 && swapDown[x].type == 0 && moveByWeight && 
                    !bottomChunkDataCopy[x].updated && 
                    tiles[bottomChunkDataCopy[x].type].weight < tiles[blocks[x][y].type].weight) {
                swapDown[x] = blocks[x][y];
                blocks[x][y] = bottomChunkDataCopy[x];
                toBeUpdated = true;
                blocks[x][y].updated = true;
                lastUpdate = 0;
                bottomChunkDataCopy[x].updated = true;
                return true;
            }
        }
        return false;
    }
    bool MoveUp(int x, int y, std::vector<Tile> &tiles, bool moveByWeight=false) {
        if (y - 1 >= 0) {
            if (blocks[x][y - 1].type == 0 && !blocks[x][y - 1].updated) {
                blocks[x][y - 1] = blocks[x][y];
                blocks[x][y].type = 0;
                toBeUpdated = true;
                blocks[x][y - 1].updated = true;
                blocks[x][y].updated = true;
                lastUpdate = 0;
                return true;
            }
            else if (blocks[x][y-1].type!=0 && blocks[x][y-1].type!=255 && !blocks[x][y-1].updated && moveByWeight && tiles[blocks[x][y-1].type].weight<tiles[blocks[x][y].type].weight) {
                Cell oldcell = blocks[x][y-1];
                blocks[x][y - 1] = blocks[x][y];
                blocks[x][y] = oldcell;
                toBeUpdated = true;
                blocks[x][y - 1].updated = true;
                blocks[x][y].updated = true;
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
                blocks[x][y].updated = true;
                topChunkDataCopy[x].updated = true;
                return true;
            }
        }
        return false;
    }
    bool MoveLeft(int x, int y, std::vector<Tile> &tiles, bool moveByWeight=false) {
        if (x - 1 >= 0) {
            if (blocks[x - 1][y].type == 0 && !blocks[x - 1][y].updated) {
                blocks[x - 1][y] = blocks[x][y];
                blocks[x][y].type = 0;
                toBeUpdated = true;
                blocks[x - 1][y].updated = true;
                blocks[x][y].updated = true;
                lastUpdate = 0;
                return true;
            }
            else if (blocks[x-1][y].type!=0 && blocks[x-1][y].type!=255 && !blocks[x-1][y].updated && moveByWeight && tiles[blocks[x-1][y].type].weight<tiles[blocks[x][y].type].weight) {
                Cell oldcell = blocks[x-1][y];
                blocks[x - 1][y] = blocks[x][y];
                blocks[x][y] = oldcell;
                toBeUpdated = true;
                blocks[x - 1][y].updated = true;
                blocks[x][y].updated = true;
                lastUpdate = 0;
                return true;
            }
        }
        else {
            if (leftChunkDataCopy[y].type == 0 && moveLeft[y].type == 0 && !leftChunkDataCopy[y].updated) {
                moveLeft[y] = blocks[x][y];
                blocks[x][y].type = 0;
                blocks[x][y].updated = true;
                toBeUpdated = true;
                lastUpdate = 0;
                leftChunkDataCopy[y].updated = true;
                return true;
            }
        }
        return false;
    }
    bool MoveRight(int x, int y,  std::vector<Tile> &tiles,bool moveByWeight=false) {
        if (x + 1 < c_chunkSize) {
            if (blocks[x + 1][y].type == 0 && !blocks[x + 1][y].updated) {
                blocks[x + 1][y] = blocks[x][y];
                blocks[x][y].type = 0;
                toBeUpdated = true;
                blocks[x + 1][y].updated = true;
                blocks[x][y].updated = true;
                lastUpdate = 0;
                return true;
            }
            else if (blocks[x+1][y].type!=0 && blocks[x+1][y].type!=255 && !blocks[x+1][y].updated && moveByWeight && tiles[blocks[x+1][y].type].weight<tiles[blocks[x][y].type].weight) {
                Cell oldcell = blocks[x+1][y];
                blocks[x + 1][y] = blocks[x][y];
                blocks[x][y] = oldcell;
                toBeUpdated = true;
                blocks[x + 1][y].updated = true;
                blocks[x][y].updated = true;
                lastUpdate = 0;
                return true;
            }
        }
        else {
            if (rightChunkDataCopy[y].type == 0 && moveRight[y].type == 0 && !rightChunkDataCopy[y].updated) {
                moveRight[y] = blocks[x][y];
                blocks[x][y].type = 0;
                toBeUpdated = true;
                lastUpdate = 0;
                blocks[x][y].updated = true;
                rightChunkDataCopy[y].updated = true;
                return true;
            }
        }
        return false;
    }
    void UpdatePhysics(std::vector<Tile>&tiles) {
        lastUpdate++;

        for (int y = c_chunkSize - 1; y >= 0; y--) {
            for (int x = 0; x < c_chunkSize; x++) {

                if (blocks[x][y].updated || blocks[x][y].type == 0)
                    continue;

                switch (blocks[x][y].type) {

                // Sand
                case 1:
                {
                    if (!MoveDown(x, y,tiles,true)) {
                        if (GetRandomValue(0,1))
                            MoveLeft(x,y,tiles,true);
                        else
                            MoveRight(x,y,tiles,true);
                    }
                    break;
                }

                case 3:
                {
                    
                    if (!MoveDown(x, y,tiles,false)) {
                        if (GetRandomValue(0,1)) {
                            MoveLeft(x,y,tiles,false);
                            break;
                        }
                        else {
                            MoveRight(x,y,tiles,false);
                            break;
                        }
                    }
                    if (blocks[x][y].direction==0) {
                        if (!MoveRight(x,y,tiles,false)) {
                            blocks[x][y].direction = 1;
                        }
                    }
                    else {
                        if (!MoveLeft(x,y,tiles,false)) {
                            blocks[x][y].direction =01;
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
        chunk.moveUp[i].direction = 0;
        chunk.moveUp[i].updated = false;
        
        chunk.moveLeft[i].type = 0;
        chunk.moveLeft[i].direction = 0;
        chunk.moveLeft[i].updated = false;
        
        chunk.moveRight[i].type = 0;
        chunk.moveRight[i].direction = 0;
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
    void Init() {
        
		for (int x = 0; x < chunksX; x++) {
			for (int y = 0; y < chunksY; y++) {
				 chunkMap[std::tuple<int, int>{x, y}] = GenCleanChunk();
			}
		}
        loadMaterials("data/tile_set.txt");
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
    void Draw(Vector2 cameraPosition) {
		for (int x = 0; x < chunksX; x++) {
			for (int y = 0; y < chunksY; y++) {
				if (chunkMap[{x,y}].toBeUpdated) {
					chunkMap[{x, y}].UpdateDisplayBuffer(materials);
				}
				if (chunkMap[{x, y}].containsData) {
					chunkMap[{x, y}].Draw(x, y,cameraPosition);
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
                    
                    for (int dx = 0; dx < c_chunkSize; dx++)
                        for (int dy = 0; dy < c_chunkSize; dy++)
                            chunkMap[{x,y}].blocks[dx][dy].updated = false;
                    chunkMap[{x,y}].UpdatePhysics(tiles);
                    //swaps and moves the data
                    for (int cx = 0; cx < c_chunkSize; cx++) {
                            
                        if (chunkMap[{x,y}].swapDown[cx].type != 0 && y+1 < chunksY) {
                            if (chunkMap[{x,y+1}].blocks[cx][0].type != 0 && 
                                chunkMap[{x,y+1}].blocks[cx][0].type != 255) {
                                Cell temp = chunkMap[{x,y+1}].blocks[cx][0];
                                chunkMap[{x,y+1}].blocks[cx][0] = chunkMap[{x,y}].swapDown[cx];
                                chunkMap[{x,y}].blocks[cx][c_chunkSize-1] = temp;
                                
                                chunkMap[{x,y}].toBeUpdated = true;
                                chunkMap[{x,y+1}].toBeUpdated = true;
                                chunkMap[{x,y}].lastUpdate = 0;
                                chunkMap[{x,y+1}].lastUpdate = 0;
                            }
                            chunkMap[{x,y}].swapDown[cx].type = 0;
                        }
                        chunkMap[{x,y}].bottomChunkDataCopy[cx].updated = false;
                        chunkMap[{x,y}].topChunkDataCopy[cx].updated = false;
                        chunkMap[{x,y}].leftChunkDataCopy[cx].updated = false;
                        chunkMap[{x,y}].rightChunkDataCopy[cx].updated = false;
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
                }
            }
        }
        
        for (int x = 0; x < chunksX; x++) {
            for (int y = 0; y < chunksY; y++) {
                for (int i = 0; i < c_chunkSize; i++) {
                    chunkMap[{x,y}].moveDown[i].type = 0;
                    chunkMap[{x,y}].moveUp[i].type = 0;
                    chunkMap[{x,y}].moveLeft[i].type = 0;
                    chunkMap[{x,y}].moveRight[i].type = 0;
                }
            }
        }
    }
};