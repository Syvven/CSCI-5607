#ifndef PERLIN_NOISE_H_
#define PERLIN_NOISE_H_

#include <iostream>
#include <vector>
#include <numeric>
#include <random>

using namespace std;

class PerlinNoise
{
public:
	PerlinNoise(unsigned int seed);
	double gen(double x, double y, double z);

private:
	double fade(double t);
	double lerp(double t, double a, double b);
	double grad(int hash, double x, double y, double z);
	vector<int> perm_vec;
};

#endif