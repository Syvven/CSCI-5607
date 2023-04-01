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

    Color capMax(float val)
    {
        return Color(min(r, val), min(g, val),  min(b, val));
    }

    Color capMin(float val)
    {
        return Color(max(r, val), max(g, val), max(b, val));
    }

    void capMax2(float val)
    {
        r = min(r, val);
        g = min(g, val);
        b = min(b, val);
    }

    void capMin2(float val)
    {
        r = max(r, val);
        g = max(g, val);
        b = max(b, val);
    }

    void operator=(const Color& oc)
    {
        r = oc.r;
        g = oc.g;
        b = oc.b;
    }

    Color operator+(float mod)
    {
        return Color(r+mod, g+mod, b+mod);
    }

    Color operator+(const Color& oc)
    {
        return Color(r+oc.r, g+oc.g, b+oc.b);
    }

    void operator+=(const Color& ovec)
    {
        r += ovec.r;
        g += ovec.g;
        b += ovec.b;
    }

    Color operator*(float mod)
    {
        return Color(r*mod, g*mod, b*mod);
    }

    Color operator*(const Color& oc)
    {
        return Color(r*oc.r, g*oc.g, b*oc.b);
    }

    void operator*=(const Color& ovec)
    {
        r *= ovec.r;
        g *= ovec.g;
        b *= ovec.b;
    }

    void operator*=(float f)
    {
        r *= f;
        g *= f;
        b *= f;
    }

    friend Color operator*(float f, Color c)
    {
        return Color(f*c.r, f*c.g, f*c.b);
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