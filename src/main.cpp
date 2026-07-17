#include "raylib.h"
#include <cinttypes>
#include <map>
const int c_chunkSize = 64;
struct Chunk {
	uint8_t blocks[c_chunkSize][c_chunkSize];
	Image image;
	Texture texture;
	bool toBeUpdated = false;
	bool containsData = false;
	void Draw(int x, int y) {
		DrawTexture(texture, x * c_chunkSize, y * c_chunkSize, WHITE);
	}
	void UpdateDisplayBuffer() {
		containsData = false;
		const Color colorMap[10] = { SKYBLUE,YELLOW,GRAY,BLUE };
		for (int x = 0; x < c_chunkSize; x++) {
			for (int y = 0; y < c_chunkSize; y++) {
				if (blocks[x][y] != 0) {
					containsData = true;
				}
				ImageDrawPixel(&image, x, y, colorMap[blocks[x][y]]);
			}
		}
		UpdateTexture(texture, image.data);
		toBeUpdated = false;
	}
};
Chunk GenCleanChunk() {
	Chunk chunk;
	for (int x = 0; x < c_chunkSize; x++) for (int y = 0; y < c_chunkSize; y++) chunk.blocks[x][y] = 0;
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
			if (IsMouseButtonDown(0)) {
				for (int x = 0; x < editSize; x++) {
					for (int y = 0; y < editSize; y++) {
						int updateX = x + GetMouseX();
						int updateY = y + GetMouseY();
						chunkMap[{updateX / c_chunkSize, updateY / c_chunkSize}].blocks[updateX % c_chunkSize][updateY % c_chunkSize] = 1;
						chunkMap[{updateX / c_chunkSize, updateY / c_chunkSize}].toBeUpdated = true;
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