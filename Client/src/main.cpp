#include "raylib.h"
#include <iostream>
#include <vector>
#include <map>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

enum RenderCmdType { CMD_CLEAR_BG = 0, CMD_DRAW_RECT, CMD_DRAW_TEXT, CMD_DRAW_SPRITE };

struct RenderCommand {
    int type;
    float x, y, w, h;       
    float sx, sy, sw, sh;   
    float vx, vy;           
    int textureId;          
    unsigned char color[4]; 
    char text[32];              
};

// CRITICAL FIX: Shrunk the packet to ~1400 bytes total to survive UDP/Wi-Fi MTU limits
struct RenderPacket {
    int packetType; // 0 = Game Render, 1 = Asset Chunk
    int count;
    
    int assetId;
    int totalSize;
    int chunkOffset;
    int chunkSize;
    unsigned char rawData[512]; 
    
    RenderCommand commands[10]; 
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
    bind(sock, (struct sockaddr*)&serverAddr, sizeof(serverAddr));
    fcntl(sock, F_SETFL, O_NONBLOCK); 

    std::map<int, Texture2D> textureLibrary;
    std::map<int, std::vector<unsigned char>> assetBuffers;
    std::map<int, int> bytesReceived;

    RenderPacket lastPacket = { 0 };
    bool connected = false;

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        
        RenderPacket p;
        while (true) {
            int bytesRead = recvfrom(sock, &p, sizeof(RenderPacket), 0, NULL, NULL);
            if (bytesRead == sizeof(RenderPacket)) {
                connected = true;
                if (p.packetType == 1) {
                    // --- ASSET RECONSTRUCTION ---
                    int id = p.assetId;
                    if (assetBuffers[id].size() != p.totalSize) {
                        assetBuffers[id].resize(p.totalSize);
                        bytesReceived[id] = 0;
                    }
                    
                    memcpy(assetBuffers[id].data() + p.chunkOffset, p.rawData, p.chunkSize);
                    bytesReceived[id] += p.chunkSize;
                    
                    // Decode image once all chunks arrive
                    if (bytesReceived[id] >= p.totalSize) {
                        Image img = LoadImageFromMemory(".png", assetBuffers[id].data(), p.totalSize);
                        if (img.data != NULL) {
                            if (textureLibrary.count(id)) UnloadTexture(textureLibrary[id]);
                            textureLibrary[id] = LoadTextureFromImage(img);
                            UnloadImage(img);
                            std::cout << "SUCCESS: Asset " << id << " reconstructed and loaded!" << std::endl;
                        } else {
                            std::cerr << "ERROR: Failed to decode image data!" << std::endl;
                        }
                        bytesReceived[id] = 0; 
                    }
                } else {
                    lastPacket = p;
                }
            } else break;
        }

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
                        Rectangle src = { cmd.sx, cmd.sy, cmd.sw, cmd.sh };
                        Rectangle dest = { cmd.x, cmd.y, cmd.w, cmd.h };
                        DrawTexturePro(textureLibrary[cmd.textureId], src, dest, {0,0}, 0, WHITE);
                    } else {
                        DrawRectangleRec({cmd.x, cmd.y, cmd.w, cmd.h}, GRAY);
                        DrawText("Syncing...", cmd.x, cmd.y, 10, WHITE);
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
    close(sock);
    CloseWindow();
    return 0;
}