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
#include <chrono>
const bool renderLight = true;
struct Player {
    Vector2 cameraPosition;
    float cameraZoom;
	int editSize = 15;
    int choosen = 0;
    void Init() {
        cameraPosition = {0,0};
        cameraZoom = 2;
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
            this->cameraZoom = Clamp(this->cameraZoom, 2, 5.0f);
            this->cameraPosition.x = worldPos.x - (mousePos.x / this->cameraZoom);
            this->cameraPosition.y = worldPos.y - (mousePos.y / this->cameraZoom);
        }
        if (this->cameraZoom < 2.0f) this->cameraZoom = 2.0f;
        if (this->cameraZoom > 5.0f) this->cameraZoom= 5.0f;
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
    void Editor(CA::World *world) {
        bool hover = false;
        std::string hoveredOver = "";
        
        float guiScale = GetScreenHeight() / 1080.0f;
        
        float baseY = 200 * guiScale;
        float baseX = 200 * guiScale;
        float buttonSize = 32 * guiScale;
        float buttonSpacing = 32 * guiScale;
        float textSize = 16 * guiScale;
        
        for (int d = 0; d < world->materials.size()-1; d++) {
            int i = d+1; 
            float xPos = (float)i * buttonSpacing + baseX;
            
            if (GUI::Button({xPos, baseY}, {buttonSize, buttonSize}, world->materials[i].color, BLACK)) {
                choosen = i;
            }
            
            Rectangle buttonRect = {xPos, baseY, buttonSize, buttonSize};
            Rectangle mouseRect = {float(GetMouseX()), float(GetMouseY()), 1, 1};
            if (CheckCollisionRecs(buttonRect, mouseRect)) {
                hover = true;
                hoveredOver = world->materials[i].name;
            }
        }
        
        float sliderWidth = 200 * guiScale;
        float sliderHeight = 40 * guiScale;
        float sliderX = 100 * guiScale;
        float sliderY = 100 * guiScale;
        
        if (GUI::ButtonWithSlider({sliderX, sliderY}, {sliderWidth, sliderHeight}, GRAY, WHITE, &editSize, 0, 100)) {
            hover = true;
        }
        
        if (hoveredOver != "") {
            DrawText(hoveredOver.c_str(), GetMouseX(), GetMouseY(), textSize, BLACK);
        }
        Vector2 mousePos = GetMousePosition();
        Vector2 worldMousePos = {
            (mousePos.x / cameraZoom) + cameraPosition.x,
            (mousePos.y / cameraZoom) + cameraPosition.y
        };
        
        int endX = 0;
        int startX = CA::c_screenWidth;
        
        if (IsMouseButtonDown(0) && !hover) {
            for (int x = 0; x < editSize; x++) {
                for (int y = 0; y < editSize; y++) {
                    int updateX = x + worldMousePos.x;
                    int updateY = y + worldMousePos.y;
                    
                    if (updateX < startX) startX = updateX;
                    if (updateX > endX) endX = updateX;
                    
                    if (updateX >= 0 && updateY >= 0 && updateX < CA::c_screenWidth && updateY < CA::c_screenHeight) {
                        if (updateX < 0 || updateY < 0) continue;
                        int chunkX = updateX / CA::c_chunkSize;
                        int chunkY = updateY / CA::c_chunkSize;
                        if (chunkX < 0 || chunkX >= world->chunksX || chunkY < 0 || chunkY >= world->chunksY) continue;
                        world->chunkMap[{chunkX, chunkY}].blocks[updateX % CA::c_chunkSize][updateY % CA::c_chunkSize].lifeTime = world->materials[choosen].lifeTime;
                        world->chunkMap[{chunkX, chunkY}].blocks[updateX % CA::c_chunkSize][updateY % CA::c_chunkSize].type = choosen;
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
                    if (updateX < startX) startX = updateX;
                    if (updateX > endX) endX = updateX;
                    if (updateX >= 0 && updateY >= 0 && updateX < CA::c_screenWidth && updateY < CA::c_screenHeight) {
                        if (updateX < 0 || updateY < 0) continue;
                        int chunkX = updateX / CA::c_chunkSize;
                        int chunkY = updateY / CA::c_chunkSize;
                        if (chunkX < 0 || chunkX >= world->chunksX || chunkY < 0 || chunkY >= world->chunksY) continue;
                        world->chunkMap[{chunkX, chunkY}].blocks[updateX % CA::c_chunkSize][updateY % CA::c_chunkSize].type = 0;
                        world->chunkMap[{chunkX, chunkY}].toBeUpdated = true;
                        world->chunkMap[{chunkX, chunkY}].lastUpdate = 0;
                    }
                }
            }
        }
        if (startX != CA::c_screenWidth && endX != 0) {
            for (int x = startX; x < endX; x++) {
                world->toBeUpdatedLine[x] = true;
            }
        }
    }
};


class App {
    CA::World world;
    Player player;
    Physics::Map* map;
    std::vector<Physics::RigidBody> bodies;
public:
    
