# Computer Graphics I (CSCI 5607) -- Noah Hendrickson

This repository contains all code developed over the course of the Spring 2023 semester in Computer Graphics I at the University of Minnesota by myself. 
Projects include a raytracer with all self written code, various example images generated, and two homeworks as an introduction to OpenGL, affine transformations, and perspective transformations for camera control.

## Assignment 0:
Intro to PPM files. Program generates a PPM file of an image based on a number of input parameters. Options for image generation methods involve Voronoi Diagram, a SPH fluid simulation, Perlin Noise, and a basic gradient.

## Assignment 1a: 
Intro to the ray tracer, able to render simple, unshaded circles and cylinders of various colors. Main idea behind this assignment was to set up the viewing coordinate system and understand intersecting rays with various parametrically defined shapes. In addition to a perspective view, also allows for orthographic viewing. 

## Assignment 1b:
Introduces light and shadow to the ray tracer. Light is done using phong equation and shadow is done by shooting a cone of rays towards the light from around the intersection point. Additional features in this assignment include depth cueing, light attenuation, and different light types such as point and directional.

## Assignment 1c:
Introduces triangles and textures to the raytracer. Also allows for vertex normals and textures. Implements texture and normal mapping as extra functionality.

## Assignment 1d: Transparency and Mirror Reflections
Introcues transparent and reflective material properties. Transparent objects can be modified such that color rendered changes based on depth traveled. A simple bounding volume hierarchy is implemented to speed up computation. This assignment is not perfect, and more work will be done on it in the future, in a seperate repository.

## Assignment 2a: Programming Interactive 2D Graphics with OpenGL
Introduction to 2D graphics using OpenGL. Base code was provided and our own code for manipulation of a sprite was created. Includes rotation, translate, and scaling of the sprite. Custom sprites can be loaded as well, including from 3d obj files, which will be converted to 2d using an orthographic projection.

## Assignment 2b: Learning About Viewing, Projection and Viewport Transformations via a First-Person 3D Walkthrough
Introduction to 3D graphics using OpenGL. Scene is a church in which the camera can move around using various controls. Objectives of the assignment were to implement the viewing and projection matrices for the camera as well as a personal addition of transformation matrices for various Kiwis in the scene. The camera is able to yaw and pitch utilizing euler angles. 
