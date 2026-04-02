#include "raylib.h"
#include <iostream>
#include <vector>

// POSIX Socket headers
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <cstring>

// Must match the Host's struct exactly!
struct GamePacket {
    float x;
    float y;
};

// The Map (Must match Host)
struct EnvironmentItem {
    Rectangle rect;
    int blocking;
    Color color;
};

int main() {
    // 1. Initialize Raylib
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "DropCast Client - Display (With Camera)");
    SetTargetFPS(60);

    // 2. Setup UDP Socket (Listening)
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return -1;

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(4444);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Failed to bind to port 4444!" << std::endl;
        return -1;
    }
    fcntl(sock, F_SETFL, O_NONBLOCK); // Non-blocking!

    // 3. Game & Camera State
    GamePacket remotePlayer = { 400.0f, 280.0f }; 
    bool connected = false;

    // We define the same map here so the client knows what to draw
    std::vector<EnvironmentItem> envItems = {
        {{ -500, 400, 2000, 200 }, 1, GRAY},
        {{ 300, 200, 400, 10 }, 1, DARKGRAY },
        {{ 250, 300, 100, 10 }, 1, DARKGRAY },
        {{ 650, 300, 100, 10 }, 1, DARKGRAY },
        {{ 850, 250, 150, 10 }, 1, DARKGRAY }
    };

    // Setup 2D Camera
    Camera2D camera = { 0 };
    camera.target = (Vector2){ remotePlayer.x, remotePlayer.y };
    camera.offset = (Vector2){ screenWidth / 2.0f, screenHeight / 2.0f }; // Keep player centered
    camera.rotation = 0.0f;
    camera.zoom = 1.0f; // Try setting this to 1.5f for a closer view!

    // 4. Main Game Loop
    while (!WindowShouldClose()) {
        
        // --- UPDATE (Listen for network data) ---
        GamePacket incomingPacket;
        int bytesRead = recvfrom(sock, &incomingPacket, sizeof(GamePacket), 0, NULL, NULL);
        
        if (bytesRead == sizeof(GamePacket)) {
            remotePlayer = incomingPacket;
            connected = true;
        }

        // Smoothly update camera target to follow the player
        camera.target.x = remotePlayer.x;
        camera.target.y = remotePlayer.y;

        // --- DRAW ---
        BeginDrawing();
        ClearBackground(SKYBLUE); // Different background for the client
        
        if (connected) {
            // Begin 2D mode with our tracking camera
            BeginMode2D(camera);

            // Draw Environment
            for (auto& item : envItems) DrawRectangleRec(item.rect, item.color);

            // Draw Remote Player
            Rectangle playerRect = { remotePlayer.x - 20, remotePlayer.y - 40, 40, 40 };
            DrawRectangleRec(playerRect, DARKBLUE);

            EndMode2D(); // End Camera mode

            // Draw HUD (UI is drawn OUTSIDE the camera mode so it stays on screen)
            DrawText("CLIENT: Active Video Stream", 10, 10, 20, WHITE);
            DrawText("Camera tracking Host coordinates...", 10, 35, 10, RAYWHITE);
        } else {
            ClearBackground(BLACK);
            DrawText("AWAITING VIDEO STREAM FROM HOST...", 220, 220, 20, GRAY);
        }

        DrawFPS(screenWidth - 80, 10);
        EndDrawing();
    }

    // 5. Cleanup
    close(sock);
    CloseWindow();
    return 0;
}