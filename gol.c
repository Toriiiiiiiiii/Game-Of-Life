#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>
#include <sys/time.h>

#define CELL_ALIVE  '#'
#define CELL_DEAD   '.'
#define CELL_SIZE   16

static char *buffer1;
static int GRID_WIDTH = 25;
static int GRID_HEIGHT = 25;

// Print the value of the buffer to a file in a grid format
void printBuffer(FILE *f) {    
    for(int index = 0; index < GRID_WIDTH * GRID_HEIGHT; ++index) {
        if(index != 0 && index % GRID_WIDTH == 0) fprintf(f, "\n");
        fprintf(f, "%c", buffer1[index]);
    }

    fprintf(f, "\n");
}

// Load state from file
// Expected in format:
//
// +--------------------------------+
// | WIDTH HEIGHT                   |
// | .......#..... --               |
// | ........#.... --               |
// | ......###.... --               |
// | ............. --               |
// | --                             |
// +--------------------------------+
//
// Returns 0 on success, 1 on failure
int loadBufferFromFile(char *path) {
    FILE *file = fopen(path, "r");
    if(!file) return 1;

    fscanf(file, "%d %d\n", &GRID_WIDTH, &GRID_HEIGHT);    
    buffer1 = (char *)malloc(GRID_WIDTH * GRID_HEIGHT);

    for(int i = 0; i < GRID_WIDTH * GRID_HEIGHT; ++i) buffer1[i] = CELL_DEAD;  

    for(int lineIndex = 0; lineIndex < GRID_HEIGHT; ++lineIndex) {
        char *ptr = buffer1 + (lineIndex * GRID_WIDTH);
        fscanf(file, "%s\n", ptr);
    }

    fclose(file);
    return 0;
}

// Writes the buffer to a file.
int saveBufferToFile(char *path) {
    FILE *file = fopen(path, "w");
    if(!file) return 1;

    fprintf(file, "%d %d\n", GRID_WIDTH, GRID_HEIGHT);
    printBuffer(file);

    printf("Wrote file contents to '%s'.\n", path);

    fclose(file);
    return 0;
}

// Simply determines if cell at (x, y) is alive.
int isAlive(int x, int y) {
    if(buffer1[y * GRID_WIDTH + x] == CELL_ALIVE) return 1;
    return 0;
}

// Counts the number of living neighbours of cell at (x, y)
int countNeighbours(int x, int y) {
    int n = 0;
    
    for(int nx = -1; nx < 2; ++nx) {
        for(int ny = -1; ny < 2; ++ny) {
            if(nx == 0 && ny == 0) continue;

            int cx = (x + nx);
            int cy = (y + ny);

            if(cx < 0) cx += GRID_WIDTH;
            if(cy < 0) cy += GRID_HEIGHT;
            if(cx >= GRID_WIDTH) cx -= GRID_WIDTH;
            if(cy >= GRID_HEIGHT) cy -= GRID_HEIGHT;

            int index = cy * GRID_WIDTH + cx;
            if(buffer1[index] == CELL_ALIVE) n++;
        }
    }

    return n;
}

// Performs an iteration of CGOL on the buffer
void updateBuffer() {
    char *tempBuffer = (char *)malloc(GRID_WIDTH * GRID_HEIGHT);
    
    for(int x = 0; x < GRID_WIDTH; ++x) {
        for(int y = 0; y < GRID_HEIGHT; ++y) {
            int neighbours = countNeighbours(x, y);

            if(neighbours < 2) tempBuffer[y * GRID_WIDTH + x] = CELL_DEAD;
            else if(neighbours > 3) tempBuffer[y * GRID_WIDTH + x] = CELL_DEAD;
            else if(!isAlive(x, y) && neighbours == 3) tempBuffer[y * GRID_WIDTH + x] = CELL_ALIVE;
            else if(isAlive(x, y)) tempBuffer[y * GRID_WIDTH + x] = CELL_ALIVE;
            else tempBuffer[y * GRID_WIDTH + x] = CELL_DEAD;
        }
    }

    memcpy(buffer1, tempBuffer, GRID_WIDTH * GRID_HEIGHT);
    free(tempBuffer);
}

// Renders the buffer to a raylib window
void renderBuffer() {
    for(int x = 0; x < GRID_WIDTH; ++x) {
        for(int y = 0; y < GRID_HEIGHT; ++y) {
            int index = y * GRID_WIDTH + x;

            if(buffer1[index] == CELL_ALIVE) {
                DrawRectangle(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, WHITE);
            } 
        }
    }
}

