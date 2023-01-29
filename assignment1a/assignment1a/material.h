#ifndef MATERIAL_H_
#define MATERIAL_H_

#include <iostream>

#include "color.h"

struct Material 
{
    Color color;
    Material(Color c) : color(c) {}
};

#endif