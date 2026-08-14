#pragma once
#include "raylib.h"
#include <cinttypes>
#include "string"
#include <vector>
#include <tuple>
#include <cmath>
#include <cstring>
namespace CA {
    
    inline void DrawGrassBlade(float x, float y, float time, Vector2 cameraPosition, float zoom) {
        float seed = sin(x * 127.1f + y * 311.7f) * 43758.5453f;
        seed = seed - floor(seed);
        
        float height = (30.0f + seed * 40.0f) * zoom*0.05;
        float width = (2.0f + seed * 2.0f) * zoom*0.05;
        float curvature = (seed - 0.5f) * 0.8f;
        float swayOffset = seed * 6.28f;
        float swaySpeed = 0.5f + (seed * 0.5f);
        
        float sway = sin(time * swaySpeed + swayOffset) * 8.0f * zoom;
        
        Vector2 base = {
            (x - cameraPosition.x) * zoom,
            (y - cameraPosition.y) * zoom
        };
        
        Vector2 mid = {
            base.x + sway * 0.3f + curvature * height * 0.3f,
            base.y - height * 0.5f
        };
        Vector2 tip = {
            base.x + sway + curvature * height,
            base.y - height
        };
        
        int greenShade = 100 + (int)(seed * 80);
        Color color = {30, greenShade, 20, 255};
        Color tipColor = {60, greenShade + 30, 50, 255};
        
        float w = width;
        DrawTriangle(
            (Vector2){base.x - w, base.y},
            (Vector2){base.x + w, base.y},
            (Vector2){mid.x - w * 0.6f, mid.y},
            color
        );
        DrawTriangle(
            (Vector2){base.x + w, base.y},
            (Vector2){mid.x + w * 0.6f, mid.y},
            (Vector2){mid.x - w * 0.6f, mid.y},
            color
        );
        DrawTriangle(
            (Vector2){mid.x - w * 0.6f, mid.y},
            (Vector2){mid.x + w * 0.6f, mid.y},
            (Vector2){tip.x - w * 0.3f, tip.y},
            tipColor
        );
        DrawTriangle(
            (Vector2){mid.x + w * 0.6f, mid.y},
            (Vector2){tip.x + w * 0.3f, tip.y},
            (Vector2){tip.x - w * 0.3f, tip.y},
            tipColor
        );
        DrawLineEx(
            (Vector2){base.x + sway * 0.1f, base.y - height * 0.9f},
            (Vector2){tip.x + sway * 0.1f, tip.y + 5 * zoom},
            std::max(1.0f, zoom),
            (Color){color.r + 40, color.g + 40, color.b + 40, 150}
        );
    }
    const int c_chunkSize = 40; //Size of each chunk in cells 
    const int c_sleepTime = 30; //Number of frames a chunk waits before going offline untill woken up by a neighbouring chunk
    const int c_screenWidth = 1920*8; //World width
    const int c_screenHeight = 1080*8; //World height. 
    const int c_lightResolution = 2; 
    const int c_gridLightSize = c_chunkSize/c_lightResolution;
    
    /**
     * Tile - Defines the properties/behavior of a cell type.
     * Uses a palette approach: each cell stores only a uint8_t type index,
     * and the actual behavior is looked up from this Tile struct.
     */

   
    struct Tile {
        bool grassBlades = false;
        std::string name;        // Display name of the tile
        int weight;              // Used for displacement (heavier sinks, lighter floats)
        bool gas;                // If true, moves upward (like smoke)
        bool goesBothWays;       // Unused, reserved for side-to-side movement
        bool falls;              // If true, moves downward (like sand/gravel)
        int mass;                // Used for rigid bodies
        Color color;             // Color used when rendering
        int dissolves;           // Tile type that dissolves this one (-1 = no dissolve)
        bool fluid;              // If true, behaves like water (flows down and sideways)
        int replaces;            // Unused, reserved for replacement behavior
        int lifeTime;            // How many frames before the tile dies (-1 = infinite)
        int leaveBehind;         // Tile type left behind when this tile dies
        float lightAbsorb;       // How much light this tile absorbs (0.0 = transparent, 1.0 = opaque)
    };
    struct Cell {
        uint8_t type; //Index into the tile
        uint8_t direction; //Used for fluids, 0 - right, 1 - left
        bool updated; //Prevents updating the same cell multiple times per frame
        int lifeTime; //Remaining life of this specific cell
    };
    //Different buffers are for different cross chunk behaviours
    //It allows to make interactions much easier, and isn't that expensive
    
