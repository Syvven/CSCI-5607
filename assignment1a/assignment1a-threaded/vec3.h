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
    float epsilon = 9e-13;
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

    bool operator==(const vec3& ovec)
    {
        if (fabs(x - ovec.x) < epsilon &&
            fabs(y - ovec.y) < epsilon &&
            fabs(z - ovec.z) < epsilon) return true;
        return false;
    }
    bool operator!=(const vec3& ovec)
    {
        if (fabs(x - ovec.x) < epsilon &&
            fabs(y - ovec.y) < epsilon &&
            fabs(z - ovec.z) < epsilon) return false;
        return true;
    }
    
    vec3 operator+(const vec3& ovec) 
    {
        return vec3(x+ovec.x, y+ovec.y, z+ovec.z);
    }
    void operator+=(const vec3& ovec)
    {
        x += ovec.x;
        y += ovec.y;
        z += ovec.z;
    }

    vec3 operator-(const vec3& ovec)
    {
        return vec3(x-ovec.x, y-ovec.y, z-ovec.z);
    }
    void operator-=(const vec3& ovec)
    {
        x -= ovec.x;
        y -= ovec.y;
        z -= ovec.z; 
    }   

    vec3 operator*(float f)
    {
        return vec3(x*f, y*f, z*f);
    }
    void operator*=(float f)
    {
        x *= f;
        y *= f;
        z *= f;
    }

    vec3 operator/(float f)
    {
        if (f < epsilon) 
        {
            cerr << "vec3 divide: Divide By Zero Error" << endl;
            exit(EXIT_FAILURE);
        }
        return vec3(x/f, y/f, z/f);
    }
    void operator/=(float f)
    {
        if (f < epsilon)
        {
            cerr << "vec3 divide: Divide By Zero Error" << endl;
            exit(EXIT_FAILURE);
        }
        x /= f;
        y /= f;
        z /= f;
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
        if (mag < epsilon) 
        {
            cerr << "vec3 normalize: Divide By Zero Error" << endl;
            exit(EXIT_FAILURE);
        }
        x /= mag;
        y /= mag;
        z /= mag;
    }
    vec3 normalized()
    {
        float mag = sqrt(x*x+y*y+z*z);
        if (mag < epsilon) {
            cerr << "vec3 normalized: Divide By Zero Error" << endl;
            exit(EXIT_FAILURE);
        }
        return vec3(x/mag, y/mag, z/mag);
    }

    /* sets to the specified length i feel thats pretty obvious */
    void setToLength(float newL)
    {
        float magnitude = sqrt(x*x + y*y + z*z);
        if (magnitude < epsilon)
        {
            cerr << "vec3 setToLength: Divide By Zero Error" << endl;
            exit(EXIT_FAILURE);
        }
        x *= newL/magnitude;
        y *= newL/magnitude;
        z *= newL/magnitude;
    }

    vec3 toLength(float newL)
    {
        float magnitude = sqrt(x*x + y*y + z*z);
        if (magnitude < epsilon)
        {
            cerr << "vec3 setToLength: Divide By Zero Error" << endl;
            exit(EXIT_FAILURE);
        }
        float f = newL/magnitude;
        return vec3(x*f, y*f, z*f);
    }

    /* i never used this lol */
    vec3 ortho_project_onto(vec3& ovec)
    {
        return ovec * (this->dot(ovec) / ovec.dot(ovec));
    }

    /* in uxv, this is u and ovec is v */
    /* produces vector orthogonal to plane spanned by u and v */
    vec3 cross(const vec3& ovec)
    {
        return vec3(
            y*ovec.z - z*ovec.y,
            z*ovec.x - x*ovec.z,
            x*ovec.y - y*ovec.x
        );
    }

    float dot(const vec3& ovec)
    {
        return x*ovec.x + y*ovec.y + z*ovec.z;
    }

    /* basic distance formula */
    float distanceTo(const vec3& ovec)
    {
        float dx = pow(x - ovec.x, 2);
        float dy = pow(y - ovec.y, 2);
        float dz = pow(z - ovec.z, 2);
        return sqrt(dx + dy + dz);
    }

    float angle_between_rad(vec3& v2)
    {
    	float n = this->dot(v2);
    	float d = this->length() * v2.length();
        if (d < epsilon)
        {
            cerr << "vec3 angle_between_rad: Divide By Zero Error" << endl;
            exit(EXIT_FAILURE);
        }
    	return acos(n / d);
    }

    float angle_between_deg(vec3& v2)
    {
        float n = this->dot(v2);
    	float d = this->length() * v2.length();
        if (d < epsilon)
        {
            cerr << "vec3 angle_between_deg: Divide By Zero Error" << endl;
            exit(EXIT_FAILURE);
        }
    	return acos(n / d) * 180 / M_PI;
    }

    /* allows printing of vec3 objects */
    string toString() const
    {   
        return 
            "(" + 
            to_string(x) + 
            ", " + 
            to_string(y) + 
            ", " + 
            to_string(z) + 
            ")";
    }
	friend ostream& operator<<(ostream& s, const vec3& vec) {
		s << vec.toString();

		return s;
	}
};

#endif