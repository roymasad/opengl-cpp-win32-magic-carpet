# opengl-cpp-win32-magic-carpet
OpenGL Magic Carpet Demo in c++ and  win32 



Code : Roy Massaad


License: MIT


Date: 2005


Category: Archive/Nostalgia

**Info**

This is an OpenGl demo i did long ago. It uses c++, opengl/glut and win32 api.

I used NeHe OpenGl tutorials back then to get the basics running

The demo allows you to view and manipulate a flying 3d 'magic carpet'

I updated/cleaned the project to build under Visual Studio 2019 before uploading to Github

It has some interesting code snippets

For a list of options, press F1 for help


**Notes about the demo:**


-Allows you to rotate/zoom the magic carpet
-Allows you to drag and drop 1 or multiple images on it, it will use them as textures and u can cycle between them with arrows
-You can resize the window or run it in full screen
-You pause or modify the wave animation on the carpet
-You can switch between wireframe and textured mode
-You can only load BMP images (it used to use GLAUX for multiple formats before it was deprecated)
-It prebakes the wave/sin calculations to speed things up
-It is well documented with original inline comments
-Allows you to turn on some special effects on the textures
-Must have the default open.bmp file present in its folder or it will crash