    struct Chunk {
        bool generated = false;
        Cell blocks[c_chunkSize][c_chunkSize];
        int gx;
        int gy;
        int cellCount=0;
        Cell moveDown[c_chunkSize];
        Cell moveUp[c_chunkSize];
        Cell moveLeft[c_chunkSize];
        Cell moveRight[c_chunkSize];
        bool onTheLeftEdge;
        bool onTheRighEdge;
        bool onTheBottomEdge;
        bool onTheUpEdge;
        bool grassBlades=false;
        Cell topChunkDataCopy[c_chunkSize]; //Y+ Copy
        Cell bottomChunkDataCopy[c_chunkSize]; //Y- Copy
        Cell leftChunkDataCopy[c_chunkSize]; //X- copy
        Cell rightChunkDataCopy[c_chunkSize]; //X+ copy
        Cell oldValues[c_chunkSize][c_chunkSize];
        Cell swapDown[c_chunkSize];   // For weight-based swaps going down
        Cell swapUp[c_chunkSize];     // For weight-based swaps going up
        Image image;
        Texture texture;
        bool toBeUpdated;
        bool containsData;
        int lastUpdate;
        bool updatedYLine[c_chunkSize];
        //Debug functions for seeing the raw information, needs a proper init in a gen function
        inline void Draw(int x, int y, Vector2 cameraPosition, float zoom) {
            Rectangle sourceRect = {0, 0, texture.width, texture.height};
            Rectangle destRect = {
                (x * c_chunkSize - cameraPosition.x) * zoom,
                (y * c_chunkSize - cameraPosition.y) * zoom,
                texture.width * zoom,
                texture.height * zoom
            };
            DrawTexturePro(texture, sourceRect, destRect, Vector2{0, 0}, 0.0f, WHITE);
        }   
        
