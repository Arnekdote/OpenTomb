-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2 GM, LEVEL1.TR2
print("level/tr2/level1->level_loaded !");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR2));
end;

level_PreLoad = function()
    --------------------------------------------------------------------------------
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;