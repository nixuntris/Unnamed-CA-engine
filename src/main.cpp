#include "raylib.h"
#include <cinttypes>
#include <map>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
const int c_chunkSize = 64;
const int c_sleepTime = 30;
const int c_screenWidth = 1920;
const int c_screenHeight = 1080;

struct Tile {
    std::string name;
    int weight;
    bool goesBothWays;
    bool falls;
    Color color;
    int dissolves;
};
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
	bool updated[c_chunkSize][c_chunkSize] = {false};

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
    bool MoveDown(int x, int y) {
        if (y + 1 < c_chunkSize) {
            if (blocks[x][y + 1] == 0 && !updated[x][y + 1]) {
                blocks[x][y + 1] = blocks[x][y];
                blocks[x][y] = 0;
                toBeUpdated = true;
                updated[x][y + 1] = true;
                updated[x][y] = true;
                lastUpdate = 0;
                return true;
            }
        }
        else {
            if (bottomChunkDataCopy[x] == 0 && moveDown[x] == 0) {
                moveDown[x] = blocks[x][y];
                blocks[x][y] = 0;
                toBeUpdated = true;
                lastUpdate = 0;

                updated[x][y] = true;
                return true;
            }
        }
        return false;
    }
    bool MoveUp(int x, int y) {
        if (y - 1 >= 0) {
            if (blocks[x][y - 1] == 0 && !updated[x][y - 1]) {
                blocks[x][y - 1] = blocks[x][y];
                blocks[x][y] = 0;
                toBeUpdated = true;
                updated[x][y - 1] = true;
                updated[x][y] = true;
                lastUpdate = 0;
                return true;
            }
        }
        else {
            if (topChunkDataCopy[x] == 0 && moveUp[x] == 0) {
                moveUp[x] = blocks[x][y];
                blocks[x][y] = 0;
                toBeUpdated = true;
                lastUpdate = 0;
                updated[x][y] = true;
                return true;
            }
        }
        return false;
    }

    bool MoveLeft(int x, int y) {
        if (x - 1 >= 0) {
            if (blocks[x - 1][y] == 0 && !updated[x - 1][y]) {
                blocks[x - 1][y] = blocks[x][y];
                blocks[x][y] = 0;
                toBeUpdated = true;
                updated[x - 1][y] = true;
                updated[x][y] = true;
                lastUpdate = 0;
                return true;
            }
        }
        else {
            if (leftChunkDataCopy[y] == 0 && moveLeft[y] == 0) {
                moveLeft[y] = blocks[x][y];
                blocks[x][y] = 0;
                updated[x][y] = true;
                toBeUpdated = true;
                lastUpdate = 0;
                return true;
            }
        }
        return false;
    }

    bool MoveRight(int x, int y) {
        if (x + 1 < c_chunkSize) {
            if (blocks[x + 1][y] == 0 && !updated[x + 1][y]) {
                blocks[x + 1][y] = blocks[x][y];
                blocks[x][y] = 0;
                toBeUpdated = true;
                updated[x + 1][y] = true;
                updated[x][y] = true;
                lastUpdate = 0;
                return true;
            }
        }
        else {
            if (rightChunkDataCopy[y] == 0 && moveRight[y] == 0) {
                moveRight[y] = blocks[x][y];
                blocks[x][y] = 0;
                toBeUpdated = true;
                lastUpdate = 0;
                updated[x][y] = true;
                return true;
            }
        }
        return false;
    }
    bool MoveDownLeft(int x, int y)
    {
        if (x > 0 && y + 1 < c_chunkSize)
        {
            if (blocks[x - 1][y + 1] == 0 && !updated[x - 1][y + 1])
            {
                blocks[x - 1][y + 1] = blocks[x][y];
                blocks[x][y] = 0;

                updated[x - 1][y + 1] = true;
                updated[x][y] = true;

                toBeUpdated = true;
                lastUpdate = 0;
                return true;
            }
        }
        return false;
    }

    bool MoveDownRight(int x, int y)
    {
        if (x + 1 < c_chunkSize && y + 1 < c_chunkSize)
        {
            if (blocks[x + 1][y + 1] == 0 && !updated[x + 1][y + 1])
            {
                blocks[x + 1][y + 1] = blocks[x][y];
                blocks[x][y] = 0;

                updated[x + 1][y + 1] = true;
                updated[x][y] = true;

                toBeUpdated = true;
                lastUpdate = 0;
                return true;
            }
        }
        return false;
    }
    void UpdatePhysics() {
        lastUpdate++;

        for (int x = 0; x < c_chunkSize; x++)
            for (int y = 0; y < c_chunkSize; y++)
                updated[x][y] = false;

        for (int y = c_chunkSize - 1; y >= 0; y--) {
            for (int x = 0; x < c_chunkSize; x++) {

                if (updated[x][y] || blocks[x][y] == 0)
                    continue;

                switch (blocks[x][y]) {

                // Sand
                case 1:
                {
                    if (!MoveDown(x, y)) {
                        if (GetRandomValue(0,1))
                            MoveLeft(x,y);
                        else
                            MoveRight(x,y);
                    }
                    break;
                }

                case 3:
                {
                    if (MoveDown(x, y))
                        break;

                    if (GetRandomValue(0, 1))
                    {
                        if (MoveDownLeft(x, y))
                            break;
                        if (MoveDownRight(x, y))
                            break;
                    }
                    else
                    {
                        if (MoveDownRight(x, y))
                            break;
                        if (MoveDownLeft(x, y))
                            break;
                    }

                    if (GetRandomValue(0, 1))
                    {
                        if (MoveLeft(x, y))
                            break;
                        MoveRight(x, y);
                    }
                    else
                    {
                        if (MoveRight(x, y))
                            break;
                        MoveLeft(x, y);
                    }

                    break;
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
    int chunksX = (c_screenWidth + c_chunkSize - 1) / c_chunkSize;
    int chunksY = (c_screenHeight + c_chunkSize - 1) / c_chunkSize;
    std::vector<Tile> materials;
    std::string trim(const std::string& s) {
        size_t f = s.find_first_not_of(" \t");
        size_t l = s.find_last_not_of(" \t");
        return (f == std::string::npos) ? "" : s.substr(f, (l - f + 1));
    }

    Color parseColor(std::string s) {
        s = s.substr(1, s.size() - 2); // Remove parens
        std::stringstream ss(s);
        std::string val;
        std::vector<int> c;
        while (std::getline(ss, val, ',')) c.push_back(std::stoi(val));
        return {c[0], c[1], c[2], c[3]};
    }
public:
    void loadMaterials(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) return;

        std::string line;
        Tile* current = nullptr;

        while (std::getline(file, line)) {
            if (line.empty()) continue;

            if (line.back() == ':') {
                materials.emplace_back();
                current = &materials.back();
                current->name = line.substr(0, line.size() - 1);
                current->dissolves = -1; 
            } else if (current) {
                size_t sep = line.find('=');
                std::string k = trim(line.substr(0, sep));
                std::string v = trim(line.substr(sep + 1));

                if (k == "weight") current->weight = std::stoi(v);
                else if (k == "falls") current->falls = (v == "true");
                else if (k == "goes_to_sides") current->goesBothWays = (v == "true");
                else if (k == "color") current->color = parseColor(v);
                else if (k == "dissolve" && v != "None") current->dissolves = std::stoi(v);
            }
        }
    }
	App() {
		InitWindow(c_screenWidth, c_screenHeight, "a");
		for (int x = 0; x < chunksX; x++) {
			for (int y = 0; y < chunksY; y++) {
				chunkMap[std::tuple<int, int>{x, y}] = GenCleanChunk();
			}
		}
        loadMaterials("tile_set.txt");
        for (auto t : materials) {
            std::cout<<"color: "<<t.color.r<<" "<<t.color.g<<" "<<t.color.b<<" dissolves:"<<t.dissolves<<" falls:"<<t.falls<<" goesBothWays: "<<t.goesBothWays<<" name:"<<t.name<<" weight:"<<t.weight<<"\n";
        }
        //SetTargetFPS(60);
        
    }
	void Run() {
        int choosen = 0;
		while (!WindowShouldClose()) {
			BeginDrawing();
			ClearBackground(SKYBLUE);
			for (int x = 0; x < chunksX; x++) {
				for (int y = 0; y < chunksY; y++) {
					if (chunkMap[{x,y}].toBeUpdated) {
						chunkMap[{x, y}].UpdateDisplayBuffer();
					}
					if (chunkMap[{x, y}].containsData) {
						chunkMap[{x, y}].Draw(x, y);
                    }

				}
			}

			for (int x = 0; x < chunksX; x++) {
				for (int y = 0; y < chunksY; y++) {
                    if (chunkMap[{x,y}].containsData) {
                        DrawRectangleLines(x*c_chunkSize,y*c_chunkSize,c_chunkSize,c_chunkSize,BLACK);
                        DrawText(TextFormat("%d", chunkMap[{x,y}].lastUpdate),x*c_chunkSize,y*c_chunkSize,16,BLACK);
                    }
                }
            }
           for (int x = 0; x < chunksX; x++) {
                for (int y = 0; y < chunksY; y++) {
                    if (chunkMap[{x,y}].containsData ) {
                        for (int cx = 0; cx < c_chunkSize; cx++) {

                            if (y+1<chunksY) chunkMap[{x,y}].bottomChunkDataCopy[cx] = chunkMap[{x,y+1}].blocks[cx][0];
                            else chunkMap[{x,y}].bottomChunkDataCopy[cx] = 255;
                            if (y-1>=0) chunkMap[{x,y}].topChunkDataCopy[cx] = chunkMap[{x,y-1}].blocks[cx][c_chunkSize-1];
                            else chunkMap[{x,y}].topChunkDataCopy[cx] = 255;
                            if (x-1>=0) chunkMap[{x,y}].leftChunkDataCopy[cx] = chunkMap[{x-1,y}].blocks[c_chunkSize-1][cx];
                            else chunkMap[{x,y}].leftChunkDataCopy[cx] = 255;
                            if (x+1<chunksX) chunkMap[{x,y}].rightChunkDataCopy[cx] = chunkMap[{x+1,y}].blocks[0][cx];
                            else chunkMap[{x,y}].rightChunkDataCopy[cx] = 255;
                            if (chunkMap[{x,y}].moveDown[cx] != 0 && y+1<chunksY) {
                                if (chunkMap[{x,y+1}].blocks[cx][0] == 0) {
                                    chunkMap[{x,y+1}].blocks[cx][0] = chunkMap[{x,y}].moveDown[cx];
                                    chunkMap[{x,y}].moveDown[cx] = 0;
                                    chunkMap[{x,y}].toBeUpdated = true;
                                    chunkMap[{x,y+1}].toBeUpdated = true;
                                    chunkMap[{x,y+1}].containsData = true;
                                    chunkMap[{x,y+1}].lastUpdate = 0;
                                }
                            }

                            if (chunkMap[{x,y}].moveUp[cx] != 0 && y - 1 >= 0) {
                                if (chunkMap[{x,y-1}].blocks[cx][c_chunkSize-1] == 0) {
                                    chunkMap[{x,y-1}].blocks[cx][c_chunkSize-1] = chunkMap[{x,y}].moveUp[cx];
                                    chunkMap[{x,y}].moveUp[cx] = 0;
                                    chunkMap[{x,y}].toBeUpdated = true;
                                    chunkMap[{x,y-1}].toBeUpdated = true;
                                    chunkMap[{x,y-1}].containsData = true;
                                    chunkMap[{x,y-1}].lastUpdate = 0;
                                }
                            }

                            if (chunkMap[{x,y}].moveLeft[cx] != 0 && x - 1 >= 0) {
                                if (chunkMap[{x-1,y}].blocks[c_chunkSize-1][cx] == 0) {
                                    chunkMap[{x-1,y}].blocks[c_chunkSize-1][cx] = chunkMap[{x,y}].moveLeft[cx];
                                    chunkMap[{x,y}].moveLeft[cx] = 0;
                                    chunkMap[{x,y}].toBeUpdated = true;
                                    chunkMap[{x-1,y}].toBeUpdated = true;
                                    chunkMap[{x-1,y}].containsData = true;
                                    chunkMap[{x-1,y}].lastUpdate = 0;
                                }
                            }

                            if (chunkMap[{x,y}].moveRight[cx] != 0 && x + 1 < chunksX) {
                                if (chunkMap[{x+1,y}].blocks[0][cx] == 0) {
                                    chunkMap[{x+1,y}].blocks[0][cx] = chunkMap[{x,y}].moveRight[cx];
                                    chunkMap[{x,y}].moveRight[cx] = 0;
                                    chunkMap[{x,y}].toBeUpdated = true;
                                    chunkMap[{x+1,y}].toBeUpdated = true;
                                    chunkMap[{x+1,y}].containsData = true;
                                    chunkMap[{x+1,y}].lastUpdate = 0;
                                }
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

                        if (updateX < 0 || updateY < 0) continue;
                        int chunkX = updateX / c_chunkSize;
                        int chunkY = updateY / c_chunkSize;
                        if (chunkX < 0 || chunkX >= chunksX || chunkY < 0 || chunkY >= chunksY) continue;

						chunkMap[{chunkX, chunkY}].blocks[updateX % c_chunkSize][updateY % c_chunkSize] = choosen;
						chunkMap[{chunkX, chunkY}].toBeUpdated = true;
                        chunkMap[{chunkX, chunkY}].lastUpdate = 0;
					}
				}
			}
            if (IsKeyDown(KEY_ONE)) {
                choosen = 1;
            }
            if (IsKeyDown(KEY_TWO)) {
                choosen = 2;
            }
            if (IsKeyDown(KEY_THREE)) {
                choosen = 3;
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