    void Init() {
        
        world.loadMaterials("data/tile_set.txt");
        
    }
	App() {
        SetTraceLogLevel(LOG_NONE); 
        SetConfigFlags(FLAG_WINDOW_RESIZABLE);
		InitWindow(1920, 1080, "a");
        omp_set_num_threads(32);
        Init();   

        map = new Physics::Map;
    }
    
	void Run() {
        int frame = 0;
        
        std::chrono::high_resolution_clock::time_point frameStart, frameEnd;
        std::chrono::high_resolution_clock::time_point physicsStart, physicsEnd;
        std::chrono::high_resolution_clock::time_point chunkGenStart, chunkGenEnd;
        std::chrono::high_resolution_clock::time_point lightUpdateStart, lightUpdateEnd;
        std::chrono::high_resolution_clock::time_point drawStart, drawEnd;
        
        while (!WindowShouldClose()) {
            frameStart = std::chrono::high_resolution_clock::now();
            
            int generated = 0;
            
            BeginDrawing();
            ClearBackground(SKYBLUE);
            
            physicsStart = std::chrono::high_resolution_clock::now();
            world.UpdatePhysics(world.materials, player.cameraPosition, {(float)GetScreenWidth(), (float)GetScreenHeight()});
            physicsEnd = std::chrono::high_resolution_clock::now();
            auto physicsDuration = std::chrono::duration_cast<std::chrono::microseconds>(physicsEnd - physicsStart).count();
            
            int beginX = (int)(player.cameraPosition.x / CA::c_chunkSize);
            int endX = (int)((player.cameraPosition.x + (GetScreenWidth() / player.cameraZoom)) / CA::c_chunkSize) + 1;
            int beginY = (int)(player.cameraPosition.y / CA::c_chunkSize);
            int endY = (int)((player.cameraPosition.y + (GetScreenHeight() / player.cameraZoom)) / CA::c_chunkSize) + 1;

            beginX = std::max(0, beginX);
            endX = std::min(world.chunksX, endX);
            beginY = std::max(0, beginY);
            endY = std::min(world.chunksY, endY);
            /*
            const int subSteps = 8; 
            for (int s = 0; s < subSteps; s++) {
                for (auto& ball : map->balls) {
                    if (!ball.held) {
                        ball.y_vel += 0.5f / subSteps; 
                        ball.x += ball.x_vel / subSteps;
                        ball.y += ball.y_vel / subSteps;
                    }
                }
                M_RecalculateGrid(map);
                for (int i = 0; i < (int)map->balls.size(); i++) {
                    En_CollisionBall(i, map, &world);
                }
            }
            */
            chunkGenStart = std::chrono::high_resolution_clock::now();
            for (int x = beginX; x < endX; x++) {
                for (int y = beginY; y < endY; y++) {
                    if (!world.lightMap[{x,y}].generated) {
                        CA::CAGI cagi;
                        cagi.Init(x,y);
                        world.lightMap[{x, y}] = cagi; 
                        world.lightMap[{x,y}].generated = true;
                        
                        world.chunkMap[{x, y}] = GenCleanChunkTerrain(x*CA::c_chunkSize,y*CA::c_chunkSize);
                        world.chunkMap[{x,y}].generated = true;
                        world.chunkMap[{x, y}].toBeUpdated = true;
                        world.chunkMap[{x, y}].lastUpdate = 0;
                        for (int d = 0; d < CA::c_chunkSize; d++) {
                            world.toBeUpdatedLine[x*CA::c_chunkSize+d] = true;
                        }
                        frame = 3;
                    }
                    
                }
            }
            for (int x = beginX; x < endX; x++) {
                for (int y = beginY; y < endY; y++) {
                    world.lightMap[{x,y}].Draw(x,y,player.cameraPosition,player.cameraZoom);
                }
            }
            chunkGenEnd = std::chrono::high_resolution_clock::now();
            auto chunkGenDuration = std::chrono::duration_cast<std::chrono::microseconds>(chunkGenEnd - chunkGenStart).count();
            
            lightUpdateStart = std::chrono::high_resolution_clock::now();
            if (frame%5==0) {
                bool hasChunks = false;
                for (int x = beginX; x < endX; x++) {
                    for (int y = beginY; y < endY; y++) {
                        if (world.chunkMap[{x,y}].generated && world.lightMap[{x,y}].generated) {
                            hasChunks = true;
                            bool update = false;
                            for (int cx = 0; cx < CA::c_chunkSize; cx++) {
                                if (world.toBeUpdatedLine[cx+x*CA::c_chunkSize]) {
                                    update=true;
                                    break;
                                }
                            }
                            if (world.chunkMap[{x,y}].toBeUpdated) update = true;
                            if (world.chunkMap[{x,y}].lastUpdate==0) update =true;
                            if (update) world.lightMap[{x,y}].Update(world.chunkMap[{x,y}].blocks,world.materials,renderLight);
                        }
                    }
                }
                world.UpdateLighting(world.materials,player.cameraPosition,{(float)GetScreenWidth(),(float)GetScreenHeight()});
            }
            lightUpdateEnd = std::chrono::high_resolution_clock::now();
            auto lightUpdateDuration = std::chrono::duration_cast<std::chrono::microseconds>(lightUpdateEnd - lightUpdateStart).count();
            
            drawStart = std::chrono::high_resolution_clock::now();
            
            DrawRectangleLines(
                -player.cameraPosition.x * player.cameraZoom,
                -player.cameraPosition.y * player.cameraZoom,
                CA::c_screenWidth * player.cameraZoom,
                CA::c_screenHeight * player.cameraZoom,
                WHITE
            );
            
            for (auto& b : map->balls) {          
                Rectangle destRect = {
                    (b.x - player.cameraPosition.x) * player.cameraZoom,
                    (b.y - player.cameraPosition.y) * player.cameraZoom,
                    b.radius * player.cameraZoom,
                    b.radius * player.cameraZoom
                }; 
                DrawCircle(destRect.x, destRect.y, destRect.width, b.color); 
            }
            
            for (auto &t : bodies) {
                // t.UpdateRigidBody(map);
                // t.Draw(player.cameraPosition,player.cameraZoom);
            }
            
            player.Control();
            player.Editor(&world);
            
            if (IsKeyDown(KEY_G)) {
                world.SaveWorld();
            }
            if (IsKeyDown(KEY_H)) {
                world.LoadWorld();
            }
            
            DrawFPS(0, 0);
            drawEnd = std::chrono::high_resolution_clock::now();
            auto drawDuration = std::chrono::duration_cast<std::chrono::microseconds>(drawEnd - drawStart).count();
            
            EndDrawing();
            
            frameEnd = std::chrono::high_resolution_clock::now();
            auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - frameStart).count();
            std::cout<<player.cameraZoom<<"\n";
            if (frame % 60 == 0) {
                std::cout << "=== Frame Timing (microseconds) ===" << std::endl;
                std::cout << "Total Frame: " << frameDuration << " μs (" << (1000000.0f / frameDuration) << " FPS)" << std::endl;
                std::cout << "  Physics: " << physicsDuration << " μs (" << (physicsDuration * 100.0f / frameDuration) << "%)" << std::endl;
                std::cout << "  Chunk Gen: " << chunkGenDuration << " μs (" << (chunkGenDuration * 100.0f / frameDuration) << "%)" << std::endl;
                std::cout << "  Light Update: " << lightUpdateDuration << " μs (" << (lightUpdateDuration * 100.0f / frameDuration) << "%)" << std::endl;
                std::cout << "  Drawing: " << drawDuration << " μs (" << (drawDuration * 100.0f / frameDuration) << "%)" << std::endl;
                std::cout << "  Other: " << (frameDuration - physicsDuration - chunkGenDuration - lightUpdateDuration - drawDuration) << " μs" << std::endl;
                std::cout << "====================================" << std::endl;
            }
            
            frame++;
            
            if (IsMouseButtonDown(2) && frame%10==0) {
                Vector2 mousePos = GetMousePosition();
                Vector2 worldPos = {
                    (mousePos.x / player.cameraZoom) + player.cameraPosition.x,
                    (mousePos.y / player.cameraZoom) + player.cameraPosition.y
                };
                
                Physics::RigidBody rigidBody;
                rigidBody.pos = worldPos;
                rigidBody.Init(16,16);
                for (int x = 0; x < 16; x++) {
                    for (int y = 0; y < 16; y++) {
                        if (Vector2Distance({(float)x,(float)y},{8,8})<6) {
                            ImageDrawPixel(&rigidBody.image,x,y,WHITE);
                        }
                    }
                }
                rigidBody.Gen(map);
                bodies.push_back(rigidBody);
            }
        }
	}
};

int main() {
	App app;
	app.Run();
}