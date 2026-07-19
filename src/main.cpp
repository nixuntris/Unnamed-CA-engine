#include "raylib.h"
#include <cinttypes>
#include <map>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "Gui.hpp"
#include <map>
#include <sstream>
#include "Chunk.hpp"
#include "Terrain.hpp"
#include "Physics.hpp"

struct Player {
    Vector2 cameraPosition;
    float cameraZoom;
	int editSize = 15;
    int choosen = 0;
    void Init() {
        cameraPosition = {0,0};
        cameraZoom = 0;
    }
    void Control() {
        
        float wheel = GetMouseWheelMove();
        this->cameraZoom += wheel * 0.1f;
            
        if (wheel != 0) {
            Vector2 mousePos = GetMousePosition();
            Vector2 worldPos = {
                (mousePos.x / this->cameraZoom) + this->cameraPosition.x,
                (mousePos.y / this->cameraZoom) + this->cameraPosition.y
            };
            this->cameraZoom += wheel * 0.1f;
            this->cameraZoom = Clamp(this->cameraZoom, 0.5f, 3.0f);
            this->cameraPosition.x = worldPos.x - (mousePos.x / this->cameraZoom);
            this->cameraPosition.y = worldPos.y - (mousePos.y / this->cameraZoom);
        }
        if (this->cameraZoom < 0.5f) this->cameraZoom = 0.5f;
        if (this->cameraZoom > 3.0f) this->cameraZoom= 3.0f;
        if (IsKeyDown(KEY_A)) {
            cameraPosition.x -= 2/this->cameraZoom;
        }
        if (IsKeyDown(KEY_D)) {
            cameraPosition.x += 2/this->cameraZoom;
        }
        if (IsKeyDown(KEY_W)) {
            cameraPosition.y -= 2/this->cameraZoom;
        }
        if (IsKeyDown(KEY_S)) {
            cameraPosition.y += 2/this->cameraZoom;
        }
    }
    void Editor(World *world) {
         bool hover = false;
            std::string hoveredOver = "";
            for (int d = 0; d < world->materials.size()-1; d++) {
                int i = d+1; 
                if (GUI::Button({(float)i*32+200,200}, {32,32}, world->materials[i].color,BLACK)) {
                    choosen = i;
                }
                if (CheckCollisionRecs({(float)i*32+200,200,32,32},{float(GetMouseX()),float(GetMouseY()),1,1})) {
                    hover = true;
                    hoveredOver = world->materials[i].name;
                }
            }
            
            if (GUI::ButtonWithSlider({100, 100}, {200, 40}, GRAY, WHITE, &editSize, 0, 100)) {
                hover = true;
            }
            if (hoveredOver!="") {
                DrawText(hoveredOver.c_str(),GetMouseX(),GetMouseY(),16,BLACK);
            }
                        
            Vector2 mousePos = GetMousePosition();

            Vector2 worldMousePos = {
                (mousePos.x / cameraZoom) + cameraPosition.x,
                (mousePos.y / cameraZoom) + cameraPosition.y
            };
            int endX = 0;
            int startX = c_screenWidth;
			if (IsMouseButtonDown(0) && !hover) {
				for (int x = 0; x < editSize; x++) {
					for (int y = 0; y < editSize; y++) {
						
                        int updateX = x + worldMousePos.x;
						int updateY = y + worldMousePos.y;
                        
                        if (updateX<startX) startX = updateX;
                        if (updateX>endX) endX = updateX;

                        if (updateX>=0 && updateY>=0 && updateX<c_screenWidth && updateY<c_screenHeight) {
                                
                            if (updateX < 0 || updateY < 0) continue;
                            int chunkX = updateX / c_chunkSize;
                            int chunkY = updateY / c_chunkSize;
                            if (chunkX < 0 || chunkX >= world->chunksX || chunkY < 0 || chunkY >= world->chunksY) continue;
                            world->chunkMap[{chunkX, chunkY}].blocks[updateX % c_chunkSize][updateY % c_chunkSize].lifeTime = world->materials[choosen].lifeTime;
                            world->chunkMap[{chunkX, chunkY}].blocks[updateX % c_chunkSize][updateY % c_chunkSize].type = choosen;
                            world->chunkMap[{chunkX, chunkY}].toBeUpdated = true;
                            world->chunkMap[{chunkX, chunkY}].lastUpdate = 0;
                        }
					}
				}
			}
            else if (IsMouseButtonDown(1) && !hover) {
				for (int x = 0; x < editSize; x++) {
					for (int y = 0; y < editSize; y++) {
						int updateX = x + worldMousePos.x;
						int updateY = y + worldMousePos.y;
                        if (updateX<startX) startX = updateX;
                        if (updateX>endX) endX = updateX;
                        if (updateX>=0 && updateY>=0 && updateX<c_screenWidth && updateY<c_screenHeight) {
                                
                            if (updateX < 0 || updateY < 0) continue;
                            int chunkX = updateX / c_chunkSize;
                            int chunkY = updateY / c_chunkSize;
                            if (chunkX < 0 || chunkX >= world->chunksX || chunkY < 0 || chunkY >= world->chunksY) continue;

                            world->chunkMap[{chunkX, chunkY}].blocks[updateX % c_chunkSize][updateY % c_chunkSize].type = 0;
                            world->chunkMap[{chunkX, chunkY}].toBeUpdated = true;
                            world->chunkMap[{chunkX, chunkY}].lastUpdate = 0;
                        }
					}
				}
			}
            if (startX!=c_screenWidth && endX!=0) {
                for (int x = startX; x < endX; x++) {
                    world->toBeUpdatedLine[x] = true;
                }
            }
    }
};


