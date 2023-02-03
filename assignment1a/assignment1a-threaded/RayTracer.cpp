#include "RayTracer.h"

RayTracer::RayTracer(vec3& e, vec3& v, 
    vec3& u, float hf, float w, float h,  
    Color& bk, vector<Material*>* m,
    vector<Object*>* o, bool parallel, int threads)
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
        view.normalize();
    }

    vec3 abs_view(fabs(view.x), fabs(view.y), fabs(view.z));
    vec3 abs_up(fabs(up.x), fabs(up.y), fabs(up.z));

    if (abs_view == abs_up)
    {
        view.y += 9e-10;
        view.normalize();
    }

    this->hfov = hf * M_PI / 180;
    this->p_width = w;
    this->p_height = h;
    this->bkgcolor = bk;
    this->mats = m;
    this->objs = o;
    this->parallel = parallel;

    this->aspect_ratio = w / h;

    this->num_threads = threads;
    if (threads == -1)
        this->num_threads = 1;
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
                &pixels, 
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
        vector<Color>* pixels, vec3 ro, vec3* sw)
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

            vw_pos -= ro;
            vw_pos.normalize();

            /* this is a lot of dereferencing ? do better? */
            (*pixels)[i * (int)(this->p_width) + j] = this->trace_ray(
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

Color& RayTracer::trace_ray(Ray r)
{
    /* Find the closest intersection */
    float min_int = 9e15;
    Object* min = nullptr;
    for (auto& o : *this->objs)
    {
        tuple<bool, float> t = o->intersects(r);
        if (get<0>(t) && get<1>(t) < min_int)
        {
            min_int = get<1>(t);
            min = o;
        }
    }

    return this->shade_ray(min);
}

Color& RayTracer::shade_ray(Object* o)
{
    /* no intersection, color background color */
    /* intersection, get color from object material */

    return ((o == nullptr) ?
                this->bkgcolor :
                o->mat->color);
}

