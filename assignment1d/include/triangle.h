#ifndef TRIANGLE_H_
#define TRIANGLE_H_

#include "object.h"

using namespace std;

class Triangle : public Object 
{
public:
    vec3 v0,v1,v2;
    vec3 v0n,v1n,v2n;
    vec3 v0t,v1t,v2t;
    vec3 e1,e2;
    vec3 normal;
    Texture* texture;
    NormalMap* norm_map;
    float A,B,C,D;
    float d11,d22,d12,det;
    int can_intersect = 1;
    int smooth = 0;
    int textured = 0;
    int normal_mapped = 0;

    vec3 Tvec,Bvec,centroid;

    Triangle(vec3* _v1, vec3* _v2, vec3* _v3, 
             vec3* _v1n, vec3* _v2n, vec3* _v3n, 
             vec3* _v1t, vec3* _v2t, vec3* _v3t,
             Material *m, Texture* text, NormalMap* nmap)
    {
        id = Utils::getUid();
        v0 = *_v1; 
        v1 = *_v2; 
        v2 = *_v3; 

        int all_null = (_v1n == nullptr && _v2n == nullptr && _v3n == nullptr);
        int none_null = (_v1n != nullptr && _v2n != nullptr && _v3n != nullptr);
        if (none_null)
        {
            v0n = *_v1n;
            v1n = *_v2n;
            v2n = *_v3n;

            v0n.normalize();
            v1n.normalize();
            v2n.normalize();

            smooth = 1;
        } 
        else if (!all_null)
        {
            cerr << "Missing a vertex normal." << endl;
            exit(EXIT_FAILURE);
        }
        
        all_null = (_v1t == nullptr && _v2t == nullptr && _v3t == nullptr);
        none_null = (_v1t != nullptr && _v2t != nullptr && _v3t != nullptr);
        if (none_null)
        {
            v0t = *_v1t;
            v1t = *_v2t;
            v2t = *_v3t;

            assert(text != nullptr || nmap != nullptr);

            if (text != nullptr)
            {
                texture = text;
                textured = 1;
            }

            if (nmap != nullptr)
            {
                norm_map = nmap;
                normal_mapped = 1;
            }

        } 
        else if (!all_null)
        {
            cerr << "Missing a vertex texture coordinate." << endl;
            exit(EXIT_FAILURE);
        }

        mat = m;

        e1 = v1 - v0;
        e2 = v2 - v0;

        this->calculate_normal();
        this->calculate_abc();

        this->centroid = v0 + v1 + v2;
        this->centroid /= 3;
    }

    ~Triangle() {}

    bool intersects(Ray& ray, bool shadow)
    {
        /* 
            tn = (-Ax0 - By0 - Cz0 - D) 
            td = (Axd + Byd + Czd)
            if td = 0, no intersect, parallel
        */

        if (!can_intersect) return false;

        float td = (A*ray.dir.x) + (B*ray.dir.y) + (C*ray.dir.z);
        if (abs(td) < 1e-15f) return false;
        
        float tn = (-A*ray.orig.x) - (B*ray.orig.y) - (C*ray.orig.z) - D;

        float t = tn / td;

        if (t < 0 || ray.t < t) return false;

        /* got t, need to find barycentric coords */
        vec3 p = ray.dir;
        p *= t;
        p += ray.orig;

        vec3 ep = p;
        ep -= v0;

        float d1p = e1.dot(ep);
        float d2p = e2.dot(ep);
        
        float beta = (d22*d1p - d12*d2p) / det;
        float gamma = (d11*d2p - d12*d1p) / det;
        float alpha = 1 - (beta + gamma);

        /* make sure in bounds of triangle */
        if (beta > 1 || beta < 0) 
            return false;
        if (gamma > 1 || gamma < 0)
            return false;
        if (alpha > 1 || alpha < 0)
            return false;

        ray.t = t;
        ray.obj = this;
        ray.alpha = alpha;
        ray.beta = beta;
        ray.gamma = gamma;

        return true;
    }

    Material* getMat(Ray& r) {return mat;}

