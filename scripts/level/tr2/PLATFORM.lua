-- OPENTOMB LEVEL SCRIPT
-- FOR TOMB RAIDER 2, PLATFORM.TR2
print("level/tr2/platform->level_loaded !");

level_PostLoad = function()
    addCharacterHair(player, getHairSetup(HAIR_TR2));
    playStream(58);
end;

level_PreLoad = function()
    --------------------------------------------------------------------------------
    -- STATIC COLLISION FLAGS ------------------------------------------------------
    --------------------------------------------------------------------------------
end;