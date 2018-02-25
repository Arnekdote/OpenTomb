-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER, LEVEL10A

print("Level script loaded (LEVEL10A.lua)");

level_PostLoad = function()

end;

level_PreLoad = function()
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
    static_tbl[06] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};           -- Hanging plant
    static_tbl[08] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};           -- Hanging plant
    static_tbl[10] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};       -- Wood barrier
    static_tbl[33] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};       -- Bridge part 1
    static_tbl[34] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};       -- Bridge part 2
    static_tbl[38] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};           -- Door frame
    static_tbl[39] = {coll = COLLISION_GROUP_STATIC_OBLECT, shape = COLLISION_SHAPE_TRIMESH};       -- Wall bricks
    static_tbl[43] = {coll = COLLISION_NONE,                shape = COLLISION_SHAPE_BOX};           -- Icicle
end;


function natla_cabin_TR1_init(id)

    entity_funcs[id].onActivate = function(object_id, activator_id)
        local st = getEntityAnimState(object_id, ANIM_TYPE_BASE);
        local a, f, c = getEntityAnim(object_id, ANIM_TYPE_BASE);
        setEntityAnim(object_id, ANIM_TYPE_BASE, a, c - 1, c - 1);  -- force it to avoid broken state change
        setEntityAnimState(object_id, ANIM_TYPE_BASE, st + 1);
        return ENTITY_TRIGGERING_ACTIVATED;
    end

    entity_funcs[id].onLoop = function(object_id, tick_state)
        if(getEntityEnability(object_id)) then
            local st = getEntityAnimState(object_id, ANIM_TYPE_BASE);
            if(st == 4) then
                disableEntity(object_id);
                setFlipMap(0x03, 0x1F, TRIGGER_OP_XOR);    --setFlipMap(flip_index, flip_mask, TRIGGER_OP_OR / XOR)
                setFlipState(0x03, 1);                     --setFlipState(flip_index, FLIP_STATE_ON)
                entity_funcs[id].onLoop = nil;
            end;
        else
            entity_funcs[id].onLoop = nil;
        end;
    end
end


function anim_single_init(id)      -- Ordinary one way animatings

    setEntityTypeFlag(id, ENTITY_TYPE_GENERIC);
    setEntityActivity(id, false);

    entity_funcs[id].onActivate = function(object_id, activator_id)
        setEntityAnimState(object_id, ANIM_TYPE_BASE, 1);
        setEntityActivity(object_id, true);
        return ENTITY_TRIGGERING_ACTIVATED;
    end;

    entity_funcs[id].onDeactivate = function(object_id, activator_id)
        setEntityActivity(object_id, false);
        return ENTITY_TRIGGERING_DEACTIVATED;
    end;

    entity_funcs[id].onLoop = function(object_id, tick_state)
        local x, y, z = getEntityPos(object_id);
        if(x > 48896) then
            x = x - 1024.0 * frame_time;
            if(x < 48896) then
                x = 48896;
            end;
            setEntityPos(object_id, x, y, z);
        else
            entity_funcs[id].onLoop = nil;
        end;
    end;
end