    vec3 get_normal(Ray& r)
    {
        if (!smooth && !normal_mapped) return this->normal;

        /* 
            same thing as if textured but this time
             for the normal map. Have to find the 
             TBN matrix in order to translate to 
             coordinate space.
        */

        if (!normal_mapped)
        {
            vec3 n;
            n.x = r.alpha*v0n.x + r.beta*v1n.x + r.gamma*v2n.x;
            n.y = r.alpha*v0n.y + r.beta*v1n.y + r.gamma*v2n.y;
            n.z = r.alpha*v0n.z + r.beta*v1n.z + r.gamma*v2n.z;
            n.normalize();

            return n;
        }

        r.u = v0t.x*r.alpha + v1t.x*r.beta + v2t.x*r.gamma;
        r.v = v0t.y*r.alpha + v1t.y*r.beta + v2t.y*r.gamma;

        vec3 m(norm_map->get_normal_at(r.u, r.v));

        return vec3(
            m.x*Tvec.x + m.y*Bvec.x + m.z*normal.x,
            m.x*Tvec.y + m.y*Bvec.y + m.z*normal.y,
            m.x*Tvec.z + m.y*Bvec.z + m.z*normal.z
        ).normalized();
    }

    Color* get_diffuse(Ray& r) 
    {
        if (!textured) return &this->mat->diffuse;

        if (!normal_mapped)
        {
            r.u = v0t.x*r.alpha + v1t.x*r.beta + v2t.x*r.gamma;
            r.v = v0t.y*r.alpha + v1t.y*r.beta + v2t.y*r.gamma;
        }

        return this->texture->get_color_at(r.u, r.v);
    }
    Color* get_specular(Ray& r) 
    {
        return &this->mat->specular;
    }

    Object* getIntersectedObject(Ray& r)
    {
        return this;
    }

    void calculate_normal() { 
        normal = e1.cross(e2);
        normal.normalize(); 

        /* find TBN matrix if normal mapped */
        if (normal_mapped)
        {
            float du1 = v1t.x - v0t.x;
            float du2 = v2t.x - v0t.x;

            float dv1 = v1t.y - v0t.y;
            float dv2 = v2t.y - v0t.y;

            float d = -du1*dv2 + du2*dv1;
            assert(abs(d) > 1e-15);

            d = 1 / d;

            Tvec = (e1*-dv2 + e2*dv1) * d;
            Bvec = (e1*-du2 + e2*du1) * d;

            Tvec.normalize();
            Bvec.normalize();
        }
    }

    void calculate_abc()
    {
        /* 
            A,B,C are components of vec
             produced by cross prod of e1 and e2
        */
        A = normal.x; B = normal.y; C = normal.z;

        D = -A*v1.x - B*v1.y - C*v1.z;

        d11 = e1.dot(e1);
        d22 = e2.dot(e2);
        d12 = e1.dot(e2);

        det = d11*d22 - d12*d12;
        if (abs(det) < 1e-15f) can_intersect = 0;
    }

    /* printing reasons */
    string toString() const
    {   
        string str("Vertices:\n");
        if (!smooth)
        {
            return 
                str + "(\n"  
                + "   Vertex 1: " + v0.toString() + 
                ",\n" + 
                + "   Vertex 2: " + v1.toString() + 
                ",\n" + 
                + "   Vertex 3: " + v2.toString() + 
                "\n)";
        }

        return 
            str + "(\n" 
            + "   Vertex 1: " + v0.toString() + 
            ",\n" + 
            + "   Vertex 2: " + v1.toString() + 
            ",\n" + 
            + "   Vertex 3: " + v2.toString() + 
            "\n)" + "\nVertex Normals:\n" +
            "(\n"
            + "   Vertex 1: " + v0n.toString() + 
            ",\n" + 
            + "   Vertex 2: " + v1n.toString() + 
            ",\n" + 
            + "   Vertex 3: " + v2n.toString() +
            "\n)";
    }
	friend ostream& operator<<(ostream& s, const Triangle& t) {
		s << t.toString();

		return s;
	}
};

#endif