        inline void UpdateDisplayBuffer(std::vector<Tile> &tiles) {
            containsData = false;
            Color* pixels = (Color*)image.data;
            for (int x = 0; x < c_chunkSize; x++) {
                for (int y = 0; y < c_chunkSize; y++) {
                    if (blocks[x][y].type != 0) {
                        containsData = true;
                    }
                    if (blocks[x][y].type==0) {
                        pixels[y * c_chunkSize + x] = SKYBLUE;
                    }
                    else if (blocks[x][y].type<tiles.size()) {
                        
                        Color color = tiles[blocks[x][y].type].color;
                        pixels[y * c_chunkSize + x] = color;
                        
                    }
                }
            }
            UpdateTexture(texture, image.data);
            toBeUpdated = false;
        }
        //Calculates the difference between the previous frame, will be used for streaming data
        inline int CalculateTheDelta() {
            struct Delta {
                uint8_t x;
                uint8_t y;
                uint8_t type;
            };
            Delta deltas[c_chunkSize*c_chunkSize];
            int r= 0;
            for (int x = 0; x < c_chunkSize; x++) {
                for (int y = 0; y < c_chunkSize; y++) {
                    if (oldValues[x][y].type!=blocks[x][y].type) {
                        deltas[r].x = x;
                        deltas[r].y = y;
                        deltas[r].type = blocks[x][y].type;

                        r++;
                    }
                }
            }
            
            for (int x = 0; x < c_chunkSize; x++) {
                for (int y = 0; y < c_chunkSize; y++) {
                    oldValues[x][y].type = blocks[x][y].type;
                }
            }
            return r;
        }
        //Move functions "move", it's meant to change it's position by +1 y for example, it handles both neighbours and itself
        //It makes it easier to make new behaviours, and allows to simplify the code and ease of use
        inline bool MoveDown(int x, int y, std::vector<Tile> &tiles, bool moveByWeight=false) {
            if (y + 1 < c_chunkSize) {
                if (blocks[x][y + 1].type == 0 && !blocks[x][y + 1].updated) {
                    blocks[x][y + 1] = blocks[x][y];
                    blocks[x][y].type = 0;
                    toBeUpdated = true;
                    blocks[x][y + 1].updated = true;
                    blocks[x][y].updated = true;
                    lastUpdate = 0;
                    updatedYLine[x] = true;
                    return true;
                }
                else if (blocks[x][y+1].type!=0 && blocks[x][y+1].type!=255 && !blocks[x][y+1].updated && moveByWeight && tiles[blocks[x][y+1].type].weight<tiles[blocks[x][y].type].weight) {
                    Cell oldcell = blocks[x][y+1];
                    blocks[x][y + 1] = blocks[x][y];
                    blocks[x][y] = oldcell;
                    toBeUpdated = true;
                    blocks[x][y + 1].updated = true;
                    blocks[x][y].updated = true;
                    lastUpdate = 0;
                    updatedYLine[x] = true;
                    return true;
                }
            }
            else {
                if (bottomChunkDataCopy[x].type == 0 && moveDown[x].type == 0) {
                    moveDown[x] = blocks[x][y];
                    blocks[x][y].type = 0;
                    toBeUpdated = true;
                    lastUpdate = 0;
                    updatedYLine[x] = true;
                    blocks[x][y].updated = true;
                    return true;
                }
                else if (bottomChunkDataCopy[x].type != 0 && bottomChunkDataCopy[x].type != 255 && 
                        moveDown[x].type == 0 && swapDown[x].type == 0 && moveByWeight && 
                        !bottomChunkDataCopy[x].updated && 
                        tiles[bottomChunkDataCopy[x].type].weight < tiles[blocks[x][y].type].weight) {
                    updatedYLine[x] = true;
                    swapDown[x] = blocks[x][y];
                    blocks[x][y] = bottomChunkDataCopy[x];
                    toBeUpdated = true;
                    blocks[x][y].updated = true;
                    lastUpdate = 0;
                    bottomChunkDataCopy[x].updated = true;
                    return true;
                }
            }
            return false;
        }
        inline bool MoveUp(int x, int y, std::vector<Tile> &tiles, bool moveByWeight=false) {
            if (y - 1 >= 0) {
                if (blocks[x][y - 1].type == 0 && !blocks[x][y - 1].updated) {
                    blocks[x][y - 1] = blocks[x][y];
                    blocks[x][y].type = 0;
                    toBeUpdated = true;
                    blocks[x][y - 1].updated = true;
                    blocks[x][y].updated = true;
                    lastUpdate = 0;
                    return true;
                }
                else if (blocks[x][y-1].type!=0 && blocks[x][y-1].type!=255 && !blocks[x][y-1].updated && moveByWeight && tiles[blocks[x][y-1].type].weight<tiles[blocks[x][y].type].weight) {
                    Cell oldcell = blocks[x][y-1];
                    blocks[x][y - 1] = blocks[x][y];
                    blocks[x][y] = oldcell;
                    toBeUpdated = true;
                    blocks[x][y - 1].updated = true;
                    blocks[x][y].updated = true;
                    lastUpdate = 0;
                    return true;
                }
            }
            else {
                if (topChunkDataCopy[x].type == 0 && moveUp[x].type == 0) {
                    moveUp[x] = blocks[x][y];
                    blocks[x][y].type = 0;
                    toBeUpdated = true;
                    lastUpdate = 0;
                    blocks[x][y].updated = true;
                    topChunkDataCopy[x].updated = true;
                    updatedYLine[x] = true;
                    return true;
                }
            }
            return false;
        }
        inline bool MoveLeft(int x, int y, std::vector<Tile> &tiles, bool moveByWeight=false) {
            if (x - 1 >= 0) {
                if (blocks[x - 1][y].type == 0 && !blocks[x - 1][y].updated) {
                    blocks[x - 1][y] = blocks[x][y];
                    blocks[x][y].type = 0;
                    toBeUpdated = true;
                    blocks[x - 1][y].updated = true;
                    blocks[x][y].updated = true;
                    lastUpdate = 0;
                    updatedYLine[x] = true;
                    return true;
                }
                else if (blocks[x-1][y].type!=0 && blocks[x-1][y].type!=255 && !blocks[x-1][y].updated && moveByWeight && tiles[blocks[x-1][y].type].weight<tiles[blocks[x][y].type].weight) {
                    Cell oldcell = blocks[x-1][y];
                    blocks[x - 1][y] = blocks[x][y];
                    blocks[x][y] = oldcell;
                    toBeUpdated = true;
                    blocks[x - 1][y].updated = true;
                    blocks[x][y].updated = true;
                    lastUpdate = 0;
                    updatedYLine[x] = true;
                    return true;
                }
            }
            else {
                if (leftChunkDataCopy[y].type == 0 && moveLeft[y].type == 0 && !leftChunkDataCopy[y].updated) {
                    moveLeft[y] = blocks[x][y];
                    blocks[x][y].type = 0;
                    blocks[x][y].updated = true;
                    toBeUpdated = true;
                    lastUpdate = 0;
                    updatedYLine[x] = true;
                    leftChunkDataCopy[y].updated = true;
                    return true;
                }
            }
            return false;
        }
        inline bool MoveRight(int x, int y,  std::vector<Tile> &tiles,bool moveByWeight=false) {
            if (x + 1 < c_chunkSize) {
                if (blocks[x + 1][y].type == 0 && !blocks[x + 1][y].updated) {
                    blocks[x + 1][y] = blocks[x][y];
                    blocks[x][y].type = 0;
                    toBeUpdated = true;
                    blocks[x + 1][y].updated = true;
                    blocks[x][y].updated = true;
                    updatedYLine[x] = true;
                    lastUpdate = 0;
                    return true;
                }
                else if (blocks[x+1][y].type!=0 && blocks[x+1][y].type!=255 && !blocks[x+1][y].updated && moveByWeight && tiles[blocks[x+1][y].type].weight<tiles[blocks[x][y].type].weight) {
                    Cell oldcell = blocks[x+1][y];
                    blocks[x + 1][y] = blocks[x][y];
                    blocks[x][y] = oldcell;
                    toBeUpdated = true;
                    blocks[x + 1][y].updated = true;
                    blocks[x][y].updated = true;
                    updatedYLine[x] = true;
                    lastUpdate = 0;
                    return true;
                }
            }
            else {
                if (rightChunkDataCopy[y].type == 0 && moveRight[y].type == 0 && !rightChunkDataCopy[y].updated) {
                    moveRight[y] = blocks[x][y];
                    blocks[x][y].type = 0;
                    toBeUpdated = true;
                    lastUpdate = 0;
                    blocks[x][y].updated = true;
                    updatedYLine[x] = true;
                    rightChunkDataCopy[y].updated = true;
                    return true;
                }
            }
            return false;
        }
        
