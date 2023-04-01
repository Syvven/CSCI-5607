
#ifndef MESH_H_
#define MESH_H_

#include <stack>

#include "object.h"
#include "triangle.h"

// static bool compareAxis0(Object* o1, Object* o2)
// {
//     return (
//         ((Triangle*)o1)->centroid.x < ((Triangle*)o2)->centroid.x
//     );
// }

// static bool compareAxis1(Object* o1, Object* o2)
// {
//     return (
//         ((Triangle*)o1)->centroid.y < ((Triangle*)o2)->centroid.y
//     );
// }

// static bool compareAxis2(Object* o1, Object* o2)
// {
//     return (
//         ((Triangle*)o1)->centroid.z < ((Triangle*)o2)->centroid.z
//     );
// }

typedef struct Level
{
    bool is_leaf;
    float value;
    int axis;
    vec3 min;
    vec3 max;
    Level* l;
    Level* r;
    vector<Object*> objs;
    
} Level;

class Mesh : public Object
{
public:
    Mesh(vector<Object*>& polygons, int levels)
    {
        /* start bvh construction */
        bvh_head = new Level;

        /* first add polygons to the local vector */
        for (int i = 0; i < polygons.size(); i++)
            polys.push_back(polygons[i]);

        for (int i = 0; i < polygons.size(); i++)
        {
            vec3 v0 = ((Triangle*)polygons[i])->v0;
            vec3 v1 = ((Triangle*)polygons[i])->v1;
            vec3 v2 = ((Triangle*)polygons[i])->v2;

            bvh_head->min.x = min(bvh_head->min.x, min(v0.x, min(v1.x, v2.x)));
            bvh_head->min.y = min(bvh_head->min.y, min(v0.y, min(v1.y, v2.y)));
            bvh_head->min.z = min(bvh_head->min.z, min(v0.z, min(v1.z, v2.z)));

            bvh_head->max.x = max(bvh_head->max.x, max(v0.x, max(v1.x, v2.x)));
            bvh_head->max.y = max(bvh_head->max.y, max(v0.y, max(v1.y, v2.y)));
            bvh_head->max.z = max(bvh_head->max.z, max(v0.z, max(v1.z, v2.z)));
        }

        bvh_head->l = new Level;
        bvh_head->r = new Level;

        bvh_head->objs = polys;

        construct_bvh_level(bvh_head, levels);

        // this->levels = max(0, levels);

        // this->head = build_tree(0, polys.size(), 0);
        // assert(assert_tree(this->head) == polys.size());

        id = Utils::getUid();
    }

    // int assert_tree(Level* l)
    // {
    //     if (l == nullptr) return 0;

    //     int count = l->objs.size();
    //     for (auto& o : l->objs)
    //     {
    //         cout << o->id << endl;
    //     }
    //     count += assert_tree(l->left);
    //     count += assert_tree(l->right);
    //     return count;
    // }


    // /* akin to building a kd tree for knn */
    // Level* build_tree(int start, int end, int axis_control)
    // {

    //     if ((end - start) <= 0) return nullptr;

    //     Level* l = new Level;

    //     l->axis = axis_control % 3;

    //     /* find the median of the data along the axis */
    //     /* oh god i wish i had numpy rn */

    //     switch (l->axis) 
    //     {
    //         case 0: {
    //             sort(
    //                 polys.begin() + start, 
    //                 polys.begin() + end, 
    //                 compareAxis0
    //             );
    //         }
    //         case 1: {
    //             sort(
    //                 polys.begin() + start, 
    //                 polys.begin() + end, 
    //                 compareAxis1
    //             );
    //         }
    //         case 2: {
    //             sort(
    //                 polys.begin() + start, 
    //                 polys.begin() + end, 
    //                 compareAxis2
    //             );
    //         }
    //     }

    //     int half = (start+end)/2;
    //     l->objs.push_back(polys[half]);
    //     Triangle* t = (Triangle*)(l->objs[0]);

