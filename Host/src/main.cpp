#include "raylib.h"
#include <iostream>
#include <vector>

// POSIX Socket headers
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

struct GamePacket {
    float x;
    float y;
};

struct EnvironmentItem {
    Rectangle rect;
    int blocking;
    Color color;
};

int main() {
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "DropCast Host - Physics & Network Controller");
    SetTargetFPS(60);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return -1;
    
    struct sockaddr_in clientAddr;
    memset(&clientAddr, 0, sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(4444);
    clientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    Vector2 playerPos = { 400.0f, 280.0f };
    float velocityY = 0.0f;
    bool canJump = false;
    
    const float gravity = 800.0f; // Increased gravity for snappier jumping
    const float playerJumpStrength = 450.0f;
    const float playerHorizSpeed = 300.0f;

    std::vector<EnvironmentItem> envItems = {
        {{ -500, 400, 2000, 200 }, 1, GRAY},
        {{ 300, 200, 400, 10 }, 1, DARKGRAY },
        {{ 250, 300, 100, 10 }, 1, DARKGRAY },
        {{ 650, 300, 100, 10 }, 1, DARKGRAY },
        {{ 850, 250, 150, 10 }, 1, DARKGRAY }
    };

    // Mobile Touch Button Layout
    Rectangle btnLeft = { 30, 320, 80, 80 };
    Rectangle btnRight = { 130, 320, 80, 80 };
    Rectangle btnJump = { screenWidth - 110, 320, 80, 80 };

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        // --- INPUT HANDLING (Keyboard + Touch) ---
        bool moveLeft = IsKeyDown(KEY_LEFT);
        bool moveRight = IsKeyDown(KEY_RIGHT);
        bool jump = IsKeyPressed(KEY_SPACE); // Fixed: IsKeyPressed instead of IsKeyDown

        // Check all potential simultaneous touch points (up to 10 fingers)
        for (int i = 0; i < GetTouchPointCount(); i++) {
            Vector2 touchPos = GetTouchPosition(i);
            if (CheckCollisionPointRec(touchPos, btnLeft)) moveLeft = true;
            if (CheckCollisionPointRec(touchPos, btnRight)) moveRight = true;
            
            // For touch, we allow holding the button to jump as soon as you touch the ground
            if (CheckCollisionPointRec(touchPos, btnJump)) jump = true; 
        }
        // Also allow mouse clicks to simulate touch for testing on PC
        if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
            Vector2 mousePos = GetMousePosition();
            if (CheckCollisionPointRec(mousePos, btnLeft)) moveLeft = true;
            if (CheckCollisionPointRec(mousePos, btnRight)) moveRight = true;
            if (CheckCollisionPointRec(mousePos, btnJump)) jump = true;
        }

        // --- UPDATE LOGIC ---
        if (moveLeft) playerPos.x -= playerHorizSpeed * deltaTime;
        if (moveRight) playerPos.x += playerHorizSpeed * deltaTime;
        if (jump && canJump) {
            velocityY = -playerJumpStrength;
            canJump = false;
        }

        playerPos.y += velocityY * deltaTime;
        velocityY += gravity * deltaTime;

        bool hitObstacle = false;
        for (auto& item : envItems) {
            if (item.blocking &&
                playerPos.x >= item.rect.x - 20 &&
                playerPos.x <= item.rect.x + item.rect.width + 20 &&
                playerPos.y >= item.rect.y - 40 &&
                playerPos.y <= item.rect.y) 
            {
                hitObstacle = true;
                velocityY = 0.0f;
                playerPos.y = item.rect.y;
            }
        }
        canJump = hitObstacle;

        // --- NETWORK ---
        GamePacket packet = { playerPos.x, playerPos.y };
        sendto(sock, &packet, sizeof(GamePacket), 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));

        // --- DRAW ---
        BeginDrawing();
        ClearBackground(RAYWHITE);

        for (auto& item : envItems) DrawRectangleRec(item.rect, item.color);

        Rectangle playerRect = { playerPos.x - 20, playerPos.y - 40, 40, 40 };
        DrawRectangleRec(playerRect, MAROON);

        // Draw Touch Buttons (Semi-transparent)
        DrawRectangleRec(btnLeft, Fade(DARKGRAY, 0.5f));
        DrawText("<", btnLeft.x + 30, btnLeft.y + 25, 40, WHITE);

        DrawRectangleRec(btnRight, Fade(DARKGRAY, 0.5f));
        DrawText(">", btnRight.x + 30, btnRight.y + 25, 40, WHITE);

        DrawRectangleRec(btnJump, Fade(MAROON, 0.5f));
        DrawText("^", btnJump.x + 30, btnJump.y + 30, 40, WHITE);

        DrawText("HOST APP: Touch or Click the buttons to move!", 10, 10, 20, DARKGRAY);
        DrawFPS(screenWidth - 80, 10);
        EndDrawing();
    }

    close(sock);
    CloseWindow();
    return 0;
}