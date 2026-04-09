#include "raylib.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <dirent.h> // For reading the Android filesystem

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

enum RenderCmdType { CMD_CLEAR_BG = 0, CMD_DRAW_RECT, CMD_DRAW_TEXT };
struct RenderCommand {
    int type;
    float x, y, w, h;
    float vx, vy;           // ADDED: Velocity sent to TV for lag-smoothing!
    unsigned char color[4]; 
    char text[32];          
};
struct RenderPacket {
    int count;
    RenderCommand commands[20]; 
};

RenderPacket g_outPacket;
bool g_btnLeft = false;
bool g_btnRight = false;
bool g_btnJump = false;

void PushCommand(RenderPacket& packet, int type, float x, float y, float w, float h, float vx, float vy, Color c, const char* text = "") {
    if (packet.count >= 20) return;
    RenderCommand& cmd = packet.commands[packet.count];
    cmd.type = type; cmd.x = x; cmd.y = y; cmd.w = w; cmd.h = h;
    cmd.vx = vx; cmd.vy = vy; // Smooth it!
    cmd.color[0] = c.r; cmd.color[1] = c.g; cmd.color[2] = c.b; cmd.color[3] = c.a;
    if (text != nullptr) strncpy(cmd.text, text, 31);
    packet.count++;
}

