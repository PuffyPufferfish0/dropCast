-- HolyOriface Dungeon Crawler
-- Step Two: Sprites, Animation, and Rooms

local state = "MAIN"
local selection = 0
local pl, pr, pj = false, false, false

-- Physics & Rendering
local p_id = -1
local g_id = -1
local texture_loaded = false

-- Metroidvania Map Data
local room_x = 0
local room_y = 0

-- === SPRITE TUNING VARIABLES ===
-- 1. Source Size (Increase these if Richard is being cut off/cropped)
local frame_w = 200 
local frame_h = 200 

-- 2. Destination Size (Decrease these to shrink him on the TV screen)
local draw_w = 64 
local draw_h = 64 

-- 3. Visual Offset (Shifts the sprite so his feet touch the C++ physics floor)
local offset_x = -16 
local offset_y = -16
-- ===============================

-- Richard Animation Data
local frame_timer = 0
local current_frame = 0
local flip_x = 1

local function DrawMenu(text, idx, x, y)
    local color = {200, 200, 200}
    local prefix = ""
    if selection == idx then 
        color = {255, 50, 50}
        prefix = "> "
    end
    DropCast.DrawText(prefix .. text, x, y, 20, color[1], color[2], color[3])
end

local function LoadRoom(startX, startY)
    DropCast.ClearPhysics()
    g_id = DropCast.AddBody(0, 400, 800, 50, true)
    
    -- The C++ Physics hitbox remains a tight 32x48 so he fits through doors
    p_id = DropCast.AddBody(startX, startY, 32, 48, false)
end

function Init()
    state = "MAIN"
    selection = 0
    texture_loaded = false
end

function Update(dt)
    if not texture_loaded then
        -- Updated to look for the transparent .png!
        DropCast.LoadTexture(1, "HolyOriface/Richard_SpriteSheet.png")
        texture_loaded = true
    end

    local l = DropCast.IsButtonDown(0)
    local r = DropCast.IsButtonDown(1)
    local j = DropCast.IsButtonDown(2)

    if state ~= "PLAY" then
        if l and not pl then selection = selection - 1 end
        if r and not pr then selection = selection + 1 end
        if selection < 0 then selection = 1 end
        if selection > 1 then selection = 0 end
    end

    if state == "MAIN" then
        if j and not pj then
            if selection == 0 then
                room_x, room_y = 0, 0
                LoadRoom(50, 200)
                state = "PLAY"
            end
        end
    
    elseif state == "PLAY" then
        local px, py, vx, vy = DropCast.GetBodyPos(p_id)
        local mvx = 0
        
        if l then 
            mvx = -250 
            flip_x = -1 
        end
        if r then 
            mvx = 250 
            flip_x = 1  
        end
        
        local mvy = vy
        if j and (vy > -2 and vy < 2) then mvy = -550 end
        DropCast.SetBodyVel(p_id, mvx, mvy)

        if mvx ~= 0 and vy == 0 then
            frame_timer = frame_timer + dt
            if frame_timer > 0.15 then
                current_frame = (current_frame + 1) % 4
                frame_timer = 0
            end
        else
            current_frame = 0 
        end

        if px > 800 then
            room_x = room_x + 1
            LoadRoom(10, py) 
        elseif px < -32 and room_x > 0 then
            room_x = room_x - 1
            LoadRoom(780, py)
        end
    end

    -- === RENDERING ===
    DropCast.ClearBG(30, 20, 40)
    
    if state == "MAIN" then
        DropCast.DrawText("H O L Y   O R I F A C E", 230, 100, 35, 255, 215, 0)
        DrawMenu("START", 0, 360, 200)
        DrawMenu("SETTINGS", 1, 340, 250)
    elseif state == "PLAY" then
        local px, py, vx, vy = DropCast.GetBodyPos(p_id)
        
        DropCast.DrawRect(0, 400, 800, 50, 0, 0, 60, 50, 70)
        
        -- Draw the C++ Hitbox for debugging (Optional, uncomment to see it)
        -- DropCast.DrawRect(px, py, 32, 48, vx, vy, 255, 0, 0, 100) 
        
        -- RICHARD RENDERER (Using the tuning variables!)
        local src_x = current_frame * frame_w
        local src_y = 0 
        
        DropCast.DrawSprite(1, src_x, src_y, frame_w * flip_x, frame_h, px + offset_x, py + offset_y, draw_w, draw_h, vx, vy)
        
        DropCast.DrawText("Underworld Map: X=" .. tostring(room_x) .. " Y=" .. tostring(room_y), 10, 10, 20, 120, 120, 120)
    end

    pl, pr, pj = l, r, j
end