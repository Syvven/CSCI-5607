#ifndef CYLINDER_H_
#define CYLINDER_H_

#include <iostream>
#include <tuple>

#include "object.h"

class Cylinder : public Object
{
public:
    vec3 dir, top, cent_d, top_d, neg_dir, actual_center;
    float rad, len, r_sq;
    Texture* texture;
    NormalMap* normal_map;
    int textured = 0; 
    int normal_mapped = 0;
    Cylinder(vec3 c, vec3 d, float r, float l, Material* m,
             Texture* text=nullptr, NormalMap* nmap=nullptr)
        : rad(r), len(l)
    {
        mat = m;
        dir = d;
        neg_dir = d*-1;
        dir.normalize();
        neg_dir.normalize();
        actual_center = c;
        center = actual_center;
        center -= (dir*(l*0.5f));
        top = actual_center;
        top += (dir*(l*0.5f));
        r_sq = r*r;
        id = Utils::getUid();

        if (text != nullptr)
        {
            texture = text;
            textured = 1;
        }

        if (nmap != nullptr)
        {
            normal_map = nmap;
            normal_mapped = 1;
        }
    }

    Material* getMat(Ray& r) {return mat;}

    Object* getIntersectedObject(Ray& r)
    {
        return this;
    }

    bool intersects(Ray& ray, bool shadow)
    {
        /* 
            Cylinder Equation:
                ||point - center||^2 - [(point - center) dot dir]^2 = r^2
            Parametric Line:
                pt = p0 + t*r
            Derivation Here:
                http://www.illusioncatalyst.com/notes_files/mathematics/line_cylinder_intersection.php
            Cap Intersection Resources:
                https://math.stackexchange.com/questions/395119/ray-disk-intersection
                https://www.nagwa.com/en/explainers/373101390857/#:~:text=Definition%3A%20General%20Form%20of%20the,vector%20parallel%20to%20the%20plane.
                (I couldn't remember the scalar form plane equation lol)
        */

        /* vector pointing from ray orig to center of cylinder */
        vec3 w = ray.orig - this->center;

        float wdoth = w.dot(this->dir);
        float vdoth = ray.dir.dot(this->dir);

        float a = 
            ray.dir.lengthSqr() - vdoth*vdoth;
            
        float b = 
            2.0f * (ray.dir.dot(w) - vdoth * wdoth);

        float c =
            w.lengthSqr() - wdoth*wdoth - this->r_sq;

        float d = b*b - 4.0f*a*c;
        float halfa = 0.5f * (1.0f / a);
        
        /* no intersection */
        if (d < 0.0f) return false;

        /* single intersection, or parallel */
        if (d < 9e-10f) 
        {
            /* 
                In the case where the ray is exactly parallel,
                 we want to act like it did not hit.
            */
            // if (fabs(vdoth-1) < 9e-10) 
            //     return tuple<bool, float>{false, 0.0};
            
            float t = -b * halfa;
            return this->handle_finite(t, ray);
        }

        /* 
            Two intersection points.
            Only care about the closest 
        */
        float sqrtd = sqrt(d);
        float t1 = (-b + sqrtd) * halfa;
        float t2 = (-b - sqrtd) * halfa;


        if (t1 < t2 && t1 >= 0.0f)
            return this->handle_finite(t1, ray);

        if (t2 < t1 && t2 >= 0.0f)
            return this->handle_finite(t2, ray);
        
        /* 
            I needed this case because when the cylinder
             direction was exactly aligned with the camera
             the bottom/top cap of the cylinder wouldn't 
             render.
            This should never be reached unless this specific case
             happens though so its not really contributing to 
             any extra work outside of what is needed.
        */
        float d1 = ray.orig.distanceTo(this->center);
        float d2 = ray.orig.distanceTo(this->top);
        if (d1 < d2)
            return this->intersect_caps(this->center, ray);

        return this->intersect_caps(this->top, ray);
    }

    bool handle_finite(float t, Ray& r)
    { 
        float proj = ((r.orig + r.dir*t) - this->center).dot(this->dir);

        /* intersection on surface of side of cylinder */
        if (0.0f <= proj && proj <= this->len && t < r.t)
        {
            r.t = t;
            r.obj = this;
            return true;
        }

        /* need to test for the caps */

        
        /* if proj less than 0, test base cap */
        if (proj < 0.0f)
            return this->intersect_caps(this->center, r);
        
        /* if proj > len, top cap */
        return this->intersect_caps(this->top, r);
        
    }

    bool intersect_caps(vec3& cap, Ray& r)
    {
        /* 
            Define scalar form plane equation for base cap:
                a(x - x0) + b(y - y0) + c(z - z0) = 0
                a, b, c: components of normal vector to plane
                    -> normal vector is the direction vector of cylinder
                x0, y0, z0: point on plane
            Define ray parametric equation:
                pt = p0 + t*r
                p0: ray origin
                r: ray direction
            We have all those!
            Substitue:
                a(p0x + t*rx - x0) + 
                b(p0y + t*ry - y0) +
                c(p0z + t*rz - z0) = 0

                a*p0x + a*t*rx - a*x0 + 
                b*p0y + b*t*ry - b*y0 +
                c*p0z + c*t*rz - c*z0 = 0

                (a*p0x - a*x0 + 
                b*p0y - b*y0 +
                c*p0z - c*z0) 
                    / (-a*rx - b*ry - c*rz) 
                    = t
        */
        float n1 = this->dir.x*r.orig.x - this->dir.x*cap.x;
        float n2 = this->dir.y*r.orig.y - this->dir.y*cap.y;
        float n3 = this->dir.z*r.orig.z - this->dir.z*cap.z;

        float d1 = -this->dir.x*r.dir.x;
        float d2 = -this->dir.y*r.dir.y;
        float d3 = -this->dir.z*r.dir.z;

        float n = n1 + n2 + n3;
        float d = d1 + d2 + d3;

        float t_cap = n / d;

        /* ensure point is within radius of center of cap */
        if (t_cap < 0.0f) return false;

        vec3 int_point = r.orig + r.dir * t_cap;
        if (int_point.distanceTo(cap) > this->rad) return false;

        if (t_cap < r.t)
        {
            r.t = t_cap;
            r.obj = this;
            return true;
        }

        return false;
    }

    vec3 get_normal(Ray& r)
    {   
        float d = r.int_point.distanceTo(center);
        if (d < rad)
            return vec3(this->neg_dir);
        
        d = r.int_point.distanceTo(this->top);
        if (d < rad)
            return vec3(this->dir);

        /* 
            Project point onto center axis
            Get distance from bottom
            Find center point
            Get normal vector from that 
        */
        float c = r.int_point.distanceTo(this->top);
        c = c*c;
        float b = r_sq;
        float a = sqrt(c-b);

        vec3 n = r.int_point;
        n -= this->top;
        n += (dir*a);
        n.normalize();

        if (!normal_mapped) return n;

        return n;

        // /* in the case of a cylinder, T is the dir */
        // vec3 Tvec = this->dir;

        // vec3 Bvec = Tvec.cross(n);
        // Bvec.normalize();
    }

    Color* get_diffuse(Ray& r)
    {
        if (!textured) return &this->mat->diffuse;
        return &this->mat->diffuse;   
    }
    Color* get_specular(Ray& r) { return &this->mat->specular; }
};

#endif