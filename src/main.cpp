#include "raylib.h"
#include <cinttypes>
#include <map>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include "Chunk.hpp"
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
    void UpdatePhysics() {
        
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

                    chunkMap[{x,y}].UpdatePhysics();
                }
            }
        }
    }
};
class App {
	int editSize = 15;
    World world;
public:
    
	App() {
		InitWindow(c_screenWidth, c_screenHeight, "a");
		for (int x = 0; x < world.chunksX; x++) {
			for (int y = 0; y < world.chunksY; y++) {
				 world.chunkMap[std::tuple<int, int>{x, y}] = GenCleanChunk();
			}
		}
        world.loadMaterials("data/tile_set.txt");
        
    }
	void Run() {
        int choosen = 0;
		while (!WindowShouldClose()) {
			BeginDrawing();
			ClearBackground(SKYBLUE);

			if (IsMouseButtonDown(0)) {
				for (int x = 0; x < editSize; x++) {
					for (int y = 0; y < editSize; y++) {
						int updateX = x + GetMouseX();
						int updateY = y + GetMouseY();

                        if (updateX < 0 || updateY < 0) continue;
                        int chunkX = updateX / c_chunkSize;
                        int chunkY = updateY / c_chunkSize;
                        if (chunkX < 0 || chunkX >= world.chunksX || chunkY < 0 || chunkY >= world.chunksY) continue;

						world.chunkMap[{chunkX, chunkY}].blocks[updateX % c_chunkSize][updateY % c_chunkSize].type = choosen;
						world.chunkMap[{chunkX, chunkY}].toBeUpdated = true;
                        world.chunkMap[{chunkX, chunkY}].lastUpdate = 0;
					}
				}
			}
            if (IsKeyDown(KEY_ONE)) {
                choosen = 1;
            }
            if (IsKeyDown(KEY_TWO)) {
                choosen = 2;
            }
            if (IsKeyDown(KEY_THREE)) {
                choosen = 3;
            }
            world.Draw();
            world.UpdatePhysics();
			DrawFPS(0, 0);
			EndDrawing();
		}
	}
};

int main() {
	App app;
	app.Run();
}