        inline bool SurroundedByDissolvingTiles(int x, int y, int dissolvingTile) {
            auto checkPosition = [&](int checkX, int checkY) -> bool {
                if (checkX >= 0 && checkX < c_chunkSize && checkY >= 0 && checkY < c_chunkSize) {
                    return blocks[checkX][checkY].type == dissolvingTile;
                }
                if (checkY < 0) return topChunkDataCopy[checkX].type == dissolvingTile;
                if (checkY >= c_chunkSize) return bottomChunkDataCopy[checkX].type == dissolvingTile;
                if (checkX < 0) return leftChunkDataCopy[checkY].type == dissolvingTile;
                if (checkX >= c_chunkSize) return rightChunkDataCopy[checkY].type == dissolvingTile;
                
                return false;
            };
            return checkPosition(x, y - 1) || // UP
                checkPosition(x, y + 1) || // DOWN
                checkPosition(x - 1, y) || // LEFT
                checkPosition(x + 1, y);   // RIGHT
        }
        inline void UpdatePhysics(std::vector<Tile>&tiles) {
            containsData = false;
            lastUpdate++;
            onTheBottomEdge = false;
            onTheUpEdge = false;
            onTheLeftEdge = false;
            onTheRighEdge = false;
            cellCount = 0;
            grassBlades = false;
            for (int y = c_chunkSize - 1; y >= 0; y--) {
                for (int x = 0; x < c_chunkSize; x++) {
                    uint8_t type = blocks[x][y].type;
                    if (blocks[x][y].updated || type == 0)
                        continue;
                    
                    if (type!=0) {
                        cellCount++;
                        containsData = true;
                        if (x==c_chunkSize-1) onTheRighEdge = true;
                        if (x==0) onTheLeftEdge = true;
                        if (y==0) onTheUpEdge = true;
                        if (y==c_chunkSize-1) onTheBottomEdge = true;
                        if (tiles[type].grassBlades) {
                            grassBlades = true;
                        }    
                    }
                    if (tiles[type].lifeTime!=-1) {
                        if (blocks[x][y].lifeTime<=0) {
                                
                            blocks[x][y].type = tiles[type].leaveBehind;
                            type = blocks[x][y].type;
                            blocks[x][y].lifeTime = tiles[type].lifeTime;
                        }
                        blocks[x][y].lifeTime--;
                        updatedYLine[x] = true;
                    }
                    if (tiles[type].dissolves!=-1 && SurroundedByDissolvingTiles(x,y,tiles[type].dissolves)) {
                        updatedYLine[x] = true;
                        blocks[x][y].type = tiles[type].leaveBehind;
                        type = blocks[x][y].type;
                    }
                    else if (tiles[type].fluid) { 
                        if (!MoveDown(x, y,tiles,false)) {
                            if (GetRandomValue(0,1)) {
                                MoveLeft(x,y,tiles,false);
                                continue;
                            }
                            else {
                                MoveRight(x,y,tiles,false);
                                continue;
                            }
                        }
                        if (blocks[x][y].direction==0) {
                            if (!MoveRight(x,y,tiles,false)) {
                                blocks[x][y].direction = 1;
                                updatedYLine[x] = true;
                                continue;
                            }
                        }
                        else {
                            if (!MoveLeft(x,y,tiles,false)) {
                                blocks[x][y].direction = 0;
                                updatedYLine[x] = true;
                                continue;
                            }
                        
                        }
                    }
                    else if (tiles[type].falls) {

                        if (!MoveDown(x, y,tiles,true)) {
                            if (GetRandomValue(0,1))
                                MoveLeft(x,y,tiles,true);
                            else
                                MoveRight(x,y,tiles,true);
                        }
                        continue;
                    }
                    
                    else if (tiles[type].gas) {

                        if (!MoveUp(x, y,tiles,true)) {
                            if (GetRandomValue(0,1))
                                MoveLeft(x,y,tiles,true);
                            else
                                MoveRight(x,y,tiles,true);
                        }
                        continue;
                    }
                }
            }
        }
        
    };

