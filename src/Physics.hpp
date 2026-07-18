#pragma once
#include "raylib.h"
#include "raymath.h"
#include <cinttypes>
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
    
    void Update() {
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
