#ifndef VORONOI_H_
#define VORONOI_H_

#include <iostream>
#include <vector>
#include <tuple>
#include <random>

#include "utils.h"
#include "vec2.h"

using namespace std;

class Voronoi
{
public:
	Voronoi(int seed_, int num_nodes_, float w, float h,
		float r, float g, float b, bool type, bool rand_c);
	~Voronoi();

	vector<tuple<float, float, float>> gen();
private:
	int seed, num_nodes;
	float width, height, cell_r, cell_g, cell_b;
	bool shaded, random_colors;
};

#endif
