#ifndef RAYTRACER_H_
#define RAYTRACER_H_ 

#include <iostream>
#include <vector>
#include <typeinfo>
#include <cmath>
#include <tuple>
#include <thread>
#include <algorithm>
#include <random>
#include <stack>

#include "color.h"
#include "material.h"
#include "cylinder.h"
#include "sphere.h"
#include "mesh.h"
#include "vec3.h"
#include "ray.h"
#include "light.h"
#include "triangle.h"

class RayTracer 
{
public:
    RayTracer(vec3& e, vec3& v, vec3& u, 
        float hf, float w, float h, Color& bk, float bk_eta, 
        vector<Object*>* o, vector<Light*>* l, bool parallel, 
        bool cueing, float amin, float amax, float dmin, 
        float dmax, Color& cueingcolor, int threads);

    void gen(vector<Color>& pixels);

private:
    void split_work(int start, int end, vector<Color>& pixels, vec3 ro, vec3* sw);
    void define_viewing_system();
    // Color trace_ray(Ray& r, int depth);
    // Color shade_ray(Ray& r, float t, Object* o, int depth);

    void trace_ray(Ray* ray, int depth, stack<Object*>& whences);
    void shade_ray(Ray* ray, int depth, stack<Object*>& whences);

    void get_min_intersect(Ray* r);
    void get_all_intersects_in_distance(
        Ray* r, vector<Object*>& fill, float dist, Object* other
    );
    float arbitrary_random(float min, float max);
    float calc_atten(float c1, float c2, float c3, float d);

    /* variables input by user */
    vec3 eye, view, up;
    Color bkgcolor;
    float bkg_eta;
    float hfov, p_width, p_height;
    vector<Object*>* objs;
    vector<Material*>* mats;
    vector<Light*>* lights;
    bool parallel, cueing;
    float amin, amax, dmin, dmax;
    Color cueingcolor;

    /* derived variables */
    vec3 ul, ur, ll, lr, dv, dh; /* four corners of view window */
    vec3 u, v, w; /* vectors defining the view window */
    float d = 5; /* arbitrary */
    float w_width, w_height; /* height and width of view window */
    float aspect_ratio;

    /* threading */
    int num_threads;

    /* recursion */
    int max_depth = 10;
    float od3;
};

#endif 