#include "SPH.hpp"

SPH::SPH(int np, int ts, float w, float h, bool grav)
{
	/* set values of sim */
	this->grav_circle = grav;

	this->krd = 0.5; /* resting density */
	this->ksN = 200.0; /* near stiffness */
	this->ks = 100.0; /* stiffness */
	this->ksr = 0.9; /* smoothing radius */
	this->dt = 0.8; /* delta time */

	/* 
		set gravity
		- positive beecause of coordinate system
	*/
	if (!grav)
	{
		this->gravity.x = 0;
		this->gravity.y = 100;
	}

	this->num_particles = np;
	this->num_timesteps = ts;

	this->width = w;
	this->height = h;

	/* initialize press and dens records */
	this->pressure_records.resize(ts * w * h);
	this->density_records.resize(ts * w * h);

	/* fun times */
	/*for (int i = 0; i < ts; i++) 
	{
		for (int j = 0; j < h; j++)
		{
			for (int k = 0; k < w; k++)
			{
				cout << index(i, j, k) << endl;
				this->pressure_records[index(i, j, k)];
			}
		}
	}*/

	/* particle vectors for position and velocity */
	this->opos.resize(num_particles);
	this->ppos.resize(num_particles);
	this->pvel.resize(num_particles);

	/* density vectors */
	this->dens.resize(num_particles);
	this->densN.resize(num_particles);

	/* pressure vectors */
	this->press.resize(num_particles);
	this->pressN.resize(num_particles);
}

SPH::~SPH(){}

void SPH::run()
{
	/* step 1: generate particles in a ball in center towards top */
	float starting_circle_rad = 20;
	vec2 center(this->width / 2, 20);

	for (int i = 0; i < num_particles; i++)
	{
		vec2 v;
		float rand_r = arbitraryRand(0, starting_circle_rad);
		v.x = arbitraryRand(-1, 1);
		v.y = arbitraryRand(-1, 1);
		v.setToLength(rand_r);

		this->ppos[i] = center.clone();
		this->ppos[i].add(v);

		this->opos[i] = this->ppos[i].clone();
	}

	/* step 2: update in a loop */

	for (int i = 0; i < this->num_timesteps; i++)
	{
		update(this->dt, i);
	}
}

/* 
	Main update function
	Goes through the SPH algorithm and updates positions and vels
	Also records the densities and pressures at each timestep
*/
void SPH::update(float dt, int ts)
{
	vec2 dtGrav = gravity.clone();
	dtGrav.multiplyScalar(dt);

	vec2 center(this->width / 2, this->height / 2);

	/*
		This loop applies gravity and current velocity to 
		each of the particles. It also resets the density vectors.
	*/
	for (int i = 0; i < this->num_particles; i++)
	{

		this->pvel[i] = this->ppos[i].clone();
		this->pvel[i].sub(this->opos[i]);

		this->pvel[i].multiplyScalar(1 / dt);

		if (!grav_circle)
		{
			this->pvel[i].add(dtGrav);
		}
		else 
		{
			vec2 newGrav = this->ppos[i].clone();

			newGrav.sub(center);

			float dist = this->ppos[i].distanceTo(center);

			newGrav.setToLength(300/dist);
			newGrav.multiplyScalar(dt);
			this->pvel[i].add(newGrav);
		}

		/* wall collision handling */
		if (this->ppos[i].y < 0)
		{
			this->ppos[i].y = 0;
			this->pvel[i].y *= -0.3;
		}
		if (this->ppos[i].y > this->height)
		{
			this->ppos[i].y = this->height;
			this->pvel[i].y *= -0.3;
		}
		if (this->ppos[i].x < 0)
		{
			this->ppos[i].x = 0;
			this->pvel[i].x *= -0.5;
		}
		if (this->ppos[i].x > this->width)
		{
			this->ppos[i].x = this->width;
			this->pvel[i].x *= -0.5;
		}

		this->opos[i] = this->ppos[i].clone();
		vec2 vel = this->pvel[i].clone();

		vel.multiplyScalar(dt);

		this->ppos[i].add(vel);

		this->dens[i] = 0.0;
		this->densN[i] = 0.0;
	}

	/* 
		No BVH will be used for this pair search
		This sim is not a large SPH sim thus,
		 there is no particular reason to use something
		 like a spatial grid.
		If this was actually being rendered, maybe I would.
	*/
	vector<tuple<int, int, float>> pairs;
	for (int i = 0; i < this->num_particles; i++)
	{
		for (int j = 0; j < this->num_particles; j++)
		{
			if (i < j)
			{
				float dist = this->ppos[i].distanceTo(this->ppos[j]);
				if (dist < this->ksr)
				{
					float q = 1 - (dist / this->ksr);
					pairs.push_back(tuple<int, int, float>{i, j, q});
				}
			}
		}
	}

	for (auto& pair : pairs)
	{
		float q = get<2>(pair);
		float q2 = q * q;
		float q3 = q * q * q; 

		this->dens[get<0>(pair)] += q2;
		this->dens[get<1>(pair)] += q2;

		this->densN[get<0>(pair)] += q3;
		this->densN[get<1>(pair)] += q3;
	}

	for (int i = 0; i < num_particles; i++)
	{
		this->press[i] = this->ks * (this->dens[i] - this->krd);
		this->pressN[i] = ksN * this->densN[i];

		if (this->press[i] > 30) this->press[i] = 30.0;
		if (this->pressN[i] > 300) this->pressN[i] = 300.0;
	}

	for (int i = 0; i < num_particles; i++)
	{
		int x = (int)this->ppos[i].x;
		int y = (int)this->ppos[i].y;

		if (x < 0) x = 0;
		if (x >= this->width) x = this->width-1;
		if (y < 0) y = 0;
		if (y >= this->height) y = this->height-1;

		this->pressure_records[index(ts, y, x)] = this->press[i];
	}

	for (auto& pair : pairs)
	{
		int i = get<0>(pair);
		int j = get<1>(pair);
		float q = get<2>(pair);

		float tp1 = (this->press[i] + this->press[j]) * q;
		float tp2 = (this->pressN[i] + this->pressN[j]) * q * q;
		float tp = tp1 + tp2; /* total pressure */

		float displacement = tp + (dt * dt);

		vec2 vi = this->ppos[i].clone();
		vec2 vj = this->ppos[j].clone();

		vi.sub(this->ppos[j]);
		vi.multiplyScalar(displacement);

		vj.sub(this->ppos[i]);
		vj.multiplyScalar(displacement);

		this->ppos[i].add(vi);
		this->ppos[j].add(vj);
	}
}

