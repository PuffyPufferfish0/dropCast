#include "raylib.h"
#include <iostream>
#include <cstring>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

// --- THE NEW "DUMB DISPLAY" PROTOCOL ---
enum RenderCmdType { CMD_CLEAR_BG = 0, CMD_DRAW_RECT, CMD_DRAW_TEXT };

struct RenderCommand {
    int type;
    float x, y, w, h;
    float vx, vy;           // ADDED: Velocity for network smoothing
    unsigned char color[4]; 
    char text[32];          
};

struct RenderPacket {
    int count;
    RenderCommand commands[20]; // 20 max commands to keep packet small
};

int main() {
    const int screenWidth = 800;
    const int screenHeight = 450;
    InitWindow(screenWidth, screenHeight, "DropCast Client - Universal Display");
    SetTargetFPS(60);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return -1;

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(4444);
    serverAddr.sin_addr.s_addr = INADDR_ANY;

    bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    fcntl(sock, F_SETFL, O_NONBLOCK); 

    RenderPacket lastPacket = { 0 };
    bool connected = false;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        
        // 1. NETWORK DRAIN
        RenderPacket incomingPacket;
        bool gotNewData = false;
        
        while (true) {
            int bytesRead = recvfrom(sock, &incomingPacket, sizeof(RenderPacket), 0, NULL, NULL);
            if (bytesRead == sizeof(RenderPacket)) {
                lastPacket = incomingPacket;
                connected = true;
                gotNewData = true;
            } else if (bytesRead > 0) {
                // If we get a packet but the struct sizes don't match, warn us in the terminal!
                std::cout << "NETWORK ERROR: Size mismatch! Expected: " << sizeof(RenderPacket) 
                          << " Got: " << bytesRead << std::endl;
            } else {
                break; // Buffer is empty
            }
        }

        // 2. KINEMATIC DEAD RECKONING (Lag Smoothing)
        // If the Android Wi-Fi went to sleep this frame, we just keep moving 
        // the shapes in whatever direction they were already going!
        if (connected && !gotNewData) {
            for (int i = 0; i < lastPacket.count; i++) {
                lastPacket.commands[i].x += lastPacket.commands[i].vx * dt;
                lastPacket.commands[i].y += lastPacket.commands[i].vy * dt;
            }
        }

        // 3. BLIND RENDERING
        BeginDrawing();
        
        if (connected && lastPacket.count > 0) {
            for (int i = 0; i < lastPacket.count; i++) {
                RenderCommand& cmd = lastPacket.commands[i];
                Color c = { cmd.color[0], cmd.color[1], cmd.color[2], cmd.color[3] };

                if (cmd.type == CMD_CLEAR_BG) {
                    ClearBackground(c);
                } else if (cmd.type == CMD_DRAW_RECT) {
                    DrawRectangleRec({cmd.x, cmd.y, cmd.w, cmd.h}, c);
                } else if (cmd.type == CMD_DRAW_TEXT) {
                    DrawText(cmd.text, cmd.x, cmd.y, (int)cmd.w, c);
                }
            }
        } else {
            ClearBackground(BLACK);
            DrawText("AWAITING HOST RENDER COMMANDS...", 200, 220, 20, GRAY);
        }

        DrawFPS(screenWidth - 80, 10);
        EndDrawing();
    }

    close(sock);
    CloseWindow();
    return 0;
}