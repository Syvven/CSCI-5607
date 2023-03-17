#include "../include/RayTracer.h"

RayTracer::RayTracer(vec3& e, vec3& v, vec3& u, 
        float hf, float w, float h, Color& bk, 
        vector<Object*>* o, vector<Light*>* l, bool parallel, 
        bool cueing, float amin, float amax, float dmin, 
        float dmax, Color& cueingcolor, int threads)
{
    this->eye = e;
    this->view = v.normalized();
    this->up = u.normalized();

    /* 
        Threshold for vector equality is 9e-14.
        Making sure the up dir is just ever so slightly
         away from the view direction ensures no issues
         even when the up dir is entered as equal or opposite
         to view dir
    */
    if (view == up)
    {
        view.y += 9e-10;
        view.x += 9e-10;
        view.z += 9e-10;
        view.normalize();
    }

    vec3 abs_view(fabs(view.x), fabs(view.y), fabs(view.z));
    vec3 abs_up(fabs(up.x), fabs(up.y), fabs(up.z));

    if (abs_view == abs_up)
    {
        view.y += 9e-10;
        view.x += 9e-10;
        view.z += 9e-10;
        view.normalize();
    }

    this->hfov = hf * M_PI / 180;
    this->p_width = w;
    this->p_height = h;
    this->bkgcolor = bk;
    this->objs = o;
    this->lights = l;
    this->parallel = parallel;

    this->aspect_ratio = w / h;

    this->num_threads = threads;
    if (threads == -1)
        this->num_threads = 1;
    
    this->cueing = cueing;
    this->amin = amin;
    this->amax = amax;
    this->dmin = dmin;
    this->dmax = dmax;
    this->cueingcolor = cueingcolor;
}

void RayTracer::gen(vector<Color>& pixels)
{
    
    /* defines the viewing system for the RayCaster */
    /* 
        Variables Initialized:
            w:          negative of view dir
            u:          horizontal vector for view window
            v:          vertical vector for view window
            w_width:    width of view window
            w_height:   height of view window
            ul:         upper left corner
            ur:         upper right corner
            ll:         lower left corner
            lr:         lower right corner
    */
    this->define_viewing_system();
    
    /* each ray corresponds 1-1 with the pixels vector */
    pixels.clear();
    pixels.resize(this->p_height * this->p_width);

    vec3 ray_orig = this->eye;
    vec3 scaled_w = this->w.toLength(this->d);
    /* find center of each window pane and generate ray */

    int step = ceil(this->p_height / this->num_threads);

    /* thread creation */
    vector<thread> threads;
    for (int i = 0; i < this->num_threads; i++)
    {
        threads.push_back(
            thread(
                &RayTracer::split_work, 
                this, 
                i*step, 
                ((i+1)*step > this->p_height ? this->p_height : (i+1)*step), 
                std::ref(pixels), 
                ray_orig, 
                &scaled_w
            )
        );
    }

    for (auto& t : threads)
    {
        t.join();
    }
}

void RayTracer::split_work(int start, int end, 
        vector<Color>& pixels, vec3 ro, vec3* sw)
{
    /*
        Each thread gets a number of rows of pixels to fill
        There is no locking structures because this is
         easily parallelizable
    */
    for (int i = start; i < end; i++)
    {
        for (int j = 0; j < this->p_width; j++)
        {
            vec3 vw_pos = this->ul + this->dv*i + this->dh*j;

            /* 
                Commented this out because of this assumption:
                This is obviously not real time, so there's no reason
                 that the projection would change mid rendering,
                 so don't have to reassign ray_orig each time if 
                 non-parallel
                    ray_orig.x = this->eye.x;
                    ray_orig.y = this->eye.y;
                    ray_orig.z = this->eye.z;
            */
            if (this->parallel)
                ro = vw_pos + *sw;

            // ro *= !this->parallel;
            // ro += ((vw_pos + *sw) * this->parallel);

            vw_pos -= ro;
            vw_pos.normalize();

            /* this is a lot of dereferencing ? do better? */
            pixels[i * (int)(this->p_width) + j] = this->trace_ray(
                Ray(ro, vw_pos)
            );
        }
    }
}  

void RayTracer::define_viewing_system()
{
    /* w is negative of view direction */
    this->w = this->view * -1;

    /* u is unit vector produced by cross of view and up */
    this->u = this->view.cross(this->up);
    this->u.normalize();
    /*****************************************/
    /* come back for when view is same as up */
    /*****************************************/

    /* v is cross of u and view */
    this->v = this->u.cross(this->view);

    /* now to define the four corners */
    
    /* first we need width and height in world coordinates */
    this->w_width = 2 * this->d * tan(0.5 * this->hfov);
    this->w_height = this->w_width / this->aspect_ratio;

    /* then use these values */
    vec3 view_norm = this->view.normalized();

    vec3 p1 = view_norm * this->d;
    vec3 p2 = this->u * (this->w_width/2);
    vec3 p3 = this->v * (this->w_height/2);

    this->ul = this->eye + p1 - p2 + p3;
    this->ur = this->eye + p1 + p2 + p3;
    this->ll = this->eye + p1 - p2 - p3;
    this->lr = this->eye + p1 + p2 - p3;

    /* 
        Once viewing system has been defined, all that is
         needed is to shoot the rays and check for intersection.
    */
    this->dh = (this->ur - this->ul) / (this->p_width - 1);
    this->dv = (this->ll - this->ul) / (this->p_height - 1);
}

Color RayTracer::trace_ray(Ray r)
{
    tuple<float, Object*> min = this->get_min_intersect(r, nullptr);

    return this->shade_ray(r, get<0>(min), get<1>(min));
}

