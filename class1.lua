function sol.main:on_started()
    -- This function is called when Solarus starts.
    print("This is a sample quest for Solarus.")
    sol.test.show()

    -- First Class Bind Method


    -- Second Class Bind Method
    local sprite = Sprite2:new(1, 2)
    local posx = sprite:getx()
    print(posx)
    function sprite:dosome()
        print("do some")
    end
    sprite = nil
    collectgarbage("collect");
    devsp = Sprite2:new(3, 4)
    devsp:getx()
    -- devsp:dosome()
end