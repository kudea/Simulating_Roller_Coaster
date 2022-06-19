Subject 	: CSCI420 - Computer Graphics 
Assignment 2: Simulating a Roller Coaster
Author		: < Hsuan Yeh >
USC ID 		: < 9566797656 >

Description: In this assignment, we use Catmull-Rom splines along with OpenGL core profile shader-based texture mapping and Phong shading to create a roller coaster simulation.

Core Credit Features: (Answer these Questions with Y/N; you can also insert comments as appropriate)
======================

1. Uses OpenGL core profile, version 3.2 or higher - Y

2. Completed all Levels:
  Level 1 : - Y
  level 2 : - Y
  Level 3 : - Y
  Level 4 : - Y
  Level 5 : - Y

3. Rendered the camera at a reasonable speed in a continuous path/orientation - Y

4. Run at interactive frame rate (>15fps at 1280 x 720) - Y

5. Understandably written, well commented code - Y

6. Attached an Animation folder containing not more than 1000 screenshots - Y (800 pics)

7. Attached this ReadMe File - Y

Extra Credit Features: (Answer these Questions with Y/N; you can also insert comments as appropriate)
======================

1. Render a T-shaped rail cross section - Y (I create 3 rail to mimic T-shaped rail)

2. Render a Double Rail - Y

3. Made the track circular and closed it with C1 continuity - Y, I create a customized circular track with C0 & C1 continuity (same position and same tangent)

4. Any Additional Scene Elements? (list them here) - Y, I also add a crossbar between two rails.

5. Render a sky-box - Y

6. Create tracks that mimic real world roller coaster - Y, I create my own track mytrack.sp and add a crossbars between the rails, which seems like a real world roller coaster.

7. Generate track from several sequences of splines - N

8. Draw splines using recursive subdivision - Y (845 line : subdivide function)

9. Render environment in a better manner -  Y

10. Improved coaster normals - Y

11. Modify velocity with which the camera moves - Y, but not the "gravity". (I use the distance between spline points to display the speed up and slow down of the camera movement.)

12. Derive the steps that lead to the physically realistic equation of updating u - Y (derivesteps.pdf)

Additional Features: (Please document any additional features you may have implemented other than the ones described above)
======================

1. I make it like a mimic real world roller coaster experience. 
(I adjust the view of the camera, thus, I can see the whole view with real world direction instead of unrealistic upside down or wrong rotation.)
 
2. 's', 'g', 'q', 'w', 'r', 't', '5', '6', '7', these are all extra Keyboard controls.

3. I also add a crossbar between "Double Rail".

4. I create my own track mytrack.sp, and it has more realistic rail structure.

5. I create my own animation 420hw2demo.mp4 combines the one with T-shaped rail and the other doesn't because I don't think T-shaped rail is beautiful enough for demo.


Open-Ended Problems: (Please document approaches to any open-ended problems that you have tackled)
======================

1. When creating skybox vertices, I give all my heart to make it correctly suit to scene instead of upside down.

2. Figuring out the suitable constant of Phong lighting.

3. Adding crossbar between rails, which needs to draw numerous xyz 3D to assign the correct vector.

Keyboard/Mouse controls: (Please document Keyboard/Mouse controls if any)
======================

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


Names of the .cpp files you made changes to:
======================

1. hw2.cpp


Comments : (If any)
======================

1. The screenshot is in the Animation folder.

2. Texture pictures are in the skybox (Download from "http://titan.csit.rmit.edu.au/~e20068/teaching/i3dg&a/2020/tute-tex.html")

3. In openGLHelper-starterCode, I create texture.fragmentShader.glsl, texture.vertexShader.glsl, texturePipelineProgram.cpp, texturePipelineProgram.h

4. I write down the process of deriving the formula derivesteps.pdf

5. Use make and ./hw2 track.txt to execute my code