#pragma once
#include <iostream>
#include <raylib.h>
#include <vector>
#include <raymath.h>
#include <cmath>
#include "Chunk.hpp"
#include <algorithm>
namespace Physics {
    
    
    const Color colors[10] = {
        { 10, 30, 80, 255 }, { 15, 50, 120, 255 }, { 20, 80, 160, 255 },
        { 30, 110, 190, 255 }, { 40, 140, 210, 255 }, { 60, 170, 220, 255 },
        { 80, 200, 230, 255 }, { 100, 220, 240, 255 }, { 140, 240, 250, 255 },
        { 180, 250, 255, 255 }
    };
    const int WORLD_WIDTH = 1920;
    const int WORLD_HEIGHT = 1080;
    const int chunkSize = 40; 
    const int MAX_BALL_COUNT_PER_CHUNK = 128;
    const int GRID_W = (WORLD_WIDTH / chunkSize) + 1;
    const int GRID_H = (WORLD_HEIGHT / chunkSize) + 1;

    struct Ball {
        float x, y;
        int id;
        float y_vel, x_vel;
        Color color;
        bool held;
        bool ownedByObject;
        float radius;
    };


    struct Chunk {
        int ids[MAX_BALL_COUNT_PER_CHUNK];
        int count;
    };

    struct Map {
        std::vector<Ball> balls;
        Chunk grid[GRID_W][GRID_H];
    };


    void M_RecalculateGrid(Map* map) {
        for (int x = 0; x < GRID_W; x++) {
            for (int y = 0; y < GRID_H; y++) map->grid[x][y].count = 0;
        }

        for (int i = 0; i < (int)map->balls.size(); i++) {
            int cx = (int)(map->balls[i].x / chunkSize);
            int cy = (int)(map->balls[i].y / chunkSize);
            if (cx >= 0 && cx < GRID_W && cy >= 0 && cy < GRID_H) {
                Chunk& c = map->grid[cx][cy];
                if (c.count < MAX_BALL_COUNT_PER_CHUNK) {
                    c.ids[c.count++] = i;
                }
                else {
                    std::cout<<"WARNING: MAX COLLISION SPHERES PER CHUNK REACHED";
                }
            }
        }
    }

