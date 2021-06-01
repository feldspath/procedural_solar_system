# INF443 Project: a proceduraly generated solar system.

Author: Marius Debussche and Lilian Bichot

## Source code
The VCL library for the INF443 course is used and included in the repository. The source code we wrote is found in the src/ and shaders/ folder.

## Explore mode
By default, the program runs in exploration mode. The controls are Z,Q,S,D respectively for forward, left, backward and right deplacement of the player. The camera is rotated with the mouse.

## Edit mode
You can change the variable `CAMERA_TYPE` in the `src/display.hpp` source code before compiling to run in edit mode. The planet meshes are updated in real time and can be saved by clicking the save button, after giving it a name. If the planet files are overwritten, the the newly generated planets will be loaded at the next run of the program.

## Program parameters
The planets can take a lot of time and memory to generate. The resolution of the planet meshes can be changed in the same file.
The default window resolution can also be changed.