    //     l->value = t->centroid.getAxis(l->axis);

    //     /* find min and max for current data point */

    //     l->min = vec3::min3(t->v0, t->v1, t->v2);
    //     l->max = vec3::max3(t->v0, t->v1, t->v2);

    //     /* recurse */
    //     l->left  = build_tree(start, half, axis_control+1);
    //     l->right = build_tree(half+1, end, axis_control+1);

    //     /* merge bounding boxes of children recursively */

    //     if (l->left != nullptr && l->right != nullptr)
    //     {
    //         l->min = vec3::min3(l->min, l->right->min, l->left->min);
    //         l->max = vec3::max3(l->max, l->right->max, l->left->max);
            
    //         // if (axis_control >= this->levels)
    //         // {
    //         //     for (auto& o : l->right->objs)
    //         //         l->objs.push_back(o);

    //         //     for (auto& o : l->left->objs)
    //         //         l->objs.push_back(o);

    //         //     l->left = nullptr;
    //         //     l->right = nullptr;
    //         //     l->is_leaf = true;
    //         // }
    //     }

    //     else if (l->left != nullptr)
    //     {
    //         l->min = vec3::min2(l->min, l->left->min);
    //         l->max = vec3::max2(l->max, l->left->max);

    //         // if (axis_control >= this->levels)
    //         // {
    //         //     for (auto& o : l->left->objs)
    //         //         l->objs.push_back(o);

    //         //     l->left = nullptr;
    //         //     l->is_leaf = true;
    //         // }
    //     }

    //     else if (l->right != nullptr)
    //     {
    //         l->min = vec3::min2(l->min, l->right->min);
    //         l->max = vec3::max2(l->max, l->right->max);

    //         // if (axis_control >= this->levels)
    //         // {
    //         //     for (auto& o : l->right->objs)
    //         //         l->objs.push_back(o);

    //         //     l->right = nullptr;
    //         //     l->is_leaf = true;
    //         // }
    //     }
    //     else 
    //     {
    //         l->is_leaf = true;
    //     }
        
    //     return l;
    // }

    void construct_bvh_level(Level* box, int level)
    {
        if (level == 0) 
        {
            box->l = nullptr; box->r = nullptr;
            for (auto& face : polys)
            {
                vec3 v0 = ((Triangle*)face)->v0;
                vec3 v1 = ((Triangle*)face)->v1;
                vec3 v2 = ((Triangle*)face)->v2;
                vec3 c = ((Triangle*)face)->centroid;

                if ((v0.x <= box->max.x && v0.x >= box->min.x &&
                     v0.y <= box->max.y && v0.y >= box->min.y &&
                     v0.z <= box->max.z && v0.z >= box->min.z) ||
                    (v1.x <= box->max.x && v1.x >= box->min.x &&
                     v1.y <= box->max.y && v1.y >= box->min.y &&
                     v1.z <= box->max.z && v1.z >= box->min.z) ||
                    (v2.x <= box->max.x && v2.x >= box->min.x &&
                     v2.y <= box->max.y && v2.y >= box->min.y &&
                     v2.z <= box->max.z && v2.z >= box->min.z) ||
                    (c.x <= box->max.x && c.x >= box->min.x &&
                     c.y <= box->max.y && c.y >= box->min.y &&
                     c.z <= box->max.z && c.z >= box->min.z))
                {
                    box->objs.push_back(face);
                    count++;
                }
            }
            return;
        }

        /* 
            Find the min and max of the new box 
            First step is to find longest of width/length/height
        */

        box->l = new Level;
        box->r = new Level;

        box->r->min = box->min;
        box->r->max = box->max;
        box->l->min = box->min;
        box->l->max = box->max;

        vec3 lengths = box->max - box->min;

        if (lengths.x >= lengths.y && lengths.x >= lengths.z)
        {
            box->r->min.x += lengths.x*0.5;
            box->l->max.x -= lengths.x*0.5;
        }

        else if (lengths.y >= lengths.x && lengths.y >= lengths.z)
        {
            box->r->min.y += lengths.y*0.5;
            box->l->max.y -= lengths.y*0.5;
        }

        else if (lengths.z >= lengths.x && lengths.z >= lengths.y)
        {
            box->r->min.z += lengths.z*0.5;
            box->l->max.z -= lengths.z*0.5;
        }

        construct_bvh_level(box->l, level-1);
        construct_bvh_level(box->r, level-1);
    }

