#include "raylib.h"
#include <cinttypes>
#include <map>
#include <iostream>
const int c_chunkSize = 64;
struct Chunk {
	uint8_t blocks[c_chunkSize][c_chunkSize];
    
    uint8_t moveDown[c_chunkSize];
    uint8_t moveUp[c_chunkSize];
    uint8_t moveLeft[c_chunkSize];
    uint8_t moveRight[c_chunkSize];

    uint8_t topChunkDataCopy[c_chunkSize]; //Y+ Copy
    uint8_t bottomChunkDataCopy[c_chunkSize]; //Y- Copy    
    uint8_t leftChunkDataCopy[c_chunkSize]; //X- copy
    uint8_t rightChunkDataCopy[c_chunkSize]; //X+ copy
	
    Image image;
	Texture texture;
	bool toBeUpdated;
	bool containsData;
    int lastUpdate;
	
    void Draw(int x, int y) {
		DrawTexture(texture, x * c_chunkSize, y * c_chunkSize, WHITE);
	}
	void UpdateDisplayBuffer() {
		containsData = false;
        const Color colorMap[10] = { 
            BLANK,  
            YELLOW, 
            GRAY, 
            BLUE,  
            BLANK,
            BLANK, 
            BLANK, 
            BLANK, 
            BLANK, 
            BLANK 
        };
        Color* pixels = (Color*)image.data;
		for (int x = 0; x < c_chunkSize; x++) {
			for (int y = 0; y < c_chunkSize; y++) {
				if (blocks[x][y] != 0) {
					containsData = true;
				}
                pixels[y * c_chunkSize + x] = colorMap[blocks[x][y]];
			}
		}
		UpdateTexture(texture, image.data);
		toBeUpdated = false;
	}
    void UpdatePhysics() {
        lastUpdate+=1;
        bool updated[c_chunkSize][c_chunkSize] = {false};
        
        for (int y = c_chunkSize - 1; y >= 0; y--) {
            for (int x = 0; x < c_chunkSize; x++) {
                if (blocks[x][y] != 0 && !updated[x][y]) {
                    if (y + 1 < c_chunkSize) {
                        if (blocks[x][y + 1] == 0 && !updated[x][y + 1]) {
                            blocks[x][y + 1] = blocks[x][y];
                            blocks[x][y] = 0;
                            toBeUpdated = true;
                            updated[x][y + 1] = true;
                            updated[x][y] = true;
                            lastUpdate = 0;
                        }
                    }
                    else {
                        if (bottomChunkDataCopy[x] == 0) {
                            moveDown[x] = blocks[x][y];
                            blocks[x][y] = 0;
                            toBeUpdated = true;
                            lastUpdate = 0;
                        }
                    }
                }
            }
        }
    }
};
Chunk GenCleanChunk() {
	Chunk chunk;
    chunk.toBeUpdated = false;
    chunk.containsData = false;
    chunk.lastUpdate = 100;
	for (int x = 0; x < c_chunkSize; x++) {
        chunk.moveDown[x] = 0;
        chunk.moveUp[x] = 0;
        chunk.moveLeft[x] = 0;
        chunk.moveRight[x] = 0;
        chunk.bottomChunkDataCopy[x] = 0;
        chunk.leftChunkDataCopy[x] = 0;
        chunk.rightChunkDataCopy[x] = 0;
        chunk.topChunkDataCopy[x] = 0;
        for (int y = 0; y < c_chunkSize; y++) chunk.blocks[x][y] = 0;
    }
	chunk.image = GenImageColor(c_chunkSize, c_chunkSize, SKYBLUE);
	chunk.texture = LoadTextureFromImage(chunk.image);
	return chunk;
}
class App {
	std::map<std::tuple<int, int>, Chunk> chunkMap;
	int editSize = 15;
public:
	App() {
		InitWindow(1920, 1080, "a");
		for (int x = 0; x < 1920 / c_chunkSize; x++) {
			for (int y = 0; y < 1080 / c_chunkSize; y++) {
				chunkMap[std::tuple<int, int>{x, y}] = GenCleanChunk();
			}
		}
        //SetTargetFPS(60);
	}
	void Run() {
		while (!WindowShouldClose()) {
			BeginDrawing();
			ClearBackground(SKYBLUE);
			for (int x = 0; x < 1920 / c_chunkSize; x++) {
				for (int y = 0; y < 1080 / c_chunkSize; y++) {
					if (chunkMap[{x,y}].toBeUpdated) {
						chunkMap[{x, y}].UpdateDisplayBuffer();
					}
					if (chunkMap[{x, y}].containsData) {
						chunkMap[{x, y}].Draw(x, y);
                    }
					
				}
			}
            
			for (int x = 0; x < 1920 / c_chunkSize; x++) {
				for (int y = 0; y < 1080 / c_chunkSize; y++) {
                    if (chunkMap[{x,y}].containsData) {
                   //     DrawRectangleLines(x*c_chunkSize,y*c_chunkSize,c_chunkSize,c_chunkSize,BLACK);
                  //      DrawText(TextFormat("%d", chunkMap[{x,y}].lastUpdate),x*c_chunkSize,y*c_chunkSize,16,BLACK);
                    }
                }
            }
            for (int x = 0; x < 1920 / c_chunkSize; x++) {
                for (int y = 0; y < 1080 / c_chunkSize; y++) {
                    if (chunkMap[{x,y}].containsData && chunkMap[{x,y}].lastUpdate<30) {
                        for (int cx = 0; cx < c_chunkSize; cx++) {
                            chunkMap[{x,y}].bottomChunkDataCopy[cx] = chunkMap[{x,y+1}].blocks[cx][0];
                            chunkMap[{x,y}].topChunkDataCopy[cx] = chunkMap[{x,y+1}].blocks[cx][c_chunkSize-1];
                            chunkMap[{x,y}].leftChunkDataCopy[cx] = chunkMap[{x-1,y}].blocks[c_chunkSize-1][cx];
                            chunkMap[{x,y}].rightChunkDataCopy[cx] = chunkMap[{x+1,y}].blocks[0][cx];

                            if (chunkMap[{x,y}].moveDown[cx] != 0) {
                                chunkMap[{x,y+1}].blocks[cx][0] = chunkMap[{x,y}].moveDown[cx]; 
                                chunkMap[{x,y}].moveDown[cx] = 0;
                                chunkMap[{x,y}].toBeUpdated = true;
                                chunkMap[{x,y+1}].toBeUpdated = true; 
                                chunkMap[{x,y+1}].containsData = true;
                                chunkMap[{x,y+1}].lastUpdate = 0;
                            }
                                        
                            if (chunkMap[{x,y}].moveUp[cx] != 0 && y - 1 >= 0) {
                                chunkMap[{x,y-1}].blocks[cx][c_chunkSize-1] = chunkMap[{x,y}].moveUp[cx];
                                chunkMap[{x,y}].moveUp[cx] = 0;
                                chunkMap[{x,y}].toBeUpdated = true;
                                chunkMap[{x,y-1}].toBeUpdated = true;
                                chunkMap[{x,y-1}].containsData = true;
                                chunkMap[{x,y-1}].lastUpdate = 0;
                            }
                            
                            if (chunkMap[{x,y}].moveLeft[cx] != 0 && x - 1 >= 0) {
                                chunkMap[{x-1,y}].blocks[c_chunkSize-1][cx] = chunkMap[{x,y}].moveLeft[cx];
                                chunkMap[{x,y}].moveLeft[cx] = 0;
                                chunkMap[{x,y}].toBeUpdated = true;
                                chunkMap[{x-1,y}].toBeUpdated = true;
                                chunkMap[{x-1,y}].containsData = true;
                                chunkMap[{x-1,y}].lastUpdate = 0;
                            }
                            
                            if (chunkMap[{x,y}].moveRight[cx] != 0 && x + 1 < 1920 / c_chunkSize) {
                                chunkMap[{x+1,y}].blocks[0][cx] = chunkMap[{x,y}].moveRight[cx];
                                chunkMap[{x,y}].moveRight[cx] = 0;
                                chunkMap[{x,y}].toBeUpdated = true;
                                chunkMap[{x+1,y}].toBeUpdated = true;
                                chunkMap[{x+1,y}].containsData = true;
                                chunkMap[{x+1,y}].lastUpdate = 0;
                            }
                        }
                        
                        chunkMap[{x,y}].UpdatePhysics();
                    }
                }
            }
			if (IsMouseButtonDown(0)) {
				for (int x = 0; x < editSize; x++) {
					for (int y = 0; y < editSize; y++) {
						int updateX = x + GetMouseX();
						int updateY = y + GetMouseY();
						chunkMap[{updateX / c_chunkSize, updateY / c_chunkSize}].blocks[updateX % c_chunkSize][updateY % c_chunkSize] = 2;
						chunkMap[{updateX / c_chunkSize, updateY / c_chunkSize}].toBeUpdated = true;
                        chunkMap[{updateX / c_chunkSize, updateY / c_chunkSize}].lastUpdate = 0;
					}
				}
			}
			DrawFPS(0, 0);
			EndDrawing();
		}
	}
};

int main() {
	App app;
	app.Run();
}