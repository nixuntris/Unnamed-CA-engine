#pragma once
#include <iostream>
#include <raylib.h>
#include <vector>
#include <raymath.h>
#include <cmath>
#include "Chunk.hpp"
#include <algorithm>
#include <omp.h>

namespace Physics {

    const Color colors[10] = {
        { 10, 30, 80, 255 }, { 15, 50, 120, 255 }, { 20, 80, 160, 255 },
        { 30, 110, 190, 255 }, { 40, 140, 210, 255 }, { 60, 170, 220, 255 },
        { 80, 200, 230, 255 }, { 100, 220, 240, 255 }, { 140, 240, 250, 255 },
        { 180, 250, 255, 255 }
    };
    const int WORLD_WIDTH = 1920*4;
    const int WORLD_HEIGHT = 1080*4;
    const int chunkSize = 40; 
    const int MAX_BALL_COUNT_PER_CHUNK = 128;
    const int GRID_W = (WORLD_WIDTH / chunkSize) + 1;
    const int GRID_H = (WORLD_HEIGHT / chunkSize) + 1;

    const int MAX_SHAPE_SIZE = 8;
    const int MAX_PIXELS = 64;
    
    struct ShapeGrid {
        float x, y;
        int id;
        float x_vel, y_vel;
        float rotation;    
        float angularVelocity;  
        Color color;
        bool held;
        bool ownedByObject;
        float radius;         
        bool grid[MAX_SHAPE_SIZE][MAX_SHAPE_SIZE];
        int pixelCount;
        float pivotX;
        float pivotY;
        int width;
        int height;
        float pixelPositions[MAX_PIXELS][2];
        float collisionRadius;
        float pixelSize;
        float mass;
        float restitution;
        float momentOfInertia;  
        
        void CalculatePixels() {
            pixelCount = 0;
            float maxDist = 0;
            pivotX = width * pixelSize / 2.0f;
            pivotY = height * pixelSize / 2.0f;
            
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    if (grid[y][x]) { 
                        float px = x * pixelSize + pixelSize/2 - pivotX;
                        float py = y * pixelSize + pixelSize/2 - pivotY;
                        pixelPositions[pixelCount][0] = px;
                        pixelPositions[pixelCount][1] = py;
                        pixelCount++;
                        float dist = sqrt(px*px + py*py);
                        if (dist > maxDist) maxDist = dist;
                    }
                }
            }
            collisionRadius = maxDist + pixelSize;
            radius = collisionRadius;
            
