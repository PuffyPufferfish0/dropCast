#include "raylib.h"
#include <iostream>
#include <vector>
#include <string>
#include <cstring>
#include <dirent.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

extern "C" {
    #include "lua.h"
    #include "lualib.h"
    #include "lauxlib.h"
}

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

struct RenderPacket {
    int packetType; 
    int count;
    int assetId;
    int totalSize;
    int chunkOffset;
    int chunkSize;
    unsigned char rawData[512]; 
    RenderCommand commands[10]; 
};

struct PhysBody { int id; Rectangle rect; Vector2 velocity; bool isStatic; };

std::vector<PhysBody> g_physWorld;
RenderPacket g_outPacket;
bool g_keys[3] = {false, false, false};
lua_State* L = nullptr;

int g_sock = -1;
struct sockaddr_in g_clientAddr;
std::string g_gamesPath = "/storage/emulated/0/Download/DropCastGames";

// --- LUA BINDINGS ---
int api_ClearPhysics(lua_State* l_ptr) { g_physWorld.clear(); return 0; }
int api_AddBody(lua_State* l_ptr) {
    float x = luaL_checknumber(l_ptr, 1); float y = luaL_checknumber(l_ptr, 2);
    float w = luaL_checknumber(l_ptr, 3); float h = luaL_checknumber(l_ptr, 4);
    bool s = lua_toboolean(l_ptr, 5);
    g_physWorld.push_back({(int)g_physWorld.size(), {x,y,w,h}, {0,0}, s});
    lua_pushinteger(l_ptr, g_physWorld.size() - 1); return 1;
}
int api_GetBodyPos(lua_State* l_ptr) {
    int id = luaL_checkinteger(l_ptr, 1);
    if (id >= 0 && id < (int)g_physWorld.size()) {
        lua_pushnumber(l_ptr, g_physWorld[id].rect.x); lua_pushnumber(l_ptr, g_physWorld[id].rect.y);
        lua_pushnumber(l_ptr, g_physWorld[id].velocity.x); lua_pushnumber(l_ptr, g_physWorld[id].velocity.y);
        return 4;
    }
    return 0;
}
int api_SetBodyVel(lua_State* l_ptr) {
    int id = luaL_checkinteger(l_ptr, 1);
    float vx = luaL_checknumber(l_ptr, 2); float vy = luaL_checknumber(l_ptr, 3);
    if (id >= 0 && id < (int)g_physWorld.size()) { g_physWorld[id].velocity.x = vx; g_physWorld[id].velocity.y = vy; }
    return 0;
}
int api_ClearBG(lua_State* l_ptr) {
    if (g_outPacket.count < 10) {
        RenderCommand& c = g_outPacket.commands[g_outPacket.count++];
        c.type = CMD_CLEAR_BG; c.color[0]=luaL_checkinteger(l_ptr,1); c.color[1]=luaL_checkinteger(l_ptr,2); c.color[2]=luaL_checkinteger(l_ptr,3); c.color[3]=255;
    }
    return 0;
}
int api_DrawRect(lua_State* l_ptr) {
    if (g_outPacket.count < 10) {
        RenderCommand& c = g_outPacket.commands[g_outPacket.count++];
        c.type = CMD_DRAW_RECT; c.x=luaL_checknumber(l_ptr,1); c.y=luaL_checknumber(l_ptr,2); c.w=luaL_checknumber(l_ptr,3); c.h=luaL_checknumber(l_ptr,4);
        c.vx=luaL_checknumber(l_ptr,5); c.vy=luaL_checknumber(l_ptr,6);
        c.color[0]=luaL_checkinteger(l_ptr,7); c.color[1]=luaL_checkinteger(l_ptr,8); c.color[2]=luaL_checkinteger(l_ptr,9); c.color[3]=255;
    }
    return 0;
}
int api_DrawText(lua_State* l_ptr) {
    if (g_outPacket.count < 10) {
        RenderCommand& c = g_outPacket.commands[g_outPacket.count++];
        c.type = CMD_DRAW_TEXT; strncpy(c.text, luaL_checkstring(l_ptr,1), 31); c.x=luaL_checknumber(l_ptr,2); c.y=luaL_checknumber(l_ptr,3); c.w=luaL_checknumber(l_ptr,4);
        c.color[0]=luaL_checkinteger(l_ptr,5); c.color[1]=luaL_checkinteger(l_ptr,6); c.color[2]=luaL_checkinteger(l_ptr,7); c.color[3]=255;
    }
    return 0;
}