// Sets all cells in the buffer to dead.
// Should only be called if the buffer has not 
// yet been created.
void initBuffer() {
    GRID_WIDTH = 1000;
    GRID_HEIGHT = 1000;
    buffer1 = (char *)malloc(GRID_WIDTH * GRID_HEIGHT);

    for(int i = 0; i < GRID_WIDTH * GRID_HEIGHT; ++i) buffer1[i] = CELL_DEAD;
}

double milliseconds() {
    struct timeval time;
    gettimeofday(&time, NULL);

    return (double)time.tv_sec * 1000 + (double)time.tv_usec / 1000;
}

int main(int argc, char **argv) {
    if(argc >= 2) {
        int loadResult = loadBufferFromFile(argv[1]);
        if(loadResult) return 1;
    } else {
        initBuffer();
    }
    
    InitWindow(CELL_SIZE * 50, CELL_SIZE * 50, "Game of Life");
    SetTargetFPS(60);

    double targetSimMillis = 1000 * (1.0 / 15.0);

    Camera2D cam = {0};
    cam.zoom = 1;
    cam.target = (Vector2){500 * CELL_SIZE, 500 * CELL_SIZE};
    cam.offset = (Vector2){CELL_SIZE * 25, CELL_SIZE * 25};

    int simRunning = 0;
    char *preBuffer = (char*)malloc(GRID_WIDTH * GRID_HEIGHT);
    
    double start = 0, end = 0, next = 0;

    while(!WindowShouldClose()) {
        BeginDrawing();
        BeginMode2D(cam);
        ClearBackground(GRAY);
        DrawRectangle(0, 0, GRID_WIDTH * CELL_SIZE, GRID_HEIGHT * CELL_SIZE, BLACK); 

        renderBuffer();
        if(!simRunning) {
            Vector2 pos = GetMousePosition();
            int x = ((pos.x - cam.offset.x) / cam.zoom + cam.target.x) / CELL_SIZE;
            int y = ((pos.y - cam.offset.y) / cam.zoom + cam.target.y) / CELL_SIZE;
            DrawRectangleLines(x * CELL_SIZE, y * CELL_SIZE, CELL_SIZE, CELL_SIZE, RED);
        }

        EndMode2D();
        EndDrawing();

        if(IsKeyDown(KEY_LEFT)) {
            cam.target.x -= 10;
        } if(IsKeyDown(KEY_RIGHT)) {
            cam.target.x += 10;
        } if(IsKeyDown(KEY_UP)) {
            cam.target.y -= 10;
        } if(IsKeyDown(KEY_DOWN)) {
            cam.target.y += 10;
        } if(IsKeyDown(KEY_MINUS)) {
            cam.zoom -= 0.01;
            if(cam.zoom <= 0) cam.zoom = 0.0001;
        } if(IsKeyDown(KEY_EQUAL)) {
            cam.zoom += 0.01;
        }

        if(IsKeyPressed(KEY_SPACE) && !simRunning) {
            memcpy(preBuffer, buffer1, GRID_WIDTH * GRID_HEIGHT);
            simRunning = 1;
        } else if(IsKeyPressed(KEY_SPACE)) {
            memcpy(buffer1, preBuffer, GRID_WIDTH * GRID_HEIGHT);
            simRunning = 0;
        }

        if(IsKeyPressed(KEY_S) && !simRunning) {
            if(argc < 2) saveBufferToFile("out.txt");
            else saveBufferToFile(argv[1]);
        }

        if(IsMouseButtonDown(MOUSE_LEFT_BUTTON) && !simRunning) {
            Vector2 pos = GetMousePosition();

            int x = ((pos.x - cam.offset.x) / cam.zoom + cam.target.x) / CELL_SIZE;
            int y = ((pos.y - cam.offset.y) / cam.zoom + cam.target.y) / CELL_SIZE;

            if(x < 0 || y < 0) continue;
            if(x >= GRID_WIDTH || y >= GRID_HEIGHT) continue;

            int index = y * GRID_WIDTH + x;
            buffer1[index] = CELL_ALIVE;
        } if(IsMouseButtonDown(MOUSE_RIGHT_BUTTON) && !simRunning) {
            Vector2 pos = GetMousePosition();

            int x = ((pos.x - cam.offset.x) / cam.zoom + cam.target.x) / CELL_SIZE;
            int y = ((pos.y - cam.offset.y) / cam.zoom + cam.target.y) / CELL_SIZE;

            if(x < 0 || y < 0) continue;
            if(x >= GRID_WIDTH || y >= GRID_HEIGHT) continue;

            int index = y * GRID_WIDTH + x;
            buffer1[index] = CELL_DEAD;
        }
        
        if(end < next) end = milliseconds();
        else {
            start = milliseconds();
            next = start + targetSimMillis;
            if(simRunning) updateBuffer();
            end = milliseconds();
        }
    }

    CloseWindow();
    return 0;
}
