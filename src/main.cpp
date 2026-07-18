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

namespace GUI {
    bool Button(Vector2 position, Vector2 size, Color color, Color outline) {
        DrawRectangle(position.x,position.y,size.x,size.y,color);
        if (CheckCollisionRecs({position.x,position.y,size.x,size.y},{(float)GetMouseX(),(float)GetMouseY(),1,1})) {
            DrawRectangle(position.x,position.y,size.x,size.y,{255,255,255,120});
            DrawRectangleLines(position.x,position.y,size.x,size.y,outline);
            return true;
        }
        DrawRectangleLines(position.x,position.y,size.x,size.y,outline);
        return false;
    }
}

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
            DrawRectangleLines(0-player.cameraPosition.x,0-player.cameraPosition.y,1920,1080,WHITE);
            world.Draw(player.cameraPosition);
            world.UpdatePhysics(world.materials);
			player.Control();
            bool hover = false;
            std::string hoveredOver = "";
            for (int d = 0; d < world.materials.size()-1; d++) {
                int i = d+1; 
                if (GUI::Button({(float)i*32+200,200}, {32,32}, world.materials[i].color,BLACK)) {
                    choosen = i;
                }
                if (CheckCollisionRecs({(float)i*32+200,200,32,32},{float(GetMouseX()),float(GetMouseY()),1,1})) {
                    hover = true;
                    hoveredOver = world.materials[i].name;
                }
            }
            if (hoveredOver!="") {
                DrawText(hoveredOver.c_str(),GetMouseX(),GetMouseY(),16,BLACK);
            }
            
			if (IsMouseButtonDown(0) && !hover) {
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
            else if (IsMouseButtonDown(1) && !hover) {
				for (int x = 0; x < editSize; x++) {
					for (int y = 0; y < editSize; y++) {
						int updateX = x + GetMouseX()+player.cameraPosition.x;
						int updateY = y + GetMouseY()+player.cameraPosition.y;
                        if (updateX>=0 && updateY>=0 && updateX<1920 && updateY<1080) {
                                
                            if (updateX < 0 || updateY < 0) continue;
                            int chunkX = updateX / c_chunkSize;
                            int chunkY = updateY / c_chunkSize;
                            if (chunkX < 0 || chunkX >= world.chunksX || chunkY < 0 || chunkY >= world.chunksY) continue;

                            world.chunkMap[{chunkX, chunkY}].blocks[updateX % c_chunkSize][updateY % c_chunkSize].type = 0;
                            world.chunkMap[{chunkX, chunkY}].toBeUpdated = true;
                            world.chunkMap[{chunkX, chunkY}].lastUpdate = 0;
                        }
					}
				}
			}
            DrawFPS(0, 0);
			EndDrawing();
		}
	}
};

int main() {
	App app;
	app.Run();
}