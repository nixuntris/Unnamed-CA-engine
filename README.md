
# REV
A physics engine that implements celullar automata for per pixel simulation inspired by Falling Everything engine developed by Nolla Games and a sound simulation engine that changes depending on the material.
It's currently on the shelf, I want to play around with other ideas rn.
<img width="1915" height="1077" alt="Zrzut ekranu 2026-07-21 194615" src="https://github.com/user-attachments/assets/0536893f-17dd-4f97-9d0b-49cb57721115" />


https://github.com/user-attachments/assets/37935621-160a-4eb9-bd3d-e7e7144120fc


## Features
It utilizes a little descriptor system that has an demo stored in tile_set.txt to define how each cell behaves. You can combine characteristics to make new particles.
# Example usage:
    sand:
    weight = 3
    name = sand
    falls = true
    goes_to_sides = false
    color = (194,178,128,255)
    dissolve = None
    fluid = false
    stone:
    falls = false
    goes_to_sides = false
    weight = 100
    name = stone
    fluid = false
    color = (130,130,130,255)
    dissolve = None

It's case sensitive and space sensitive

## Giving Feedback
If you want to report a bug, think a feature would be neat and want to see it implemented make an issue on github.

## Terms Of Usage
You're free to use it however you want as long as you credit the original creator (nixuntris) as the base you worked off or integrated into your engine.

## Joining The Project ?
I am currently working on it solo and I want to keep working solo on the core physics systems, however if you want to help with the GUI or sound, you're free to add me on discord. The nickname is the same as on github(nixuntris)

## Installation
In the releases tab on github there is a pre-made demo to play around with it. 

## Compiling From Source
The project uses the base setup example of raylib. Linked to where it's installed by the raylib installer.
It's only targeted to windows. If you don't have it downloaded, here's a link:
https://raysan5.itch.io/raylib
Using Visual Studio Code:
Open it in the editor
Install the C/C++ extension if not already installed
Press F5 to build and run
Select C++ (GDB/LLDB) when prompted
## Dependencies 
Raylib 5.5
GCC C++14