/* currently not used */
vector<float> SPH::calc_density()
{
	vector<float> x;
	return x;
}


vector<float> SPH::calc_pressure()
{
	vector<float> ret(this->width * this->height);
	for (int i = 0; i < this->height; i++)
	{

		for (int j = 0; j < this->width; j++)
		{
			for (int k = 0; k < this->num_timesteps; k++)
			{
				ret[ind(i, j)] += this->pressure_records[index(k, i, j)];
			}

			ret[ind(i, j)] = log(abs(ret[ind(i, j)]));

			/* this is gonna get really ugly */

			/* one diffusion level */
			if (i > 0) 
				ret[ind(i - 1, j)] += ret[ind(i, j)] / 2;
			if (j > 0) 
				ret[ind(i, j - 1)] += ret[ind(i, j)] / 2;
			if (i > 0 && j > 0) 
				ret[ind(i, j - 1)] += ret[ind(i, j)] / 2;
			if (i < height-1) 
				ret[ind(i + 1, j)] += ret[ind(i, j)] / 2; 
			if (j < width-1)
				ret[ind(i, j + 1)] += ret[ind(i, j)] / 2;
			if (i < height - 1 && j < width - 1)
				ret[ind(i + 1, j + 1)] += ret[ind(i, j)] / 2;
			if (i < height - 1 && j > 0)
				ret[ind(i + 1, j - 1)] += ret[ind(i, j)] / 2;
			if (i > 0 && j < width - 1)
				ret[ind(i - 1, j + 1)] += ret[ind(i, j)] / 2;
			
			/* second diffusion level god I don't want to write this */
			if (i > 1)
				ret[ind(i - 2, j)] += ret[ind(i, j)] / 4;
			if (j > 1)
				ret[ind(i, j - 2)] += ret[ind(i, j)] / 4;
			
		}
	}

	return ret;
}

/*
	Gets the index based on timstep and desired pixel
	This is like a 3d array smushed into a 2d
*/
int SPH::index(int ts, int pixel_h, int pixel_w)
{	
	int pt1 = (ts * (int)this->width * (int)this->height);
	int pt2 = (pixel_h * (int)this->width) + pixel_w;
	return pt1 + pt2;
}

int SPH::index(int ts, int ind)
{
	return (ts * (int)this->width * (int)this->height) + ind;
}

int SPH::ind(int i, int j)
{
	return (i * this->width) + j;
}