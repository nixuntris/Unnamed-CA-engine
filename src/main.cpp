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
#include <queue>
#include <unordered_set>
#include "BFS.hpp"
const bool renderLight = true;

struct Player {
    Vector2 playerPosition;
    Vector2 cameraPosition;
    float cameraZoom;
	int editSize = 15;
    int choosen = 0;
    float yVelocity = 0;
    void Init() {
        playerPosition = {0,0};
        cameraPosition = {0,0};
        cameraZoom = 2;
    }
    
    void Control(CA::World *world) {
        float wheel = GetMouseWheelMove();
        cameraZoom += wheel * 0.1f;
        
        if (wheel != 0) {
            this->cameraZoom += wheel * 0.1f;
            this->cameraZoom = Clamp(this->cameraZoom, 2, 5.0f);
        }
        if (this->cameraZoom < 2.0f) this->cameraZoom = 2.0f;
        if (this->cameraZoom > 5.0f) this->cameraZoom = 5.0f;
        
        if (IsKeyDown(KEY_A)) cameraPosition.x -= 5;
        if (IsKeyDown(KEY_D)) cameraPosition.x += 5;
        if (IsKeyDown(KEY_W)) cameraPosition.y -= 5;
        if (IsKeyDown(KEY_S)) cameraPosition.y += 5;
    }
    void ControlGameplay(CA::World *world) {
        float wheel = GetMouseWheelMove();
        cameraZoom += wheel * 0.1f;
        
        if (wheel != 0) {
            this->cameraZoom += wheel * 0.1f;
            this->cameraZoom = Clamp(this->cameraZoom, 2, 5.0f);
        }
        if (this->cameraZoom < 2.0f) this->cameraZoom = 2.0f;
        if (this->cameraZoom > 5.0f) this->cameraZoom = 5.0f;
        
        // Player size in world units
        const float playerWidth = 16.0f;  // Width of player hitbox
        const float playerHeight = 16.0f; // Height of player hitbox
        const float moveSpeed = 4.0f;
        const float gravity = 0.5f;
        const float jumpSpeed =-10.0f;
        
        // Calculate world position of player
        float w = GetScreenWidth();
        float h = GetScreenHeight();
        playerPosition = {cameraPosition.x + w/4, cameraPosition.y + h/4};
        
        // Apply gravity
        yVelocity += gravity;
        
        // Movement
        Vector2 moveDir = {0, 0};
        if (IsKeyDown(KEY_A)) moveDir.x = -1;
        if (IsKeyDown(KEY_D)) moveDir.x = 1;
        if (moveDir.x != 0 && moveDir.y != 0) {
            moveDir.x *= 0.7071f;
            moveDir.y *= 0.7071f;
        }
        if (IsKeyDown(KEY_SPACE) && IsOnGround(world)) {
            yVelocity = jumpSpeed;
            playerPosition.y -= 3;
        }
        float newX = playerPosition.x + moveDir.x * moveSpeed;
        float newY = playerPosition.y + yVelocity;
        if (!IsCollidingWithWorld(newX, playerPosition.y, playerWidth, playerHeight, world)) {
            playerPosition.x = newX;
        } else {
            if (moveDir.x > 0) {
                int checkX = (int)(playerPosition.x + playerWidth/2 + 1);
                int checkY = (int)(playerPosition.y);
                if (IsBlockSolid(checkX, checkY, world) || IsBlockSolid(checkX, checkY + playerHeight - 1, world)) {
                    playerPosition.x = floorf(playerPosition.x + playerWidth/2) - playerWidth/2;
                }
            } else if (moveDir.x < 0) {
                int checkX = (int)(playerPosition.x - playerWidth/2 - 1);
                int checkY = (int)(playerPosition.y);
                if (IsBlockSolid(checkX, checkY, world) || IsBlockSolid(checkX, checkY + playerHeight - 1, world)) {
                    playerPosition.x = ceilf(playerPosition.x - playerWidth/2) + playerWidth/2;
                }
            }
        }
        if (!IsCollidingWithWorld(playerPosition.x, newY, playerWidth, playerHeight, world)) {
            playerPosition.y = newY;
        } else {
            if (yVelocity > 0) {
                playerPosition.y = floorf(playerPosition.y + playerHeight/2) - playerHeight/2;
                yVelocity = 0;
            } else if (yVelocity < 0) {
                playerPosition.y = ceilf(playerPosition.y - playerHeight/2) + playerHeight/2;
                yVelocity = 0;
            }
        }
        cameraPosition.x = playerPosition.x - w/(4);
        cameraPosition.y = playerPosition.y - h/(4);
    }
    bool IsBlockSolid(int x, int y, CA::World *world) {
        if (x < 0 || x >= CA::c_screenWidth || y < 0 || y >= CA::c_screenHeight) return false;
        
        int chunkX = x / CA::c_chunkSize;
        int chunkY = y / CA::c_chunkSize;
        
        if (chunkX < 0 || chunkX >= world->chunksX || chunkY < 0 || chunkY >= world->chunksY) return false;
        
        auto it = world->chunkMap.find({chunkX, chunkY});
        if (it == world->chunkMap.end()) return false;
        
        uint8_t type = it->second.blocks[x % CA::c_chunkSize][y % CA::c_chunkSize].type;
        if (type==0) return false;
        if (!world->materials[type].fluid) return true;


        return false;
    }

