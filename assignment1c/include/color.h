#ifndef COLOR_H_
#define COLOR_H_

#include <iostream>
#include <string>

using namespace std;

struct Color
{
    float r, g, b, r_sq;
    Color() : r(0.0f), g(0.0f), b(0.0f) {}
    Color(float r_, float g_, float b_)
        : r(r_), g(g_), b(b_) {}

    Color(const Color& oc)
    {
        r = oc.r;
        g = oc.g;
        b = oc.b;
    }

    void operator=(const Color& oc)
    {
        r = oc.r;
        g = oc.g;
        b = oc.b;
    }

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