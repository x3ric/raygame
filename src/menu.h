#ifndef MENU_H
#define MENU_H

#include "raylib.h"
#include "raymath.h"

int MainMenu() {
    int BUTTON_HEIGHT = 50;
    int BUTTON_WIDTH = 200;
    Vector2 oldPos = {0.0f, 0.0f};
    const char title[] = "Prototype";
    bool ButtonFocused = false;
    Sound fxButton = LoadSound("./res/audio/Button.wav");
    Sound fxButtonHover = LoadSound("./res/audio/ButtonHover.wav");
    Font fontTtf = LoadFontEx("./res/fonts/Monocraft.ttf", 40, 0, 250);
    Vector2 textPosition = { 
        GetScreenWidth()/2 - MeasureTextEx(fontTtf, title, 40, 0).x/2, 
        GetScreenHeight()/2 - MeasureTextEx(fontTtf, title, 40, 0).y/2 - 100
    };
    while(!WindowShouldClose())
    {   
        Vector2 startButtonPos = { (GetScreenWidth() - BUTTON_WIDTH) / 2, (GetScreenHeight() - 2 * BUTTON_HEIGHT) / 2 };
        Vector2 exitButtonPos = { (GetScreenWidth() - BUTTON_WIDTH) / 2, (GetScreenHeight() + BUTTON_HEIGHT) / 2 };
        if ((Vector2Distance(oldPos, GetMousePosition()) != 0.0f)&&(CheckCollisionPointRec(GetMousePosition(), (Rectangle){ startButtonPos.x, startButtonPos.y, BUTTON_WIDTH, BUTTON_HEIGHT }))) {
            if(ButtonFocused) {PlaySound(fxButtonHover);}
            ButtonFocused = false;
            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                PlaySound(fxButton);
                return 1;
            }
        } else if ((Vector2Distance(oldPos, GetMousePosition()) != 0.0f)&&(CheckCollisionPointRec(GetMousePosition(), (Rectangle){ exitButtonPos.x, exitButtonPos.y, BUTTON_WIDTH, BUTTON_HEIGHT }))) {
            if(!ButtonFocused) {PlaySound(fxButtonHover);}
            ButtonFocused = true;
            if(IsMouseButtonPressed(MOUSE_LEFT_BUTTON)){
                PlaySound(fxButton);
                return 0;
            }
        } else if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_S) || IsKeyPressed(KEY_J) || IsKeyPressed(KEY_K)) {
            ButtonFocused = !ButtonFocused;
            PlaySound(fxButtonHover);
        } 
        if (IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE)) {
            PlaySound(fxButton);
            if (!ButtonFocused) {
                return 1;
            } else {
                return 0;
            }
        }
        oldPos = GetMousePosition();
        BeginDrawing();
            ClearBackground(BLACK);
            DrawTextEx(fontTtf, title, textPosition, (float)fontTtf.baseSize, 2, LIGHTGRAY);
            DrawRectangle(startButtonPos.x, startButtonPos.y, BUTTON_WIDTH, BUTTON_HEIGHT, ButtonFocused ? DARKGRAY : LIGHTGRAY);
            DrawRectangle(exitButtonPos.x, exitButtonPos.y, BUTTON_WIDTH, BUTTON_HEIGHT, !ButtonFocused ? DARKGRAY : LIGHTGRAY);
            DrawText("Start", startButtonPos.x + BUTTON_WIDTH / 4, startButtonPos.y + BUTTON_HEIGHT / 4, 20, BLACK);
            DrawText("Exit", exitButtonPos.x + BUTTON_WIDTH / 4, exitButtonPos.y + BUTTON_HEIGHT / 4, 20, BLACK);
        EndDrawing();  
    }
    CloseWindow();
    UnloadSound(fxButtonHover);
    UnloadSound(fxButton);
    return 0;
}

int Boot()
{
    int framesCounter = 0;
    int lettersCount = 0;
    int state = 0;
    float alpha = 1.0f;
    int topSideRecWidth = 16;
    int leftSideRecHeight = 16;
    int bottomSideRecWidth = 16;
    int rightSideRecHeight = 16;
    Sound BootSfx = LoadSound("./res/audio/Boot.wav");
    PlaySound(BootSfx);
    while(!WindowShouldClose()) {   
        int logoPositionX = GetScreenWidth()/2 - 128;
        int logoPositionY = GetScreenHeight()/2 - 128;
        float deltaTime = GetFrameTime();
        if (state == 0)
        {
            framesCounter += deltaTime;
            if (framesCounter == 1 / 120)
            {
                state = 1;
                framesCounter = 0;
            }
        }
        else if (state == 1)
        {
            topSideRecWidth += 4;
            leftSideRecHeight += 4;
            if (topSideRecWidth == 256) state = 2;
        }
        else if (state == 2)
        {
            bottomSideRecWidth += 4;
            rightSideRecHeight += 4;
            if (bottomSideRecWidth == 256) state = 3;
        }
        else if (state == 3)
        {
            framesCounter += deltaTime;
            if (framesCounter == 1 / 12)
            {
                lettersCount++;
                framesCounter = 0;
            }
            if (lettersCount >= 10)
            {
                alpha -= 0.02f;
                if (alpha <= 0.0f)
                {
                    alpha = 0.0f;
                    UnloadSound(BootSfx);
                    return 0;
                }
            }
        }
        BeginDrawing();
            ClearBackground(BLACK);
            if (state == 0)
            {
                if ((framesCounter/15)%2) DrawRectangle(logoPositionX, logoPositionY, 16, 16, WHITE);
            }
            else if (state == 1)
            {
                DrawRectangle(logoPositionX, logoPositionY, topSideRecWidth, 16, WHITE);
                DrawRectangle(logoPositionX, logoPositionY, 16, leftSideRecHeight, WHITE);
            }
            else if (state == 2)
            {
                DrawRectangle(logoPositionX, logoPositionY, topSideRecWidth, 16, WHITE);
                DrawRectangle(logoPositionX, logoPositionY, 16, leftSideRecHeight, WHITE);
                DrawRectangle(logoPositionX + 240, logoPositionY, 16, rightSideRecHeight, WHITE);
                DrawRectangle(logoPositionX, logoPositionY + 240, bottomSideRecWidth, 16, WHITE);
            }
            else if (state == 3)
            {
                DrawRectangle(logoPositionX, logoPositionY, topSideRecWidth, 16, Fade(WHITE, alpha));
                DrawRectangle(logoPositionX, logoPositionY + 16, 16, leftSideRecHeight - 32, Fade(WHITE, alpha));
                DrawRectangle(logoPositionX + 240, logoPositionY + 16, 16, rightSideRecHeight - 32, Fade(WHITE, alpha));
                DrawRectangle(logoPositionX, logoPositionY + 240, bottomSideRecWidth, 16, Fade(WHITE, alpha));
                DrawRectangle(GetScreenWidth()/2 - 112, GetScreenHeight()/2 - 112, 224, 224, Fade(BLACK, alpha));
                DrawText(TextSubtext("raylib", 0, lettersCount), GetScreenWidth()/2 - 44, GetScreenHeight()/2 + 48, 50, Fade(WHITE, alpha));
            }
        EndDrawing();
    }
    CloseWindow();
    UnloadSound(BootSfx);
    return 0;
}

#endif // MENU_H