    ~Mesh()
    {
        for (int i = 0; i < polys.size(); i++)
            delete polys[i];
    }

    /* 
        Pseudocode sources: 
            https://www.keithlantz.net/2013/04/kd-tree-construction-using-the-surface-area-heuristic-stack-based-traversal-and-the-hyperplane-separation-theorem/
            https://graphics.stanford.edu/papers/i3dkdtree/gpu-kd-i3d.pdf
            http://www.sci.utah.edu/~wald/PhD/wald_phd.pdf
    */
    // bool traverse_tree(Ray& ray, float tmin, float tmax)
    // {
    //     bool intersect = false;
    //     stack<tuple<Level*, float, float>> s;
    //     s.push(
    //         tuple<Level*, float, float>{
    //             head, tmin, tmax
    //         }
    //     );

    //     while (!s.empty() && !intersect)
    //     {
    //         auto tup = s.top();
    //         Level* node = get<0>(tup);
    //         tmin = get<1>(tup);
    //         tmax = get<2>(tup);
    //         s.pop();
    //         while (!node->is_leaf)
    //         {
    //             Level* first;
    //             Level* second;
    //             int a = node->axis;
    //             int cond1 = (
    //                 node->value - ray.orig.getAxis(a)
    //             );

    //             first = node->right;
    //             second = node->left;
    //             if (cond1 >= 0)
    //             {
    //                 first = node->left;
    //                 second = node->right;
    //             }

    //             float tsplit = cond1 / ray.dir.getAxis(a);
    //             if (tsplit >= tmax || tsplit < 0) node = first;
    //             else if (tsplit <= tmin) node = second;
    //             else 
    //             {
    //                 s.push(tuple<Level*, float, float>{
    //                     second, tsplit, tmax
    //                 });
    //                 node = first;
    //                 tmax = tsplit;
    //             }
    //         }

    //         for (auto& o : node->objs) 
    //         {
    //             if (o->intersects(ray, false)) 
    //             {
    //                 intersect = true;
    //                 ray.mesh_obj = o;
    //             }
    //         }
    //         if (intersect && ray.t > tmax) intersect = false;
    //     }

    //     return intersect;
    // }

