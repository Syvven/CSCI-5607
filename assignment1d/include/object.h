#ifndef OBJECT_H_
#define OBJECT_H_

#include <iostream>
#include <tuple>
#include <vector>
#include <map>
#include <thread>

#include "vec3.h"
#include "material.h"
#include "ray.h"
#include "secondutils.h"
#include "color.h"
#include "texture.h"
#include "normalmap.h"

/*
    Base class for object.
    I think this will be helpful when
     getting into more complicated models
*/

class Ray;

class Object 
{
public:
    vec3 center;
    Material* mat;
    int id;
    int type1 = 0;

    virtual ~Object(){}
    virtual bool intersects(Ray& ray, bool shadow) = 0;
    virtual vec3 get_normal(Ray& r) = 0;
    virtual Color* get_diffuse(Ray& r) = 0;
    virtual Color* get_specular(Ray& r) = 0;
    virtual Material* getMat(Ray& r) = 0;
    virtual Object* getIntersectedObject(Ray& r) = 0;
};

#endif