#ifndef GAME_H
#define GAME_H

#include "camera.h"
#include "player.h"
#include "texture.h"

int Game() {
    CameraInit();
    CheckedInit();
    PlayerInit();
    while(!WindowShouldClose()) {    
        PlayerLoop();
        CameraLoop(PlayerPosition,deltaTime);
        BeginDrawing();
            ClearBackground(BLACK);
            DrawCenteredTiledTexture(checked,camera);
            BeginMode2D(camera);
                Player();
            EndMode2D();
            PlayerInfo();
        EndDrawing();
    }
    UnloadChecked();
    UnloadPlayer();
    CloseWindow();
    return 0;
}

#endif // GAME_H