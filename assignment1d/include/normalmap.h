#ifndef NORMAL_MAP_H_
#define NORMAL_MAP_H_

#include <iostream>
#include <vector>

#include "vec3.h"

class NormalMap
{
public:
    int w,h,n_col;
    int wm1,hm1;
    vector<vec3> values;
    NormalMap(int width, int height, int num_colors, vector<vec3> vals)
    {
        w = width; h = height;
        n_col = num_colors;
        values = vals;

        wm1 = w-1;
        hm1 = h-1;
    }

    ~NormalMap(){}

    vec3 get_normal_at(float u, float v)
    {
        float x = u * wm1; 
        float y = v * hm1;
        
        int i = (int)x; 
        int j = (int)y;
        
        float alpha = (x - i);
        float beta  = (y - j);

        float oma = 1-alpha;
        float omb = 1-beta;

        int j1 = (j+1)*(j < hm1) + j*(j == hm1);
        int i1 = (i+1)*(i < wm1) + i*(i == wm1);

        vec3 n = values[j*w + i];
        vec3* ni  = &values[j*w  + i1];
        vec3* nj  = &values[j1*w + i];
        vec3* nij = &values[j1*w + i1];

        float one = oma   * omb;
        float two = alpha * omb;
        float thr = oma   * beta;
        float fou = alpha * beta;

        return 
            (values[j*w+i] * one) +
            (*ni * two) + 
            (*nj * thr) + 
            (*nij * fou);
    }
};

#endif