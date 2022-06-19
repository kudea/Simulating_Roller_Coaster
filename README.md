# Computer Graphic - Simulating Roller Coaster

##### taught by Dr. Jernej Barbič

## Before you start the code, please check your system environment.

For the Windows platform, we provide the Visual Studio 2017 and 2019 solution/project files in ./hw1-starterCode (same files for both 2017 and 2019).

For Mac OS X, before you do any coding, you must install command-line utilities (make, gcc, etc.). Install XCode from the Mac app store, then go to XCode, and use "Preferences/Download" to install the command line tools. Important: If you are using Mac OS X Mojave, you need to update the OS to the latest version of Mojave. Otherwise, OpenGL does not work. Or, you can use Catalina, Big Sur or Monterey.

On Linux, you need the libjpeg library, which can be obtained by "sudo apt-get install libjpeg62-dev libglew-dev". For Windows and Mac OS X, the starter code contains a precompiled jpeg library. On Intel-based Apple chips, the jpeg library should work as is. On Apple M1 chips (https://en.wikipedia.org/wiki/Apple_M1), you need to take the following steps to get jpeg library to compile and link:

// Do this for Apple M1 chips ONLY. If you don't do it, you will
// get linker errors related to the libjpeg library.
// The below steps are not necessary for Windows, Linux or Intel-based Apple computers.
```
cd external/jpeg-9a-mac

chmod +x configure
./configure --prefix=$(pwd)

make clean

make
chmod +x install-sh
make install

cd ../../hw1-starterCode
make
./hw1 heightmap/spiral.jpg

```
```
./hw1 heightmap/spiral.jpg
the .jpg can be any pic inside the heightmap folder
```


## My Environment:
```
OpenGL Version: 4.1 Metal - 76.1
OpenGL Renderer: Apple M1
Shading Language Version: 4.10
API: OpenGL (Core Profile)
```
## Animation
Demo video: https://youtu.be/UNpsC-wrgHY

## Description
In this work, I use Catmull-Rom splines along with OpenGL core profile shader-based texture mapping and Phong shading to create a roller coaster simulation.

Looking into the file [hw2.pdf](https://drive.google.com/file/d/1m7DXCSWqoE29S2vG3WnpgD3PtUhHAKCZ/view?usp=sharing) for detail.

## Features
1. Uses OpenGL core profile, version 3.2 or higher

2. Completed all Levels:
  Level 1 (spline)
  level 2 (the ride)
  Level 3 (rail cross-section)
  Level 4 (ground)
  Level 5 (Phong shading)

3. Rendered the camera at a reasonable speed in a continuous path/orientation

4. Run at interactive frame rate (>15fps at 1280 x 720)

## Special Feature

1. Render a T-shaped rail cross section

2. Render a Double Rail

3. Made the track circular and closed it with C1 continuity 

4. Any Additional Scene Elements? (list them here)

5. Render a sky-box

6. Create tracks that mimic real world roller coaster

7. Draw splines using recursive subdivision

8. Render environment in a better manner

9. Improved coaster normals - Y

10. Modify velocity with which the camera moves 

## Other Features

1. I make it like a mimic real world roller coaster experience. 
(I adjust the view of the camera, thus, I can see the whole view with real world direction instead of unrealistic upside down or wrong rotation.)
 
2. 's', 'g', 'q', 'w', 'r', 't', '5', '6', '7', these are all extra Keyboard controls.

3. I also add a crossbar between "Double Rail".

4. I create my own track mytrack.sp, and it has more realistic rail structure.

5. I create my own animation 420hw2demo.mp4 combines the one with T-shaped rail and the other doesn't because I don't think T-shaped rail is beautiful enough for demo.

## Keyboard/Mouse controls

1. Press 's' to stop the rollercoaster.

2. Press 'g' to resume the stopped rollercoaster.

3. Press 'q' to see the whole view (whole rollercoaster structure). (Although the scene is stopped, but the rollercoaster(camera) is still running.)

4. Press 'w' to resume the view on the rollercoaster. (I will automatically address the viewing position.)

5. Press 'r' to initialize the extra control and center the viewing position.

6. Press 't' to take screenshots automatically(<900 pics).

7. Press '5' to bounce mode.

8. Press '6' to quit bounce mode.

9. Press '7' to rotate automatically.

10. Translate: /*(CTRL is not working on my MacBook m1 clip, too I change it to "ALT".)*/
   Pressing 'ALT' and dragging the mouse with the left mouse clicked can translate along x and y axis. 
   Pressing 'ALT' and dragging the mouse with the middle mouse clicked can translate along z axis. 

11. Scale: 
   Pressing 'SHIFT' and dragging mouse with the left mouse clicked can scale on x and y axis.
   Pressing 'SHIFT' and dragging mouse with the middle mouse clicked can scale on z axis.

12. Rotate: 
   Dragging mouse with the left mouse clicked can rotate at x and y axis.
   Dragging mouse with the middle mouse clicked can rotate at z axis.

## FYI

1. Texture pictures are in the skybox (Download from "http://titan.csit.rmit.edu.au/~e20068/teaching/i3dg&a/2020/tute-tex.html")

## PIC
