#ifndef LIGHT_H_
#define LIGHT_H_

#include <iostream>

#include "vec3.h"

class Light 
{
public:
    vec3 position;
    Color color;
    int atten;
    float c1, c2, c3;
    virtual ~Light(){}
    virtual vec3 compute_L(vec3& x) = 0;
};

class DirectionalLight : public Light 
{
public:
    vec3 neg_dir;
    DirectionalLight(vec3 dir, Color c)
    {
        this->position = dir.normalized();
        this->neg_dir = this->position;
        this->neg_dir *= -1;  
        this->color = c;  
        this->atten = 0;
    }

    vec3 compute_L(vec3& x) 
    { 
        return this->neg_dir;
    }
};

class PointLight : public Light
{
public:
    PointLight(vec3 pos, Color c)
    {
        this->position = pos;
        this->color = c;
        this->atten = 0;
    }

    vec3 compute_L(vec3& x)
    {
        vec3 n = this->position;
        n -= x;
        n.normalize();

        return n;
    }
};

class AttenPointLight : public Light 
{
public:
    AttenPointLight(vec3 pos, Color c, float c1, float c2, float c3)
    {
        this->position = pos;
        this->color = c;
        this->atten = 1;
        this->c1 = c1;
        this->c2 = c2;
        this->c3 = c3;
    }

    vec3 compute_L(vec3& x)
    {
        vec3 n = this->position;
        n -= x;
        n.normalize();

        return n;
    }
};

#endif