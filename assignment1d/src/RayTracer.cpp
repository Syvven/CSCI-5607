#include "../include/RayTracer.h"

RayTracer::RayTracer(vec3& e, vec3& v, vec3& u, 
        float hf, float w, float h, Color& bk, float bk_eta, 
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
    this->bkg_eta = bk_eta;
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

    if (cueing)
        this->bkgcolor = cueingcolor;

    this->od3 = 1/3;
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

            Ray r(ro, vw_pos, bkg_eta, 1);

            /* this is a lot of dereferencing ? do better? */

            stack<Object*> s;
            this->trace_ray(&r, 0, s);
            pixels[i*(int)(this->p_width) + j] = r.color.capMax(1.0f).capMin(0.0f);
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

void RayTracer::trace_ray(Ray* ray, int depth, stack<Object*>& whences)
{
    // tuple<float, Object*> min = this->get_min_intersect(r, nullptr);
    // return this->shade_ray(r, get<0>(min), get<1>(min), depth);

    if (depth >= this->max_depth) 
    {
        ray->color = this->bkgcolor;
        return;
    }

    ray->obj = nullptr;
    ray->t = 9e15;
    this->get_min_intersect(ray);

    if (ray->obj == nullptr)
    {
        ray->color = this->bkgcolor;
        return;
    } 

    this->shade_ray(ray, depth, whences);
}

void RayTracer::shade_ray(Ray* ray, int depth, stack<Object*>& whences)
{
    /*
        This first section is just setting 
         up values that will be used repeatedly later
         so as to not waste time reassigning values
         that never change
    */
    // cout << "before " << ray->id << endl;
    vec3 V = ray->dir;
    V *= -1;

    ray->int_point = ray->dir;
    ray->int_point *= ray->t;
    ray->int_point += ray->orig;

    vec3 n = ray->obj->get_normal(*ray);

    float ndotI = n.dot(V);
    if (ndotI < 0) n *= -1;  

    float ka = ray->obj->getMat(*ray)->ka;
    float kd = ray->obj->getMat(*ray)->kd;
    float ks = ray->obj->getMat(*ray)->ks;

    Color* diffuse = ray->obj->get_diffuse(*ray);
    Color* specular = ray->obj->get_specular(*ray);

    // cout << "after" << endl;

    float r = ka * diffuse->r;
    float g = ka * diffuse->g;
    float b = ka * diffuse->b;

    float num_extra = 10;
    int transform_mod_count = 8;
    float transform_mod_val = 0.2;
    float full = 6.283;
    float theta = full / num_extra;
    float radius = 0.03;
    float div_factor = num_extra+1;

    float shadow_avg = 1 / (num_extra*transform_mod_count + 1);

    vec3 L;
    vec3 H;

    Ray rar = *ray;
    rar.orig = ray->int_point;

    // cout << "after" << endl;

    for (auto& light : *this->lights)
    {   
        /* gets the L vector from the light */
        L = light->compute_L(*ray);
        rar.dir = L.normalized();

        /* shadow coming directly towards light */
        Color shadow(1,1,1);

        int isPoint = (
            (typeid(*light) == typeid(PointLight)) + 
            (typeid(*light) == typeid(AttenPointLight))
        );
        float max_dist = 
            !isPoint * 9e16 +
            isPoint * ray->int_point.distanceTo(light->position);

        Object* dont_int = ray->obj->getIntersectedObject(*ray);
        if (ndotI < 0) dont_int = nullptr;
        // if (typeid(ray->obj) != typeid(Mesh) && !whences.empty())
        //     dont_int = nullptr;

        /* intersect every object */
        vector<Object*> obj_vec;
        this->get_all_intersects_in_distance(&rar, obj_vec, max_dist, dont_int);

        for (auto& obj : obj_vec) 
        {
            shadow.r *= (1.0f - obj->getMat(rar)->alpha.r);
            shadow.g *= (1.0f - obj->getMat(rar)->alpha.g);
            shadow.b *= (1.0f - obj->getMat(rar)->alpha.b);
        }

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
            The cone varies in angle as well.
            The soft shadows resulting from this I think look
            good but there's an ever so slight patchwork look
            when the soft shadows are really big.
        */
        vec3 transform_up = L.get_orthogonal();
        transform_up.normalize();

        vec3 transform_right = L.cross(transform_up);
        transform_right.normalize();

        for (int ang = 0; ang < transform_mod_count; ang++)
        {
            float modifier = 0.1 + ang*transform_mod_val;
            for (int i = 0; i < num_extra; i++)
            {   
                Color temps(1,1,1);

                rar.dir = L;
                rar.dir += (transform_up*(cos(i*theta)*radius));
                rar.dir += (transform_right*modifier * (sin(i*theta)*radius));
                rar.dir.normalize();

                /* allows for colored shadows! */

                obj_vec.clear();
                this->get_all_intersects_in_distance(&rar, obj_vec, max_dist, dont_int);
                for (auto& obj : obj_vec)
                {
                    temps.r *= (1.0f - obj->getMat(rar)->alpha.r);
                    temps.g *= (1.0f - obj->getMat(rar)->alpha.g);
                    temps.b *= (1.0f - obj->getMat(rar)->alpha.b);
                } 
                shadow += temps;
            }
        }

        shadow *= shadow_avg;
        shadow.capMax2(1.0f);

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

        ndoth = pow(ndoth, ray->obj->getMat(*ray)->n);

        // float mod = shadow*atten;
        // float modks = mod * ndoth*ks;
        // float modkd = mod * ndotl*kd;

        // r += light->color.r*(modkd*diffuse->r + modks*specular->r);
        // g += light->color.g*(modkd*diffuse->g + modks*specular->g);
        // b += light->color.b*(modkd*diffuse->b + modks*specular->b);

        shadow *= atten;

        Color modks = shadow;
        modks *= (ndoth*ks);
        modks *= (*specular);

        Color modkd = shadow;
        modkd *= (ndotl*kd);
        modkd *= (*diffuse);

        modks += modkd;
        modks *= light->color;

        r += modks.r;
        g += modks.g;
        b += modks.b;
    }  
    
    /* depth cueing */
    if (cueing)
    {
        int one = (ray->t <= this->dmin);
        int two = (ray->t >= this->dmax);
        int three = ((one + two) == 0);
        float adc = 
            amax * one + 
            amin * two + 
            three * (amin + (amax-amin)*((dmax-ray->t)/(dmax-dmin)));

        float inv = (1.0f - adc);
        r = r*adc + cueingcolor.r*inv;
        g = g*adc + cueingcolor.g*inv;
        b = b*adc + cueingcolor.b*inv;
    }

    /* 
        get set up for the recursion by finding new ray direction 
        I: negative of ray direction (already have V)
        N: just the normal (already have n)
        R = 2(N.dot(I))*N - I
    */
    Ray reflect_ray = *ray;
    reflect_ray.orig = ray->int_point;
    reflect_ray.obj = nullptr;
    reflect_ray.mesh_obj = nullptr;
    reflect_ray.t = 9e15;

    Ray transmit_ray = *ray;
    transmit_ray.orig = ray->int_point;
    transmit_ray.obj = nullptr;
    transmit_ray.mesh_obj = nullptr;
    transmit_ray.t = 9e15;

    /* stack for entering/exiting */
    float in_eta = bkg_eta;
    float out_eta = ray->obj->getMat(*ray)->eta;
    if (!whences.empty())
    {
        in_eta = whences.top()->getMat(*ray)->eta;
        if (ndotI < 0)
        {
            Object* hold = whences.top();
            whences.pop();
            if (whences.empty()) out_eta = bkg_eta;
            else out_eta = whences.top()->getMat(*ray)->eta;
        }
    }

    float ndi = n.dot(V);
    float omndi = 1.0f - ndi;
    float ndi2 = 2.0f * ndi;

    /* 
        set up the transmitted ray for a material 
        I is still negative dir (V)
        cos(thetai) = NdotI
        eta_i = material IoR for incoming ray
        eta_t = material IoR for transmitted ray
        T = that big equation from the slides im not putting it here

        need to remember the proper normal direction based on entering or leaving
        the 'whence' variable in rays handles this
    */

    float niont = in_eta / out_eta;
    float under = 1.0f - (niont*niont * (1.0f - ndi*ndi));

    float Fo = pow((out_eta - in_eta)/(out_eta + in_eta), 2);
    float Fr = Fo + (1.0f - Fo) * pow(omndi, 5);

    Color a = ray->obj->getMat(*ray)->alpha;
    float avg_alpha = (a.r + a.g + a.b) / 3;

    /* find transmission ray if not opaque */
    if (avg_alpha < 1.0f)
    {
        transmit_ray.dir = n;
        if (under < 0.0f) 
        {
            transmit_ray.dir *= ndi2;
            transmit_ray.dir -= V;
            transmit_ray.dir.normalize();
            transmit_ray.orig += transmit_ray.dir*0.001;

            if (ndotI < 0) whences.push(ray->obj);

            this->trace_ray(&transmit_ray, depth+1, whences);
        }
        else 
        {
            float mod = sqrt(under);
            /* already have ndi */
            transmit_ray.dir *= -1.0f;
            transmit_ray.dir *= mod;

            vec3 right = n;
            right *= ndi;
            right -= V;
            right *= niont;

            transmit_ray.dir += right;
            transmit_ray.dir.normalize();
            transmit_ray.orig += transmit_ray.dir*0.001;

            whences.push(ray->obj);
            this->trace_ray(&transmit_ray, depth+1, whences);
        }
    }

    /* find reflection ray after transmission */
    if (ray->obj->getMat(*ray)->ks > 1e-10f)
    {
        reflect_ray.dir = n;
        reflect_ray.dir *= ndi2;
        reflect_ray.dir -= V;
        reflect_ray.dir.normalize();
        reflect_ray.orig += reflect_ray.dir*0.001;
        
        this->trace_ray(&reflect_ray, depth+1, whences);
    }   

    reflect_ray.color *= Fr;
    float omfr = 1 - Fr;

    /* beers law */
    if (ray->obj->getMat(*ray)->beers)
    {
        if (transmit_ray.t >= 9e14 || !whences.empty())
        {
            transmit_ray.color.r *= omfr*(1-avg_alpha);
            transmit_ray.color.g *= omfr*(1-avg_alpha);
            transmit_ray.color.b *= omfr*(1-avg_alpha);
        }
        else
        {
            transmit_ray.color.r *= omfr*exp(-a.r*transmit_ray.t);
            transmit_ray.color.g *= omfr*exp(-a.g*transmit_ray.t);
            transmit_ray.color.b *= omfr*exp(-a.b*transmit_ray.t);
        }
    }
    /* not beers */
    else
    {
        transmit_ray.color.r *= omfr*(1 - a.r);
        transmit_ray.color.g *= omfr*(1 - a.r);
        transmit_ray.color.b *= omfr*(1 - a.r);
    }
    
    /* accumulate colors */
    ray->color.r = r;
    ray->color.g = g;
    ray->color.b = b;

    ray->color += reflect_ray.color;
    ray->color += transmit_ray.color;

    ray->color.capMax2(1.0f);
    ray->color.capMin2(0.0f);
}

void RayTracer::get_min_intersect(Ray* r)
{
    /* Find the closest intersection */
    for (auto& o : *this->objs) o->intersects(*r, false);
}

void RayTracer::get_all_intersects_in_distance(Ray* rar, 
        vector<Object*>& fill, float dist, Object* other)
{
    for (auto& o : *this->objs)
    {
        rar->t = dist;
        if (other != nullptr && o->id == other->id) continue;
        if (typeid(*o) == typeid(Mesh))
        {
            rar->mesh_objs2.clear();
            ((Mesh*)o)->shadow_intersect(*rar);
            for (auto& kv : rar->mesh_objs2)
            {
                // Object* k =  iter->second;
                if (other == nullptr || kv.first != other->id) fill.push_back(kv.second);
            }
        }
        else if (o->intersects(*rar, true)) fill.push_back(o);
    }
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