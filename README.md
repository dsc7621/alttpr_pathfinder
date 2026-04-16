# The Legend of Zelda: A Link to the Past Randomizer Pathfinder

## Overview

This is a simple command-line program that lets you load in a randomized (or vanilla, if you want), legally acquired Link to the Past SNES ROM (JP v1.0) and find a path between any two locations.

I built it because I like to play with insanity-level entrance shuffle on, but I hate tediously maintaining a spreadsheet of what goes where.

## Usage

If run without any arguments, it just displays a simple list from which you choose options:

- Load a ROM file
- Print paths from Link's four possible starting locations to a target location
- Print the path between two arbitrary locations
- List all locations
- Quit

It can be run with two arguments, the path to a ROM file and a target location, and this will print the path between all of Link's four possible starting locations to the target location. Example on Windows:

    .\alttpr_pathfinder.exe "C:\Temp\AP_43235001429691667795_P1_P1.sfc" "Palace of Darkness"
    
    Start: Inside Links House
        Leave --> Outside Links House [in region Light World Overworld]
        Go to --> Outside Kakariko Shop
        Enter --> Inside Palace of Darkness
    
    
    
    Start: Inside Sanctuary
        Leave --> Outside Hookshot Cave (South) [in region Dark World Death Mountain (Upper)]
         Jump --> Region Dark World Death Mountain (Lower, East)
        Go to --> Outside Cave Shop (Dark Death Mountain)
        Enter --> Inside Fairy Ascension Cave (Bottom)
        Leave --> Outside Fairy Ascension Cave (Bottom)
        Enter --> Inside Lost Woods Hideout Stump
        Leave --> Outside Lumberjack Tree Cave [in region Light World Overworld]
        Go to --> Outside Kakariko Shop
        Enter --> Inside Palace of Darkness
    
    
    
    Start: Inside Old Man House (Top) [in region Light World Death Mountain Old Man House]
        Go to --> Inside Old Man House (Bottom)
        Leave --> Outside Eastern Palace [in region Light World Overworld]
        Go to --> Outside Kakariko Shop
        Enter --> Inside Palace of Darkness
    
    
    
    Start: Outside Pyramid Hole [in region Dark World Overworld]
        Mirror --> Region Light World Overworld
         Go to --> Outside Kakariko Shop
         Enter --> Inside Palace of Darkness    

Or it can be run with three arguments -- the path to a ROM file, a starting location, and a target location -- to print the path between those two locations. Example on Windows:

    Start: Inside Skull Woods Final Section
          Leave --> Outside Skull Woods Final Section [in region Dark World Lost Woods (North)]
          Go to --> Outside Skull Woods Second Section Hole
        Fall in --> Inside Hyrule Castle (South)
          Leave --> Outside Bat Cave Cave [in region Light World Overworld]
          Go to --> Outside Lumberjack House
          Enter --> Inside Potion Shop

## Known Issue

- Some shops and large fairy fountains may be incorrectly identified.

There are several interior areas that I've only been able to identify by tileset, not specific location. It's not super important, but I'd like to fix it.

## Future

Things this application doesn't do/have, but I would one day like for it to do/have:

1. Have more Light World and Dark World overworld region granularity, to potentially avoid paths requiring you to traverse large sections of the overworld unnecessarily.
2. Allow you to specify what items you do/don't have and have connections between areas be conditional.
3. Connect to Archipelago and simply fetch your current inventory.
4. Should probably, like, write some tests or something.

## Disclaimer

None of this code was LLM-written.