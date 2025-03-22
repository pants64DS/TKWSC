The Koopa Who Stole Christmas v1.1.2
-----------------------------------
A Christmas SM64DS mod by Hailbot and the Super Mario 64 DS Hacking Community!

Mario and friends have come together this holiday season to celebrate the holidays with a Christmas party! However, Bowser has shown up 
and stolen all of the Christmas decorations and everyone's holiday spirit! Can you help the Mario crew stop Bowser and save this Christmas 
from falling apart?

Patch Instructions:
    1. Open the included xdeltaUI.exe
    2. In "patch", select the .xdelta patch that matches with your ROM type/region (US V1, EUR, etc).
    3. In "Source File", select your unaltered clean Super Mario 64 DS.nds ROM.
    4. In "Output File", set up the output ROM name and location.
    5. Hit patch and enjoy!

THIS GAME CAN BE PLAYED WITH FULL JOYSTICK CONTROLS ON PC!!!!!!!!!!!!!!

ToxInput Instructions:
    1. Open The Koopa Who Stole Christmas in the included version of DeSmuMe.
    2. Open the included ToxInput.exe.
    3. Select your controller in ToxInput (Must be an Xinput controller)
    4. IN TOXINPUT, select the emulator window that is running TKWSC.
    5. Restart The Koopa Who Stole Christmas in DeSmuMe.
    6. You should now have proper joystick input!


Crash Reporting Instructions:

DO NOT CLOSE THE GAME before taking a picture of the screen (or a screenshot if you're using an emulator).
When the game crashes, it should display a debug screen automatically, unlike in vanilla SM64DS.
The debug screen contains crucial information that can help us to fix the crash.
Make sure that all numbers on the debug screen are clearly visible in the picture.
Send the picture to pants64 on Discord and describe what happened in-game before the crash.
(That's his exact Discord tag which you can use to add him as a friend if you don't share a server with him.)

Reporting other bugs:

If you notice something in the game that doesn't seem intentional, tell that to pants64 on Discord.
    - That's his exact Discord tag which you can use to add him as a friend if you don't share a server with him.
    - If the issue is hard to describe accurately, sending a screenshot or a picture of the screen might help.


The source code of this hack is available at https://github.com/pants64DS/TKWSC

Credits:

Hailbot: Project lead, contest manager, hub areas, level before final boss, title screen graphics, secret level, polishing other courses, missions, main story, credits cutscene

pants64: Final boss, ending cutscene, engine optimization, Launch Star and Star Chip models, custom secret number colors, various ASM patches, debugging, bugfixing

Splatty: Frozen Frostbite Land, different Yoshi colors, cutscenes, Snow Pokey texture, Star Chip code, Colored Goomba, Character Block, Colored Coin, custom NPCs, other custom objects

ShaneMGD: Frostbite Summit, release trailer

HayashiSTL: Dynamic library loader, Launch Star code, Star Chip sound, Yoshi NPC, Magikoopa, Shy Guy, Colored Goomba, Berry, other custom objects

Gota7: Note Block, Shrinking Platform, Object Lighting Modifier, other custom objects

Floralz: Winter at Mount Bob-omb, Chain Chomp’s Den, up-down lift platform texture, door wreath texture

KingYoshi a.k.a. Eugene: Winter at Mount Bob-omb music

0reo: Frosty Town

chico88959: Santa's Backup

Confidan: Shifting Snow Land

McBiggie: Koopa Mall

siba1208: Snowy Town

RMY: The Lost Valley

SawyerR13 Mt. Shiver

Rafa4k: Frosty Town music



Version History:
v1.1.2          3/26/25
    - Fixed a bug in Frosty Town where an NPC would spawn a star after using a Blue Coin switch
    - The debug screen now opens automatically in the event of a crash
    - Improved the Kamella fight in Frozen Frostbite Land and fixed a crash that happened in it
    - Added HUD counters for different missions in Mount Bobomb and Frozen Frostbite Land
    - Removed the coin counter and 100 coin stars in levels with death coins
        - Death coin stars are counted as 100 coin stars, so the total number of stars doesn't change
    - Fixed a bug where the player would warp long distances after throwing a Yoshi Egg far away
    - Fixed an issue where the player's head would be visible shortly before spawning from a story book
    - Ice Coins now count when eaten by Yoshi
    - The life count is now saved in the save data
        - Previous save files will start with four lives
        - The life count is still reset to four when getting a "game over"
    - Fixed a bug in Mt. Shiver where the Blue Coin switch and the !-switch would trigger the effects of each other
    - The shadows of colored coins now behave like those of normal coins when Yoshi eats them
    - Fixed a bug in Frozen Frostbite Land where the number of berries eaten would not reset properly when leaving the level
    - Fixed an issue where the ROM icon and banner would not display correctly on certain platforms

v1.1.1          4/7/24
    - Removed weird clear walls on the boundaries of Snowy Town to stop wireframe effect on real hardware
    - Made it so that secret in Snowy Town appears in all missions so people stop reporting it as a bug
    - Made it so that the small slope at the start of Mount Bobomb is unslippable
    - Updated Confidan's name on the Shifting Snow Land book
    - Fixed Bully audio in Frostbite Summit
    - Fixed Silver Star jingle audio in Frosty Town, Santa's Backup, and Shifting Snow Land
    - Peach's painting in the inside hub now only appears after you beat Bowser the first time
    - Made Peach painting in the inside hub not stretched
    - Fixed Z Fighting on stone cave path in Mount Bob-omb and Mount Bob-omb Cave on real hardware
    - Fixed ALL audio problems in Koopa Mall
    - Delfino Plaza no longer forces you to leave the level when collecting a star
    - Fixed roofs in Snowy Town
    - Fixed all solid surfaces you could get stuck in for every level
    - Fixed wall UVs in Snowy Town
    - Lighthouse glass in Bowser In The Cold Winds made easier to see

v1.1.0          12/31/23
    - Fixed a memory leak in the ridable Yoshi code
    - When hopping on a ridable Yoshi, a Yoshi voice clip is now played
    - When hopping on a ridable Yoshi, the direction the Yoshi faces no longer changes
    - When dismounting from a ridable Yoshi, the direction the Yoshi faces no longer changes
    - When riding a Yoshi, the Yoshi's shadow is no longer cast on the rider
    - When riding a Yoshi, Yoshi's voice is now used instead of the rider's voice
    - Prevented Metal Wario and Vanish Luigi from hopping on a ridable Yoshi
    - Fixed a bug that made Wario unable to break black brick blocks by punching
    - Fixed a bug in Shifting Snow Land that made some objects intangible
    - Changed tree particles to snow in all levels
    - Allowed long jumping off of colored Goombas
    - Fixed a bug that made death coins stop working when taking damage so that only 1/8 of the health meter is left
    - Added sign post to intentional hub out of bounds
    - Removed the empty page in the 8 star text box
    - Removed an extra line in one of the save dialogs
    - Removed extra space from one of Blue Toad's dialogues
    - Fixed the word "good" missing from one of Green Toad's dialogues
    - Removed the stars spawned by NPCs in Mt. Shiver
    - Removed the block switch from Snowy Town in missions where its star can't be collected
    - Made the 1-UP text and silver star number models not use lighting
    - Made the 1-UP Mushrooms use lighting
    - Finished the 100% and 101% hub dialogues
    - Removed 6th Silver Star from Delfino Plaza
    - Fixed Silver Star jingle in Mount Bobomb

v1.0.2          12/27/23
    - Fix the patch for the US v1 ROM (it was actually for the US v2 ROM previously)
    - No changes were made to the resulting ROM

v1.0.1          12/24/23
    - Fix a crash caused by a bug in Shy Guy's code