    void En_CollisionBall(int ballIdx, Map* map, const CA::World *world) {
        Ball& ball = map->balls[ballIdx];
        int cx = (int)(ball.x / chunkSize);
        int cy = (int)(ball.y / chunkSize);

        for (int nx = cx - 1; nx <= cx + 1; nx++) {
            for (int ny = cy - 1; ny <= cy + 1; ny++) {
                if (nx < 0 || nx >= GRID_W || ny < 0 || ny >= GRID_H) continue;

                Chunk& chunk = map->grid[nx][ny];
                for (int i = 0; i < chunk.count; i++) {
                    Ball& other = map->balls[chunk.ids[i]];
                    if (other.id <= ball.id) continue;

                    float dx = ball.x - other.x;
                    float dy = ball.y - other.y;
                    float distSq = dx * dx + dy * dy;
                    float minFill = ball.radius + other.radius;

                    if (distSq < minFill * minFill && distSq > 0.0001f) {
                        float dist = sqrtf(distSq);
                        float overlap = (minFill - dist) * 0.5f;
                        float nx_norm = dx / dist;
                        float ny_norm = dy / dist;

                        ball.x += nx_norm * overlap;
                        ball.y += ny_norm * overlap;
                        other.x -= nx_norm * overlap;
                        other.y -= ny_norm * overlap;

                        float dvx = ball.x_vel - other.x_vel;
                        float dvy = ball.y_vel - other.y_vel;
                        float impact = (dvx * nx_norm + dvy * ny_norm);
                        if (impact < 0) {
                            ball.x_vel -= impact * nx_norm;
                            ball.y_vel -= impact * ny_norm;
                            other.x_vel += impact * nx_norm;
                            other.y_vel += impact * ny_norm;
                        }
                    }
                }
            }
        }
        
        int chunkX = (int)(ball.x / CA::c_chunkSize);
        int chunkY = (int)(ball.y / CA::c_chunkSize);
        
        // Check surrounding chunks
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                int checkX = chunkX + dx;
                int checkY = chunkY + dy;
                
                auto chunkIt = world->chunkMap.find({checkX, checkY});
                if (chunkIt == world->chunkMap.end()) continue;
                
                const CA::Chunk& chunk = chunkIt->second;
                
                // Check blocks around ball position
                int startBlockX = std::max(0, (int)((ball.x - ball.radius - 1) / CA::c_chunkSize * CA::c_chunkSize - checkX * CA::c_chunkSize));
                int endBlockX = std::min(CA::c_chunkSize - 1, (int)((ball.x + ball.radius + 1) / CA::c_chunkSize * CA::c_chunkSize - checkX * CA::c_chunkSize));
                int startBlockY = std::max(0, (int)((ball.y - ball.radius - 1) / CA::c_chunkSize * CA::c_chunkSize - checkY * CA::c_chunkSize));
                int endBlockY = std::min(CA::c_chunkSize - 1, (int)((ball.y + ball.radius + 1) / CA::c_chunkSize * CA::c_chunkSize - checkY * CA::c_chunkSize));
                
                for (int bx = startBlockX; bx <= endBlockX; bx++) {
                    for (int by = startBlockY; by <= endBlockY; by++) {
                        uint8_t blockType = chunk.blocks[bx][by].type;
                        if (blockType == 0) continue;
                        float blockWorldX = (checkX * CA::c_chunkSize + bx) + 0.5f;
                        float blockWorldY = (checkY * CA::c_chunkSize + by) + 0.5f;
                        float dxBlock = ball.x - blockWorldX;
                        float dyBlock = ball.y - blockWorldY;
                        float distSq = dxBlock * dxBlock + dyBlock * dyBlock;
                        float blockRadius = 0.5f;
                        float minDist = ball.radius + blockRadius;
                        
                        if (distSq < minDist * minDist && distSq > 0.0001f) {
                            float dist = sqrtf(distSq);
                            float overlap = (minDist - dist) * 1.0f;
                            float nx = dxBlock / dist;
                            float ny = dyBlock / dist;
                            ball.x += nx * overlap;
                            ball.y += ny * overlap;
                            float velocityDot = ball.x_vel * nx + ball.y_vel * ny;
                            if (velocityDot < 0) {
                                ball.x_vel -= 2.0f * velocityDot * nx * 0.8f;
                                ball.y_vel -= 2.0f * velocityDot * ny * 0.8f;
                                
                                if (rand() % 100 < 10) {
                                    ball.x_vel += (rand() % 100 - 50) * 0.01f;
                                    ball.y_vel += (rand() % 100 - 50) * 0.01f;
                                }
                            }
                        }
                    }
                }
            }
        }
        if (ball.y > WORLD_HEIGHT-ball.radius*2) {
            ball.y = WORLD_HEIGHT-ball.radius*2;
            ball.y_vel *= -0.2f;
            if (fabs(ball.y_vel) < 0.1f) ball.y_vel = 0;
        }
        if (ball.y <ball.radius*2) {
            ball.y = ball.radius*2;
            ball.y_vel *= -0.2f;
            if (fabs(ball.y_vel) < 0.1f) ball.y_vel = 0;
        }
        if (ball.x>WORLD_WIDTH-ball.radius*2) {
            ball.x = WORLD_WIDTH-ball.radius*2;
            ball.x_vel *= -0.2f;
            if (fabs(ball.x_vel) < 0.1f) ball.x_vel = 0;
            
        }
        if (ball.x<ball.radius*2) {
            ball.x = ball.radius*2;
            ball.x_vel *= -0.2f;
            if (fabs(ball.x_vel) < 0.1f) ball.x_vel = 0;
            
        }
    }

}
