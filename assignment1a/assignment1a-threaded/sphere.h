#ifndef SPHERE_H_
#define SPHERE_H_

#include <iostream>
#include <tuple>

#include "object.h"

using namespace std;

class Sphere : public Object
{
public:
    float rad, r_sq;
    Sphere(vec3 c, float r, Material* m)
    {
        rad = r;
        mat = m;
        center = c;
        r_sq = r*r;
    }

    tuple<bool, float> intersects(Ray& ray)
    {
        /* float a = 1; /* ray is guaranteed to be normalized */
        /* direction dot (origin - center) */
        vec3 omc = ray.orig - this->center;
        float b = 2 * (ray.dir.dot(omc));
        float c = omc.lengthSqr() - this->r_sq;

        float d = b*b - 4*c;

        /* no intersection if discriminant is less than 0 */        
        if (d < 0) return tuple<bool, float>{false, 0.0};

        /* actual distances */
        float sqrtd = sqrt(d);
        float t1 = (-b + sqrtd) * 0.5;
        float t2 = (-b - sqrtd) * 0.5;
        
        if (t1 < t2 && t1 >= 0)
            return tuple<bool, float>{true, t1};
        
        if (t2 < t1 && t2 >= 0)
            return tuple<bool, float>{true, t2};

        /* both intersection points were behind */
        return tuple<bool, float>{false, 0.0};
    }
};

#endif