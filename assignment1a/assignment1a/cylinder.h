#ifndef CYLINDER_H_
#define CYLINDER_H_

#include <iostream>
#include <tuple>

#include "object.h"

class Cylinder : public Object
{
public:
    vec3 dir, top;
    float rad, len;
    Cylinder(vec3 c, vec3 d, float r, float l, Material* m)
        : rad(r), len(l)
    {
        mat = m;
        center = c;
        dir = d;
        dir.normalize();
        top = center + dir.toLength(len);
    }

    tuple<bool, float> intersects(Ray& ray)
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
            No code was gotten from these resources, just the math.
            Hope that's okay :)
        */

        /* vector pointing from ray orig to center of cylinder */
        vec3 w = ray.orig - this->center;

        float wdoth = w.dot(this->dir);
        float vdoth = ray.dir.dot(this->dir);

        float a = 
            ray.dir.dot(ray.dir) - vdoth*vdoth;
        float b = 
            2 * (ray.dir.dot(w) - vdoth * wdoth);
        float c =
            w.dot(w) - wdoth*wdoth - this->rad*this->rad;

        float d = b*b - 4*a*c;
        
        /* no intersection */
        if (d < 0) return tuple<bool, float>{false, 0.0};

        /* single intersection, or parallel */
        if (d < 9e-10) 
        {
            /* 
                In the case where the ray is exactly parallel,
                 we want to act like it did not hit.
            */
            // if (fabs(vdoth-1) < 9e-10) 
            //     return tuple<bool, float>{false, 0.0};
            
            float t = -b / (2*a);
            return this->handle_finite(t, ray);
        }

        /* 
            Two intersection points.
            Only care about the closest 
        */
        float t1 = (-b + sqrt(d)) / (2*a);
        float t2 = (-b - sqrt(d)) / (2*a);


        if (t1 < t2 && t1 >= 0)
            return this->handle_finite(t1, ray);

        if (t2 < t1 && t2 >= 0)
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

    tuple<bool, float> handle_finite(float t, Ray& r)
    { 
        float proj = ((r.orig + r.dir*t) - this->center).dot(this->dir);

        /* intersection on surface of side of cylinder */
        if (0 <= proj && proj <= this->len)
            return tuple<bool, float>{true, t};

        /* need to test for the caps */

        
        /* if proj less than 0, test base cap */
        if (proj < 0)
            return this->intersect_caps(this->center, r);
        
        /* if proj > len, top cap */
        return this->intersect_caps(this->top, r);
        
    }

    tuple<bool, float> intersect_caps(vec3& cap, Ray& r)
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
        vec3 int_point = r.orig + r.dir * t_cap;
        if (int_point.distanceTo(cap) > this->rad)
            return tuple<bool, float>{false, 0.0};
        
        if (t_cap < 0)
            return tuple<bool, float>{false, 0.0};

        return tuple<bool, float>{true, t_cap};
    }
};

#endif