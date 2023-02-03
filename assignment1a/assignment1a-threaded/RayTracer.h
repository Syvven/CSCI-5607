#ifndef RAYCASTER_H_
#define RAYCASTER_H_ 

#include <iostream>
#include <vector>
#include <typeinfo>
#include <cmath>
#include <tuple>
#include <thread>

#include "color.h"
#include "material.h"
#include "object.h"
#include "vec3.h"
#include "ray.h"

class RayTracer 
{
public:
    RayTracer(vec3& e, vec3& v, vec3& u, 
        float hf, float w, float h, Color& bk, 
        vector<Material*>* m, vector<Object*>* o, bool parallel, int threads);

    void gen(vector<Color>& pixels);
private:
    void split_work(int start, int end, vector<Color>* pixels, vec3 ro, vec3* sw);
    void define_viewing_system();
    Color& trace_ray(Ray r);
    Color& shade_ray(Object* o);

    /* variables input by user */
    vec3 eye, view, up;
    Color bkgcolor;
    float hfov, p_width, p_height;
    vector<Object*>* objs;
    vector<Material*>* mats;
    bool parallel;

    /* derived variables */
    vec3 ul, ur, ll, lr, dv, dh; /* four corners of view window */
    vec3 u, v, w; /* vectors defining the view window */
    float d = 5; /* arbitrary */
    float w_width, w_height; /* height and width of view window */
    float aspect_ratio;

    /* threading */
    int num_threads;
};

#endif 