#ifndef MATERIAL_H_
#define MATERIAL_H_

#include <iostream>

#include "color.h"

/*
    Maberial
*/

struct Material 
{
    Color diffuse, specular;
    float ka, kd, ks, n;
    Material(Color d, Color s, 
        float ka, float kd, float ks, float n) 
    {
        this->ka = ka;
        this->kd = kd;
        this->ks = ks;
        this->n = n;
        this->diffuse = d;
        this->specular = s;
    }
};

#endif