class App {
    World world;
    Player player;
public:
    
    void Init() {
        
        world.loadMaterials("data/tile_set.txt");
		for (int x = 0; x < world.chunksX; x++) {
			for (int y = 0; y < world.chunksY; y++) {
				world.chunkMap[std::tuple<int, int>{x, y}] = GenCleanChunkTerrain(x*c_chunkSize,y*c_chunkSize);
                CAGI cagi;
                cagi.Init(x,y);
                world.lightMap[std::tuple<int, int>{x, y}] = cagi; 

            }
		}
        for (int x = 0; x < c_screenWidth; x++) {
            world.toBeUpdatedLine[x] = true;
            world.lastUpdated[x] = 0;
        }
 
        world.UpdateLighting(world.materials);
               
        for (int x = 0; x < world.chunksX; x++) {
            for (int y = 0; y < world.chunksY; y++) {
                world.lightMap[{x,y}].Update(world.chunkMap[{x,y}].blocks, world.materials);
            }
        }
    }
	App() {
        SetTraceLogLevel(LOG_NONE); 
		InitWindow(1920, 1080, "a");
        Init();   
        //SetTargetFPS(60);

    }
    
	void Run() {
        int frame = 0;
		while (!WindowShouldClose()) {
			BeginDrawing();
			ClearBackground(SKYBLUE);
            if (IsMouseButtonDown(2) && frame%10==0) {
                Vector2 mousePos = GetMousePosition();
                Vector2 worldPos = {
                    (mousePos.x / player.cameraZoom) +  player.cameraPosition.x,
                    (mousePos.y /  player.cameraZoom) +  player.cameraPosition.y
                };

            }    
            DrawRectangleLines(
                -player.cameraPosition.x * player.cameraZoom,
                -player.cameraPosition.y *  player.cameraZoom,
                c_screenWidth *  player.cameraZoom,
                c_screenHeight *  player.cameraZoom,
                WHITE
            );
            //world.Draw(player.cameraPosition,player.cameraZoom);
            world.UpdatePhysics(world.materials);
                        
            int beginX = (int)(player.cameraPosition.x / c_chunkSize);
            int endX = (int)((player.cameraPosition.x + (GetScreenWidth() / player.cameraZoom)) / c_chunkSize) + 1;
            int beginY = (int)(player.cameraPosition.y / c_chunkSize);
            int endY = (int)((player.cameraPosition.y + (GetScreenHeight() / player.cameraZoom)) / c_chunkSize) + 1;

            beginX = std::max(0, beginX);
            endX = std::min(world.chunksX, endX);
            beginY = std::max(0, beginY);
            endY = std::min(world.chunksY, endY);
            
            for (int x = beginX; x < endX; x++) {
                for (int y = beginY; y < endY; y++) {
                    
                    world.lightMap[{x,y}].Draw(x,y,player.cameraPosition,player.cameraZoom);
                    

                }
            }
            if (frame%5==0) {
                    
                for (int x = 0; x < world.chunksX; x++) {
                    for (int y = 0; y < world.chunksY; y++) {
                
                        bool update = false;
                        for (int cx = 0; cx < c_chunkSize; cx++) {
                            if (world.toBeUpdatedLine[cx+x*c_chunkSize]) {
                                update=true;
                                break;
                            }
                        }
                        if (world.chunkMap[{x,y}].lastUpdate==0) update =true;
                        if (update) world.lightMap[{x,y}].Update(world.chunkMap[{x,y}].blocks,world.materials);
                        
                    }
                }
                world.UpdateLighting(world.materials);
            
            }
            if (IsKeyDown(KEY_G)) {
                world.SaveWorld();
            }
            if (IsKeyDown(KEY_H)) {
                world.LoadWorld();
            }
            player.Control();
            
            player.Editor(&world);
            DrawFPS(0, 0);
			EndDrawing();
            frame++;
		}
	}
};

int main() {
	App app;
	app.Run();
}