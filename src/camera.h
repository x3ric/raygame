#ifndef CAMERA_H
#define CAMERA_H

#include "raylib.h"
#include <math.h>

float cameraLerpFactor =  1.25f;
Camera2D camera = { 0 };

void CameraInit() {
    float midx = GetScreenWidth()/2.0f;
    float midy = GetScreenHeight()/2.0f;
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;
    camera.offset = (Vector2){ midx, midy };
    camera.target = (Vector2){ 0, 0 };
}

void CameraLoop(Vector2 PlayerPosition,float deltaTime) {
    camera.target.x += (PlayerPosition.x - camera.target.x) * cameraLerpFactor * deltaTime;
    camera.target.y += (PlayerPosition.y - camera.target.y) * cameraLerpFactor * deltaTime;
}

#endif // CAMERA_H