    bool IsCollidingWithWorld(float px, float py, float width, float height, CA::World *world) {
        float halfW = width / 2.0f;
        float halfH = height / 2.0f;
        
        Vector2 corners[4] = {
            {px - halfW, py - halfH},
            {px + halfW, py - halfH},
            {px - halfW, py + halfH},
            {px + halfW, py + halfH}
        };
        
        for (int i = 0; i < 4; i++) {
            if (IsBlockSolid((int)corners[i].x, (int)corners[i].y, world)) {
                return true;
            }
        }
        
        for (int x = (int)(px - halfW); x <= (int)(px + halfW); x++) {
            if (IsBlockSolid(x, (int)(py - halfH), world) || 
                IsBlockSolid(x, (int)(py + halfH), world)) {
                return true;
            }
        }
        for (int y = (int)(py - halfH); y <= (int)(py + halfH); y++) {
            if (IsBlockSolid((int)(px - halfW), y, world) || 
                IsBlockSolid((int)(px + halfW), y, world)) {
                return true;
            }
        }
        
        return false;
    }

    bool IsOnGround(CA::World *world) {
        float halfW = 10.0f;
        float halfH = 16.0f;
        Vector2 feet[2] = {
            {playerPosition.x - halfW, playerPosition.y + halfH + 1},
            {playerPosition.x + halfW, playerPosition.y + halfH + 1}
        };
        
        for (int i = 0; i < 2; i++) {
            if (IsBlockSolid((int)feet[i].x, (int)feet[i].y, world)) {
                return true;
            }
        }
        return false;
    }
    void Draw() {
        float screenX = (playerPosition.x - cameraPosition.x) * cameraZoom;
        float screenY = (playerPosition.y - cameraPosition.y) * cameraZoom;
        float scaledWidth = 16 * cameraZoom;
        float scaledHeight = 16 * cameraZoom;
        DrawRectangle(
            screenX - scaledWidth/2,
            screenY - scaledHeight/2,
            scaledWidth,
            scaledHeight,
            RED
        );
    }
    void Editor(CA::World *world,Physics::Map* map) {
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
            std::vector<Vector2> checkBlocks;
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
                        if (world->chunkMap[{chunkX, chunkY}].blocks[updateX % CA::c_chunkSize][updateY % CA::c_chunkSize].type!=0) {
                            checkBlocks.push_back({(float)updateX,(float)updateY});
                        
                        }
                        world->chunkMap[{chunkX, chunkY}].blocks[updateX % CA::c_chunkSize][updateY % CA::c_chunkSize].type = 0;
                        world->chunkMap[{chunkX, chunkY}].toBeUpdated = true;
                        world->chunkMap[{chunkX, chunkY}].lastUpdate = 0;
                        
                                                
                    }
                }
            }
            FrameBFS bfs;
            for (auto t : checkBlocks) {
                
                bool wouldSplit = wouldSplitStructure(world, t.x, t.y, map,&bfs);
                
                if (wouldSplit) {
                    if (CheckBlock(world,t.x,t.y)) {
                        for (int x = -64; x < 64; x++) {
                            for (int y = -64; y < 64; y++) {
                                if (CheckBlock(world,x+t.x,y+t.y)) {
                                    if (!bfs.canReachEdge(world,x+t.x,y+t.y)) {
                                        carveShape(0,0,t.x+x,t.y+y,world,map);
                                    }
                                }
                            }   
                        }       
                     }

                    std::cout<<"yeah, split\n"; 
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
struct LightSource {
    Vector2 position={0,0};
    Color lightValue=RED;
    float strength=0.03;
    void Draw(CA::World *world) {
        for (int x = -10; x <= 10; x++) {
            for (int y = -10; y <= 10;y++) {
                int fx = (position.x+x)/CA::c_chunkSize;
                int fy = (position.y+y)/CA::c_chunkSize;
                int dx = (int(position.x+x)%CA::c_chunkSize)/CA::c_lightResolution;
                int dy = (int(position.y+y)%CA::c_chunkSize)/CA::c_lightResolution;
                
                world->lightMap[{fx,fy}].rSource[dx][dy]+=lightValue.r*strength;
                world->lightMap[{fx,fy}].gSource[dx][dy]+=lightValue.g*strength;
                world->lightMap[{fx,fy}].bSource[dx][dy]+=lightValue.b*strength;
            }
        }
    }
    void UnDraw(CA::World *world) {
        for (int x = -10; x <= 10; x++) {
            for (int y = -10; y <= 10;y++) {
                int fx = (position.x+x)/CA::c_chunkSize;
                int fy = (position.y+y)/CA::c_chunkSize;
                int dx = (int(position.x+x)%CA::c_chunkSize)/CA::c_lightResolution;
                int dy = (int(position.y+y)%CA::c_chunkSize)/CA::c_lightResolution;
                
                world->lightMap[{fx,fy}].r[dx][dy]-=lightValue.r*strength;
                world->lightMap[{fx,fy}].g[dx][dy]-=lightValue.g*strength;
                world->lightMap[{fx,fy}].b[dx][dy]-=lightValue.b*strength;
            }
        }
    }
};


class App {
    CA::World world;
    Physics::Map* map;
    Player player;
    std::vector<LightSource> sources;
    int targetFrames = 120;
    int simulationFrames = 60;
    int stepChange = targetFrames/simulationFrames;
public:
    
    void Init() {
        
        world.loadMaterials("data/tile_set.txt");
        map = new Physics::Map;
    }
	App() {
        SetTraceLogLevel(LOG_NONE); 
        SetConfigFlags(FLAG_WINDOW_RESIZABLE);
		InitWindow(1920, 1080, "a");
        
        omp_set_num_threads(32);
        SetTargetFPS(targetFrames);
        
        Init();   
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
            BeginDrawing();
            ClearBackground(SKYBLUE);
            physicsStart = std::chrono::high_resolution_clock::now();
            if (frame%stepChange==0) {
                world.UpdatePhysics(world.materials, player.cameraPosition, {(float)GetScreenWidth(), (float)GetScreenHeight()});
                    
                const int subSteps = 4; 
                for (int s = 0; s < subSteps; s++) {
                    
                        for (auto& ball : map->balls) {
                            if (!ball.held) {
                                ball.y_vel += 0.5f / subSteps; 
                                ball.x += ball.x_vel / subSteps;
                                ball.y += ball.y_vel / subSteps;
                                ball.rotation += ball.angularVelocity / subSteps;
                                
                            }
                        }

                    M_RecalculateGrid(map);
                    for (int i = 0; i < (int)map->balls.size(); i++) {
                        map->balls[i].angularVelocity *= 0.98;
                        En_CollisionBall(i, map, &world);
                    }
                }
            }
            physicsEnd = std::chrono::high_resolution_clock::now();
            auto physicsDuration = std::chrono::duration_cast<std::chrono::microseconds>(physicsEnd - physicsStart).count();
            if (IsKeyPressed(KEY_F)) {
                LightSource lights; 
                lights.position = player.playerPosition;
                lights.Draw(&world);
                sources.push_back(lights);

            }
            int beginX = (int)(player.cameraPosition.x / CA::c_chunkSize);
            int endX = (int)((player.cameraPosition.x + (GetScreenWidth() / player.cameraZoom)) / CA::c_chunkSize) + 1;
            int beginY = (int)(player.cameraPosition.y / CA::c_chunkSize);
            int endY = (int)((player.cameraPosition.y + (GetScreenHeight() / player.cameraZoom)) / CA::c_chunkSize) + 1;

            beginX = std::max(0, beginX);
            endX = std::min(world.chunksX, endX);
            beginY = std::max(0, beginY);
            endY = std::min(world.chunksY, endY);
            chunkGenStart = std::chrono::high_resolution_clock::now();
            if (frame%stepChange==0) {
                    
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
            }
            for (int x = beginX; x < endX; x++) {
                for (int y = beginY; y < endY; y++) {
                    world.lightMap[{x,y}].Draw(x,y,player.cameraPosition,player.cameraZoom);
                    if (world.chunkMap[{x,y}].grassBlades) {
                        
                        for (int dx =0 ; dx < CA::c_chunkSize; dx++) {
                            for (int dy =0 ; dy < CA::c_chunkSize; dy++) {
                                if (world.chunkMap[{x,y}].blocks[dx][dy].type!=0) {
                                    if (world.chunkMap[{x,y}].blocks[dx][dy-1].type==0) {

                                        if (world.materials[world.chunkMap[{x,y}].blocks[dx][dy].type].grassBlades) {
                                            
                                            CA::DrawGrassBlade(dx+x*CA::c_chunkSize,dy+y*CA::c_chunkSize,1,player.cameraPosition,player.cameraZoom);
                                        }
                                    }
                                }
                            }
                        }    
                    }
                    
                }
            }
            chunkGenEnd = std::chrono::high_resolution_clock::now();
            auto chunkGenDuration = std::chrono::duration_cast<std::chrono::microseconds>(chunkGenEnd - chunkGenStart).count();
            
            lightUpdateStart = std::chrono::high_resolution_clock::now();
            if (frame%3==0) {
                
                for (int i = 0; i < (int)map->balls.size(); i++) {
                    memset(map->balls[i].cleanOut,false,sizeof(map->balls[i].cleanOut));
                    float cosA = cosf(map->balls[i].rotation);
                    float sinA = sinf(map->balls[i].rotation);
                    for (int dy = 0; dy < map->balls[i].height; dy++) {
                        for (int dx = 0; dx < map->balls[i].width; dx++) {
                            if (map->balls[i].grid[dx][dy]!=0) {
                                float rx = dx * cosA - dy * sinA+map->balls[i].x;
                                float ry = dx * sinA + dy * cosA+map->balls[i].y;
                                if (world.chunkMap[{rx/CA::c_chunkSize,ry/CA::c_chunkSize}].blocks[(int)rx%CA::c_chunkSize][(int)ry%CA::c_chunkSize].type==0) {

                                    map->balls[i].cleanOut[dx][dy] = true;
                                    world.chunkMap[{rx/CA::c_chunkSize,ry/CA::c_chunkSize}].blocks[(int)rx%CA::c_chunkSize][(int)ry%CA::c_chunkSize].type = map->balls[i].grid[dx][dy];
                                    world.toBeUpdatedLine[(int)rx] = true;
                                }        
                            }
                        }
                    }
                }
                for (int x = beginX; x < endX; x++) {
                    for (int y = beginY; y < endY; y++) {
                        if (world.chunkMap[{x,y}].generated && world.lightMap[{x,y}].generated) {
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
                
                for (int i = 0; i < (int)map->balls.size(); i++) {
                    float cosA = cosf(map->balls[i].rotation);
                    float sinA = sinf(map->balls[i].rotation);
                    for (int dy = 0; dy < map->balls[i].height; dy++) {
                        for (int dx = 0; dx < map->balls[i].width; dx++) {
                            if (map->balls[i].grid[dx][dy]!=0 && map->balls[i].cleanOut[dx][dy]) {
                                        
                                float rx = dx * cosA - dy * sinA+map->balls[i].x;
                                float ry = dx * sinA + dy * cosA+map->balls[i].y;
                                world.chunkMap[{rx/CA::c_chunkSize,ry/CA::c_chunkSize}].blocks[(int)rx%CA::c_chunkSize][(int)ry%CA::c_chunkSize].type = 0;
                                world.toBeUpdatedLine[(int)rx] = true;
                            }
                        }
                    }
                    map->balls[i].Draw(&world);
                }
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
            player.ControlGameplay(&world);
            player.Editor(&world,map);
            
            if (IsKeyDown(KEY_G)) {
                world.SaveWorld();
            }
            if (IsKeyDown(KEY_H)) {
                world.LoadWorld();
            }
            
            player.Draw();
            
            DrawFPS(0, 0);
            drawEnd = std::chrono::high_resolution_clock::now();
            auto drawDuration = std::chrono::duration_cast<std::chrono::microseconds>(drawEnd - drawStart).count();
            for (auto& b : map->balls) {
                float screenX = (b.x - player.cameraPosition.x) * player.cameraZoom;
                float screenY = (b.y - player.cameraPosition.y) * player.cameraZoom;
                
                float scaledPixelSize = b.pixelSize * player.cameraZoom;
                float scaledWidth = b.width * scaledPixelSize;
                float scaledHeight = b.height * scaledPixelSize;
                
                float rotationDegrees = b.rotation * (180.0f / PI);
                
                Vector2 pivot = {
                    scaledWidth / 2.0f,
                    scaledHeight / 2.0f
                };
                b.CalculateAABB();
                b.CalculatePixelRot();
                DrawTexturePro(
                    b.texture,
                    (Rectangle){0, 0, (float)b.texture.width, (float)b.texture.height},
                    (Rectangle){screenX - scaledWidth/2, screenY - scaledHeight/2, scaledWidth, scaledHeight},
                    pivot,
                    rotationDegrees,
                    WHITE
                );
            }
            player.Draw();
            //world.DebugActivityDisplay(player.cameraPosition,player.cameraZoom);
            EndDrawing();
            
            frameEnd = std::chrono::high_resolution_clock::now();
            auto frameDuration = std::chrono::duration_cast<std::chrono::microseconds>(frameEnd - frameStart).count();
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
//            std::cout << " Object count: " << map->balls.size()<<"\n";
            frame++;
            
        }
	}
};

int main() {
	App app;
	app.Run();
}