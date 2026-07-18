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
    void Draw(Vector2 cameraPosition, float zoom) {
        
        float screenX = (position.x-cameraPosition.x)*zoom;
        float screenY = (position.y-cameraPosition.y)*zoom;
        
        float screenRadius = radius*zoom;
        DrawRectangle(screenX,screenY,screenRadius,screenRadius,RED);
    }
    void Update(World *world) {
        velocity.y += 1;
        if (velocity.y>10) velocity.y = 10;
        position.x += velocity.x;
        if (IsPositionSolid(world,position.x+1,position.y) || IsPositionSolid(world,position.x-1,position.y)) {
            position.x -= velocity.x;
            velocity.x *= -0.3f;
            if (fabs(velocity.x) < 0.5f) velocity.x = 0;
        }
        position.y += velocity.y;
        if (IsPositionSolid(world,position.x,position.y+1) || IsPositionSolid(world,position.x,position.y-1)) {
            position.y -= velocity.y;
            velocity.y *= -0.3f;
            if (fabs(velocity.y)<1.0f) {
                velocity.x *= 0.95f;
            }
        }
    }   
    
    void ResolveCollisionWithBall(Ball& other) {
        float dx = other.position.x - position.x;
        float dy = other.position.y - position.y;
        float distSq = dx * dx + dy * dy;
        float minDist = radius + other.radius;
        
        if (distSq < minDist * minDist && distSq > 0.0001f) {
            float dist = sqrtf(distSq);
            float overlap = (minDist - dist) * 0.5f;
            float nx = dx / dist;
            float ny = dy / dist;
            
            position.x -= nx * overlap;
            position.y -= ny * overlap;
            other.position.x += nx * overlap;
            other.position.y += ny * overlap;
            
            float dvx = velocity.x - other.velocity.x;
            float dvy = velocity.y - other.velocity.y;
            float impact = dvx * nx + dvy * ny;
            
            if (impact < 0) {
                float impulse = -(1.0f + 0.8) * impact / 2.0f;
                velocity.x += impulse * nx;
                velocity.y += impulse * ny;
                other.velocity.x -= impulse * nx;
                other.velocity.y -= impulse * ny;
            }
        }
    }

};
class PhysicalObject {
    Image image;
    Texture texture;
    float rotation;
    Vector2 position;
    Vector2 velocity;
    std::vector<Ball> balls;
    public:
        uint8_t grid[c_chunkSize][c_chunkSize];
            
        inline Vector2 LocalToWorld(Vector2 localPos) {
            float cosA = cosf(rotation * DEG2RAD);
            float sinA = sinf(rotation * DEG2RAD);
            return {
                position.x + localPos.x * cosA - localPos.y * sinA,
                position.y + localPos.x * sinA + localPos.y * cosA
            };
        }
        
        void Init(std::vector<Tile> &tiles, Vector2 position, float rotate) {
            image = GenImageColor(c_chunkSize,c_chunkSize,BLANK);
            for (int x = 0; x < c_chunkSize; x++) {
                for (int y= 0; y < c_chunkSize; y++) {
                    if (grid[x][y]!=0) {
                        Ball ball;
                        ball.position = LocalToWorld({
                            (float)(x - c_chunkSize/2), 
                            (float)(y - c_chunkSize/2)
                        });
                        ball.velocity = {0, 0};
                        ball.radius = 0.4f;
                        ball.type = grid[x][y];
                        balls.push_back(ball);
                    }
                    ImageDrawPixel(&image,x,y,tiles[grid[x][y]].color);
                }
            }
            this->rotation = rotate;
            this->position = position;
            this->texture = LoadTextureFromImage(image);
            
        }
        void CalculateVelocityFromBalls() {
            if (balls.empty()) {
                velocity = {0,0};
                return;
            }
            velocity = {0,0};
            for (const auto& ball : balls) {
                velocity.x += ball.velocity.x;
                velocity.y += ball.velocity.y;
            }
            velocity.x /= balls.size();
            velocity.y /= balls.size();
        }
        void Update(World *world) {
            position.y += velocity.y;
            position.x += velocity.x;
            for (auto &t : balls) {
                t.Update(world);
            }
            CalculateVelocityFromBalls();
            std::cout<<velocity.x<<" "<<velocity.y<<"\n";
            int r = 0;
            for (int x = 0; x < c_chunkSize; x++) {
                for (int y= 0; y < c_chunkSize; y++) {
                    if (grid[x][y]!=0) {
                        balls[r].position = LocalToWorld({
                            (float)(x - c_chunkSize/2), 
                            (float)(y - c_chunkSize/2)
                        });
                    }
                    r++;
                }
            }
        }
        void Draw(Vector2 cameraPosition, float zoom) {
            float worldX = position.x-cameraPosition.x;
            float worldY = position.y-cameraPosition.y;
            Rectangle dest = {
                worldX*zoom,
                worldY*zoom,
                texture.width * zoom,
                texture.height * zoom
            };
            Rectangle source = {0,0, texture.width,texture.height};
            Vector2 origin = {texture.width*0.5f,texture.height*0.5f};
            DrawTexturePro(texture,source,dest,origin,rotation,WHITE);
            for (const auto& ball : balls) {
                float screenX = (ball.position.x-cameraPosition.x)*zoom;
                float screenY = (ball.position.y-cameraPosition.y)*zoom;
                float screenRadius = ball.radius*zoom;
                DrawCircle(screenX,screenY,screenRadius,RED);
            }
        }
};
class App {
    World world;
    Player player;
    std::vector<PhysicalObject> objects;
    std::vector<Ball> balls;
public:
    
    void Init() {
        
		for (int x = 0; x < world.chunksX; x++) {
			for (int y = 0; y < world.chunksY; y++) {
				 world.chunkMap[std::tuple<int, int>{x, y}] = GenCleanChunkTerrain(x*c_chunkSize,y*c_chunkSize);
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
           /*     PhysicalObject objectrs;
                for (int x = 0; x < c_chunkSize; x++) {
                    for (int y = 0; y < c_chunkSize; y++) {
                        objectrs.grid[x][y] = 1;

                    }   
                }
                objectrs.Init(world.materials,worldPos,0);
                objects.push_back(objectrs);*/
                Ball ball;
                ball.position = worldPos;
                ball.radius = 2;
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
			world.DebugActivityDisplay(player.cameraPosition,player.cameraZoom);
            for (auto &t : objects) {
                t.Update(&world);
            }
            int id = 0;
            for (auto &t : balls) {
                for (int r = 0; r < balls.size(); r++) {
                    if (id!=r) t.ResolveCollisionWithBall(balls[r]);
                }
                t.Update(&world);
                id++;
            }
            for (auto &t : balls) {
                t.Draw(player.cameraPosition,player.cameraZoom);
            
            }
            for (auto &t : objects) {
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