# Fluff2D
Development in process, public to have something on my resume.

Animation system made 
Permission to use a certain SDK ignored, fine I'll make my own then.

## Basics
Hold space and drag to move camera, or wasd.
Scroll to zoom.

Load in a model under File at the top left.
	Select your own file, or the testing files.
Select part(s) on the left side, click or ctrl + click.
	Touch one of the sliders to select it.
	After selecting, assign 2, 3, or manual keypoints under Parameters.
	Right click to snap to closest keypoint on the slider.
	Move around the vertices.
	Play around with the assigned slider.
Select part(s) again.
	Under Meshes, add a warp or rotation deformer.
	Assign parameters again.
	Controls the child object's vertices.
Other controlable stuff: position, rotation, scale, color, blending modes, clip/mask.

## Building
### Visual Studio
Set to C++17.
In C/C++ General, add in Additional Include Directories (Project Path)\dependencies\include.
In Linker General, add in Additional Library Directories (Project Path)\dependencies\lib\GLFW.
In Linker Input, add in Additional Dependencies glfw3.lib and opengl32.lib.
May need to put MSVCRT to Ignore Specific Default Libraries.