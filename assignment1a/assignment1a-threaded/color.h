#ifndef COLOR_H_
#define COLOR_H_

#include <iostream>
#include <string>

using namespace std;

struct Color
{
    float r, g, b, r_sq;
    Color() : r(0), g(0), b(0) {}
    Color(float r_, float g_, float b_)
        : r(r_), g(g_), b(b_) {}

    /* allows printing of vec3 objects */
    string toString() const
    {   
        return 
            "(" + 
            to_string(r) + 
            ", " + 
            to_string(g) + 
            ", " + 
            to_string(b) + 
            ")";
    }
	friend ostream& operator<<(ostream& s, const Color& c) {
		s << c.toString();

		return s;
	}
};

#endif