    bool traverse_bvh(Level* box, Ray& ray, bool shadow)
    {
        if (box->l == nullptr && box->r == nullptr)
        {
            if (box->objs.size() == 0) return false;

            bool min_found = false;

            for (auto& face : box->objs)
            {
                if (face->intersects(ray, shadow)) 
                {
                    min_found = true;
                    ray.mesh_obj = face;
                }
            }

            return min_found;
        }

        float tx1l = (box->l->min.x - ray.orig.x)*ray.dir_inv.x;
        float tx2l = (box->l->max.x - ray.orig.x)*ray.dir_inv.x;

        float tminl = min(tx1l, tx2l);
        float tmaxl = max(tx1l, tx2l);

        float ty1l = (box->l->min.y - ray.orig.y)*ray.dir_inv.y;
        float ty2l = (box->l->max.y - ray.orig.y)*ray.dir_inv.y;

        tminl = max(tminl, min(ty1l, ty2l));
        tmaxl = min(tmaxl, max(ty1l, ty2l));

        float tz1l = (box->l->min.z - ray.orig.z)*ray.dir_inv.z;
        float tz2l = (box->l->max.z - ray.orig.z)*ray.dir_inv.z;

        tminl = max(tminl, min(tz1l, tz2l));
        tmaxl = min(tmaxl, max(tz1l, tz2l));

        float tx1r = (box->r->min.x - ray.orig.x)*ray.dir_inv.x;
        float tx2r = (box->r->max.x - ray.orig.x)*ray.dir_inv.x;

        float tminr = min(tx1r, tx2r);
        float tmaxr = max(tx1r, tx2r);

        float ty1r = (box->r->min.y - ray.orig.y)*ray.dir_inv.y;
        float ty2r = (box->r->max.y - ray.orig.y)*ray.dir_inv.y;

        tminr = max(tminr, min(ty1r, ty2r));
        tmaxr = min(tmaxr, max(ty1r, ty2r));

        float tz1r = (box->r->min.z - ray.orig.z)*ray.dir_inv.z;
        float tz2r = (box->r->max.z - ray.orig.z)*ray.dir_inv.z;

        tminr = max(tminr, min(tz1r, tz2r));
        tmaxr = min(tmaxr, max(tz1r, tz2r));

        int go_left = 0, go_right = 0;
        if (tmaxl >= max(0.0f, tminl)) go_left = 1;
        if (tmaxr >= max(0.0f, tminr)) go_right = 1;

        if (!go_left && !go_right) return false;
        if (go_left && !go_right) return traverse_bvh(box->l, ray, shadow);
        if (go_right && !go_left) return traverse_bvh(box->r, ray, shadow);

        if (tminr < tmaxr) 
        {
            if (traverse_bvh(box->r, ray, shadow)) return true;
            return traverse_bvh(box->l, ray, shadow);
        }

        if (traverse_bvh(box->l, ray, shadow)) return true;
        return traverse_bvh(box->r, ray, shadow);
    }

    // void shadow_traverse_tree(Ray& ray, int tmin, int tmax)
    // {
    //     stack<tuple<Level*, float, float>> s;
    //     s.push(
    //         tuple<Level*, float, float>{
    //             head, tmin, tmax
    //         }
    //     );
    //     while (!s.empty())
    //     {
    //         auto tup = s.top();
    //         s.pop();
    //         Level* node = get<0>(tup);
    //         tmin = get<1>(tup);
    //         tmax = get<2>(tup);
    //         while (!node->is_leaf)
    //         {
    //             Level* first;
    //             Level* second;
    //             int a = node->axis;
    //             int cond1 = (
    //                 node->value - ray.orig.getAxis(a)
    //             );

    //             first = node->right;
    //             second = node->left;
    //             if (cond1 >= 0)
    //             {
    //                 first = node->left;
    //                 second = node->right;
    //             }

    //             float tsplit = cond1 / ray.dir.getAxis(a);
    //             if (tsplit >= tmax || tsplit < 0)
    //                 node = first;
    //             else if (tsplit <= tmin)
    //                 node = second;
    //             else 
    //             {
    //                 s.push(tuple<Level*, float, float>{
    //                     second, tsplit, tmax
    //                 });
    //                 node = first;
    //                 tmax = tsplit;
    //             }
    //         }

    //         for (auto& o : node->objs) 
    //         {
    //             ray.t = 9e15;
    //             if (o->intersects(ray, true)) ray.mesh_objs.push_back(o);
    //         }
    //     }
    // }

