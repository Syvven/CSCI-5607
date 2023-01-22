#ifndef SPH_H_
#define SPH_H_

#include <iostream>
#include <vector>
#include <random>
#include <tuple>

#include "vec2.hpp"
#include "utils.h"

using namespace std;

/*
	Credit to Stephen Guy for the knowledge of how to do SPH
*/

class SPH
{
public:
	SPH(int np, int ts, float w, float h, bool gravity);
	~SPH();

	/*
		I'm trying to think of how to record pixel colors here...
		I was initially thinking average pressure over timesteps
		but I'm not quite sure how to get that...
		Maybe each timestep I could record the position of the
		particle, the density of the particle, and spread that
		out to the surrounding pixels with diminishing as ksr goes out...
		Oh I could do both pressure and density...
	*/
	void run();

	/*
		These two functions take the float arrays and compress them into a single vector
		That vector is then returned to the ppm_main to be turned into pixel values
	*/
	vector<float> calc_pressure();
	vector<float> calc_density();

private:
	/* used for getting index of vectors */
	int index(int ts, int pixel_h, int pixel_w);
	int index(int ts, int ind);
	int ind(int i, int j);

	/* main update function where SPH goes */
	void update(float dt, int ts);

	/* 
		What I actually want to record for the image gen 
		Two 2d float arrays:
		-> one holds pressure, the other holds density
		-> first dimension is the current timestep
		-> second dimension is the pixel values
		-> access a specific index with the index() function
		-> I think what I'll do is only record the absolute position
			of the particle in the pixel and then in postprocessing
			spread out the value to the surrounding pixels
	*/
	vector<float> pressure_records;
	vector<float> density_records;

	/* helper functions go here */
	vector<vec2> opos, ppos, pvel;
	vector<float> dens, densN, press, pressN;

	/* 
		Two main art controlling vals
		-> num_particles controlls how many will be visible
		-> num_timesteps controlls how many timesteps will be used for the 
			pressure calculations and pixel colors
	*/
	int num_particles, num_timesteps;

	/* 
		Following are sim constants 
		These are predeterimed, and, if changed,
		  the sim will not work as well
		Though maybe changing them will make cool things!
	*/
	float krd, ksN, ks, ksr, dt;

	vec2 gravity;
	bool grav_circle;

	/* scene bounds */
	float width, height;
};

#endif