Color RayTracer::shade_ray(Ray& ray, float t, Object* o)
{
    /*
        This first section is just setting 
         up values that will be used repeatedly later
         so as to not waste time reassigning values
         that never change
    */
    if (!cueing && o == nullptr) return this->bkgcolor;
    if (o == nullptr) return this->cueingcolor;

    float r,g,b;
    if (o != nullptr)
    {
        vec3 V = ray.dir;
        V *= -1;

        vec3 int_point = ray.dir;
        int_point *= t;
        int_point += ray.orig;

        vec3 n = o->get_normal(int_point);

        float ka = o->mat->ka;
        float kd = o->mat->kd;
        float ks = o->mat->ks;

        Color* diffuse = o->get_diffuse(int_point);
        Color* specular = o->get_specular();

        r = ka * diffuse->r;
        g = ka * diffuse->g;
        b = ka * diffuse->b;

        float num_extra = 50;
        float full = 6.283;
        float theta = full / num_extra;
        float radius = 0.03;
        float div_factor = num_extra+1;

        vec3 L;
        Ray rar;
        rar.orig = int_point;
        vec3 H;
        for (auto& light : *this->lights)
        {   
            /* gets the L vector from the light */
            L = light->compute_L(int_point);
            rar.dir = L;

            /* shadow coming directly towards light */
            float shadow = 0;

            int isPoint = (
                (typeid(*light) == typeid(PointLight)) + 
                (typeid(*light) == typeid(AttenPointLight))
            );
            float max_dist = 
               !isPoint * 9e16 +
                isPoint * int_point.distanceTo(light->position);

            /* intersect every object */
            tuple<float, Object*> m = this->get_min_intersect(rar, o);
            float ot = get<0>(m);
            /* frankly, this shit is magic to me. I love C/C++ */
            shadow += ((ot < 0.0f) + (ot >= max_dist));

            /*
                Find vector perpendicular to 
                the L vector using get_orthogonal()
                Normalize it
                Get a third vector using cross prod
                Normalize that 
                Then rotate that in a circle to get 
                a cone of vectors at which point each are
                intersected with all spheres and shadow is 
                calculated.
                The soft shadows resulting from this I think look
                good but there's an ever so slight patchwork look
                when the soft shadows are really big.
            */
            vec3 transform_up = L.get_orthogonal();
            transform_up.normalize();

            vec3 transform_right = L.cross(transform_up);
            transform_right.normalize();

            for (int i = 0; i < num_extra; i++)
            {   
                rar.dir = L;
                rar.dir += (transform_up*(cos(i*theta)*radius));
                rar.dir += (transform_right*(sin(i*theta)*radius));
                rar.dir.normalize();

                tuple<float, Object*> m(this->get_min_intersect(rar, o));

                ot = get<0>(m);
                
                /* t cannot be less than 0 AND greater than max dist */
                shadow += ((ot < 0.0f) + (ot >= max_dist));
            }

            shadow /= div_factor;

            /* 
                I may be going overboard with the branchless stuff but uhhhh
                    its fun :3 
            */
            float atten = light->atten * (
                calc_atten(light->c1, light->c2, light->c3, max_dist)
            ) + !light->atten;

            /* 
                This is just the phong light equation
                with a couple simplifications to cut down
                on multiplications
            */
            H = V;

            H += L;
            H.normalize();

            float ndotl = n.dot(L);
            ndotl *= (ndotl >= 0.0f);

            float ndoth = n.dot(H);
            ndoth *= (ndoth >= 0.0f);

            ndoth = pow(ndoth, o->mat->n);

            float mod = shadow*atten;
            float modks = mod * ndoth*ks;
            float modkd = mod * ndotl*kd;
            r += light->color.r*(modkd*diffuse->r + modks*specular->r);
            g += light->color.g*(modkd*diffuse->g + modks*specular->g);
            b += light->color.b*(modkd*diffuse->b + modks*specular->b);
        }  
    }
    
    /* depth cueing */
    if (cueing)
    {
        int one = (t <= this->dmin);
        int two = (t >= this->dmax);
        int three = ((one + two) == 0);
        float adc = 
            amax * one + 
            amin * two + 
            three * (amin + (amax-amin)*((dmax-t)/(dmax-dmin)));

        float inv = (1.0f - adc);
        r = r*adc + cueingcolor.r*inv;
        g = g*adc + cueingcolor.g*inv;
        b = b*adc + cueingcolor.b*inv;
    }

    return Color(
        r * (r <= 1.0f) + (r > 1.0f),
        g * (g <= 1.0f) + (g > 1.0f),
        b * (b <= 1.0f) + (b > 1.0f)
    );
}

tuple<float, Object*> RayTracer::get_min_intersect(Ray& r, Object* other)
{
    /* Find the closest intersection */
    bool flag = 0;
    float min_int = 9e15;
    Object* min = nullptr;
    for (auto& o : *this->objs)
    {
        if (other != nullptr && other->id == o->id) continue;
        tuple<bool, float> t = o->intersects(r);
        if (get<0>(t) && get<1>(t) < min_int)
        {
            flag = 1;
            min_int = get<1>(t);
            min = o;
        }
    }

    min_int = min_int*flag + -1*!flag;
    return tuple<float, Object*>{min_int, min};
}

float RayTracer::arbitrary_random(float low, float high)
{
    return (
        low + static_cast <float> (rand()) 
        / 
        (static_cast <float> (RAND_MAX / (high - low)))
    );
}

float RayTracer::calc_atten(float c1, float c2, float c3, float d)
{
    /* 
        Find attenutation factor and account for div by 0
        Also clamp to 1 
    */
    float f = 
        c1 + 
        c2*d + 
        c3*d*d;
    float atten = 1.0f / (f * (f > 1e-10f) + (f < 1e-10f));
    return (atten * (atten <= 1.0f) + (atten > 1.0f));
}