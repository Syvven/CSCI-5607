#ifndef TEXTURE_H_
#define TEXTURE_H_

#include <iostream>
#include <vector>
#include <cassert>

#include "color.h"

class Texture
{
public:
    int w,h,n_col;
    int wm1,hm1;
    vector<Color> values;
    Texture(int width, int height, int num_colors, vector<Color> vals)
    {
        w = width; h = height;
        n_col = num_colors;
        values = vals;

        wm1 = w-1;
        hm1 = h-1;
    }

    ~Texture(){}

    Color* get_color_at(float u, float v)
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

        Color* c   = &values[j*w  + i];
        Color* ci  = &values[j*w  + i1];
        Color* cj  = &values[j1*w + i];
        Color* cij = &values[j1*w + i1];

        float one = oma   * omb;
        float two = alpha * omb;
        float thr = oma   * beta;
        float fou = alpha * beta;

        float r = 
            c->r   * one + 
            ci->r  * two +
            cj->r  * thr +
            cij->r * fou;

        float g = 
            c->g   * one + 
            ci->g  * two +
            cj->g  * thr +
            cij->g * fou;

        float b = 
            c->b   * one + 
            ci->b  * two +
            cj->b  * thr +
            cij->b * fou;

        return new Color(r,g,b);
    }
};


#endif