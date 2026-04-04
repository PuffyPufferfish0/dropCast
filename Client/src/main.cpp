#include "raylib.h"
#include <iostream>
#include <vector>

// POSIX Socket headers
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
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

int main() {
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "DropCast Client - Display");
    SetTargetFPS(60);

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
    fcntl(sock, F_SETFL, O_NONBLOCK);

    // Setup for Dead Reckoning
    GamePacket remotePlayer = { 400.0f, 280.0f, 0.0f, 0.0f }; 
    GamePacket lastNetworkPacket = { 400.0f, 280.0f, 0.0f, 0.0f };
    float timeSinceLastPacket = 0.0f;
    const float gravity = 800.0f; // Must match the host's gravity!
    bool connected = false;

    std::vector<EnvironmentItem> envItems = {
        {{ -500, 400, 2000, 200 }, 1, GRAY},
        {{ 300, 200, 400, 10 }, 1, DARKGRAY },
        {{ 250, 300, 100, 10 }, 1, DARKGRAY },
        {{ 650, 300, 100, 10 }, 1, DARKGRAY },
        {{ 850, 250, 150, 10 }, 1, DARKGRAY }
    };

    Camera2D camera = { 0 };
    camera.target = (Vector2){ remotePlayer.x, remotePlayer.y };
    camera.offset = (Vector2){ screenWidth / 2.0f, screenHeight / 2.0f };
    camera.rotation = 0.0f;
    camera.zoom = 1.0f;

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();
        
        // 1. NETWORK DRAIN
        GamePacket incomingPacket;
        bool gotNewData = false;
        
        while (true) {
            int bytesRead = recvfrom(sock, &incomingPacket, sizeof(GamePacket), 0, NULL, NULL);
            if (bytesRead == sizeof(GamePacket)) {
                lastNetworkPacket = incomingPacket;
                connected = true;
                gotNewData = true;
            } else {
                break; // Buffer is empty
            }
        }

        // 2. DEAD RECKONING MATH
        if (gotNewData) {
            timeSinceLastPacket = 0.0f; // Reset our blind-prediction timer
        } else if (connected) {
            timeSinceLastPacket += deltaTime; // Count how long we've been blind
        }

        // Predict exactly where the player should be using Physics (Position + Velocity*Time + 1/2*Accel*Time^2)
        float predictedX = lastNetworkPacket.x + (lastNetworkPacket.velX * timeSinceLastPacket);
        float predictedY = lastNetworkPacket.y + (lastNetworkPacket.velY * timeSinceLastPacket) + (0.5f * gravity * timeSinceLastPacket * timeSinceLastPacket);

        // 3. APPLY SMOOTHING
        if (timeSinceLastPacket > 1.0f) {
            // If it's been over a full second, the network is dead. Just snap to position.
            remotePlayer.x = predictedX;
            remotePlayer.y = predictedY;
        } else {
            // Smoothly slide towards our predicted location
            remotePlayer.x += (predictedX - remotePlayer.x) * 15.0f * deltaTime;
            remotePlayer.y += (predictedY - remotePlayer.y) * 15.0f * deltaTime;
        }

        camera.target.x = remotePlayer.x;
        camera.target.y = remotePlayer.y;

        // --- DRAW ---
        BeginDrawing();
        ClearBackground(SKYBLUE);
        
        if (connected) {
            BeginMode2D(camera);
            for (auto& item : envItems) DrawRectangleRec(item.rect, item.color);

            Rectangle playerRect = { remotePlayer.x - 20, remotePlayer.y - 40, 40, 40 };
            DrawRectangleRec(playerRect, DARKBLUE);
            EndMode2D();

            DrawText("CLIENT: Dead Reckoning Active", 10, 10, 20, WHITE);
            // Show exactly how long the Android OS is sleeping the Wi-Fi for debugging
            DrawText(TextFormat("Blind Time: %.3f sec", timeSinceLastPacket), 10, 35, 20, (timeSinceLastPacket > 0.05f) ? RED : LIGHTGRAY);
        } else {
            ClearBackground(BLACK);
            DrawText("AWAITING VIDEO STREAM FROM HOST...", 220, 220, 20, GRAY);
        }

        DrawFPS(screenWidth - 80, 10);
        EndDrawing();
    }

    close(sock);
    CloseWindow();
    return 0;
}