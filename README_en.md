[[Tiếng Việt]](README.md)
# Plants Versus Zombies - Kaito-NMH-Edition
# Game Introduction
Plants versus Zombies is a tower defense game that immerses players in a battle between plants and zombies. Players must do their best to protect their house using only strange types of plants. By planting and harvesting these plants, players can destroy the various types of zombies that are attacking their house.

- [0. How to download the game](#0-how-to-download-the-game)
  * [a. Option 1: Without source code.](#a-option-1-without-source-code)
  * [b. Option 2: With source code and can be compiled.](#b-option-2-with-source-code-and-can-be-compiled)
- [1. Starting the game](#1-starting-the-game)
- [2. Choosing a level](#2-choosing-a-level)
- [3. Components of the game:](#3-components-of-the-game)
- [4. How to play](#4-how-to-play)
  * [a. Types of plants](#a-types-of-plants)
  * [b. Types of zombies](#b-types-of-zombies)
- [5. Winning and losing](#5-winning-and-losing)
- [About the game's graphics:](#about-the-games-graphics)
- [About the game's source code](#about-the-games-source-code)


# 0. How to download the game
## a. Option 1: Without source code.
Download the game (compressed into a .rar file) at the following link: https://drive.google.com/drive/u/0/folders/1awM0sfex1InCzlSA1ZIenKUGbhhC_hc0<br/>
This method saves a lot of memory and download time (around 90MB).<br/>
Extract the game into a folder and run pvz.exe to play.<br/>

## b. Option 2: With source code and can be compiled.
**Step 1:** Clone this repo (About 800 MB - includes version history). <br/>
Or select Code -> Download Zip (About 90MB)

![image](https://user-images.githubusercontent.com/65478890/236433509-2d4e5f97-ddc4-4daa-ae8f-9119c31b8d29.png)

**All the code is in the src folder.<br/>
**You can skip to step 5 to start playing.

**Step 2:** Install MinGW. Here, I install the MinGW-W64 GCC-8.1.0 [x86_64-posix-seh](https://sourceforge.net/projects/mingw-w64/files/Toolchains%20targetting%20Win64/Personal%20Builds/mingw-builds/8.1.0/threads-posix/seh/x86_64-8.1.0-release-posix-seh-rt_v6-rev0.7z/download) version.<br/>
Some notes: <br/>
- If you use another compiler, you need to recompile the icon: go to the icon folder and use the command `windres main.rc -O coff main.res` in cmd.<br/>
- You need to delete three c++ dll files in the folder containing pvz.exe: `libgcc_s_seh-1.dll`, `libstdc++-6.dll`, `libwinpthread-1.dll`. These dll files are only for players who don't have c++ installed on their computer.

**Step 3:** Install GNU make.

**Step 4:** Open cmd in the folder containing the Makefile and type the command "make".

**Step 5:** Run pvz.exe and start playing.

# 1. Starting the game
Click on the "CLICK TO START!" button to start playing the game.

![image](https://user-images.githubusercontent.com/65478890/235644003-664c5a3f-65e3-4f55-a308-1388f379226f.png)

When entering the game for the first time, players should enter their name to start the game. Then click "OK" or press Enter to continue.

  ![image](https://user-images.githubusercontent.com/65478890/235644015-ef6de0e5-1f8a-46de-bd92-6b626c9327c2.png)

Players will immediately receive their first plant - the Peashooter.

![image](https://user-images.githubusercontent.com/65478890/235644035-f2874cd6-d245-4651-b140-aad2575600ac.png)

# 2. Choosing a level
There are a total of 12 levels with varying levels of difficulty for players to choose from. When hovering over a level, that level will light up. Simply click the mouse to play that level.

![image](https://user-images.githubusercontent.com/65478890/235644049-1eb15dbb-4df4-44b3-a089-e1bee0b1ebee.png)

However, when starting out, you can only unlock 1 level. Win each level in order to unlock and play the next level:

![image](https://user-images.githubusercontent.com/65478890/235644063-6498c78f-1215-4c73-90d5-12c78b098d4e.png)

In addition, there are 3 other function buttons for players to change their name, close the game, or even reset the game progress to experience the game again from the beginning:

![image](https://user-images.githubusercontent.com/65478890/235644088-098a0d06-de52-4b9b-8e23-f455681d954b.png)

When entering the game, there are 2 types of levels that players will encounter: morning (left side) and night (right side).

![image](https://user-images.githubusercontent.com/65478890/235644138-5a1078e7-c457-4f9a-8c1f-14ff6f799d73.png)

- In the morning, there will occasionally be sun (sun) falling from the sky.
- In the night, there is none, so the game will be slightly harder.

Before starting the game, you will know what types of zombies will appear.

![image](https://user-images.githubusercontent.com/65478890/235644159-c816334a-4125-434c-ad80-3dc7a9f1724a.png)

# 3. Components of the game:
- Sun: a unit to buy plants in the game. Click to collect. And note that if a sun exists for too long, it will gradually fade and disappear. You won't be able to collect them after they disappear.

![image](https://user-images.githubusercontent.com/65478890/235644190-003a31d5-e403-4f3e-9ecb-7edf40ad0eff.png)

- Sun bar: a scoreboard of the number of suns that the player currently has.

![image](https://user-images.githubusercontent.com/65478890/235644218-cdeb1b32-201a-418c-b2aa-4d8269a5b63e.png)

- Plant seeds: all types of plants that the player owns. To select a plant to plant, the player must click on the plant seed or press keys 1, 2, 3, 4, 5 (as marked in the game) to select the plant and then plant it on the field. Of course, you must have enough sun to buy the plant, and the plant seed must have a different waiting time to plant that type of plant again.

![image](https://user-images.githubusercontent.com/65478890/235644263-1622c04b-d871-43f6-a489-9b4bff17780b.png)

- Shovel: an extremely effective tool for you to dig up unnecessary plants and replace them with new ones.

![image](https://user-images.githubusercontent.com/65478890/235644282-0532175a-5a42-4703-a621-44c645b39192.png)

- Game progress bar: a bar that you can follow to see how many zombies are left to come or how many parts of the level you have played. On the left is the current level you are playing.

![image](https://user-images.githubusercontent.com/65478890/235644299-10aba29c-3230-4051-81b9-f15dca3bab76.png)

- Turbo button: If you find the game too slow or too easy and you need to save time, press the button to experience the game at twice the original speed.
- And if you find the game too scary at this speed, just press it again to return to normal speed.
● Game at the original speed: ![image](https://user-images.githubusercontent.com/65478890/235644327-f6565bed-b5bb-430c-b257-7de0b6b697e0.png)

● Game currently at twice the original speed: ![image](https://user-images.githubusercontent.com/65478890/235644339-df10c132-2432-4b48-840a-897e9fbab4b1.png)


- Pause Button: Do you need to pause the game to play later? Or do you want to adjust the volume of the background music and sound effects? Too simple, the game can be paused by clicking ![image](https://user-images.githubusercontent.com/65478890/235644416-9889bcca-ff91-4a8f-aa6d-b5fc2b4f1d41.png) or clicking outside the game.
- Mower: If you leave zombies too close to your house, the mower will automatically activate and destroy all zombies currently in that row. Note that there is only one mower per row.

 ![image](https://user-images.githubusercontent.com/65478890/235644448-0aeeb06a-e177-4fd0-95c5-e0baee99b2d0.png)


- Basic game layout:

 ![image](https://user-images.githubusercontent.com/65478890/235644476-8fa38f42-60b6-4ea2-8fc0-c40a09353ed8.png)


# 4. How to play
Plant plants to attack and defend, and prevent zombies from entering your house.<br/>
To plant a plant, select the seed of that plant (using the keyboard or mouse) and place it in the desired square. <br/>
You can dig up plants in use with a shovel.<br/>
Plants will be eaten by zombies.

## a. Types of plants
There are 5 types of plants, in order of availability in the game:

||Plant Name	|Function|
|-|-|-|
|![image](https://user-images.githubusercontent.com/65478890/235644737-20b7af78-0629-41c1-aa9e-455a9377eaff.png)|Peashooter	|Shoots peas at zombies.|
|![image](https://user-images.githubusercontent.com/65478890/235644753-b66d018a-3ffe-42fd-906c-28cf985c27b6.png)|Sunflower	|Produces suns that can be used to plant more plants.|
|![image](https://user-images.githubusercontent.com/65478890/235644793-a5039d3d-dfaa-4721-a5c7-4f64ddc63d7b.png)|Wall-nut	|Takes a lot of hits before breaking, protecting other plants.|
|![image](https://user-images.githubusercontent.com/65478890/235644820-78cdb1cc-a30e-4b2e-898d-99c3456bc634.png)|Snow Pea	|Shoots frozen peas that slow down zombies and reduce their attack speed, but don't deal extra damage.|
|![image](https://user-images.githubusercontent.com/65478890/235644838-96dcb09f-a49a-4cc0-bfab-459eecd4b040.png)|Cherry Bomb	|Explodes and kills all zombies in a 3x3 area.|

## b. Types of zombies
-	There are 5 types of zombies:

||Zombie Name 	|Function|
|-|-|-|
|![image](https://user-images.githubusercontent.com/65478890/235644897-e0f7fb94-3eee-4c20-8b13-751837dff84b.png)|Browncoat Zombie	|Eats your plants and brains.|
|![image](https://user-images.githubusercontent.com/65478890/235644926-9ba95a6c-e4e9-4df3-b964-59e1dd2e0c81.png)|Flag Zombie	|Signals a large wave of zombies incoming.|
|![image](https://user-images.githubusercontent.com/65478890/235644956-95940313-e349-4ab2-a669-818030be4f5a.png)|Conehead Zombie	|Has three times the health of a regular zombie.|
|![image](https://user-images.githubusercontent.com/65478890/235644975-45e2618b-84e7-4e5a-88f0-32228dbb2c76.png)|Buckethead Zombie	|Has 6.5 times the health of a regular zombie.|
|![image](https://user-images.githubusercontent.com/65478890/235644995-cc8c1f86-335e-4272-a2ec-3af6b88dfca5.png)|Door Zombie	|Has 6.5 times the health of a regular zombie and is not affected by frozen peas as long as its door is still intact. |

# 5. Winning and losing
- You will win if you eliminate all zombies in that game.

![image](https://user-images.githubusercontent.com/65478890/235645245-c6c311d6-74b9-4fe2-b76a-930781573535.png)


- You will lose if a zombie goes all the way to your house.

![image](https://user-images.githubusercontent.com/65478890/235645414-1bbb03a1-4f25-42e0-b9cf-3267d4d11c27.png)

---
### About the game's graphics:
- The parts of each plant (eyes, leaves, head, stem, etc.), zombies (eyes, hair, head, teeth, body, arms, legs, etc.), and other components are taken from [fandom](https://plantsvszombies.fandom.com/wiki/Main_Page).<br/>
- These parts are assembled using Adobe Animate and edited by Adobe Photoshop to create a gif file.<br/>
- To create a sprite sheet, use the website https://onlinegiftools.com/convert-gif-to-sprite-sheet to convert from gif. <br/>

###About the game's source code:

- Folder "draw":
    * initializes window and renderer
    * contains all images in the game (numbered)
    * contains most of the SDL2, SDL2_image, SDL2_ttf commands, used to load images, text and draw on the renderer, window.
- Folder "elements": Contains all components of the game:
    * "bullets": types of bullets
    * "button": types of buttons in the game (can be pressed or not)
    * "Map": contains random function and initializes a 5x9 board for each level.
    * "mower": grass cutter (zombie's obstacle)
    * "plants": contains all types of plants (inherited from Plants class)
    * "zombies": contains all types of zombies (inherited from Zombie class)
    * "elements": gathers all types of plants and zombies.
    * "elements_actions":
        + contains most of the interactions between game components, and with the level.
        + creates zombies randomly
        + updates all game changes every frame.
- Folder "events": Handles game events (mouse, keyboard events)
    * "in_game": handles events during a level
        + "credit": introduces different types of zombies and the concept of "ready-set-plant"
        + "game": handles all events during a level
        + "lose": handles the event of losing
        + "main_menu": handles the event of returning to level selection menu
        + "pause_menu": handles the event of pausing the game (either by pressing the pause button or clicking outside of the game)
        + "restart_menu": handles the event of restarting a level
        + "turbo": handles the event of speeding up the game
        + "volume": handles the event of adjusting background music and sound effects volume.
        + "win": handles the event of winning the game.
    * "mouse_cursor": handles the mouse cursor
    * "out_game": handles events outside of a level
       + "choose_level": handles level selection
       + "quit_menu": handles the event of quitting the game
       + "reset_level": handles the event of resetting a level's progress
       + "start_game": handles the event of clicking "Click to start" to enter a level.
       + "unlock_plant": handles the event of unlocking a new plant.
- Folder "level": handles retrieving level data.
- Folder "music": handles game's sound - SDL2_mixer
- Folder "player": handles player data. Includes retrieving and changing name, progress.
- "game_stats.hpp": contains all game parameters.
- "game.hpp" and "game.cpp": main function of the game.
