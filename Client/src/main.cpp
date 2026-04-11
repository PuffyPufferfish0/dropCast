#include "raylib.h"
#include <iostream>
#include <vector>
#include <map>
#include <cstring>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

enum RenderCmdType { 
    CMD_CLEAR_BG = 0, 
    CMD_DRAW_RECT, 
    CMD_DRAW_TEXT,
    CMD_LOAD_TEXTURE,   
    CMD_DRAW_SPRITE     
};

struct RenderCommand {
    int type;
    float x, y, w, h;       
    float sw, sh;           
    float vx, vy;           
    int textureId;          
    unsigned char color[4]; 
    char text[32];          
};

struct RenderPacket {
    int count;
    bool isLargeData;       
    int dataSize;
    unsigned char rawData[2048]; 
    RenderCommand commands[15]; 
};

int main() {
    InitWindow(800, 450, "DropCast Client - Universal Display");
    SetTargetFPS(60);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(4444);
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    
    if (bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) < 0) {
        std::cerr << "Port 4444 busy!" << std::endl;
        return -1;
    }
    fcntl(sock, F_SETFL, O_NONBLOCK); 

    std::map<int, Texture2D> textureLibrary;
    RenderPacket lastPacket = { 0 };
    bool connected = false;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        
        RenderPacket incomingPacket;
        while (true) {
            int bytesRead = recvfrom(sock, &incomingPacket, sizeof(RenderPacket), 0, NULL, NULL);
            if (bytesRead == sizeof(RenderPacket)) {
                if (incomingPacket.isLargeData) {
                    // FIX: Added PIXELFORMAT_ prefix
                    Image img = { incomingPacket.rawData, 64, 64, 1, PIXELFORMAT_UNCOMPRESSED_R8G8B8A8 };
                    int id = incomingPacket.commands[0].textureId;
                    if (textureLibrary.count(id)) UnloadTexture(textureLibrary[id]);
                    textureLibrary[id] = LoadTextureFromImage(img);
                } else {
                    lastPacket = incomingPacket;
                }
                connected = true;
            } else break;
        }

        // Dead Reckoning for lag-free movement
        if (connected) {
            for (int i = 0; i < lastPacket.count; i++) {
                lastPacket.commands[i].x += lastPacket.commands[i].vx * dt;
                lastPacket.commands[i].y += lastPacket.commands[i].vy * dt;
            }
        }

        BeginDrawing();
        if (connected) {
            for (int i = 0; i < lastPacket.count; i++) {
                RenderCommand& cmd = lastPacket.commands[i];
                Color c = { cmd.color[0], cmd.color[1], cmd.color[2], cmd.color[3] };

                if (cmd.type == CMD_CLEAR_BG) ClearBackground(c);
                else if (cmd.type == CMD_DRAW_RECT) DrawRectangleRec({cmd.x, cmd.y, cmd.w, cmd.h}, c);
                else if (cmd.type == CMD_DRAW_TEXT) DrawText(cmd.text, cmd.x, cmd.y, (int)cmd.w, c);
                else if (cmd.type == CMD_DRAW_SPRITE) {
                    if (textureLibrary.count(cmd.textureId)) {
                        Rectangle src = { cmd.w, cmd.h, cmd.sw, cmd.sh };
                        Rectangle dest = { cmd.x, cmd.y, cmd.sw, cmd.sh };
                        DrawTexturePro(textureLibrary[cmd.textureId], src, dest, {0,0}, 0, WHITE);
                    }
                }
            }
        } else {
            ClearBackground(BLACK);
            DrawText("AWAITING DROPCAST STREAM...", 240, 220, 20, GRAY);
        }
        DrawFPS(720, 10);
        EndDrawing();
    }
    
    for (auto const& [id, tex] : textureLibrary) UnloadTexture(tex);
    close(sock);
    CloseWindow();
    return 0;
}