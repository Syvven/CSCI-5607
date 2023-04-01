#ifndef RAY_H_
#define RAY_H_ 

#include <iostream>

#include "vec3.h"
#include "object.h"
#include "secondutils.h"

using namespace std;

/*
    This is pretty self explanatory.
    Contains origin and direction. 
*/

class Object;

struct Ray 
{
    vec3 orig, dir, dir_inv;

    Object* obj = nullptr;
    Object* mesh_obj = nullptr;
    vector<Object*> mesh_objs;
    map<int, Object*> mesh_objs2;
    Color color;
    vec3 int_point;
    float t = 9e15;
    int id;

    float alpha, beta, gamma;
    float u, v;

    Ray() {
        id = Utils::getUid();
    }
    Ray(vec3& o, vec3& d)
    {
        orig = o;
        dir = d;
        id = Utils::getUid();
    }
    Ray(vec3& o, vec3& d, float eta, int whence)
    {
        orig = o;
        dir = d;
        id = Utils::getUid();
    }

    Ray(const Ray& r)
    {
        orig = vec3(r.orig);
        dir = vec3(r.dir);
        t = r.t;
        int_point = r.int_point;
        color = r.color;
        obj = r.obj;
        alpha = r.alpha;
        beta = r.beta;
        gamma = r.gamma;
        u = r.u; v = r.v;
        id = Utils::getUid();
    }
    void operator=(const Ray& r)
    {
        orig = vec3(r.orig);
        dir = vec3(r.dir);
        t = r.t;
        int_point = r.int_point;
        color = r.color;
        obj = r.obj;
        alpha = r.alpha;
        beta = r.beta;
        gamma = r.gamma;
        u = r.u; v = r.v;
        id = Utils::getUid();
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