#ifndef RAY_H_
#define RAY_H_ 

#include <iostream>

#include "vec3.h"

using namespace std;


/*
    This is pretty self explanatory.
    Contains origin and direction. 
*/

struct Ray 
{
    vec3 orig, dir;
    Ray() {}
    Ray(vec3& o, vec3& d)
    {
        orig = o;
        dir = d;
    }
    Ray(float ox, float oy, float oz,
        float dx, float dy, float dz)
    {
        orig = vec3(ox, oy, oz);
        dir = vec3(dx, dy, dz);
    }
    Ray(const Ray& r)
    {
        orig = vec3(r.orig);
        dir = vec3(r.dir);
    }
    void operator=(const Ray& r)
    {
        orig = vec3(r.orig);
        dir = vec3(r.dir);
    }

    /* allows printing of vec3 objects */
    string toString() const
    {   
        return (
            "Ray Origin:    " + orig.toString() + "\n" +
            "Ray Direction: " + dir.toString()
        );
    }
	friend ostream& operator<<(ostream& s, const Ray& ray) {
		s << ray.toString();

		return s;
	}
};

#endif 