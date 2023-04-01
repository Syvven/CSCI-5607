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
    float ka, kd, ks, n, eta;
    Color alpha;
    bool beers;
    Material(Color d, Color s, 
        float ka, float kd, float ks, 
        float n, float ar, float ag, float ab, 
        bool beers, float eta) 
    {
        this->ka = ka;
        this->kd = kd;
        this->ks = ks;
        this->n = n;
        this->diffuse = d;
        this->specular = s;
        this->eta = eta;
        this->beers = beers;
        if (this->beers)
        {
            this->alpha = Color(ar, ag, ab);
        }
        else this->alpha = Color(ar, ar, ar);
    }
};

#endif