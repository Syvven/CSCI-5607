#ifndef OBJECT_H_
#define OBJECT_H_

#include <iostream>
#include <tuple>

#include "vec3.h"
#include "material.h"
#include "ray.h"

class Object 
{
public:
    vec3 center;
    Material* mat;

    virtual tuple<bool, float> intersects(Ray& ray) = 0;
};

#endif