-- DropCast Snake Game
local grid_size = 20
local screen_w = 800
local screen_h = 450

-- State variables
local snake = {}
local dir_x = 1
local dir_y = 0
local food_x = 0
local food_y = 0
local score = 0
local game_over = false
local timer = 0
local move_delay = 0.12

-- Input tracking so we only turn once per button press
local prev_left = false
local prev_right = false
local prev_jump = false

function SpawnFood()
    -- Calculate max grid positions (0-39 on X, 0-21 on Y)
    -- math.random(0, max) requires Lua to have math seed set
    food_x = math.random(0, (screen_w / grid_size) - 1) * grid_size
    food_y = math.random(0, (screen_h / grid_size) - 1) * grid_size
end

function Reset()
    snake = { {x=400, y=200}, {x=380, y=200}, {x=360, y=200} }
    dir_x = 1
    dir_y = 0
    score = 0
    move_delay = 0.12
    game_over = false
    math.randomseed(os.time() or 12345)
    SpawnFood()
end

function Init()
    Reset()
end

function Update(dt)
    -- 0: Left Button, 1: Right Button, 2: Jump Button
    local left_down = DropCast.IsButtonDown(0)
    local right_down = DropCast.IsButtonDown(1)
    local jump_down = DropCast.IsButtonDown(2)

    if game_over then
        -- Restart the game if Jump is pressed
        if jump_down and not prev_jump then
            Reset()
        end
    else
        -- STEERING LOGIC (Turn Left / Counter-Clockwise)
        if left_down and not prev_left then
            local temp_x = dir_x
            dir_x = dir_y
            dir_y = -temp_x
        end
        
        -- STEERING LOGIC (Turn Right / Clockwise)
        if right_down and not prev_right then
            local temp_x = dir_x
            dir_x = -dir_y
            dir_y = temp_x
        end

        timer = timer + dt
        if timer >= move_delay then
            timer = timer - move_delay

            -- Calculate new head position
            local head = snake[1]
            local new_head = { x = head.x + dir_x * grid_size, y = head.y + dir_y * grid_size }

            -- Check Wall Collisions
            if new_head.x < 0 or new_head.x >= screen_w or new_head.y < 0 or new_head.y >= screen_h then
                game_over = true
            end

            -- Check Self Collisions
            for i = 1, #snake do
                if snake[i].x == new_head.x and snake[i].y == new_head.y then
                    game_over = true
                end
            end

            -- Move the snake forward
            if not game_over then
                table.insert(snake, 1, new_head)
                
                -- Check Food Collision
                if new_head.x == food_x and new_head.y == food_y then
                    score = score + 10
                    SpawnFood()
                    -- Speed the game up slightly every time you eat!
                    if move_delay > 0.04 then 
                        move_delay = move_delay - 0.002 
                    end
                else
                    table.remove(snake) -- Erase the tail block
                end
            end
        end
    end

    -- Save button states for next frame
    prev_left = left_down
    prev_right = right_down
    prev_jump = jump_down

    -- ==========================
    -- RENDER COMMANDS FOR TV
    -- ==========================
    
    -- Clear background to a retro dark green
    DropCast.ClearBG(15, 35, 15, 255)

    -- Draw Food (Red)
    DropCast.DrawRect(food_x, food_y, grid_size, grid_size, 0, 0, 255, 50, 50, 255)

    -- Draw Snake (Bright Green Head, Darker Green Body)
    for i = 1, #snake do
        local s = snake[i]
        local r, g, b = 50, 200, 50 -- Body color
        if i == 1 then r, g, b = 100, 255, 100 end -- Head color
        if game_over then r, g, b = 100, 100, 100 end -- Turn grey if dead
        
        -- grid_size - 1 creates a nice grid pattern between the snake blocks
        DropCast.DrawRect(s.x, s.y, grid_size - 1, grid_size - 1, 0, 0, r, g, b, 255)
    end

    -- Draw UI Text
    DropCast.DrawText("SCORE: " .. tostring(score), 10, 10, 20, 255, 255, 255, 255)
    DropCast.DrawText("Use < and > to steer!", 550, 10, 20, 150, 150, 150, 255)

    if game_over then
        DropCast.DrawText("GAME OVER!", 320, 200, 30, 255, 50, 50, 255)
        DropCast.DrawText("PRESS '^' TO RESTART", 280, 240, 20, 200, 200, 200, 255)
    end
end
