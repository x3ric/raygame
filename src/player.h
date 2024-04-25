#ifndef PLAYER_H
#define PLAYER_H

#include "raylib.h"
#include "raymath.h"
#include <stdlib.h>

const char *animPaths[] = {
    "./res/sprites/player/idle.gif",
    "./res/sprites/player/idle.gif",
    "./res/sprites/player/idle-down.gif",
    "./res/sprites/player/idle-up.gif",
    "./res/sprites/player/move.gif",
    "./res/sprites/player/move.gif",
    "./res/sprites/player/move-down.gif",
    "./res/sprites/player/move-up.gif"
};

typedef enum {
    ANIM_IDLE_LEFT,
    ANIM_IDLE_RIGHT,
    ANIM_IDLE_DOWN,
    ANIM_IDLE_UP,
    ANIM_MOVE_LEFT,
    ANIM_MOVE_RIGHT,
    ANIM_MOVE_DOWN,
    ANIM_MOVE_UP
} Animation;

const char *animNames[] = {
    "IDLE_LEFT",
    "IDLE_RIGHT",
    "IDLE_DOWN",
    "IDLE_UP",
    "MOVE_LEFT",
    "MOVE_RIGHT",
    "MOVE_DOWN",
    "MOVE_UP"
};

int animFrames;
unsigned int nextFrameDataOffset;
int currentAnimFrame = 0;
float frameCounter = 0.0f; 
float frameDuration = 0.25f;
Vector2 PlayerPosition = (Vector2){ 0, 0 };
Animation currentAnim = ANIM_IDLE_LEFT;  
Animation lastIdleAnim = ANIM_IDLE_LEFT;
Animation prevAnim = ANIM_IDLE_LEFT;
bool isFlipped = false;
float speed = 135.0f;
float speedmult = 1.55f;
float rumblespeed = 0.0f;
float maxrumblespeed = 1.5f;
float rotation = 0.0f;
Image imPlayer;
Texture2D texPlayer;
float deltaTime;

void LoadPlayerAnimation() {
    if (imPlayer.data) UnloadImage(imPlayer);
    if (texPlayer.id) UnloadTexture(texPlayer);
    imPlayer = LoadImageAnim(animPaths[currentAnim], &animFrames);
    texPlayer = LoadTextureFromImage(imPlayer);
}

void Rumble() {
    if ((speed + rumblespeed) >= 500.0f) {
        rumblespeed += (float)rand() / (float)(RAND_MAX / (maxrumblespeed));
        rumblespeed -= 0.5f;
    } else {
        rumblespeed += (float)rand() / (float)(RAND_MAX / (maxrumblespeed));
    }  
}

void Player() {
    Rectangle sourceRec = (Rectangle){
        isFlipped ? texPlayer.width : 0.0f,
        0.0f,
        isFlipped ? -texPlayer.width : texPlayer.width,
        texPlayer.height
    };
    Rectangle destRec = (Rectangle){
        PlayerPosition.x ,
        PlayerPosition.y ,
        texPlayer.width * 4.0f,
        texPlayer.height * 4.0f
    };
    Vector2 origin = { destRec.width * 0.5f, destRec.height * 0.5f };
    DrawTexturePro(texPlayer, sourceRec, destRec, origin, rotation, WHITE);
}

