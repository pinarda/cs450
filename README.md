# cs450 - object manipulator

Warning: This repo is missing a few includes in it's current state, and cannot be built from scratch as of yet.  
        
         Use on MAC OSX: ./prog [scene_file.scene]
         
         View images: perspective_1.png, perspective_1_wireframe.png, perspectice_1_cel.png

This started as a project for CS450 - Computer Graphics at Oregon State University. We built a viewer that could read from a simple file format (.obj) listing sets of triangle vertices and render these as images, which we could then rotate, translate and scale. I improved this project by adding multiple different shader states (cel, gouraud, phong), more intuitive object manipulation, and improved the performance of the viewer by using VBOs and VAOs efficiently.

