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

bool IsPositionSolid(World* world, float x, float y) {
    int tileX = (int)x;
    int tileY = (int)y;
    
    if (tileX < 0 || tileY < 0 || tileX >= 1920 || tileY >= 1080) {
        return true;
    }
    
    int chunkX = tileX / c_chunkSize;
    int chunkY = tileY / c_chunkSize;
    
    if (chunkX < 0 || chunkX >= world->chunksX || chunkY < 0 || chunkY >= world->chunksY) {
        return true;
    }
    
    int localX = tileX % c_chunkSize;
    int localY = tileY % c_chunkSize;
    
    return world->chunkMap[{chunkX, chunkY}].blocks[localX][localY].type != 0;
}
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
			if (IsMouseButtonDown(0) && !hover) {
				for (int x = 0; x < editSize; x++) {
					for (int y = 0; y < editSize; y++) {
						int updateX = x + worldMousePos.x;
						int updateY = y + worldMousePos.y;
                        if (updateX>=0 && updateY>=0 && updateX<1920 && updateY<1080) {
                                
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
                        if (updateX>=0 && updateY>=0 && updateX<1920 && updateY<1080) {
                                
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
    }
};
struct Ball {
    Vector2 position;
    Vector2 velocity;
    float radius;
    uint8_t type;
    bool isStatic;
    
    void Draw(Vector2 cameraPosition, float zoom) {
        
        float screenX = (position.x-cameraPosition.x)*zoom;
        float screenY = (position.y-cameraPosition.y)*zoom;
        
        float screenRadius = radius*zoom;
        DrawRectangle(screenX,screenY,screenRadius,screenRadius,RED);
    }
    
    void Update(World *world) {
        if (isStatic) return;
        velocity.y += 1;
        velocity.x = Clamp(velocity.x,-1,1);
        velocity.y = Clamp(velocity.y,-1,1);
        if (velocity.y>10) velocity.y = 10;
        if (fabs(velocity.y)<1.0f) {
            velocity.x *= 0.95f;
        }
        if (position.x < radius) {
            position.x = radius;
            velocity.x *= -0.1f;
        }

        if (position.x + radius > 1920) {
            position.x = 1920 - radius;
            velocity.x *= -0.1f;
        }
        if (position.y < radius) {
            position.y = radius;
            velocity.y *= -0.1f;
        }
        if (position.y + radius > 1080) {
            position.y = 1080 - radius;
            velocity.y *= -0.1f;
            velocity.x *= 0.1f;
            if (fabs(velocity.y) < 0.5f)
                velocity.y = 0;
        }
        position.x += velocity.x;
        position.y += velocity.y;
    }   
    void ResolveCollisionWithBall(Ball& other) {
        float dx = other.position.x - position.x;
        float dy = other.position.y - position.y;
        float distSq = dx * dx + dy * dy;
        float minDist = radius + other.radius;
        
        if (distSq < minDist * minDist && distSq > 0.0001f) {
            float dist = sqrtf(distSq);
            float overlap = minDist - dist;
            float nx = dx / dist;
            float ny = dy / dist;
            
            if (other.isStatic) {
                position.x -= nx * overlap;
                position.y -= ny * overlap;
                
                float dotProduct = velocity.x * nx + velocity.y * ny;
                if (dotProduct < 0) {
                    velocity.x -= 2.0f * dotProduct * nx * 0.3f;
                    velocity.y -= 2.0f * dotProduct * ny * 0.3f;
                }
            }
            else {
                position.x -= nx * overlap * 0.5f;
                position.y -= ny * overlap * 0.5f;
                other.position.x += nx * overlap * 0.5f;
                other.position.y += ny * overlap * 0.5f;
                
                float dvx = velocity.x - other.velocity.x;
                float dvy = velocity.y - other.velocity.y;
                float impact = dvx * nx + dvy * ny;
                
                if (impact > 0) {
                    float restitution = 0.3f;
                    float impulse = (1.0f + restitution) * impact * 0.5f;
                    velocity.x -= impulse * nx;
                    velocity.y -= impulse * ny;
                    other.velocity.x += impulse * nx;
                    other.velocity.y += impulse * ny;
                }
            }
        }
    }
};
struct PhysicsChunk {
    std::vector<Ball> staticBalls;
};
class App {
    World world;
    Player player;
    std::vector<Ball> balls;
    std::map<std::tuple<int, int>, PhysicsChunk> staticBalls;
public:
    
    void Init() {
        
		for (int x = 0; x < world.chunksX; x++) {
			for (int y = 0; y < world.chunksY; y++) {
				world.chunkMap[std::tuple<int, int>{x, y}] = GenCleanChunkTerrain(x*c_chunkSize,y*c_chunkSize);
                staticBalls[std::tuple<int,int>{x,y}].staticBalls.reserve(c_chunkSize*c_chunkSize);
                for (int cx = 0; cx < c_chunkSize; cx++) {
                    for (int cy = 0; cy < c_chunkSize; cy++) {
                        if (world.chunkMap[std::tuple<int,int>{x,y}].blocks[cx][cy].type!=0) {
                            Ball ball;
                            ball.isStatic = true;
                            ball.position = {(float)cx+x*c_chunkSize,(float)cy+y*c_chunkSize};
                            ball.radius = 1;
                            ball.velocity = {0,0};
                            staticBalls[std::tuple<int,int>{x,y}].staticBalls.push_back(ball);
                        }
                    }   
                }    
            }
		}
        world.loadMaterials("data/tile_set.txt");
    }
	App() {
		InitWindow(c_screenWidth, c_screenHeight, "a");
        Init();   
        SetTargetFPS(60);
    }
	void Run() {
        int frame = 0;
		while (!WindowShouldClose()) {
			BeginDrawing();
			ClearBackground(SKYBLUE);
            if (IsMouseButtonDown(2) && frame>GetFPS()/10) {
                Vector2 mousePos = GetMousePosition();
                Vector2 worldPos = {
                    (mousePos.x / player.cameraZoom) +  player.cameraPosition.x,
                    (mousePos.y /  player.cameraZoom) +  player.cameraPosition.y
                };
                Ball ball;
                ball.position = worldPos;
                ball.radius = 4;
                ball.type = 1;
                ball.velocity = {0,0};
                balls.push_back(ball);
                frame = 0;
            }    

            
            DrawRectangleLines(
                -player.cameraPosition.x * player.cameraZoom,
                -player.cameraPosition.y *  player.cameraZoom,
                1920 *  player.cameraZoom,
                1080 *  player.cameraZoom,
                WHITE
            );
            world.Draw(player.cameraPosition,player.cameraZoom);
            world.UpdatePhysics(world.materials);
			//world.DebugActivityDisplay(player.cameraPosition,player.cameraZoom);
            for (int i = 0; i < 8; i++) {

                int id = 0;
                
                for (auto &t : balls) {
                    int cx = t.position.x/c_chunkSize;
                    int cy = t.position.y/c_chunkSize;
                    for (int x = -1; x <= 1; x++) {
                        for (int y = -1; y <= 1; y++) {
                        
                            for (auto k : staticBalls[std::tuple<int,int>{cx+x,cy+y}].staticBalls) {
                                t.ResolveCollisionWithBall(k);
                            }
                        }   
                    }
                    for (int r = id; r < balls.size(); r++) {
                        if (id!=r) t.ResolveCollisionWithBall(balls[r]);
                    }
                    id++;
                }
                for (auto &t : balls) {
                    t.Update(&world);
                
                }
            }
            for (auto &t : balls) {
               t.Draw(player.cameraPosition,player.cameraZoom);
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