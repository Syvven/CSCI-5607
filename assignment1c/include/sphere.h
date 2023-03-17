#ifndef SPHERE_H_
#define SPHERE_H_

#include "object.h"

using namespace std;

class Sphere : public Object
{
public:
    float rad, r_sq;
    Texture* texture;
    NormalMap* normal_map;
    int textured = 0;
    int normal_mapped = 0;
    float oneoverpi;
    float oneover2pi;
    float twopi;
    Sphere(vec3 c, float r, Material* m, 
           Texture* _texture=nullptr, NormalMap* nor=nullptr)
    {
        rad = r;
        mat = m;
        center = c;
        r_sq = r*r;
        id = get_id();

        if (_texture != nullptr)
        {
            textured = 1;
            texture = _texture;
        }

        if (nor != nullptr)
        {
            normal_mapped = 1;
            normal_map = nor;
        }

        oneover2pi = 1 / (2*M_PI);
        twopi = 2*M_PI;
        oneoverpi = 1 / M_PI;
    }

    tuple<bool, float> intersects(Ray& ray)
    {
        /* float a = 1; /* ray is guaranteed to be normalized */
        /* direction dot (origin - center) */
        vec3 omc = ray.orig - this->center;
        float b = 2.0f * (ray.dir.dot(omc));
        float c = omc.lengthSqr() - this->r_sq;

        float d = b*b - 4.0f*c;

        /* no intersection if discriminant is less than 0 */        
        if (d < 0.0f) return tuple<bool, float>{false, -9e15f};

        /* actual distances */
        float sqrtd = sqrt(d);
        float t1 = (-b + sqrtd) * 0.5f;
        float t2 = (-b - sqrtd) * 0.5f;
        
        if (t1 < t2 && t1 >= 0)
            return tuple<bool, float>{true, t1};
        
        if (t2 < t1 && t2 >= 0)
            return tuple<bool, float>{true, t2};

        /* both intersection points were behind */
        return tuple<bool, float>{false, -9e15f};
    }

    vec3 get_normal(vec3& x)
    {
        vec3 n = x;
        n -= center;
        n.normalize();

        if (!normal_mapped) return n;
        
        /* calculate the TBN matrix */
        vec3 Tvec, Bvec;

        float snxy = sqrt(n.x*n.x + n.y*n.y);
        float osnxy = 1 / snxy;
        Tvec.x = -n.y * osnxy;
        Tvec.y = n.x * osnxy; 

        Tvec.normalize();

        Bvec.x = -n.z*Tvec.y;
        Bvec.y = n.z*Tvec.x;
        Bvec.z = snxy;

        /* get the u and v texture coordinates */

        float theta = atan2(n.y, n.x);
        float phi   = acos(n.z);

        float u = (theta > 0) * theta * oneover2pi + 
                  (theta < 0) * (theta + twopi) * oneover2pi;

        float v = phi * oneoverpi;

        vec3 m(normal_map->get_normal_at(u, v));

        return vec3(
            m.x*Tvec.x + m.y*Bvec.x + m.z*n.x,
            m.x*Tvec.y + m.y*Bvec.y + m.z*n.y,
            m.x*Tvec.z + m.y*Bvec.z + m.z*n.z
        ).normalized();
    }

    Color* get_diffuse(vec3& x)
    {
        if (!textured) return &this->mat->diffuse;

        /* 
            Find u and v: 
             u = (theta + pi) / (2*pi)
             v = phi / pi

             theta = atan2(Ny, Nx)
             phi    = acos(Nz)
        */

        vec3 n = x;
        n -= center;
        n.normalize();

        float theta = atan2(n.y, n.x);
        float phi   = acos(n.z);

        float u = (theta > 0) * theta * oneover2pi + 
                  (theta < 0) * (theta + twopi) * oneover2pi;

        float v = phi * oneoverpi;
 
        return this->texture->get_color_at(u,v);
    }

    Color* get_specular()
    {
        return &this->mat->specular;
    }
};

#endif