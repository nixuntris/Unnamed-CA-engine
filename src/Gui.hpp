#pragma once
#include "raylib.h"
#include "raymath.h"
namespace GUI {
    bool Button(Vector2 position, Vector2 size, Color color, Color outline) {
        DrawRectangle(position.x,position.y,size.x,size.y,color);
        if (CheckCollisionRecs({position.x,position.y,size.x,size.y},{(float)GetMouseX(),(float)GetMouseY(),1,1})) {
            DrawRectangle(position.x,position.y,size.x,size.y,{255,255,255,120});
            DrawRectangleLines(position.x,position.y,size.x,size.y,outline);
            if (IsMouseButtonDown(0)) return true;
        }
        DrawRectangleLines(position.x,position.y,size.x,size.y,outline);
        return false;
    }
    bool ButtonWithSlider(Vector2 position, Vector2 size, Color color, Color outline, int* value, int minValue, int maxValue) {
        Rectangle buttonRect = {position.x, position.y, size.x, size.y};
        bool isHovered = CheckCollisionRecs(buttonRect, {(float)GetMouseX(), (float)GetMouseY(), 1, 1});
        
        DrawRectangle(position.x, position.y, size.x, size.y, color);
        
        char text[32];
        sprintf(text, "%d", *value);
        int fontSize = 20;
        int textWidth = MeasureText(text, fontSize);
        Vector2 textPos = {position.x + (size.x - textWidth) / 2, position.y + (size.y - fontSize) / 2 - 10};
        DrawText(text, textPos.x, textPos.y, fontSize, BLACK);
        
        Vector2 sliderPos = {position.x, position.y + size.y + 10};
        Vector2 sliderSize = {size.x, 15};
        Rectangle sliderRect = {sliderPos.x, sliderPos.y, sliderSize.x, sliderSize.y};
        
        float progress = (float)(*value - minValue) / (maxValue - minValue);
        float handleX = sliderPos.x + progress * (sliderSize.x - 20);
        float handleSize = 20;
        
        DrawRectangle(sliderPos.x, sliderPos.y, sliderSize.x, sliderSize.y, GRAY);
        DrawRectangle(sliderPos.x, sliderPos.y, progress * sliderSize.x, sliderSize.y, DARKGRAY);
        
        DrawRectangle(handleX, sliderPos.y - 2, handleSize, sliderSize.y + 4, outline);
        DrawRectangle(handleX + 2, sliderPos.y, handleSize - 4, sliderSize.y, color);
        
        static bool isDragging = false;
        static bool wasPressed = false;
        Rectangle handleRect = {handleX, sliderPos.y - 2, handleSize, sliderSize.y + 4};
        bool isHandleHovered = CheckCollisionRecs(handleRect, {(float)GetMouseX(), (float)GetMouseY(), 1, 1});  
        if (IsMouseButtonDown(MOUSE_LEFT_BUTTON)) {
            if (isHandleHovered || isDragging) {
                isDragging = true;
                float mouseX = GetMouseX();
                float newProgress = (mouseX - sliderPos.x) / sliderSize.x;
                newProgress = Clamp(newProgress, 0.0f, 1.0f);
                *value = minValue + (int)(newProgress * (maxValue - minValue));
                wasPressed = true;
            }
        } else {
            if (wasPressed) {
                isDragging = false;
                wasPressed = false;
            }
        }
        if (isHovered) {
            DrawRectangle(position.x, position.y, size.x, size.y, {255, 255, 255, 120});
            DrawRectangleLines(position.x, position.y, size.x, size.y, outline);
            return true;
        }
        
        DrawRectangleLines(position.x, position.y, size.x, size.y, outline);
        return isDragging; 
    }
}
