player_x = 400
player_y = 280
vel_x = 0
vel_y = 0
gravity = 800
jump_strength = 450
speed = 300

function Init()
end

function Update(dt)
    vel_x = 0
    if DropCast.IsButtonDown(0) then vel_x = -speed end
    if DropCast.IsButtonDown(1) then vel_x = speed end
    
    player_x = player_x + vel_x * dt
    vel_y = vel_y + gravity * dt
    
    if player_y >= 280 and vel_y > 0 then
        vel_y = 0
        player_y = 280
        can_jump = true
    else
        can_jump = false
    end

    if DropCast.IsButtonDown(2) and can_jump then
        vel_y = -jump_strength
    end

    player_y = player_y + vel_y * dt

    -- Send rendering instructions AND velocity to the TV!
    DropCast.ClearBG(135, 206, 235, 255)
    
    -- Draw Floor: x, y, w, h, velX, velY, R, G, B, A
    DropCast.DrawRect(300, 320, 400, 10, 0, 0, 80, 80, 80, 255)
    
    -- Draw Player (We pass vel_x and vel_y so the TV can smooth the lag!)
    DropCast.DrawRect(player_x - 20, player_y - 40, 40, 40, vel_x, vel_y, 0, 0, 139, 255)
    
    DropCast.DrawText("Running from Download/DropCastGames!", 10, 10, 20, 255, 255, 255, 255)
end