// --- LUA API ---
int api_ClearBG(lua_State* L) {
    int r = luaL_checkinteger(L, 1); int g = luaL_checkinteger(L, 2);
    int b = luaL_checkinteger(L, 3); int a = luaL_checkinteger(L, 4);
    PushCommand(g_outPacket, CMD_CLEAR_BG, 0, 0, 0, 0, 0, 0, {(unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a});
    return 0;
}

int api_DrawRect(lua_State* L) {
    float x = luaL_checknumber(L, 1); float y = luaL_checknumber(L, 2);
    float w = luaL_checknumber(L, 3); float h = luaL_checknumber(L, 4);
    float vx = luaL_checknumber(L, 5); float vy = luaL_checknumber(L, 6); // NEW API REQUIRES VELOCITY
    int r = luaL_checkinteger(L, 7); int g = luaL_checkinteger(L, 8);
    int b = luaL_checkinteger(L, 9); int a = luaL_checkinteger(L, 10);
    PushCommand(g_outPacket, CMD_DRAW_RECT, x, y, w, h, vx, vy, {(unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a});
    return 0;
}

int api_DrawText(lua_State* L) {
    const char* text = luaL_checkstring(L, 1);
    float x = luaL_checknumber(L, 2); float y = luaL_checknumber(L, 3);
    float size = luaL_checknumber(L, 4);
    int r = luaL_checkinteger(L, 5); int g = luaL_checkinteger(L, 6);
    int b = luaL_checkinteger(L, 7); int a = luaL_checkinteger(L, 8);
    PushCommand(g_outPacket, CMD_DRAW_TEXT, x, y, size, 0, 0, 0, {(unsigned char)r, (unsigned char)g, (unsigned char)b, (unsigned char)a}, text);
    return 0;
}

int api_IsButtonDown(lua_State* L) {
    int btnId = luaL_checkinteger(L, 1);
    lua_pushboolean(L, (btnId == 0) ? g_btnLeft : (btnId == 1) ? g_btnRight : g_btnJump);
    return 1;
}

// --- VM MANAGER ---
lua_State* L = nullptr;

void InitLuaVM() {
    if (L) lua_close(L);
    L = luaL_newstate();
    luaL_openlibs(L);
    lua_newtable(L);
    lua_pushcfunction(L, api_ClearBG); lua_setfield(L, -2, "ClearBG");
    lua_pushcfunction(L, api_DrawRect); lua_setfield(L, -2, "DrawRect");
    lua_pushcfunction(L, api_DrawText); lua_setfield(L, -2, "DrawText");
    lua_pushcfunction(L, api_IsButtonDown); lua_setfield(L, -2, "IsButtonDown");
    lua_setglobal(L, "DropCast");
}

struct Button { Rectangle rect; const char* text; char val; };

enum AppState { STATE_IP, STATE_LIBRARY, STATE_PLAY };

int main() {
    InitWindow(800, 450, "DropCast Host - Engine");
    SetTargetFPS(60);

    AppState state = STATE_IP;
    std::string ipAddress = "192.168."; 
    int sock = -1;
    struct sockaddr_in clientAddr;
    memset(&clientAddr, 0, sizeof(clientAddr));

    std::string gamesFolder = "/storage/emulated/0/Download/DropCastGames";
    std::vector<std::string> gameFiles;

    Rectangle btnLeft = { 30, 320, 80, 80 };
    Rectangle btnRight = { 130, 320, 80, 80 };
    Rectangle btnJump = { 800 - 110, 320, 80, 80 };
    Rectangle btnQuit = { 10, 10, 80, 40 };

    std::vector<Button> numpad = {
        {{ 250, 120, 80, 60 }, "7", '7'}, {{ 350, 120, 80, 60 }, "8", '8'}, {{ 450, 120, 80, 60 }, "9", '9'},
        {{ 250, 190, 80, 60 }, "4", '4'}, {{ 350, 190, 80, 60 }, "5", '5'}, {{ 450, 190, 80, 60 }, "6", '6'},
        {{ 250, 260, 80, 60 }, "1", '1'}, {{ 350, 260, 80, 60 }, "2", '2'}, {{ 450, 260, 80, 60 }, "3", '3'},
        {{ 250, 330, 80, 60 }, "DEL", '<'}, {{ 350, 330, 80, 60 }, "0", '0'}, {{ 450, 330, 80, 60 }, ".", '.'}
    };
    Rectangle btnConnect = { 250, 400, 280, 40 };

    while (!WindowShouldClose()) {
        float deltaTime = GetFrameTime();

        // Check Input Globals
        Vector2 tapPos = GetMousePosition();
        if (GetTouchPointCount() > 0) tapPos = GetTouchPosition(0);
        bool tapClick = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || GetTouchPointCount() > 0;
        bool tapDown = IsMouseButtonDown(MOUSE_BUTTON_LEFT) || GetTouchPointCount() > 0;

        if (state == STATE_IP) {
            // --- 1. IP ENTRY MENU ---
            int key = GetCharPressed();
            while (key > 0) {
                if ((key >= '0' && key <= '9') || key == '.') { if (ipAddress.length() < 15) ipAddress += (char)key; }
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE) && ipAddress.length() > 0) ipAddress.pop_back();

            static float tapCooldown = 0.0f;
            if (tapClick && tapCooldown <= 0.0f) {
                for (auto& btn : numpad) {
                    if (CheckCollisionPointRec(tapPos, btn.rect)) {
                        if (btn.val == '<' && ipAddress.length() > 0) ipAddress.pop_back();
                        else if (btn.val != '<' && ipAddress.length() < 15) ipAddress += btn.val;
                        tapCooldown = 0.2f; 
                    }
                }
                if (CheckCollisionPointRec(tapPos, btnConnect) && ipAddress.length() > 0) {
                    // Connect Socket
                    sock = socket(AF_INET, SOCK_DGRAM, 0);
                    clientAddr.sin_family = AF_INET;
                    clientAddr.sin_port = htons(4444);
                    clientAddr.sin_addr.s_addr = inet_addr(ipAddress.c_str());
                    
                    // Scan Directory
                    gameFiles.clear();
                    DIR* dir = opendir(gamesFolder.c_str());
                    if (dir) {
                        struct dirent* ent;
                        while ((ent = readdir(dir)) != nullptr) {
                            std::string fname = ent->d_name;
                            if (fname.length() >= 4 && fname.substr(fname.length() - 4) == ".lua") {
                                gameFiles.push_back(fname);
                            }
                        }
                        closedir(dir);
                    }
                    state = STATE_LIBRARY; // Move to Game Browser
                }
            }
            if (tapCooldown > 0.0f) tapCooldown -= deltaTime;

            BeginDrawing();
            ClearBackground(DARKBLUE);
            DrawText("DROPCAST CONNECT", 250, 20, 30, WHITE);
            DrawRectangle(250, 85, 280, 30, RAYWHITE);
            DrawText(ipAddress.c_str(), 260, 90, 20, BLACK);
            for (auto& btn : numpad) {
                DrawRectangleRec(btn.rect, btn.val == '<' ? MAROON : DARKGRAY);
                DrawText(btn.text, btn.rect.x + 30, btn.rect.y + 20, 20, WHITE);
            }
            DrawRectangleRec(btnConnect, GREEN);
            DrawText("CONNECT TO TV", btnConnect.x + 60, btnConnect.y + 10, 20, BLACK);
            EndDrawing();

        } else if (state == STATE_LIBRARY) {
            // --- 2. GAME BROWSER ---
            BeginDrawing();
            ClearBackground(DARKBLUE);
            DrawText("DROPCAST LIBRARY", 250, 20, 30, WHITE);
            DrawText(TextFormat("Looking in: %s", gamesFolder.c_str()), 10, 60, 15, LIGHTGRAY);

            if (gameFiles.empty()) {
                DrawText("No .lua games found!", 250, 200, 20, RED);
                DrawText("1. Create 'DropCastGames' folder in Downloads.", 150, 240, 20, WHITE);
                DrawText("2. Grant Storage Permissions in Android App Settings.", 150, 270, 20, WHITE);
            } else {
                for (int i = 0; i < gameFiles.size(); i++) {
                    Rectangle itemRect = { 100, 120.0f + i * 50, 600, 40 };
                    DrawRectangleRec(itemRect, LIGHTGRAY);
                    DrawText(gameFiles[i].c_str(), 110, 130 + i * 50, 20, BLACK);

                    if (tapClick && CheckCollisionPointRec(tapPos, itemRect)) {
                        InitLuaVM();
                        std::string fullPath = gamesFolder + "/" + gameFiles[i];
                        if (luaL_dofile(L, fullPath.c_str()) != LUA_OK) {
                            std::cerr << "Lua Load Error: " << lua_tostring(L, -1) << std::endl;
                        } else {
                            lua_getglobal(L, "Init");
                            if (lua_isfunction(L, -1)) lua_pcall(L, 0, 0, 0); else lua_pop(L, 1);
                            state = STATE_PLAY;
                        }
                    }
                }
            }
            
            Rectangle refreshBtn = { 650, 20, 120, 30 };
            DrawRectangleRec(refreshBtn, GREEN);
            DrawText("REFRESH", 670, 25, 20, BLACK);
            if (tapClick && CheckCollisionPointRec(tapPos, refreshBtn)) {
                gameFiles.clear();
                DIR* dir = opendir(gamesFolder.c_str());
                if (dir) {
                    struct dirent* ent;
                    while ((ent = readdir(dir)) != nullptr) {
                        std::string fname = ent->d_name;
                        if (fname.length() >= 4 && fname.substr(fname.length() - 4) == ".lua") { gameFiles.push_back(fname); }
                    }
                    closedir(dir);
                }
            }
            EndDrawing();

        } else if (state == STATE_PLAY) {
            // --- 3. PLAYING GAME ---
            g_btnLeft = IsKeyDown(KEY_LEFT); g_btnRight = IsKeyDown(KEY_RIGHT); g_btnJump = IsKeyPressed(KEY_SPACE);
            if (tapDown) {
                if (CheckCollisionPointRec(tapPos, btnLeft)) g_btnLeft = true;
                if (CheckCollisionPointRec(tapPos, btnRight)) g_btnRight = true;
                if (CheckCollisionPointRec(tapPos, btnJump) && tapClick) g_btnJump = true;
                if (CheckCollisionPointRec(tapPos, btnQuit) && tapClick) state = STATE_LIBRARY; // Back to Menu!
            }

            g_outPacket.count = 0;
            if (L) {
                lua_getglobal(L, "Update");
                if (lua_isfunction(L, -1)) {
                    lua_pushnumber(L, deltaTime);
                    if (lua_pcall(L, 1, 0, 0) != LUA_OK) std::cerr << "Lua Error: " << lua_tostring(L, -1) << std::endl;
                } else lua_pop(L, 1);
            }

            if (sock >= 0 && g_outPacket.count > 0) {
                sendto(sock, &g_outPacket, sizeof(RenderPacket), 0, (struct sockaddr*)&clientAddr, sizeof(clientAddr));
            }

            BeginDrawing();
            ClearBackground(BLACK);
            DrawRectangleRec(btnQuit, MAROON);
            DrawText("QUIT", btnQuit.x + 15, btnQuit.y + 10, 20, WHITE);
            DrawText("LUA VM ACTIVE - LOOK AT THE TV!", 230, 150, 20, GREEN);

            DrawRectangleRec(btnLeft, Fade(DARKGRAY, 0.5f)); DrawText("<", btnLeft.x + 30, btnLeft.y + 25, 40, WHITE);
            DrawRectangleRec(btnRight, Fade(DARKGRAY, 0.5f)); DrawText(">", btnRight.x + 30, btnRight.y + 25, 40, WHITE);
            DrawRectangleRec(btnJump, Fade(MAROON, 0.5f)); DrawText("^", btnJump.x + 30, btnJump.y + 30, 40, WHITE);
            EndDrawing();
        }
    }

    if (L) lua_close(L);
    if (sock >= 0) close(sock);
    CloseWindow();
    return 0;
}