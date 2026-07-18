# REV
A physics engine that implements celullar automata for per pixel simulation inspired by Falling Everything engine developed by Nolla Games and a sound simulation engine that changes depending on the material.

## Features
It utilizes a little descriptor system that has an example stored in tile_set.txt to define how each cell behaves. You can combine characteristics to make new particles.

For more accurate fluid simulations I have also implemented a pressure system. 

It also contains a rigid body simulation.

## Giving Feedback
If you want to report a bug, think a feature would be neat and want to see it implemented make an issue on github.

## Terms Of Usage
You're free to use it however you want as long as you credit the original creator (nixuntris) as the base you worked off or integrated into your engine.

## Joining The Project ?
I am currently working on it solo and I want to keep working solo on the core physics systems, however if you want to help with the GUI or sound, you're free to add me on discord. The nickname is the same as on github(nixuntris)

## Installation
In the releases tab on github there is a pre-made demo to play around with it. 

## Compiling From Source
The project has raylib linked to where it's installed by the raylib installer, if you have it installed 
somewhere else you need to change the COMPILER_PATH variable in the Makefile to where it's saved.

## Dependencies 
Raylib 5.5
GCC C++14
