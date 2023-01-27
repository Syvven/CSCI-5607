#ifndef VEC3_H_
#define VEC3_H_

#include <iostream>
#include <math.h>
#include <string>

using namespace std;

struct vec3 
{
    /* x, y, z positions */
    float x, y, z;

    /* default constructor sets x,y,z to 0 */
    vec3() : x(0), y(0), z(0) {}
    /* normal and copy constructors */
    vec3(float x_, float y_, float z_)
        : x(x_), y(y_), z(z_) {}
    vec3(const vec3& ovec)
    {
        x = ovec.x;
        y = ovec.y;
        z = ovec.z;
    }
    /* overloading assignment */
    void operator=(const vec3& ovec)
    {
        x = ovec.x;
        y = ovec.y;
        z = ovec.z;
    }

    void operator+(const vec3& ovec) 
    {
        x += ovec.x;
        y += ovec.y;
        z += ovec.z;
    }
    void operator-(const vec3& ovec)
    {
        x -= ovec.x;
        y -= ovec.y;
        z -= ovec.z;
    }
    void operator/(float f)
    {
        if (f < 9e-15) return;
        x /= f;
        y /= f;
        z /= f;
    }
    void operator*(float f)
    {
        x *= f;
        y *= f;
        z *= f;
    }

    /* euclidean norm */
    float length()
    {
        return sqrt(x*x + y*y + z*z);
    }

    /* the other norm */
    float lengthSqr()
    {
        return (x*x + y*y + z*z);
    }

    /* unit length */
    void normalize()
    {
        float mag = sqrt(x*x + y*y + z*z);
        if (mag < 9e-15) return;
        x /= mag;
        y /= mag;
        z /= mag;
    }

    /* in uxv, this is u and ovec is v */
    vec3& cross(vec3& ovec)
    {
        vec3 *v = new vec3(
            y*ovec.z - z*ovec.y,
            z*ovec.x - x*ovec.z,
            x*ovec.y - y*ovec.x
        );

        return *v;
    }

    float dot(vec3& ovec)
    {
        return x*ovec.x + y*ovec.y + z*ovec.z;
    }

    /* basic distance formula */
    float distanceTo(vec3& ovec)
    {
        float dx = pow(x - ovec.x, 2);
        float dy = pow(y - ovec.y, 2);
        float dz = pow(z - ovec.z, 2);
        return sqrt(dx + dy + dz);
    }


    string toString()
    {   
        return "(" + to_string(x) + ", " + to_string(y) + ", " + to_string(z) + ")";
    }
	friend ostream& operator<<(ostream& s, vec3& vec) {
		s << vec.toString();

		return s;
	}
};

#endif