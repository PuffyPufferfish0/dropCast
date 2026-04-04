#include "raylib.h"
#include <iostream>
#include <vector>
#include <string>

// POSIX Socket headers
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

struct GamePacket {
    float x;
    float y;
    float velX;
    float velY;
};

struct EnvironmentItem {
    Rectangle rect;
    int blocking;
    Color color;
};

// Helper struct for our on-screen Numpad
struct Button {
    Rectangle rect;
    const char* text;
    char val;
};

int main() {
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "DropCast Host - Physics & Network Controller");
    SetTargetFPS(60);

    // --- APP STATES ---
    bool inMenu = true;
    std::string ipAddress = "192.168."; // Pre-filled for convenience!
    
    // --- NETWORK SETUP (Deferred until Connect is pressed) ---
    int sock = -1;
    struct sockaddr_in clientAddr;
    memset(&clientAddr, 0, sizeof(clientAddr));

    // --- GAME STATE ---
    Vector2 playerPos = { 400.0f, 280.0f };
    float velocityY = 0.0f;
    bool canJump = false;
    
    const float gravity = 800.0f;
    const float playerJumpStrength = 450.0f;
    const float playerHorizSpeed = 300.0f;

    std::vector<EnvironmentItem> envItems = {
        {{ -500, 400, 2000, 200 }, 1, GRAY},
        {{ 300, 200, 400, 10 }, 1, DARKGRAY },
        {{ 250, 300, 100, 10 }, 1, DARKGRAY },
        {{ 650, 300, 100, 10 }, 1, DARKGRAY },
        {{ 850, 250, 150, 10 }, 1, DARKGRAY }
    };

    // --- UI LAYOUTS ---
    // Gameplay Mobile Controls
    Rectangle btnLeft = { 30, 320, 80, 80 };
    Rectangle btnRight = { 130, 320, 80, 80 };
    Rectangle btnJump = { screenWidth - 110, 320, 80, 80 };

    // IP Input Numpad Layout
    std::vector<Button> numpad = {
        {{ 250, 120, 80, 60 }, "7", '7'}, {{ 350, 120, 80, 60 }, "8", '8'}, {{ 450, 120, 80, 60 }, "9", '9'},
        {{ 250, 190, 80, 60 }, "4", '4'}, {{ 350, 190, 80, 60 }, "5", '5'}, {{ 450, 190, 80, 60 }, "6", '6'},
        {{ 250, 260, 80, 60 }, "1", '1'}, {{ 350, 260, 80, 60 }, "2", '2'}, {{ 450, 260, 80, 60 }, "3", '3'},
        {{ 250, 330, 80, 60 }, "DEL", '<'}, {{ 350, 330, 80, 60 }, "0", '0'}, {{ 450, 330, 80, 60 }, ".", '.'}
    };
    Rectangle btnConnect = { 250, 400, 280, 40 };

    // --- MAIN LOOP ---
    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        if (inMenu) {
            // ==========================================
            // STATE 1: IP ENTRY MENU
            // ==========================================
            
            // Allow physical keyboard typing for testing on PC
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= '0' && key <= '9') || key == '.') {
                    if (ipAddress.length() < 15) ipAddress += (char)key;
                }
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE) && ipAddress.length() > 0) ipAddress.pop_back();
            if (IsKeyPressed(KEY_ENTER) && ipAddress.length() > 0) inMenu = false; // Proceed

            // Touch/Mouse Numpad Handling
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || GetTouchPointCount() > 0) {
                Vector2 tapPos = GetMousePosition();
                if (GetTouchPointCount() > 0) tapPos = GetTouchPosition(0);

                // Use a simple cooldown so one tap doesn't register 60 times a second
                static float tapCooldown = 0.0f;
                if (tapCooldown <= 0.0f) {
                    for (auto& btn : numpad) {
                        if (CheckCollisionPointRec(tapPos, btn.rect)) {
                            if (btn.val == '<' && ipAddress.length() > 0) ipAddress.pop_back();
                            else if (btn.val != '<' && ipAddress.length() < 15) ipAddress += btn.val;
                            tapCooldown = 0.2f; // Wait 0.2 seconds before allowing next tap
                        }
                    }
                    if (CheckCollisionPointRec(tapPos, btnConnect) && ipAddress.length() > 0) {
                        inMenu = false; // Proceed
                    }
                }
                if (tapCooldown > 0.0f) tapCooldown -= deltaTime;
            }

            // Transition: Initialize Socket if we just left the menu
            if (!inMenu) {
                sock = socket(AF_INET, SOCK_DGRAM, 0);
                if (sock >= 0) {
                    clientAddr.sin_family = AF_INET;
                    clientAddr.sin_port = htons(4444);
                    clientAddr.sin_addr.s_addr = inet_addr(ipAddress.c_str());
                } else {
                    std::cerr << "Failed to create socket!" << std::endl;
                    inMenu = true; // Stay in menu if it failed
                }
            }

            BeginDrawing();
            ClearBackground(DARKBLUE);
            DrawText("DROPCAST HOST", 280, 20, 30, WHITE);
            DrawText("Enter Client (PC) IP Address:", 230, 60, 20, LIGHTGRAY);
            
            // Draw IP Display Box
            DrawRectangle(250, 85, 280, 30, RAYWHITE);
            DrawText(ipAddress.c_str(), 260, 90, 20, BLACK);

            // Draw Numpad
            for (auto& btn : numpad) {
                DrawRectangleRec(btn.rect, btn.val == '<' ? MAROON : DARKGRAY);
                DrawRectangleLinesEx(btn.rect, 2, BLACK);
                int textOffset = (btn.val == '<') ? 15 : 30; // Center text roughly
                DrawText(btn.text, btn.rect.x + textOffset, btn.rect.y + 20, 20, WHITE);
            }
            // Draw Connect Button
            DrawRectangleRec(btnConnect, GREEN);
            DrawRectangleLinesEx(btnConnect, 2, BLACK);
            DrawText("CONNECT", btnConnect.x + 90, btnConnect.y + 10, 20, BLACK);
            
            EndDrawing();

        } else {
            // ==========================================
            // STATE 2: GAMEPLAY & NETWORK
            // ==========================================
            
            bool moveLeft = IsKeyDown(KEY_LEFT);
            bool moveRight = IsKeyDown(KEY_RIGHT);
            bool jump = IsKeyPressed(KEY_SPACE);

            for (int i = 0; i < GetTouchPointCount(); i++) {
                Vector2 touchPos = GetTouchPosition(i);
                if (CheckCollisionPointRec(touchPos, btnLeft)) moveLeft = true;
                if (CheckCollisionPointRec(touchPos, btnRight)) moveRight = true;
                if (CheckCollisionPointRec(touchPos, btnJump)) jump = true; 
            }
            
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
                Vector2 mousePos = GetMousePosition();
                if (CheckCollisionPointRec(mousePos, btnLeft)) moveLeft = true;
                if (CheckCollisionPointRec(mousePos, btnRight)) moveRight = true;
                if (CheckCollisionPointRec(mousePos, btnJump) && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) jump = true;
            }

            // Calculate exact velocity for Dead Reckoning
            float velocityX = 0.0f;
            if (moveLeft) velocityX = -playerHorizSpeed;
            if (moveRight) velocityX = playerHorizSpeed;
            
            playerPos.x += velocityX * deltaTime;
            
            velocityY += gravity * deltaTime;

            if (jump && canJump) {
                velocityY = -playerJumpStrength;
                canJump = false;
            }

            playerPos.y += velocityY * deltaTime;

            bool hitObstacle = false;
            if (velocityY >= 0.0f) {
                for (auto& item : envItems) {
                    if (item.blocking &&
                        playerPos.x >= item.rect.x - 20 &&
                        playerPos.x <= item.rect.x + item.rect.width + 20 &&
                        playerPos.y >= item.rect.y &&             
                        playerPos.y <= item.rect.y + 30)          
                    {
                        hitObstacle = true;
                        velocityY = 0.0f;
                        playerPos.y = item.rect.y;
                        break;
                    }
                }
            }
            canJump = hitObstacle;

            // Send Network Packet including velocity!
            if (sock >= 0) {
                GamePacket packet = { playerPos.x, playerPos.y, velocityX, velocityY };
                sendto(sock, &packet, sizeof(GamePacket), 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
            }

            // --- DRAW GAME ---
            BeginDrawing();
            ClearBackground(RAYWHITE);

            for (auto& item : envItems) DrawRectangleRec(item.rect, item.color);

            Rectangle playerRect = { playerPos.x - 20, playerPos.y - 40, 40, 40 };
            DrawRectangleRec(playerRect, MAROON);

            DrawRectangleRec(btnLeft, Fade(DARKGRAY, 0.5f));
            DrawText("<", btnLeft.x + 30, btnLeft.y + 25, 40, WHITE);

            DrawRectangleRec(btnRight, Fade(DARKGRAY, 0.5f));
            DrawText(">", btnRight.x + 30, btnRight.y + 25, 40, WHITE);

            DrawRectangleRec(btnJump, Fade(MAROON, 0.5f));
            DrawText("^", btnJump.x + 30, btnJump.y + 30, 40, WHITE);

            DrawText(TextFormat("Broadcasting to: %s:4444", ipAddress.c_str()), 10, 10, 10, DARKGRAY);
            DrawFPS(screenWidth - 80, 10);
            EndDrawing();
        }
    }

    if (sock >= 0) close(sock);
    CloseWindow();
    return 0;
}