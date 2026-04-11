-- HolyOriface Dungeon Crawler
local state = "MAIN"
local selection = 0
local p_id = -1
local g_id = -1

-- Input Debounce
local pl, pr, pj = false, false, false

function Init()
    state = "MAIN"
    selection = 0
end

function StartGame()
    DropCast.ClearPhysics()
    -- AddBody params: x, y, w, h, isStatic
    g_id = DropCast.AddBody(0, 400, 800, 50, true)
    p_id = DropCast.AddBody(380, 200, 32, 48, false)
    state = "PLAY"
end

function Update(dt)
    -- Inputs: 0=Left, 1=Right, 2=Jump
    local l = DropCast.IsButtonDown(0)
    local r = DropCast.IsButtonDown(1)
    local j = DropCast.IsButtonDown(2)

    if state == "MAIN" then
        if l and not pl then selection = 0 end
        if r and not pr then selection = 1 end
        if j and not pj then
            if selection == 0 then
                StartGame()
            end
        end
    elseif state == "PLAY" then
        local px, py, vx, vy = DropCast.GetBodyPos(p_id)
        local mvx = 0
        if l then mvx = -250 end
        if r then mvx = 250 end
        
        local mvy = vy
        -- Jump if velocity is near zero (grounded)
        if j and (vy > -2 and vy < 2) then
            mvy = -450
        end
        
        DropCast.SetBodyVel(p_id, mvx, mvy)
    end

    -- Render Commands
    DropCast.ClearBG(30, 20, 40)
    
    if state == "MAIN" then
        DropCast.DrawText("HOLY ORIFACE", 280, 100, 35, 255, 215, 0)
        
        local color1 = {255,255,255}
        if selection == 0 then color1 = {255, 50, 50} end
        DropCast.DrawText("START ADVENTURE", 300, 200, 20, color1[1], color1[2], color1[3])
        
        local color2 = {255,255,255}
        if selection == 1 then color2 = {255, 50, 50} end
        DropCast.DrawText("SETTINGS", 340, 240, 20, color2[1], color2[2], color2[3])
    elseif state == "PLAY" then
        local px, py, vx, vy = DropCast.GetBodyPos(p_id)
        DropCast.DrawRect(0, 400, 800, 50, 0, 0, 60, 50, 70)
        DropCast.DrawRect(px, py, 32, 48, vx, vy, 180, 180, 200)
        DropCast.DrawText("UNDERWORLD - DEPTH 1", 10, 10, 20, 120, 120, 120)
    end

    pl, pr, pj = l, r, j
end