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
            world.UpdatePhysics(world.materials);
			DrawFPS(0, 0);
			EndDrawing();
		}
	}
};

int main() {
	App app;
	app.Run();
}