void PlayerLoop() {
        deltaTime = GetFrameTime();
        frameCounter += deltaTime;
        if (frameCounter >= frameDuration) {
            currentAnimFrame++;
            if (currentAnimFrame >= animFrames) currentAnimFrame = 0;
            nextFrameDataOffset = imPlayer.width * imPlayer.height * 4 * currentAnimFrame;
            UpdateTexture(texPlayer, ((unsigned char *)imPlayer.data) + nextFrameDataOffset);
            frameCounter = 0.0f;
        }
        prevAnim = currentAnim;
        // Horizontal Movement
        if (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D)) {
            if ((IsKeyDown(KEY_LEFT_SHIFT))&&(prevAnim == ANIM_MOVE_UP||prevAnim == ANIM_MOVE_DOWN)&&(IsKeyDown(KEY_UP)||IsKeyDown(KEY_W)||IsKeyDown(KEY_DOWN)||IsKeyDown(KEY_S))) {Rumble();}
            PlayerPosition.x += (speed + rumblespeed) * deltaTime;
            currentAnim = ANIM_MOVE_RIGHT;
            lastIdleAnim = ANIM_IDLE_RIGHT;
            isFlipped = false;
        } else if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A)) {
            if ((IsKeyDown(KEY_LEFT_SHIFT))&&(prevAnim == ANIM_MOVE_UP||prevAnim == ANIM_MOVE_DOWN)&&(IsKeyDown(KEY_UP)||IsKeyDown(KEY_W)||IsKeyDown(KEY_DOWN)||IsKeyDown(KEY_S))) {Rumble();}
            PlayerPosition.x -= (speed + rumblespeed) * deltaTime;
            currentAnim = ANIM_MOVE_LEFT;
            lastIdleAnim = ANIM_IDLE_LEFT;
            isFlipped = true;
        } else if (IsKeyReleased(KEY_RIGHT) || IsKeyReleased(KEY_D) || IsKeyReleased(KEY_LEFT) || IsKeyReleased(KEY_A)) {
            rumblespeed = 0.0f;
            currentAnim = lastIdleAnim;
        }
        // Vertical Movement
        if (IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) {
            if ((IsKeyDown(KEY_LEFT_SHIFT))&&(prevAnim == ANIM_MOVE_LEFT||prevAnim == ANIM_MOVE_RIGHT)&&(IsKeyPressed(KEY_RIGHT)||IsKeyPressed(KEY_D)||IsKeyPressed(KEY_LEFT)||IsKeyPressed(KEY_A))) {Rumble();}
            PlayerPosition.y -= (speed + rumblespeed) * deltaTime;
            currentAnim = ANIM_MOVE_UP;
            lastIdleAnim = ANIM_IDLE_UP;
        } else if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) {
            if ((IsKeyDown(KEY_LEFT_SHIFT))&&(prevAnim == ANIM_MOVE_LEFT||prevAnim == ANIM_MOVE_RIGHT)&&(IsKeyPressed(KEY_RIGHT)||IsKeyPressed(KEY_D)||IsKeyPressed(KEY_LEFT)||IsKeyPressed(KEY_A))) {Rumble();}
            PlayerPosition.y += (speed + rumblespeed) * deltaTime;
            currentAnim = ANIM_MOVE_DOWN;
            lastIdleAnim = ANIM_IDLE_DOWN;
        } else if (IsKeyReleased(KEY_UP) || IsKeyReleased(KEY_W) || IsKeyReleased(KEY_DOWN) || IsKeyReleased(KEY_S)) {
            rumblespeed = 0.0f;
            currentAnim = lastIdleAnim;
        }
        // Animation
        if (prevAnim != currentAnim) {
            LoadPlayerAnimation();
            currentAnimFrame = 0;
            frameCounter = 0.0f;
        }
        // Diagonal Movement
        float siderotation = 10.0f;
        const float targetRotation = 
            ((IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) && (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))) ? siderotation :
            ((IsKeyDown(KEY_UP) || IsKeyDown(KEY_W)) && (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))) ? -siderotation :
            ((IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) && (IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_D))) ? -siderotation :
            ((IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_S)) && (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_A))) ? siderotation :
            0.0f ;
        // Interpolating rotation
        const float rotationSpeed = 180.0f;
        const float maxRotationChange = rotationSpeed * deltaTime;
        if (targetRotation > rotation) {
            rotation = fmin(rotation + maxRotationChange, targetRotation);
        } else if (targetRotation < rotation) {
            rotation = fmax(rotation - maxRotationChange, targetRotation);
        }
        // Diagonal Rumble fix
        if( ((IsKeyDown(KEY_UP)||IsKeyDown(KEY_W))||(IsKeyDown(KEY_DOWN)||IsKeyDown(KEY_S))) && ((IsKeyDown(KEY_RIGHT)||IsKeyDown(KEY_D)||(IsKeyDown(KEY_LEFT)||IsKeyDown(KEY_A))))){
            rumblespeed -= 0.65f;
        }
        // Sprint
        if (IsKeyPressed(KEY_LEFT_SHIFT)) {
            speed = (speed + rumblespeed) * speedmult;
            frameDuration = frameDuration / 2;
        } else if (IsKeyReleased(KEY_LEFT_SHIFT)) {
            speed = (speed + rumblespeed) / speedmult;
            frameDuration = frameDuration * 2;
            rumblespeed = 0.0f;
        }
}

void PlayerInit() {
    Vector2 PlayerPosition = (Vector2){ GetScreenWidth()/2.0f, GetScreenHeight()/2.0f };
    LoadPlayerAnimation();
}

void UnloadPlayer() {
    UnloadTexture(texPlayer);
    UnloadImage(imPlayer);
}

void PlayerInfo() {
    DrawText(TextFormat("X: %.0f, Y: %.0f", PlayerPosition.x, PlayerPosition.y),10, GetScreenHeight() - 20, 10, WHITE);
    DrawText(TextFormat("Anim State: %s", animNames[currentAnim]),10, GetScreenHeight() - 40, 10, WHITE);
    DrawText(TextFormat("Speed: %.0f", speed + rumblespeed),10, GetScreenHeight() - 60 , 10, WHITE);
    DrawFPS(10, 10);
}

#endif // PLAYER_H