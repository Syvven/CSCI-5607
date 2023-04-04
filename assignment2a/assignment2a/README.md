# Assignment 2A: Noah hendrickson

## Features

- **Scaling**
    - Left Arrow Button Thins
    - Right Arrow Button Widens
    - Up and Down Stretch and Compact
    - Not Allowed To Scale to 0
    - I ignored the part of the grading criteria that says don't let it flip.
        - why? whats the point? everything still works so why shouldn't it be allowed to flip?
        - therefore, it stays in.
        - thank you for coming to my TED Talk

- **Rotation**
    - Left mouse button rotates the object
    - Rotation speed is based on angle between subsequent polls
    - Spinning the mouse counterclockwise will make the object spin counterclockwise
    - Also vice versa
    - I feel this is more intuitive than just left or right so therefore, that's what I did
    - Rotation cannot be done while translating

- **Translation**
    - Ctrl + Left button moves the object to and with the mouse.
    - Cannot be done while rotating 
    - Can be done while the rotation is decelerating 

## Extra Credit

I did both extra credits worth 5%. <br>
First, the program can accept models from a file. <br>
The file must be specified as the first command line argument. <br>
The file must be structured as 

```v x y z r g b```

for each vertex, color pair, in which the z component will be ignored. <br>
Additionally, if you put a second command line argument 

```"secret_kiwi"```

and specify the file named 'kiwer.txt' included in the zip as the first argument, a funny kiwi will be loaded :) <br>
Technically, if you put any file there with the structure: <br>
```v x y z``` being a single vertex, <br>
```c r g b``` being a color that all the following vertices will be colored, <br>
and 
```f v1 v2 v3``` being indices into any of the vertices, <br>
then the output will still work. 
The image included with the submission is an example of the secret kiwi setting :) <br>

Second, the program can manipulate the individual arms of the given object. <br>
If the program is specified with no file, or the file given cannot be read, it will load the default. <br>
The commands for this individual manipulation are as follows:

- (The number key) 1 + LMB
    - Rotates the Red Triangle
- (The number key) 2 + LMB
    - Rotates the dark blue Triangle
- (The number key) 3 + LMB
    - Rotates the teal Triangle
- Each of these will rotate the thing individually
- Any combination can be done at the same time as long as those keys are held when the click happens
- If one that is held is released during movement, that triangle will stop but the rest will not
- If one is pressed while movement is happening, it will not start rotating. 
- The state of the rotation is kept even when the whole object is rotated
- Whole object rotation will not happen when any of the sub triangles are actively being moved. 
- These triangles rotation follow the same rules as the whole object rotation.
- The triangles rotate around the centroid of the model. I wasn't sure if this was what was wanted but thats what I did.