    void shadow_traverse_bvh(Level* box, Ray& ray)
    {
        if (box->l == nullptr && box->r == nullptr)
        {
            if (box->objs.size() == 0) return;

            for (auto& face : box->objs)
            {
                ray.t = 9e15;
                if (face->intersects(ray, false)) 
                    ray.mesh_objs2[face->id] = face;
            }

            return;
        }

        float tx1l = (box->l->min.x - ray.orig.x)*ray.dir_inv.x;
        float tx2l = (box->l->max.x - ray.orig.x)*ray.dir_inv.x;

        float tminl = min(tx1l, tx2l);
        float tmaxl = max(tx1l, tx2l);

        float ty1l = (box->l->min.y - ray.orig.y)*ray.dir_inv.y;
        float ty2l = (box->l->max.y - ray.orig.y)*ray.dir_inv.y;

        tminl = max(tminl, min(ty1l, ty2l));
        tmaxl = min(tmaxl, max(ty1l, ty2l));

        float tz1l = (box->l->min.z - ray.orig.z)*ray.dir_inv.z;
        float tz2l = (box->l->max.z - ray.orig.z)*ray.dir_inv.z;

        tminl = max(tminl, min(tz1l, tz2l));
        tmaxl = min(tmaxl, max(tz1l, tz2l));

        float tx1r = (box->r->min.x - ray.orig.x)*ray.dir_inv.x;
        float tx2r = (box->r->max.x - ray.orig.x)*ray.dir_inv.x;

        float tminr = min(tx1r, tx2r);
        float tmaxr = max(tx1r, tx2r);

        float ty1r = (box->r->min.y - ray.orig.y)*ray.dir_inv.y;
        float ty2r = (box->r->max.y - ray.orig.y)*ray.dir_inv.y;

        tminr = max(tminr, min(ty1r, ty2r));
        tmaxr = min(tmaxr, max(ty1r, ty2r));

        float tz1r = (box->r->min.z - ray.orig.z)*ray.dir_inv.z;
        float tz2r = (box->r->max.z - ray.orig.z)*ray.dir_inv.z;

        tminr = max(tminr, min(tz1r, tz2r));
        tmaxr = min(tmaxr, max(tz1r, tz2r));

        int go_left = 0, go_right = 0;
        if (tmaxl >= max(0.0f, tminl)) go_left = 1;
        if (tmaxr >= max(0.0f, tminr)) go_right = 1;

        if (!go_left && !go_right) return;
        if (go_left) shadow_traverse_bvh(box->l, ray);
        if (go_right) shadow_traverse_bvh(box->r, ray);
    }

    void shadow_intersect(Ray& ray)
    {
        /* box intersection gotten from https://tavianator.com/2011/ray_box.html */
        /* https://tavianator.com/cgit/dimension.git/tree/libdimension/bvh/bvh.c#n194 */
        /* its not quick but I would have to change more for that */

        ray.dir_inv.x = 1/ray.dir.x;
        ray.dir_inv.y = 1/ray.dir.y;
        ray.dir_inv.z = 1/ray.dir.z;

        float tx1 = (bvh_head->min.x - ray.orig.x)*ray.dir_inv.x;
        float tx2 = (bvh_head->max.x - ray.orig.x)*ray.dir_inv.x;

        float tmin = min(tx1, tx2);
        float tmax = max(tx1, tx2);

        float ty1 = (bvh_head->min.y - ray.orig.y)*ray.dir_inv.y;
        float ty2 = (bvh_head->max.y - ray.orig.y)*ray.dir_inv.y;

        tmin = max(tmin, min(ty1, ty2));
        tmax = min(tmax, max(ty1, ty2));

        float tz1 = (bvh_head->min.z - ray.orig.z)*ray.dir_inv.z;
        float tz2 = (bvh_head->max.z - ray.orig.z)*ray.dir_inv.z;

        tmin = max(tmin, min(tz1, tz2));
        tmax = min(tmax, max(tz1, tz2));

        if (tmax < max(0.0f, tmin)) return;

        shadow_traverse_bvh(bvh_head, ray);
    }

