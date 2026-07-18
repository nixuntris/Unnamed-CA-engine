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
struct Player {
    Vector2 cameraPosition;
    float cameraZoom;
    void Init() {
        cameraPosition = {0,0};
        cameraZoom = 0;
    }
    void Control() {
        if (IsKeyDown(KEY_A)) {
            cameraPosition.x -= 2;
        }
        if (IsKeyDown(KEY_D)) {
            cameraPosition.x += 2;
        }
        if (IsKeyDown(KEY_W)) {
            cameraPosition.y -= 2;
        }
        if (IsKeyDown(KEY_S)) {
            cameraPosition.y += 2;
        }
    }
};
class App {
	int editSize = 15;
    World world;
    Player player;
public:
    
	App() {
		InitWindow(c_screenWidth, c_screenHeight, "a");
        world.Init();   
    }
	void Run() {
        int choosen = 0;
		while (!WindowShouldClose()) {
			BeginDrawing();
			ClearBackground(SKYBLUE);

			if (IsMouseButtonDown(0)) {
				for (int x = 0; x < editSize; x++) {
					for (int y = 0; y < editSize; y++) {
						int updateX = x + GetMouseX()+player.cameraPosition.x;
						int updateY = y + GetMouseY()+player.cameraPosition.y;
                        if (updateX>=0 && updateY>=0 && updateX<1920 && updateY<1080) {
                                
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
            world.Draw(player.cameraPosition);
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