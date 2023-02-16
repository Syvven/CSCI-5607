# CSCI5607 HW1B: Ray Tracer - Noah Hendrickson

## Cool Picture :)
![image13](outputs/kiwer.png)

## How To Run:

A makefile is included in the project
In a linux terminal, input
```
> make raytracer1b
```
to compile the project. <br><br>

Input
```
> make run
```
to subsequently run the project. <br><br>

To do both at once, input
```
> make comp-run
```
<br>

If you want to easily change the input file, add 
```
> TXT=<file_path>/<file_name>.txt
```
to the end of your make command.

Output will be saved to a file called ```<file_name>.ppm``` in the ```outpus``` directory.
If the ```outputs``` directory does not exist, it will be created.

## Input Specifications:

### **Required Keywords**
- imsize:
    - defines the width and height of the output image
    - arguments:
        - \<width> \<height>
        - must be positive, >= 1, ints or floats
        - floats will be rounded down
- eye:
    - defines position of the camera eye
    - arguments:
        - \<x> \<y> \<z>
        - must be ints or floats
        - represent x, y, and z coordinates of a point in world space
- viewdir:
    - define the direction the camera faces
    - arguments:
        - \<x> \<y> \<z>
        - must be ints or floats
        - CANNOT have all values be 0
        - 0 in this case is -9e-13 < x < 9e-13
- updir:
    - define the up direction of the camera
    - arguments:
        - \<x> \<y> \<z>
        - must be ints or floats
        - CANNOT have all values be 0
        - 0 in this case is -9e-13 < x < 9e-13
- hfov:
    - defines the horizontal field of view of the camera
    - arguments:
        - \<fov>
        - must be a positive int or float
        - must be between epsilon (defined in vec3.h) and 360
- bkgcolor:
    - defines the background color of the scene
    - either depthcueing or bkgcolor must be specified
    - if both are specified, depthcueing takes priority
    - arguments:
        - \<r> \<g> \<b>
        - must be positive int or float
        - must be between 0 and 1 inclusive for both
- depthcueing
    - defines the depthcueing params of the scene
    - either depthcueing or bkgcolor must be specified
    - if both are specified, depthcueing takes priority
    - arguments:
        - \<dcr> \<dcg> \<dcb> \<amax> \<amin> \<dmax> \<dmin>
        - dcr, dcg, dcb must be float between 0 and 1
        - amax and amin should be between 0 and 1, must be float or int
        - dmax and dmin determine distances, must be float or int
- all of these keywords must have exactly that number of arguments following them
    - however, the ordering of keywords is arbitrary
    - additionally, there can be arbitrary whitespace between arguments and keywords

### **Optional Keywords**
- projection:
    - switches ray caster mode to parallel
    - argument:
        - \<parallel>
        - a string, must be spelled exactly "parallel"
- mtlcolor:
    - defines the color and phong shading properties of the following objects
    - arguments:
        - \<Odr> \<Odg> \<Odb> \<Osr> \<Osg> \<Osb> \<ka> \<kd> \<ks> \<n>
        - Odr/g/b is the color of the diffuse component
        - Osr/g/b is the color of the specular component
        - ka,kd,ks are the modifiers to the ambient, diffuse, and specular components
        - n is the power that the specular component is taken to
    - an arbitrary number or spheres and cylinders can follow mtlcolor
    - each sphere/cylinder will inherit the previously defined material
    - once a keyword besides sphere/cylinder is encountered, the current material is ended
    - in order to add another sphere or cylinder, another material has to be defined
- sphere:
    - puts a sphere into the scene
    - arguments:
        - \<cx> \<cy> \<cz> \<rad>
        - cx, cy, cz define the point that is the center of the sphere
        - rad is the radius of the circle
        - cx,cy,cz must be ints or floats
        - rad is a float or int and must be > epsilon (defined in vec3.h)
- cylinder
    - puts a cylinder into the scene
    - arguments:
        - \<cx> \<cy> \<cz> \<dx> \<dy> \<dz> \<rad> \<len>
        - cx,cy,cz define the center of the bottom cap of the cylinder
        - dx,dy,dz define the direction that the rest of the cylinder follows
        - rad is the radius of the cylinder, len is the length 
        - cx,cy,cz must be ints or floats
        - dx,dy,dz must be ints or floats
        - dx,dy,dz CANNOT all be < epsilon (defined in vec3.h)
        - rad must be a int or float and cannot be < epsilon
        - len must be a int or float and cannot be < epsilon
- threadnum
    - number of threads to be used 
    - if keyword not included, defaults to 1
    - arguments:
        - \<num>
        - positive, nonzero integer
- light
    - puts a light into the scene
    - arguments:
        - \<x> \<y> \<z> \<w> \<r> \<g> \<b>
        - w should be 0 (directional light) or 1 (point light)
        - xyz specifies location of point light or direction of directional light
        - rgb is the intensity of the light
- attlight
    - puts an attenuated light into the scene
    - arguments:
        - \<x> \<y> \<z> \<w> \<r> \<g> \<b> \<c1> \<c2> \<c3>
        - w must be 1, only point light can be attenuated
        - xyz is position of point light
        - rgb is intensity again
        - c1,c2,c3 are the coefficients of the dropoff
- all of these keywords are optional 
- projection and mtlcolor can be arbitarily ordered with arbitrary whitespace
- the circles and cylinders following mtlcolor can be arbitrarily ordered
    - once another keyword is encountered, a circle/cylinder without another mtlcolor will throw an error

### **Comments**
Comments are allowed anywhere in the input text files under two conditions:
- comments MUST be on their own line
- comments MUST start with "//" and have a space between any following text
- good example:
    - ```1. // This is a comment :)```
- bad example:
    - ```1. sphere 0 0 0 2 //this is a bad comment :(```
