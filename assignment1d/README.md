# CSCI5607 HW1D: Ray Tracer - Noah Hendrickson

## Updates:

I'm not going to be going in depth for the commands to run and such as 
nothing much has changed in regards to that. <br>
I will only mention the newly added/updated commands.

## Commands added:
- mesh
    - specifies to use a bounding box around a set of triangles
    - arguments \<start or stop> <levels (optional)>
    - start determines the start and stop the end of the set
    - levels determines how many subdivisions
        - nothing specified will use just a single box
        - don't do anything more than 2!! its not working!
        - I tried to implement a kd-tree after I realized it wasn't
           very good but uh I wasn't able to! almost but not quite.
           it speeds it up a bit though.
    - end cant happen before start and start can't happen while 
       already started
    - can have multiple materials across it
    - only handles triangles

## Commands Updated
- mtlcolor
    - arguments added: \<alpha_r> \<alpha_g> \<alpha_b> \<IoR>
    - the three alphas are opacity and can be colored
    - the IoR defines the index of refraction for the material
    - if you don't want beers law, you can just specify one alpha 
       value to use not beers law 
- bkgcolor 
    - arguments added: \<IoR>
    - index of refraction of background

- makefile
    - command 'raytracer1c' to compile was renamed to 'raytracer1d'

Outside of these updated/added things, everything else is the same. 

## Extra Credit Portions

### BVH

As mentioned in the updates section, I did try to implement a BVH. <br>
I tried to get it to go deeper than just one before realizing my method was flawed, <br>
attempting to implement a kd-tree, of which the code can be found in mesh.h, and <br>
failing because I was tired out of my mind at 3AM on the night it was due lol. <br>
Regardless, the issues mentioned above still stand. <br>
As for performance, three files were tested on of the funny kiwi model <br>
with the harbor texture reflecting all over him: 

- kiwer_mapped_not_mesh.txt: No Mesh
    - Took 1,065,646ms, 1065s, roughly 17 minutes
- kiwer_mapped_mesh_1.txt: 1 subdivisions
    - Took 63,426ms, 63s, roughly 1 minute
- kiwer_mapped_mesh_2.txt: 2 subdivisions
    - Took 137,400ms, 137s, roughly above 2 minutes
- kiwer_mapped_mesh_3.txt: 3 subdivisions
    - Took 116,380ms, 116s, roughly under 2 minutes

As we can see from the above results, without the mesh it was VERY slow, <br>
but with 1 subdivision, the speed greatly increases. <br>
The 2 and 3 subdivision ones do slow down but they also break because of my <br>
shoddy code. There are some artifacts where triangles were clearly not captured <br>
properly in the bounds of the boxes. <br>
Each input and output are present in the inputs and outputs folders. <br>

### Beer's Law

Beer's law is demonstrated in the file 'beers_law.ppm'. <br>
This file features two spheres in front of a plane. <br>
The left sphere is using beers law and the right is not. <br>
From this image, we can clearly see that the sphere on the left is <br>
more opaque than the one on the right, showing that as the ray goes deeper, <br>
exp(-alpha*t) gets smaller. My implementation of beers law also allows for <br>
colored shadows and colored glass like objects. 


