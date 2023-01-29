#ifndef SPHERE_H_
#define SPHERE_H_

#include <iostream>
#include <tuple>

#include "object.h"

using namespace std;

class Sphere : public Object
{
public:
    float rad;
    Sphere(vec3 c, float r, Material* m)
    {
        rad = r;
        mat = m;
        center = c;
    }

    tuple<bool, float> intersects(Ray& ray)
    {
        float a = 1; /* ray is guaranteed to be normalized */
        /* direction dot (origin - center) */
        float b = 2 * (ray.dir.dot(ray.orig - this->center));
        float c = (ray.orig - this->center).lengthSqr() - this->rad*this->rad;

        float d = b*b - 4*a*c;

        /* no intersection if discriminant is less than 0 */        
        if (d < 0) return tuple<bool, float>{false, 0.0};

        float t1 = (-b + sqrt(d)) / (2*a);
        float t2 = (-b - sqrt(d)) / (2*a);
        
        if (t1 < t2 && t1 >= 0)
            return tuple<bool, float>{true, t1};
        
        if (t2 < t1 && t2 >= 0)
            return tuple<bool, float>{true, t2};

        /* both intersection points were behind */
        return tuple<bool, float>{false, 0.0};
    }
};

#endif