int api_LoadTexture(lua_State* l_ptr) {
    int id = luaL_checkinteger(l_ptr, 1);
    std::string path = luaL_checkstring(l_ptr, 2);
    std::string fullPath = g_gamesPath + "/" + path;
    
    FILE* f = fopen(fullPath.c_str(), "rb");
    if (!f) return 0;
    
    fseek(f, 0, SEEK_END);
    int size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    std::vector<unsigned char> buffer(size);
    size_t rb = fread(buffer.data(), 1, size, f);
    (void)rb;
    fclose(f);
    
    int offset = 0;
    while (offset < size) {
        RenderPacket pkt;
        memset(&pkt, 0, sizeof(RenderPacket));
        pkt.packetType = 1; 
        pkt.assetId = id;
        pkt.totalSize = size;
        pkt.chunkOffset = offset;
        pkt.chunkSize = std::min(512, size - offset);
        memcpy(pkt.rawData, buffer.data() + offset, pkt.chunkSize);
        
        sendto(g_sock, &pkt, sizeof(RenderPacket), 0, (struct sockaddr*)&g_clientAddr, sizeof(g_clientAddr));
        offset += pkt.chunkSize;
        usleep(4000); 
    }
    return 0;
}

int api_DrawSprite(lua_State* l_ptr) {
    if (g_outPacket.count < 10) {
        RenderCommand& c = g_outPacket.commands[g_outPacket.count++];
        c.type = CMD_DRAW_SPRITE;
        c.textureId = luaL_checkinteger(l_ptr, 1);
        c.sx = luaL_checknumber(l_ptr, 2); c.sy = luaL_checknumber(l_ptr, 3);
        c.sw = luaL_checknumber(l_ptr, 4); c.sh = luaL_checknumber(l_ptr, 5);
        c.x = luaL_checknumber(l_ptr, 6);  c.y = luaL_checknumber(l_ptr, 7);
        c.w = luaL_checknumber(l_ptr, 8);  c.h = luaL_checknumber(l_ptr, 9);
        c.vx = luaL_checknumber(l_ptr, 10); c.vy = luaL_checknumber(l_ptr, 11);
    }
    return 0;
}

int api_IsButtonDown(lua_State* l_ptr) {
    int id = luaL_checkinteger(l_ptr, 1);
    lua_pushboolean(l_ptr, (id >= 0 && id < 3) ? g_keys[id] : false);
    return 1;
}

void StepPhysics(float dt) {
    for (auto& b : g_physWorld) {
        if (b.isStatic) continue;
        b.velocity.y += 1000.0f * dt;
        b.rect.x += b.velocity.x * dt; b.rect.y += b.velocity.y * dt;
        for (auto& o : g_physWorld) {
            if (&b == &o) continue;
            if (CheckCollisionRecs(b.rect, o.rect)) {
                if (b.velocity.y > 0 && b.rect.y < o.rect.y) { b.rect.y = o.rect.y - b.rect.height; b.velocity.y = 0; }
            }
        }
    }
}

enum ShellTab { TAB_LIBRARY, TAB_SETTINGS, TAB_USER, TAB_PLAYING };
struct ShellState {
    ShellTab currentTab = TAB_SETTINGS; 
    std::string ip = "192.168.";
    std::vector<std::string> gameList;
    bool isConnected = false;
};

struct UI_Button { Rectangle rect; const char* text; char val; };

