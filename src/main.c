#include "menu.h"
#include "game.h"

int main()
{
    InitWindow(1920, 1080, "raylib game");
    SetTargetFPS(60);
    InitAudioDevice();
    goto start; //skip to game
    Boot();
    switch(MainMenu()) {
        case 0:
            CloseWindow();
            return 0;
        case 1:
            start:
            Game();
            break;
    } 
}