    bool intersects(Ray& ray, bool shadow)
    {
        /* box intersection gotten from https://tavianator.com/2011/ray_box.html */
        /* https://tavianator.com/cgit/dimension.git/tree/libdimension/bvh/bvh.c#n194 */
        /* its not quick but I would have to change more for that */

        ray.dir_inv.x = 1/ray.dir.x;
        ray.dir_inv.y = 1/ray.dir.y;
        ray.dir_inv.z = 1/ray.dir.z;

        float tx1 = (bvh_head->min.x - ray.orig.x)*ray.dir_inv.x;
        float tx2 = (bvh_head->max.x - ray.orig.x)*ray.dir_inv.x;

        float tmin = min(tx1, tx2);
        float tmax = max(tx1, tx2);

        float ty1 = (bvh_head->min.y - ray.orig.y)*ray.dir_inv.y;
        float ty2 = (bvh_head->max.y - ray.orig.y)*ray.dir_inv.y;

        tmin = max(tmin, min(ty1, ty2));
        tmax = min(tmax, max(ty1, ty2));

        float tz1 = (bvh_head->min.z - ray.orig.z)*ray.dir_inv.z;
        float tz2 = (bvh_head->max.z - ray.orig.z)*ray.dir_inv.z;

        tmin = max(tmin, min(tz1, tz2));
        tmax = min(tmax, max(tz1, tz2));

        if (tmax < max(0.0f, tmin) || ray.t < max(0.0f, tmin)) return false;

        bool ret = traverse_bvh(bvh_head, ray, shadow);
        if (ret) ray.obj = this;

        return ret;
    }

    // bool intersects(Ray& ray, bool shadow)
    // {
    //     /* box intersection gotten from https://tavianator.com/2011/ray_box.html */
    //     /* https://tavianator.com/cgit/dimension.git/tree/libdimension/bvh/bvh.c#n194 */
    //     /* its not quick but I would have to change more for that */

    //     ray.dir_inv.x = 1/ray.dir.x;
    //     ray.dir_inv.y = 1/ray.dir.y;
    //     ray.dir_inv.z = 1/ray.dir.z;

    //     float tx1 = (head->min.x - ray.orig.x)*ray.dir_inv.x;
    //     float tx2 = (head->max.x - ray.orig.x)*ray.dir_inv.x;

    //     float tmin = min(tx1, tx2);
    //     float tmax = max(tx1, tx2);

    //     float ty1 = (head->min.y - ray.orig.y)*ray.dir_inv.y;
    //     float ty2 = (head->max.y - ray.orig.y)*ray.dir_inv.y;

    //     tmin = max(tmin, min(ty1, ty2));
    //     tmax = min(tmax, max(ty1, ty2));

    //     float tz1 = (head->min.z - ray.orig.z)*ray.dir_inv.z;
    //     float tz2 = (head->max.z - ray.orig.z)*ray.dir_inv.z;

    //     tmin = max(tmin, min(tz1, tz2));
    //     tmax = min(tmax, max(tz1, tz2));

    //     if (tmax < max(0.0f, tmin) || ray.t < max(0.0f, tmin)) return false;

    //     bool ret = traverse_tree(ray, tmin, tmax);
    //     if (ret) ray.obj = this;

    //     return ret;
    // }

    Object* getIntersectedObject(Ray& r)
    {
        assert(r.mesh_obj != nullptr);
        return r.mesh_obj;
    }

    Material* getMat(Ray& r) 
    {
        assert(r.mesh_obj != nullptr);
        return r.mesh_obj->getMat(r);
    }

    vec3 get_normal(Ray& r)
    {
        assert(r.mesh_obj != nullptr);
        return r.mesh_obj->get_normal(r);
    }

    Color* get_diffuse(Ray& r)
    {   
        assert(r.mesh_obj != nullptr);
        return r.mesh_obj->get_diffuse(r);
    }

    Color* get_specular(Ray& r)
    {
        assert(r.mesh_obj != nullptr);
        return r.mesh_obj->get_specular(r);
    }

    vector<Object*> polys;
    Level* bvh_head;
    int levels;
    int count = 0;
};

#endif