    unsigned inline int hash(unsigned int x, unsigned int y) {
        unsigned int h = x * 374761393u + y * 668265263u;
        h = (h ^ (h >> 13)) * 1274126177u;
        return h ^ (h >> 16);
    }

    struct CAGI {
        bool generated = false;
        float r[c_gridLightSize][c_gridLightSize];
        float g[c_gridLightSize][c_gridLightSize];
        float b[c_gridLightSize][c_gridLightSize];
        float rLight[c_gridLightSize][c_gridLightSize];
        float gLight[c_gridLightSize][c_gridLightSize];
        float bLight[c_gridLightSize][c_gridLightSize];
        float rSource[c_gridLightSize][c_gridLightSize];
        float gSource[c_gridLightSize][c_gridLightSize];
        float bSource[c_gridLightSize][c_gridLightSize];
        int hashValues[c_chunkSize][c_chunkSize];
        float rNext[c_chunkSize][c_chunkSize];
        float gNext[c_chunkSize][c_chunkSize];
        float bNext[c_chunkSize][c_chunkSize];
        Image image;
        Texture texture;
        bool updated;
        inline void Draw(int x, int y, Vector2 cameraPosition, float zoom) {
            Rectangle sourceRect = {0, 0, (float)texture.width, (float)texture.height};
            Rectangle destRect = {
                (x * c_chunkSize - cameraPosition.x) * zoom,
                (y * c_chunkSize - cameraPosition.y) * zoom,
                texture.width * zoom,
                texture.height * zoom
            };
            DrawTexturePro(texture, sourceRect, destRect, Vector2{0, 0}, 0.0f, WHITE);
            //DrawRectangleLines(destRect.x,destRect.y,destRect.width,destRect.height,BLACK);
            
        }   
        inline void Update(Cell cells[c_chunkSize][c_chunkSize], std::vector<Tile> &tiles, bool renderLight) {
          Color* pixels = (Color*)image.data;  
          for (int x = 0; x < c_chunkSize; x++) {
                for (int y = 0; y < c_chunkSize; y++) {
                    Color tileColor = SKYBLUE;
                    if (cells[x][y].type!=0) {
                        tileColor = tiles[cells[x][y].type].color;
                        if (hashValues[x][y]&1) {
                            tileColor.r*=0.95;
                            tileColor.g*=0.95;
                            tileColor.b*=0.95;
                        }
                        else if (hashValues[x][y]%3==0) {
                            tileColor.r*=1.05;
                            tileColor.g*=1.05;
                            tileColor.b*=1.05;
                        }
                    }
                    int dx = x/c_lightResolution;
                    int dy = y/c_lightResolution;
                    unsigned char finalR = (unsigned char)((tileColor.r / 255.0f) * (r[dx][dy]+rSource[dx][dy]));
                    unsigned char finalG = (unsigned char)((tileColor.g / 255.0f) * (g[dx][dy]+gSource[dx][dy]));
                    unsigned char finalB = (unsigned char)((tileColor.b / 255.0f) * (b[dx][dy]+bSource[dx][dy]));
                            
                    pixels[y*c_chunkSize+x] = {
                        finalR,
                        finalG,
                        finalB,
                        255
                    };

                }
            }
            UpdateTexture(texture,image.data);
            updated = false;
        }
        inline void Init(int cx, int cy) {
            updated = false;
            image = GenImageColor(c_chunkSize,c_chunkSize,BLACK);
            texture = LoadTextureFromImage(image);
            for (int x = 0; x < c_gridLightSize; x++) {
                for (int y = 0; y < c_gridLightSize; y++) {
                    r[x][y] = 0;
                    g[x][y] = 0;
                    b[x][y] = 0;
                    rLight[x][y] = 0;
                    gLight[x][y] = 0;
                    bLight[x][y] = 0;
                    rSource[x][y] = 0;
                    gSource[x][y] = 0;
                    bSource[x][y] = 0;
                    rNext[x][y] = 0;
                    gNext[x][y] = 0;
                    bNext[x][y] = 0;
                    hashValues[x][y] = hash(x+cx*c_chunkSize,y+cy*c_chunkSize);
                }
            }
        }
    };

    struct World {
        std::map<std::tuple<int, int>, Chunk> chunkMap;
        std::map<std::tuple<int, int>, CAGI> lightMap;
        
