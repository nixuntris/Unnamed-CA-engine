#pragma once
#include "Chunk.hpp"
#include "Physics.hpp"

inline bool CheckBlock(CA::World *world ,int x, int y) {
    if (x<0 || x>=CA::c_screenWidth || y<0 || y>=CA::c_screenHeight) return true;
    return world->chunkMap[{x/CA::c_chunkSize,y/CA::c_chunkSize}].blocks[x%CA::c_chunkSize][y%CA::c_chunkSize].type!=0;
}
inline void SetBlock(CA::World *world ,int x, int y, uint8_t type) {
    if (x<0 || x>=CA::c_screenWidth || y<0 || y>=CA::c_screenHeight) return;
    world->chunkMap[{x/CA::c_chunkSize,y/CA::c_chunkSize}].blocks[x%CA::c_chunkSize][y%CA::c_chunkSize].type=type;
}
inline int  GetBlock(CA::World *world ,int x, int y) {
    if (x<0 || x>=CA::c_screenWidth || y<0 || y>=CA::c_screenHeight) return 0;
    return world->chunkMap[{x/CA::c_chunkSize,y/CA::c_chunkSize}].blocks[x%CA::c_chunkSize][y%CA::c_chunkSize].type;
}
class FrameBFS {
    bool first = true;
    std::unordered_set<int> visited;
        public:
    inline bool canReachEdge(CA::World* world, int startX, int startY) {
        const int MAX_STEPS = 257;
        const int dx[] = {1, -1, 0, 0};
        const int dy[] = {0, 0, 1, -1};
        
        std::queue<std::pair<int, int>> toVisit;
        
        toVisit.push({startX, startY});
        if (first) {
            visited.insert(startY * CA::c_screenWidth + startX);
        }
        first = false;
        
        int steps = 0;
        
        while (!toVisit.empty() && steps < MAX_STEPS) {
            auto [cx, cy] = toVisit.front();
            toVisit.pop();
            steps++;
            
            if (cx <= 0 || cx >= CA::c_screenWidth - 1 || 
                cy <= 0 || cy >= CA::c_screenHeight - 1) {
                return true; 
            }
            
            for (int i = 0; i < 4; i++) {
                int nx = cx + dx[i];
                int ny = cy + dy[i];
                
                if (nx >= 0 && nx < CA::c_screenWidth && 
                    ny >= 0 && ny < CA::c_screenHeight) {
                    
                    int key = ny * CA::c_screenWidth + nx;
                    if (visited.find(key) == visited.end() && CheckBlock(world, nx, ny)) {
                        visited.insert(key);
                        toVisit.push({nx, ny});
                    }
                }
            }
        }
        
        return steps >= MAX_STEPS || visited.size() > 100;
    }
};
inline bool carveShape (int offsetX, int offsetY, int x, int y, CA::World *world, Physics::Map *map) {
    Physics::ShapeGrid newShape;
    float pixelSize = 1;
    newShape.pixelSize = pixelSize;
    newShape.rotation = 0;
    newShape.angularVelocity = 0;
    newShape.mass = 1.0f;
    newShape.restitution = 0.8f;
    memset(newShape.grid, 0, sizeof(newShape.grid));
    bool hasObjects = false;
    int maxX = 0;
    int maxY = 0;
    for (int dy = 0; dy < Physics::MAX_SHAPE_SIZE; dy++) {
        for (int dx = 0; dx < Physics::MAX_SHAPE_SIZE; dx++) {
            newShape.grid[dx][dy] = 0;
            int worldX = dx+x+offsetX-Physics::MAX_SHAPE_SIZE/2;
            int worldY = dy+y-Physics::MAX_SHAPE_SIZE/2+offsetY;
            if (worldX<0 || worldX>=CA::c_screenWidth || worldY<0 || worldY>=CA::c_screenHeight) continue;
            if (CheckBlock(world, worldX, worldY)) {
                newShape.grid[dx][dy] = GetBlock(world,worldX,worldY);
                SetBlock(world, worldX,worldY, 0);
                world->toBeUpdatedLine[worldX] = true;
                hasObjects = true;

                if (dx>maxX) maxX = dx;
                if (dy>maxY) maxY = dy;
                
            }
        }
    }
    newShape.width = maxX;
    newShape.height = maxY;
        
    newShape.CalculatePixels(world->materials);
    newShape.x = x+maxX/2;
    newShape.y = y+maxY/2;
    newShape.id = (int)map->balls.size(); // You'll need access to map
    newShape.x_vel = (float)GetRandomValue(-2, 2);
    newShape.y_vel = (float)GetRandomValue(-5, -1);
    newShape.held = false;
    newShape.ownedByObject = false;
    newShape.canPickUp = true;
    newShape.mass = newShape.pixelCount * 0.1f;
    if (hasObjects) {
        map->balls.push_back(newShape);
        return true;
    }
    return false;
};
inline bool wouldSplitStructure(CA::World* world, int x, int y, Physics::Map* map, FrameBFS *bfs) {
    bool wouldSplit = false;
    
    if (CheckBlock(world,x+1,y) && x+1<CA::c_screenWidth) {
        if (!bfs->canReachEdge(world,x+1,y)) {
            wouldSplit = carveShape(1,0,x,y,world,map);
        }
    }
    
    if (CheckBlock(world,x,y+1) && y+1<CA::c_screenHeight) {
        if (!bfs->canReachEdge(world,x,y+1)) {
             wouldSplit = carveShape(1,0,x,y,world,map);
        }
    }
    
    if (CheckBlock(world,x-1,y) && x-1>=0) {
        if (!bfs->canReachEdge(world,x-1,y)) {
             wouldSplit = carveShape(1,0,x,y,world,map);
        }
    }
    
    if (CheckBlock(world,x,y-1) && y-1>=0) {
        if (!bfs->canReachEdge(world,x,y-1)) {
             wouldSplit = carveShape(1,0,x,y,world,map);
        }
    }
    
    return wouldSplit;
}