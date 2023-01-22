# PPM Image Generator - Noah Hendrickson

## PPM Overview

PPM Generator producing PPM files based on various techniques. <br>
PPM files generated are structured as thus:
- Header
  - PPM Type (P3/P6): P3 is used exclusively in this project
  - Width: Positive integer 
  - Height: Positive integer
  - Colors: Integer for RGB values, 255 used exclusively in this project
- Pixel Values
  - Each pixel value is deterined by 3 integers: r g b
  - Between each r g b value is a single whitespace
  - Whitespace is either a space or newline
  - The exclusive structure of this project is a single pixel per line
- Below is an example of a 2x2 ppm image that is all black with a white pixel in the bottom right:

```
P3
2
2
255
0 0 0
0 0 0
0 0 0
255 255 255
```

## Features

This PPM generator features various ways to generate ppm images based on an input file.
- Basic Gradient
- Perlin Noise
- SPH Fluid Simulation
- Combination of SPH and Perlin Noise
- Voronoi Diagram

## Input Specifications -- ***🚨🚨🚨Important🚨🚨🚨***

The input of this generator is very specific so follow these guidelines otherwise it will not work. <br>
The generator is fully error handled (as far as I can tell) and will exit on incorrect input. <br>
The generator takes as a command line argument the name of the input file with the extension. <br>
An example is thus:
```
C:\Projects> <binary> input_test.txt
```
As for the structure of the input file itself, there are a couple things to note:
- The first line of the file must be structured as thus:
  - ```imsize <width> <height>```
  - imsize is a keyword and must be written exactly like that
  - width and height are positive, non-zero integers
  - they can be floats with a decimal point, but they will be rounded down
- The second line of the file specifies which generation method
  - there are 5 keywords that can be used here
  - ```none```
    - a basic gradient will be used
    - there should be NO extra lines after this keyword
    - also, it must be spelled exactly like this
  - ```perlin```
    - perlin noise will be used to generate the image
    - perlin takes a single argument that should be placed on the next line down
    - that argument is the seed
    - the seed can be any positive integer or zero
    - if the seed is zero, no seed will be used and you will get a different image each time run
    - there should be NO extra lines after the seed
  - ```particle```
    - SPH fluid simulation will be used to generate this image
    - SPH takes a single argument that should be placed on the next line down
    - that argument is the type of gravity used in the sim
    - it can take one of two values: ```normal``` or ```circle```
    - these keywords should be entered exactly as above and only one
    - normal will result in, well, normal downwards gravity
    - circle will result in gravity being applied based on the angle of the particle with respect to the origin
    - there should be NO extra lines after the type
  - ```combo```
    - combo combines SPH and perlin 
    - at any position where the pixel value is below a threshold, it will be colored with perlin noise
    - combo takes two arguments
    - the seed from perlin and the gravity type from particle in that order
    - the two arguments should be seperated by a single space
    - all the respective rules apply to both
    - there should be NO extra lines after this line
  - ```voronoi```
    - the final type is voronoi
    - a voronoi diagram is constructed and used to generate pixel colors
    - there are numerous arguments for this type that should be in this order seperated by a space:
    - ```seed```
      - all rules apply as before
    - ```node_count```
      - number of nodes to base the pixel color on
      - must be an integer greater than 1
    - ```flat``` or ```gradient```
      - this argument determines the "shading" type
      - flat will make all polygons a flat color
      - gradient will make the pixels get darker the further from the node defining the polygon
      - these arguments must be spelled exactly like this and only one can be used at a time
    - ```random``` or ```set```
      - this argument determines the coloring of the polygons
      - random will assign random r g b values to each polygon
      - set will require the specification of r g b values 
      - these arguments must be spelled exactly like this and only one can be used at a time
    - ```r g b```
      - THESE ARE THREE SEPERATE ARGUMENTS COMBINED FOR BREVITY SAKE
      - 
