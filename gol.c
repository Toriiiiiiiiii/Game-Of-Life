#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <raylib.h>

#define CELL_ALIVE  '#'
#define CELL_DEAD   '.'

static char *buffer1;
static int GRID_WIDTH = 25;
static int GRID_HEIGHT = 25;

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
    
    return 0;
}

int isAlive(int x, int y) {
    if(buffer1[y * GRID_WIDTH + x] == CELL_ALIVE) return 1;
    return 0;
}

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

void renderBuffer() {
    for(int x = 0; x < GRID_WIDTH; ++x) {
        for(int y = 0; y < GRID_HEIGHT; ++y) {
            int index = y * GRID_WIDTH + x;

            if(buffer1[index] == CELL_ALIVE) {
                DrawRectangle(x * 8, y * 8, 8, 8, WHITE);
            }
        }
    }
}

void printBuffer() {
    system("clear");
    
    for(int index = 0; index < GRID_WIDTH * GRID_HEIGHT; ++index) {
        if(index != 0 && index % GRID_WIDTH == 0) printf("\n");
        printf("%c ", buffer1[index]);
    }

    printf("\n");
}

int main(int argc, char **argv) {
    int loadResult = loadBufferFromFile(argv[1]);

    if(loadResult) return 1;

    InitWindow(8 * GRID_WIDTH, 8 * GRID_HEIGHT, "Game of Life");
    SetTargetFPS(10);
    
    while(!WindowShouldClose()) {
        BeginDrawing();
        ClearBackground(BLACK);
        
        //printBuffer();
        renderBuffer();
        
        EndDrawing();
        updateBuffer();
        //getc(stdin);
    }

    CloseWindow();
    return 0;
}