            if (pixelCount > 0) {
                float pixelMass = mass / pixelCount;
                momentOfInertia = 0.0f;
                for (int i = 0; i < pixelCount; i++) {
                    float dx = pixelPositions[i][0];
                    float dy = pixelPositions[i][1];
                    momentOfInertia += (dx*dx + dy*dy) * pixelMass;
                }
            } else {
                momentOfInertia = 0.001f; 
            }
        }
    };

    ShapeGrid CreateTriangle(float pixelSize = 5.0f) {
        ShapeGrid shape;
        shape.width = 5;
        shape.height = 3;
        shape.pixelSize = pixelSize;
        shape.rotation = 0;
        shape.angularVelocity = 0;
        shape.mass = 1.0f;
        shape.restitution = 0.8f;
        memset(shape.grid, 0, sizeof(shape.grid));
        bool triangle[3][5] = {
            {0,0,1,0,0},
            {0,1,1,1,0},
            {1,1,1,1,1}
        };
        
        for (int y = 0; y < 3; y++) {
            for (int x = 0; x < 5; x++) {
                shape.grid[y][x] = triangle[y][x];
            }
        }
        
        shape.CalculatePixels();
        return shape;
    }

    struct Chunk {
        int ids[MAX_BALL_COUNT_PER_CHUNK];
        int count;
    };

    struct Map {
        std::vector<ShapeGrid> balls;
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
            }
        }
    }

    void En_CollisionBall(int ballIdx, Map* map, const CA::World *world) {
        ShapeGrid& ball = map->balls[ballIdx];
        int cx = (int)(ball.x / chunkSize);
        int cy = (int)(ball.y / chunkSize);
        
        struct WorldPixel { float x, y; };
        WorldPixel ballPixels[MAX_PIXELS];
        for (int p = 0; p < ball.pixelCount; p++) {
            float cosA = cosf(ball.rotation);
            float sinA = sinf(ball.rotation);
            float rx = ball.pixelPositions[p][0] * cosA - ball.pixelPositions[p][1] * sinA;
            float ry = ball.pixelPositions[p][0] * sinA + ball.pixelPositions[p][1] * cosA;
            ballPixels[p].x = ball.x + rx;
            ballPixels[p].y = ball.y + ry;
        }
        
        for (int nx = cx - 1; nx <= cx + 1; nx++) {
            for (int ny = cy - 1; ny <= cy + 1; ny++) {
                if (nx < 0 || nx >= GRID_W || ny < 0 || ny >= GRID_H) continue;
                
                Chunk& chunk = map->grid[nx][ny];
                for (int i = 0; i < chunk.count; i++) {
                    ShapeGrid& other = map->balls[chunk.ids[i]];
                    if (other.id <= ball.id) continue;
                    
                    float dx = ball.x - other.x;
                    float dy = ball.y - other.y;
                    float dist = sqrtf(dx*dx + dy*dy);
                    if (dist > ball.collisionRadius + other.collisionRadius) continue;
                    
                    WorldPixel otherPixels[MAX_PIXELS];
                    for (int p = 0; p < other.pixelCount; p++) {
                        float cosA = cosf(other.rotation);
                        float sinA = sinf(other.rotation);
                        float rx = other.pixelPositions[p][0] * cosA - other.pixelPositions[p][1] * sinA;
                        float ry = other.pixelPositions[p][0] * sinA + other.pixelPositions[p][1] * cosA;
                        otherPixels[p].x = other.x + rx;
                        otherPixels[p].y = other.y + ry;
                    }
                    
                    float totalPenX = 0, totalPenY = 0;
                    int overlapCount = 0;
                    float minDist = (ball.pixelSize + other.pixelSize) * 0.5f;
                    
                    float contactSumX = 0.0f, contactSumY = 0.0f;
                    int contactCount = 0;
                    
                    for (int p1 = 0; p1 < ball.pixelCount; p1++) {
                        for (int p2 = 0; p2 < other.pixelCount; p2++) {
                            float dxp = ballPixels[p1].x - otherPixels[p2].x;
                            float dyp = ballPixels[p1].y - otherPixels[p2].y;
                            float distSq = dxp*dxp + dyp*dyp;
                            if (distSq < minDist*minDist && distSq > 0.0001f) {
                                float distP = sqrtf(distSq);
                                float pen = minDist - distP;
                                float nx = dxp / distP;
                                float ny = dyp / distP;
                                totalPenX += nx * pen;
                                totalPenY += ny * pen;
                                overlapCount++;
                                
                                contactSumX += (ballPixels[p1].x + otherPixels[p2].x) * 0.5f;
                                contactSumY += (ballPixels[p1].y + otherPixels[p2].y) * 0.5f;
                                contactCount++;
                            }
                        }
                    }
                    
                    if (overlapCount > 0) {
                        float avgPenX = totalPenX / overlapCount;
                        float avgPenY = totalPenY / overlapCount;
                        float penMag = sqrtf(avgPenX*avgPenX + avgPenY*avgPenY);
                        float normX = 0.0f;
                        float normY = 0.0f;
                        if (penMag > 0.0001f) {
                            normX = avgPenX / penMag;
                            normY = avgPenY / penMag;
                            float separation = penMag;
                            float totalMass = ball.mass + other.mass;
                            float ballRatio = other.mass / totalMass;
                            float otherRatio = ball.mass / totalMass;
                            
                            ball.x += normX * separation * ballRatio;
                            ball.y += normY * separation * ballRatio;
                            other.x -= normX * separation * otherRatio;
                            other.y -= normY * separation * otherRatio;
                            for (int p = 0; p < ball.pixelCount; p++) {
                                float cosA = cosf(ball.rotation);
                                float sinA = sinf(ball.rotation);
                                float rx = ball.pixelPositions[p][0] * cosA - ball.pixelPositions[p][1] * sinA;
                                float ry = ball.pixelPositions[p][0] * sinA + ball.pixelPositions[p][1] * cosA;
                                ballPixels[p].x = ball.x + rx;
                                ballPixels[p].y = ball.y + ry;
                            }
                        }
                        if (contactCount > 0) {
                            float contactX = contactSumX / contactCount;
                            float contactY = contactSumY / contactCount;
                            
                            float rAx = contactX - ball.x;
                            float rAy = contactY - ball.y;
                            float rBx = contactX - other.x;
                            float rBy = contactY - other.y;
                            
                            float vRelX = ball.x_vel - other.x_vel;
                            float vRelY = ball.y_vel - other.y_vel;
                            vRelX += -ball.angularVelocity * rAy + other.angularVelocity * rBy;
                            vRelY +=  ball.angularVelocity * rAx - other.angularVelocity * rBx;
                            
                            float vn = vRelX * normX + vRelY * normY;
                            if (vn < 0.0f) {
                                float invMassA = 1.0f / ball.mass;
                                float invMassB = 1.0f / other.mass;
                                float invIA = 1.0f / ball.momentOfInertia;
                                float invIB = 1.0f / other.momentOfInertia;
                                
                                float crossA = rAx * normY - rAy * normX;
                                float crossB = rBx * normY - rBy * normX;
                                
                                float denom = invMassA + invMassB + crossA * crossA * invIA + crossB * crossB * invIB;
                                float restitution = (ball.restitution + other.restitution) * 0.5f;
                                float j = -(1.0f + restitution) * vn / denom;
                                
                                float impulseX = j * normX;
                                float impulseY = j * normY;
                                
                                ball.x_vel += impulseX * invMassA;
                                ball.y_vel += impulseY * invMassA;
                                ball.angularVelocity += (rAx * impulseY - rAy * impulseX) * invIA;
                                
                                other.x_vel -= impulseX * invMassB;
                                other.y_vel -= impulseY * invMassB;
                                other.angularVelocity -= (rBx * impulseY - rBy * impulseX) * invIB;
                            }
                        }
                    }
                }
            }
        }

        int chunkX = (int)(ball.x / CA::c_chunkSize);
        int chunkY = (int)(ball.y / CA::c_chunkSize);
        
        for (int dx = -1; dx <= 1; dx++) {
            for (int dy = -1; dy <= 1; dy++) {
                int checkX = chunkX + dx;
                int checkY = chunkY + dy;
                
                auto chunkIt = world->chunkMap.find({checkX, checkY});
                if (chunkIt == world->chunkMap.end()) continue;
                
                const CA::Chunk& chunk = chunkIt->second;
                
                int startBlockX = std::max(0, (int)((ball.x - ball.collisionRadius - 1) / CA::c_chunkSize * CA::c_chunkSize - checkX * CA::c_chunkSize));
                int endBlockX = std::min(CA::c_chunkSize - 1, (int)((ball.x + ball.collisionRadius + 1) / CA::c_chunkSize * CA::c_chunkSize - checkX * CA::c_chunkSize));
                int startBlockY = std::max(0, (int)((ball.y - ball.collisionRadius - 1) / CA::c_chunkSize * CA::c_chunkSize - checkY * CA::c_chunkSize));
                int endBlockY = std::min(CA::c_chunkSize - 1, (int)((ball.y + ball.collisionRadius + 1) / CA::c_chunkSize * CA::c_chunkSize - checkY * CA::c_chunkSize));
                
                for (int bx = startBlockX; bx <= endBlockX; bx++) {
                    for (int by = startBlockY; by <= endBlockY; by++) {
                        uint8_t blockType = chunk.blocks[bx][by].type;
                        if (blockType == 0) continue;
                        
                        float blockWorldX = (checkX * CA::c_chunkSize + bx) + 0.5f;
                        float blockWorldY = (checkY * CA::c_chunkSize + by) + 0.5f;
                        float blockRadius = 1.0f;
                        float pixelRadius = ball.pixelSize * 0.5f;
                        
                        float totalPushX = 0, totalPushY = 0;
                        int pushCount = 0;
                        float contactSumX = 0.0f, contactSumY = 0.0f;
                        
                        for (int p = 0; p < ball.pixelCount; p++) {
                            float dxp = ballPixels[p].x - blockWorldX;
                            float dyp = ballPixels[p].y - blockWorldY;
                            float distSq = dxp*dxp + dyp*dyp;
                            
                            if (distSq < (blockRadius + pixelRadius)*(blockRadius + pixelRadius) && distSq > 0.0001f) {
                                float distP = sqrtf(distSq);
                                float pen = (blockRadius + pixelRadius) - distP;
                                float nx = dxp / distP;
                                float ny = dyp / distP;
                                totalPushX += nx * pen;
                                totalPushY += ny * pen;
                                pushCount++;
                                contactSumX += ballPixels[p].x;
                                contactSumY += ballPixels[p].y;
                            }
                        }
                        
                        if (pushCount > 0) {
                            float avgX = totalPushX / pushCount;
                            float avgY = totalPushY / pushCount;
                            float mag = sqrtf(avgX*avgX + avgY*avgY);
                            if (mag > 0.0001f) {
                                float normX = avgX / mag;
                                float normY = avgY / mag;
                                ball.x += normX * mag;
                                ball.y += normY * mag;
                                
                                for (int p = 0; p < ball.pixelCount; p++) {
                                    float cosA = cosf(ball.rotation);
                                    float sinA = sinf(ball.rotation);
                                    float rx = ball.pixelPositions[p][0] * cosA - ball.pixelPositions[p][1] * sinA;
                                    float ry = ball.pixelPositions[p][0] * sinA + ball.pixelPositions[p][1] * cosA;
                                    ballPixels[p].x = ball.x + rx;
                                    ballPixels[p].y = ball.y + ry;
                                }
                                float contactX = contactSumX / pushCount;
                                float contactY = contactSumY / pushCount;
                                float rX = contactX - ball.x;
                                float rY = contactY - ball.y;
                                
                                float vRelX = ball.x_vel + (-ball.angularVelocity * rY);
                                float vRelY = ball.y_vel + ( ball.angularVelocity * rX);
                                float vn = vRelX * normX + vRelY * normY;
                                
                                if (vn < 0.0f) {
                                    float invMass = 1.0f / ball.mass;
                                    float invI = 1.0f / ball.momentOfInertia;
                                    float cross = rX * normY - rY * normX; // n × r
                                    float denom = invMass + cross * cross * invI;
                                    float j = -(1.0f + ball.restitution) * vn / denom;
                                    
                                    ball.x_vel += j * normX * invMass;
                                    ball.y_vel += j * normY * invMass;
                                    ball.angularVelocity += (rX * (j * normY) - rY * (j * normX)) * invI;
                                }
                            }
                        }
                    }
                }
            }
        }
        ball.x_vel = Clamp(ball.x_vel, -5.0f, 5.0f);
        ball.y_vel = Clamp(ball.y_vel, -5.0f, 5.0f);
        ball.angularVelocity = Clamp(ball.angularVelocity, -10.0f, 10.0f); 
        if (ball.y > WORLD_HEIGHT - ball.collisionRadius) {
            ball.y = WORLD_HEIGHT - ball.collisionRadius;
            ball.y_vel *= -0.2f;
            if (fabs(ball.y_vel) < 0.1f) ball.y_vel = 0;
        }
        if (ball.y < ball.collisionRadius) {
            ball.y = ball.collisionRadius;
            ball.y_vel *= -0.2f;
            if (fabs(ball.y_vel) < 0.1f) ball.y_vel = 0;
        }
        if (ball.x > WORLD_WIDTH - ball.collisionRadius) {
            ball.x = WORLD_WIDTH - ball.collisionRadius;
            ball.x_vel *= -0.2f;
            if (fabs(ball.x_vel) < 0.1f) ball.x_vel = 0;
        }
        if (ball.x < ball.collisionRadius) {
            ball.x = ball.collisionRadius;
            ball.x_vel *= -0.2f;
            if (fabs(ball.x_vel) < 0.1f) ball.x_vel = 0;
        }
    }
}