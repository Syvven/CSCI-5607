#ifndef OBJECT_H_
#define OBJECT_H_

#include <iostream>
#include <tuple>

#include "vec3.h"
#include "material.h"
#include "ray.h"
#include "secondutils.h"

/*
    Base class for object.
    I think this will be helpful when
     getting into more complicated models
*/

class Object 
{
public:
    vec3 center;
    Material* mat;
    int id;

    virtual ~Object(){}
    virtual tuple<bool, float> intersects(Ray& ray) = 0;
    virtual vec3 get_normal(vec3& x) = 0;
};

#endif