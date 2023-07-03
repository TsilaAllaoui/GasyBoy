# GasyBoy 
Gameboy emulator made with C++/SDL2.


## Features
- Fully working Zilog Z80 CPU with all opcodes
- Working NoMBC and MBC1 roms (MBC1 may cause visual bugs)
- Working buttons


## About ROM and illegal stuff 

All stuff related to Nintendo belongs to Nintendo. For roms, get them on the internet or use test roms [here](https://github.com/c-sp/gameboy-test-roms/).


## Building

 - ### Requirements
    - Visual Stdio 15/17/22
    - SDL2 (already included)

 - ### Build
    Just build the solution of the project

## Screenshots

1) Nintendo Boot Logo 
![nintendo_boot_logo](./screenshots/nintendo_screen.png)

2) Super Mario Land

<table>
    <tr>
        <td>
            <img src="./screenshots/mario_land_1.png" />
        </td>
        <td>
            <img src="./screenshots/mario_land_2.png" />
        </td>
    </tr>
</table>

3) Tetris

<table>
    <tr>
        <td>
            <img src="./screenshots/tetris_1.png" />
        </td>
        <td>
            <img src="./screenshots/tetris_2.png" />
        </td>
    </tr>
</table>

4) Super Mario Land

<table>
    <tr>
        <td>
            <img src="./screenshots/dr_mario_1.png" />
        </td>
        <td>
            <img src="./screenshots/dr_mario_2.png" />
        </td>
    </tr>
</table>

## TODO:
- Fix PPU sprites transparency bug
- Add supprt for more MBCs memory support
- Add savestates
- Make better UI and window