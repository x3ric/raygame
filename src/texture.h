#ifndef TEXTURE_H
#define TEXTURE_H

#include "raylib.h"
#include <stdlib.h>

Texture2D checked;

Texture2D CheckedTexture() {
    const int TILE_SIZE = 16;
    int width = TILE_SIZE * 16;
    int height = TILE_SIZE * 16;
    Color *pixels = (Color *)malloc(width*height*sizeof(Color));
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            if (((x/TILE_SIZE+y/TILE_SIZE)/1)%2 == 0) pixels[y*width + x] = PURPLE;
            //else pixels[y*width + x] = PURPLE;
            else pixels[y*width + x] = (Color){0.0,0.0,0.0};
        }
    }
    Image checkedIm = {
        .data = pixels,
        .width = width,
        .height = height,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8,
        .mipmaps = 1
    };
    Texture2D checked = LoadTextureFromImage(checkedIm);
    UnloadImage(checkedIm);
    return checked;
}

void DrawCenteredTiledTexture(Texture2D texture, Camera2D camera) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int centerX = 0;
    int centerY = 0;
    int startX = centerX - ((int)camera.target.x % texture.width);
    int startY = centerY - ((int)camera.target.y % texture.height);
    startX -= texture.width;
    startY -= texture.height;
    for (int y = startY; y < screenHeight + texture.height; y += texture.height) {
        for (int x = startX; x < screenWidth + texture.width; x += texture.width) {
            Rectangle sourceRec = { 0, 0, texture.width, texture.height};
            // Left edge
            if (x < 0) {
                sourceRec.x = -x;
                sourceRec.width += x;
            }
            // Top edge
            if (y < 0) {
                sourceRec.y = -y;
                sourceRec.height += y;
            }
            // Right edge
            if (x + texture.width > screenWidth) {
                int overflowX = (x + texture.width) - screenWidth;
                sourceRec.width -= overflowX;
            }
            // Bottom edge
            if (y + texture.height > screenHeight) {
                int overflowY = (y + texture.height) - screenHeight;
                sourceRec.height -= overflowY;
            }
            Rectangle destRec = { x + sourceRec.x, y + sourceRec.y, sourceRec.width, sourceRec.height };
            DrawTexturePro(texture, sourceRec, destRec, (Vector2){0, 0}, 0.0f, Fade(DARKGRAY, 0.5f));
        }
    }
}

void DrawCenteredTexture(Texture2D texture) {
    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();
    int startX = (screenWidth / 2) % texture.width - texture.width;
    int startY = (screenHeight / 2) % texture.height - texture.height;
    for (int y = startY; y < screenHeight; y += texture.height) {
        for (int x = startX; x < screenWidth; x += texture.width) {
            DrawTexture(texture, x, y, Fade(DARKGRAY, 0.5f));
        }
    }
}

void CheckedInit() {
    checked = CheckedTexture();
}

void UnloadChecked() {
    UnloadTexture(checked);
}

#endif // TEXTURE_H