        int chunksX = (c_screenWidth + c_chunkSize - 1) / c_chunkSize;
        int chunksY = (c_screenHeight + c_chunkSize - 1) / c_chunkSize;
        int lastUpdated[c_screenWidth];
        bool toBeUpdatedLine[c_screenWidth];
        std::vector<Tile> materials;
        inline std::string trim(const std::string& s) {
            size_t f = s.find_first_not_of(" \t");
            size_t l = s.find_last_not_of(" \t");
            return (f == std::string::npos) ? "" : s.substr(f, (l - f + 1));
        }
        inline Color parseColor(std::string s) {
            s = s.substr(1, s.size() - 2); // Remove parens
            std::stringstream ss(s);
            std::string val;
            std::vector<int> c;
            while (std::getline(ss, val, ',')) c.push_back(std::stoi(val));
            return {c[0], c[1], c[2], c[3]};
        }
        inline void SaveWorld() {
            Image image = GenImageColor(c_screenWidth,c_screenHeight,BLANK);
            for (int x = 0; x < c_screenWidth; x++) {
                for (int y = 0; y < c_screenHeight; y++) {
                    int cx = x/c_chunkSize;
                    int cy = y/c_chunkSize;
                    Color color = {chunkMap[{cx,cy}].blocks[x%c_chunkSize][y%c_chunkSize].type,chunkMap[{cx,cy}].blocks[x%c_chunkSize][y%c_chunkSize].lifeTime,chunkMap[{cx,cy}].blocks[x%c_chunkSize][y%c_chunkSize].direction,255};
                    
                    ImageDrawPixel(&image, x,y,color);
                }
            }
            ExportImage(image,"world.png");
        }
        inline void LoadWorld() {
            Image image = LoadImage("world.png");
                
            for (int cx = 0; cx < chunksX; cx++) {
                for (int cy = 0; cy < chunksY; cy++) {
                    if (chunkMap.find({cx, cy}) == chunkMap.end()) {
                        chunkMap[{cx, cy}] = Chunk();
                        chunkMap[{cx, cy}].image = GenImageColor(c_chunkSize, c_chunkSize, SKYBLUE);
                        chunkMap[{cx, cy}].texture = LoadTextureFromImage(chunkMap[{cx, cy}].image);
                        chunkMap[{cx, cy}].containsData = false;
                        chunkMap[{cx, cy}].toBeUpdated = true;
                        chunkMap[{cx, cy}].lastUpdate = 0;
                        
                        for (int i = 0; i < c_chunkSize; i++) {
                            chunkMap[{cx, cy}].moveDown[i].type = 0;
                            chunkMap[{cx, cy}].moveUp[i].type = 0;
                            chunkMap[{cx, cy}].moveLeft[i].type = 0;
                            chunkMap[{cx, cy}].moveRight[i].type = 0;
                            chunkMap[{cx, cy}].updatedYLine[i] = true;
                        }
                    }
                }
            }
            for (int x = 0; x < c_screenWidth; x++) {
                for (int y = 0; y < c_screenHeight; y++) {
                    int cx = x/c_chunkSize;
                    int cy = y/c_chunkSize;
                    Color color = GetImageColor(image,x,y);
                    chunkMap[{cx,cy}].blocks[x%c_chunkSize][y%c_chunkSize].type = color.r;
                    chunkMap[{cx,cy}].blocks[x%c_chunkSize][y%c_chunkSize].lifeTime = color.g;
                    chunkMap[{cx,cy}].blocks[x%c_chunkSize][y%c_chunkSize].direction = color.b;
                }
            }
        }
        inline void loadMaterials(const std::string& filename) {
            std::ifstream file(filename);
            if (!file.is_open()){
                std::cerr<<"Failed to load materials \n";
                return;
            } 


            std::cout << "Loading materials from: " << filename << std::endl;
            std::string line;
            Tile* current = nullptr;

            while (std::getline(file, line)) {
                if (line.empty()) continue;

                if (line.back() == ':') {
                    materials.emplace_back();
                    current = &materials.back();
                    current->name = line.substr(0, line.size() - 1);
                    current->dissolves = -1; 
                    current->gas = false;
                } else if (current) {
                    size_t sep = line.find('=');
                    std::string k = trim(line.substr(0, sep));
                    std::string v = trim(line.substr(sep + 1));
                    if (k == "weight") current->weight = std::stoi(v);
                    else if (k == "falls") current->falls = (v == "true");
                    else if (k == "grassBlades") current->grassBlades = (v == "true");
                    else if (k == "gas") current->gas = (v == "true");
                    else if (k == "goes_to_sides") current->goesBothWays = (v == "true");
                    else if (k == "color") current->color = parseColor(v);
                    else if (k == "dissolve" && v != "None") current->dissolves = std::stoi(v);
                    else if (k == "mass" && v != "None") current->mass = std::stoi(v);
                    else if (k == "lifeTime" && v != "None") current->lifeTime = std::stoi(v);
                    else if (k == "leaveBehind" && v != "None") current->leaveBehind = std::stoi(v);
                    else if (k=="lightAbsorb") current->lightAbsorb = float(std::stoi(v))/100.0f;
                    else if (k == "fluid" && v != "None") current->fluid = (v == "true");
                }
            }
        }
        inline void Draw(Vector2 cameraPosition, float zoom) {
            for (int x = 0; x < chunksX; x++) {
                for (int y = 0; y < chunksY; y++) {
                    if (chunkMap[{x,y}].toBeUpdated) {
                        chunkMap[{x, y}].UpdateDisplayBuffer(materials);
                    }
                    if (chunkMap[{x, y}].containsData) {
                        chunkMap[{x, y}].Draw(x, y,cameraPosition,zoom);
                    }

                }
            }
        }
        inline void DebugActivityDisplay(Vector2 cameraPosition, float zoom) {
            for (int x = 0; x < chunksX; x++) {
                for (int y = 0; y < chunksY; y++) {
                    if (chunkMap[{x,y}].containsData) {
                        float screenX = (x * c_chunkSize - cameraPosition.x) * zoom;
                        float screenY = (y * c_chunkSize - cameraPosition.y) * zoom;
                        float size = c_chunkSize * zoom;
                        
                        if (screenX + size > 0 && screenX < c_screenWidth &&
                            screenY + size > 0 && screenY < c_screenHeight) {
                            if (chunkMap[{x,y}].containsData) {
                               DrawRectangleLines(screenX, screenY, size, size, BLACK);
                                
                                int fontSize = (int)(16 * zoom);
                                if (fontSize < 8) fontSize = 8;
                                if (fontSize > 32) fontSize = 32;
                                
                                DrawText(TextFormat("%d", chunkMap[{x,y}].lastUpdate),
                                        screenX, screenY, fontSize, BLACK);
                            }
                            
                        }
                    }
                }
            }
        }
        inline void UpdatePhysics(std::vector<Tile> &tiles, Vector2 cameraPosition, Vector2 screenSize) {
            int startX = cameraPosition.x-800;
            if (startX<0) startX = 0;
            int startY = cameraPosition.y-800;
            if (startY<0) startY = 0;
            int endX = cameraPosition.x+screenSize.x+800;
            if (endX>chunksX*c_chunkSize) endX = chunksX*c_chunkSize;
            int endY = cameraPosition.y+screenSize.y+800;
            if (endY>chunksY*c_chunkSize) endY = chunksY*c_chunkSize;
            
            #pragma omp parallel for collapse(2)
            for (int x = startX/c_chunkSize; x < endX/c_chunkSize; x++) {
                for (int y = startY/c_chunkSize; y < endY/c_chunkSize; y++) {
                    if (!chunkMap[{x,y}].generated) {
                        continue;
                    }
                    if (chunkMap[{x,y}].lastUpdate<c_sleepTime && chunkMap[{x,y}].generated) {
                        
                        for (int dx = 0; dx < c_chunkSize; dx++)
                            for (int dy = 0; dy < c_chunkSize; dy++)
                                chunkMap[{x,y}].blocks[dx][dy].updated = false;
                        chunkMap[{x,y}].UpdatePhysics(tiles);
                        //swaps and moves the data
                        for (int cx = 0; cx < c_chunkSize; cx++) {
                                
                            if (chunkMap[{x,y}].swapDown[cx].type != 0 && y+1 < chunksY) {
                                if (chunkMap[{x,y+1}].blocks[cx][0].type != 0 && 
                                    chunkMap[{x,y+1}].blocks[cx][0].type != 255) {
                                    Cell temp = chunkMap[{x,y+1}].blocks[cx][0];
                                    chunkMap[{x,y+1}].blocks[cx][0] = chunkMap[{x,y}].swapDown[cx];
                                    chunkMap[{x,y}].blocks[cx][c_chunkSize-1] = temp;
                                    
                                    chunkMap[{x,y}].toBeUpdated = true;
                                    chunkMap[{x,y+1}].toBeUpdated = true;
                                    chunkMap[{x,y}].lastUpdate = 0;
                                    chunkMap[{x,y+1}].lastUpdate = 0;
                                }
                                chunkMap[{x,y}].swapDown[cx].type = 0;
                            }
                            //Eeset neighbor data copy update flags
                            chunkMap[{x,y}].bottomChunkDataCopy[cx].updated = false;
                            chunkMap[{x,y}].topChunkDataCopy[cx].updated = false;
                            chunkMap[{x,y}].leftChunkDataCopy[cx].updated = false;
                            chunkMap[{x,y}].rightChunkDataCopy[cx].updated = false;
                            //Copy data from neighbors
                            if (y+1<chunksY) chunkMap[{x,y}].bottomChunkDataCopy[cx] = chunkMap[{x,y+1}].blocks[cx][0];
                            else chunkMap[{x,y}].bottomChunkDataCopy[cx].type = 255;
                            if (y-1>=0) chunkMap[{x,y}].topChunkDataCopy[cx] = chunkMap[{x,y-1}].blocks[cx][c_chunkSize-1];
                            else chunkMap[{x,y}].topChunkDataCopy[cx].type = 255;
                            if (x-1>=0) chunkMap[{x,y}].leftChunkDataCopy[cx] = chunkMap[{x-1,y}].blocks[c_chunkSize-1][cx];
                            else chunkMap[{x,y}].leftChunkDataCopy[cx].type = 255;
                            if (x+1<chunksX) chunkMap[{x,y}].rightChunkDataCopy[cx] = chunkMap[{x+1,y}].blocks[0][cx];
                            else chunkMap[{x,y}].rightChunkDataCopy[cx].type = 255;
                            //Proces cross chunk movement
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
                        if (chunkMap[{x,y}].updatedYLine[cx]) {
                            chunkMap[{x,y}].updatedYLine[cx] = false;
                            toBeUpdatedLine[x*c_chunkSize+cx] = true;
                        }
                        }

                    }
                }
            }
            //Clear movement for chunks that just updated
             #pragma omp parallel for collapse(2)
            
            for (int x = 0; x < chunksX; x++) {
                for (int y = 0; y < chunksY; y++) {
                    if (chunkMap[{x,y}].generated && chunkMap[{x,y}].lastUpdate==0) {
                        for (int i = 0; i < c_chunkSize; i++) {
                            chunkMap[{x,y}].moveDown[i].type = 0;
                            chunkMap[{x,y}].moveUp[i].type = 0;
                            chunkMap[{x,y}].moveLeft[i].type = 0;
                            chunkMap[{x,y}].moveRight[i].type = 0;
                        }
                    }
                    
                }
            }
        }
        inline void CalculateTotalDeltaDifference() {
            int dif = 0;
            for (int x = 0; x < chunksX; x++) {
                for (int y = 0; y < chunksY; y++) {
                    if (chunkMap[{x,y}].generated) {
                        dif += chunkMap[{x,y}].CalculateTheDelta();
                    }
                    
                }   
            }
            std::cout<<dif<<"\n";
        }
        /**
         * Updates lighting for all visible columns. Update checks are called by behavior functions
         */
        inline void UpdateYLine(int x, std::vector<Tile> &tiles) {

            
            float lightStrength = 1;
            int endedY = 0;
            for (int y = 0; y < (chunksY*c_chunkSize); y+=1) {
                int dx = x/c_chunkSize;
                int dy = y/c_chunkSize;
                int fx = (x%c_chunkSize)/c_lightResolution;
                int fy = (y%c_chunkSize)/c_lightResolution;
                auto& chunk = lightMap[{dx,dy}];
                //if (chunkMap[{dx,dy}].containsData) y+= c_chunkSize;
                if (chunkMap[{dx,dy}].generated) {

                    chunk.r[fx][fy] = WHITE.r*lightStrength;
                    chunk.g[fx][fy] = WHITE.g*lightStrength;
                    chunk.b[fx][fy] = WHITE.b*lightStrength;

                    chunk.updated = true;
                                
                    if (chunkMap[{dx,dy}].blocks[(x%c_chunkSize)][(y%c_chunkSize)].type!=0) {
                        lightStrength*=(tiles[chunkMap[{dx,dy}].blocks[(x%c_chunkSize)][(y%c_chunkSize)].type].lightAbsorb);
                    }
                    lastUpdated[x] = y;
                    if (lightStrength<0.1) break;
                }
                endedY= y;
            }
            
            for (int y = endedY; y < endedY+300 && y<c_screenHeight; y+=2) {
                int dx = x/c_chunkSize;
                int dy = y/c_chunkSize;
                auto& chunk = lightMap[{dx,dy}];
                if (chunk.generated) {
                    int fx = (x%c_chunkSize)/c_lightResolution;
                    int fy = (y%c_chunkSize)/c_lightResolution;
                    
                
                    chunk.r[fx][fy] = 30;
                    chunk.g[fx][fy] = 30;
                    chunk.b[fx][fy] = 30;
                }
            }
        }
        inline void UpdateLighting(std::vector<Tile> &tiles, Vector2 cameraPosition, Vector2 screenSize) {
            int startX = cameraPosition.x;
            if (startX<0) startX = 0;
            int endX = cameraPosition.x+screenSize.x;
            if (endX>chunksX*c_chunkSize) endX = chunksX*c_chunkSize;
            
            #pragma omp parallel for schedule(dynamic, 4)

            for (int x = startX; x < endX; x++) {
                if (toBeUpdatedLine[x]) {
                    if (x%c_lightResolution==0) {
                        UpdateYLine(x,tiles);
                        toBeUpdatedLine[x]= false;
                    }
                    
                }
            }
        }
    };
};