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
class PhysicalObject {
    Image image;
    Texture texture;
    float rotation;
    Vector2 position;
    Vector2 velocity;
    Vector2 size;
    public:
        uint8_t grid[c_chunkSize][c_chunkSize];
        void Init(std::vector<Tile> &tiles, Vector2 position, float rotate) {
            image = GenImageColor(c_chunkSize,c_chunkSize,BLANK);
            for (int x = 0; x < c_chunkSize; x++) {
                for (int y= 0; y < c_chunkSize; y++) {
                    ImageDrawPixel(&image,x,y,tiles[grid[x][y]].color);
                }
            }
            this->size = {c_chunkSize, c_chunkSize};
            this->rotation = rotate;
            this->position = position;
            this->texture = LoadTextureFromImage(image);
            
        }
        std::vector<Vector2> GetRotatedCorners() {
            std::vector<Vector2> corners(4);
            float halfWidth = size.x/2.0f;
            float halfHeight = size.y/2.0f;
            Vector2 localCorners[4] = {
                {-halfWidth,-halfHeight},
                {halfWidth,-halfHeight},
                {halfWidth,halfHeight},
                {-halfWidth,halfHeight}
            };
            float cosA = cosf(rotation*DEG2RAD);
            float sinA = sinf(rotation*DEG2RAD);
            for (int i = 0; i < 4; i++) {
                float rotatedX = localCorners[i].x * cosA - localCorners[i].y*sinA;
                float rotatedY = localCorners[i].x * sinA - localCorners[i].y*cosA;
                corners[i] = {position.x+rotatedX,position.y+rotatedY};
            }
            return corners;
        }
        bool IsPointInRotateRect(Vector2 point) {
            float dX = point.x - position.x;
            float dY = point.y - position.y;
            float cosA = cosf(-rotation*DEG2RAD);
            float sinA = sinf(-rotation*DEG2RAD);
            float localX = dX * cosA-dY*sinA;
            float localY = dX*sinA+dY*cosA;
            float halfWidth = size.x/2.0f;
            float halfHeight = size.y/2.0f;
            return (localX>= -halfWidth && localX<=halfWidth && localY >= -halfHeight && localY<=halfHeight);
        }
        Rectangle GetBoundingBox() {
            auto corners = GetRotatedCorners();
            float minX = corners[0].x, maxX = corners[0].x;
            float minY = corners[0].y, maxY = corners[0].y;
            
            for (const auto& corner : corners) {
                minX = std::min(minX, corner.x);
                maxX = std::max(maxX, corner.x);
                minY = std::min(minY, corner.y);
                maxY = std::max(maxY, corner.y);
            }
            
            return {minX, minY, maxX - minX, maxY - minY};
        }
        bool IsCollidingWithWorld(World* world, float newX, float newY, float newRotation) {
            Vector2 oldPos = position;
            float oldRot = rotation;
            position = {newX, newY};
            rotation = newRotation;
            Rectangle bbox = GetBoundingBox();
            bbox.x -= 1;
            bbox.y -= 1;
            bbox.width += 2;
            bbox.height += 2;
            int startX = std::max(0, (int)bbox.x);
            int endX = std::min(1919, (int)(bbox.x + bbox.width));
            int startY = std::max(0, (int)bbox.y);
            int endY = std::min(1079, (int)(bbox.y + bbox.height));
            
            for (int x = startX; x <= endX; x++) {
                for (int y = startY; y <= endY; y++) {
                    int chunkX = x / c_chunkSize;
                    int chunkY = y / c_chunkSize;
                    
                    if (chunkX < 0 || chunkX >= world->chunksX || 
                        chunkY < 0 || chunkY >= world->chunksY) {
                        continue;
                    }
                    
                    int localX = x % c_chunkSize;
                    int localY = y % c_chunkSize;
                    
                    if (world->chunkMap[{chunkX, chunkY}].blocks[localX][localY].type != 0) {
                        Vector2 tileCenter = {x + 0.5f, y + 0.5f};
                        if (IsPointInRotateRect(tileCenter)) {
                            position = oldPos;
                            rotation = oldRot;
                            return true;
                        }
                    }
                }
            }
            
            position = oldPos;
            rotation = oldRot;
            return false;
        }
        
        
        void Update(World* world) {
            velocity.y += 0.5f;
            Vector2 newPosition = position;
            newPosition.x = position.x + velocity.x;
            if (!IsCollidingWithWorld(world, newPosition.x, position.y, rotation)) {
                position.x = newPosition.x;
            } else {
                velocity.x = 0;
            }
            newPosition.y = position.y + velocity.y;
            if (!IsCollidingWithWorld(world, position.x, newPosition.y, rotation)) {
                position.y = newPosition.y;
            } else {
                velocity.y = 0;
            }
            if (IsCollidingWithWorld(world, position.x, position.y, rotation)) {
                position.y -= 1;
                if (IsCollidingWithWorld(world, position.x, position.y, rotation)) {
                    position.y += 1;
                    position.x -= 1;
                }
            }
        }
        
        void Draw(Vector2 cameraPosition, float zoom) {
            float worldX = position.x - cameraPosition.x;
            float worldY = position.y - cameraPosition.y;
            Rectangle dest = {
                worldX * zoom,
                worldY * zoom,
                texture.width * zoom,
                texture.height * zoom
            };
            Rectangle source = {0, 0, texture.width, texture.height};
            Vector2 origin = {texture.width * 0.5f, texture.height * 0.5f};
            DrawTexturePro(texture, source, dest, origin, rotation, WHITE);
        }
};
class App {
    World world;
    Player player;
    std::vector<PhysicalObject> objects;
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
                PhysicalObject objectrs;
                for (int x = 0; x < c_chunkSize; x++) {
                    for (int y = 0; y < c_chunkSize; y++) {
                        objectrs.grid[x][y] = 1;

                    }   
                }
                objectrs.Init(world.materials,worldPos,0);
                objects.push_back(objectrs);
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