int main() {
    InitWindow(800, 450, "DropCast Shell");
    SetTargetFPS(60);

    ShellState shell;
    g_sock = socket(AF_INET, SOCK_DGRAM, 0);
    memset(&g_clientAddr, 0, sizeof(g_clientAddr));

    Rectangle sidebar = { 0, 0, 180, 450 };

    std::vector<UI_Button> numpad = {
        {{ 250, 130, 80, 60 }, "7", '7'}, {{ 350, 130, 80, 60 }, "8", '8'}, {{ 450, 130, 80, 60 }, "9", '9'},
        {{ 250, 200, 80, 60 }, "4", '4'}, {{ 350, 200, 80, 60 }, "5", '5'}, {{ 450, 200, 80, 60 }, "6", '6'},
        {{ 250, 270, 80, 60 }, "1", '1'}, {{ 350, 270, 80, 60 }, "2", '2'}, {{ 450, 270, 80, 60 }, "3", '3'},
        {{ 250, 340, 80, 60 }, "DEL", '<'}, {{ 350, 340, 80, 60 }, "0", '0'}, {{ 450, 340, 80, 60 }, ".", '.'}
    };
    Rectangle btnLeft = { 30, 320, 80, 80 };
    Rectangle btnRight = { 130, 320, 80, 80 };
    Rectangle btnJump = { 800 - 110, 320, 80, 80 };

    while (!WindowShouldClose()) {
        float dt = GetFrameTime();
        Vector2 m = GetMousePosition();
        if (GetTouchPointCount() > 0) m = GetTouchPosition(0);
        bool click = IsMouseButtonPressed(MOUSE_BUTTON_LEFT) || (GetTouchPointCount() > 0 && IsGestureDetected(GESTURE_TAP));

        if (shell.currentTab == TAB_PLAYING) {
            g_keys[0] = IsKeyDown(KEY_LEFT) || (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(m, btnLeft));
            g_keys[1] = IsKeyDown(KEY_RIGHT) || (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(m, btnRight));
            g_keys[2] = IsKeyDown(KEY_SPACE) || (IsMouseButtonDown(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(m, btnJump));

            StepPhysics(dt);
            g_outPacket.count = 0;
            g_outPacket.packetType = 0; // Game packet type
            if (L) {
                lua_getglobal(L, "Update"); lua_pushnumber(L, dt);
                if (lua_pcall(L, 1, 0, 0) != LUA_OK) std::cerr << lua_tostring(L, -1) << std::endl;
            }
            if (shell.isConnected) sendto(g_sock, &g_outPacket, sizeof(RenderPacket), 0, (struct sockaddr*)&g_clientAddr, sizeof(g_clientAddr));

            BeginDrawing();
            ClearBackground(BLACK);
            DrawText("PLAYING - TAP TOP TO QUIT", 300, 20, 15, DARKGRAY);
            if (click && m.y < 50) shell.currentTab = TAB_LIBRARY;

            DrawRectangleRec(btnLeft, Fade(GRAY, 0.5f));
            DrawText("<", btnLeft.x + 30, btnLeft.y + 25, 40, WHITE);
            DrawRectangleRec(btnRight, Fade(GRAY, 0.5f));
            DrawText(">", btnRight.x + 30, btnRight.y + 25, 40, WHITE);
            DrawRectangleRec(btnJump, Fade(MAROON, 0.5f));
            DrawText("^", btnJump.x + 30, btnJump.y + 30, 40, WHITE);
            
            EndDrawing();
        } else {
            BeginDrawing();
            ClearBackground({30, 30, 45, 255});
            DrawRectangleRec(sidebar, {20, 20, 30, 255});

            auto DrawTab = [&](const char* label, ShellTab t, int y) {
                Rectangle r = { 10, (float)y, 160, 40 };
                bool hover = CheckCollisionPointRec(m, r);
                DrawRectangleRec(r, (shell.currentTab == t) ? BLUE : (hover ? DARKGRAY : BLANK));
                DrawText(label, 25, y + 10, 20, WHITE);
                if (click && hover) shell.currentTab = t;
            };

            DrawText("DROPCAST", 20, 20, 25, SKYBLUE);
            DrawTab("LIBRARY", TAB_LIBRARY, 80);
            DrawTab("SETTINGS", TAB_SETTINGS, 130);
            DrawTab("USER", TAB_USER, 180);

            if (shell.currentTab == TAB_LIBRARY) {
                DrawText("GAMES LIBRARY", 200, 20, 30, WHITE);
                
                Rectangle refreshRect = { 200, 60, 120, 30 };
                DrawText("Refresh", refreshRect.x, refreshRect.y, 20, GREEN);
                if (click && CheckCollisionPointRec(m, refreshRect)) {
                    shell.gameList.clear();
                    DIR* d = opendir(g_gamesPath.c_str());
                    if (d) {
                        struct dirent* e;
                        while ((e = readdir(d))) {
                            std::string s = e->d_name;
                            if (s.size() > 4 && s.substr(s.size()-4) == ".lua") shell.gameList.push_back(s);
                        }
                        closedir(d);
                    }
                }

                if (shell.gameList.empty()) {
                    DrawText("Folder empty or not set.", 250, 200, 20, GRAY);
                }

                for (int i = 0; i < (int)shell.gameList.size(); i++) {
                    Rectangle r = { 200, 100.0f + i * 50, 580, 45 };
                    DrawRectangleRec(r, {50, 50, 70, 255});
                    DrawText(shell.gameList[i].c_str(), 220, 112 + i * 50, 20, WHITE);
                    if (click && CheckCollisionPointRec(m, r)) {
                        if (L) lua_close(L);
                        L = luaL_newstate(); luaL_openlibs(L);
                        lua_newtable(L);
                        lua_pushcfunction(L, api_AddBody); lua_setfield(L,-2,"AddBody");
                        lua_pushcfunction(L, api_GetBodyPos); lua_setfield(L,-2,"GetBodyPos");
                        lua_pushcfunction(L, api_SetBodyVel); lua_setfield(L,-2,"SetBodyVel");
                        lua_pushcfunction(L, api_ClearPhysics); lua_setfield(L,-2,"ClearPhysics");
                        lua_pushcfunction(L, api_ClearBG); lua_setfield(L,-2,"ClearBG");
                        lua_pushcfunction(L, api_DrawRect); lua_setfield(L,-2,"DrawRect"); 
                        lua_pushcfunction(L, api_DrawText); lua_setfield(L,-2,"DrawText");
                        lua_pushcfunction(L, api_LoadTexture); lua_setfield(L,-2,"LoadTexture");
                        lua_pushcfunction(L, api_DrawSprite); lua_setfield(L,-2,"DrawSprite");
                        lua_pushcfunction(L, api_IsButtonDown); lua_setfield(L,-2,"IsButtonDown");
                        lua_setglobal(L, "DropCast");
                        std::string p = g_gamesPath + "/" + shell.gameList[i];
                        if (luaL_dofile(L, p.c_str()) == LUA_OK) {
                            lua_getglobal(L, "Init"); lua_pcall(L, 0, 0, 0);
                            shell.currentTab = TAB_PLAYING;
                        }
                    }
                }
            } else if (shell.currentTab == TAB_SETTINGS) {
                DrawText("SETTINGS - CONNECT TO TV", 200, 20, 30, WHITE);
                DrawText(TextFormat("Games Path: %s", g_gamesPath.c_str()), 200, 60, 15, GRAY);
                
                int key = GetCharPressed();
                while (key > 0) {
                    if ((key >= '0' && key <= '9') || key == '.') if (shell.ip.length() < 15) shell.ip += (char)key;
                    key = GetCharPressed();
                }
                if (IsKeyPressed(KEY_BACKSPACE) && shell.ip.length() > 0) shell.ip.pop_back();

                DrawRectangle(200, 85, 280, 35, RAYWHITE);
                DrawText(shell.ip.c_str(), 210, 92, 20, BLACK);

                for (auto& b : numpad) {
                    DrawRectangleRec(b.rect, GRAY);
                    DrawText(b.text, b.rect.x + 25, b.rect.y + 20, 20, WHITE);
                    if (click && CheckCollisionPointRec(m, b.rect)) {
                        if (b.val == '<' && shell.ip.length() > 0) shell.ip.pop_back();
                        else if (b.val != '<' && shell.ip.length() < 15) shell.ip += b.val;
                    }
                }
                
                Rectangle btnConn = { 550, 150, 200, 60 };
                DrawRectangleRec(btnConn, shell.isConnected ? GREEN : MAROON);
                DrawText(shell.isConnected ? "CONNECTED" : "CONNECT TV", 570, 170, 20, BLACK);
                if (click && CheckCollisionPointRec(m, btnConn)) {
                    g_clientAddr.sin_family = AF_INET; g_clientAddr.sin_port = htons(4444);
                    g_clientAddr.sin_addr.s_addr = inet_addr(shell.ip.c_str());
                    shell.isConnected = true;
                    shell.currentTab = TAB_LIBRARY;
                }
            }
            EndDrawing();
        }
    }
    if (L) lua_close(L);
    close(g_sock);
    CloseWindow();
    return 0;
}