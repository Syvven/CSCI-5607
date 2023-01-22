#include "PerlinNoise.hpp"

/* 
	permutation vector will be random 
	shuffle algorithm: https://github.com/sol-prog/Perlin_Noise/blob/master/PerlinNoise.cpp
*/
PerlinNoise::PerlinNoise(int seed)
{
	perm_vec.resize(256);

	iota(perm_vec.begin(), perm_vec.end(), 0);

	if (seed == 0)
	{
		random_device r;
		seed = r();
	}
	
	default_random_engine e(seed);

	shuffle(perm_vec.begin(), perm_vec.end(), e);

	perm_vec.insert(perm_vec.end(), perm_vec.begin(), perm_vec.end());
}

/*
	Implementation of Ken Perlin's improved noise reference
	https://mrl.cs.nyu.edu/~perlin/noise/
*/
double PerlinNoise::gen(double x, double y, double z)
{
	int X = (int)floor(x) & 255;
	int Y = (int)floor(y) & 255;
	int Z = (int)floor(z) & 255;

	x -= floor(x);
	y -= floor(y);
	z -= floor(z);

	double u = fade(x);
	double v = fade(y);
	double w = fade(z);

	int A = perm_vec[X] + Y;
	int AA = perm_vec[A] + Z;
	int AB = perm_vec[A + 1] + Z;
	int B = perm_vec[X + 1] + Y;
	int BA = perm_vec[B] + Z;
	int BB = perm_vec[B + 1] + Z;

	double g1 = grad(perm_vec[AA], x, y, z);
	double g2 = grad(perm_vec[BA], x - 1, y, z);
	double g3 = grad(perm_vec[AB], x, y - 1, z);
	double g4 = grad(perm_vec[BB], x - 1, y - 1, z);
	double g5 = grad(perm_vec[AA + 1], x, y, z - 1);
	double g6 = grad(perm_vec[BA + 1], x - 1, y, z - 1);
	double g7 = grad(perm_vec[AB + 1], x, y - 1, z - 1);
	double g8 = grad(perm_vec[BB + 1], x - 1, y - 1, z - 1);

	double l31 = lerp(u, g1, g2);
	double l32 = lerp(u, g3, g4);
	double l33 = lerp(u, g5, g6);
	double l34 = lerp(u, g7, g8);

	double l21 = lerp(v, l31, l32);
	double l22 = lerp(v, l33, l34);

	double l1 = lerp(w, l21, l22);

	return (l1 + 1.0) / 2.0;
}

double PerlinNoise::fade(double t)
{
	return t * t * t * (t * (t * 6 - 15) + 10);
}

double PerlinNoise::lerp(double t, double a, double b)
{
	return a + t * (b - a);
}

double PerlinNoise::grad(int hash, double x, double y, double z)
{
	int h = hash & 15;
	double u = h < 8 ? x : y;
	double v = h < 4 ? y : ((h == 12 || h == 14) ? x : z);
	return ((h & 1) == 0 ? u : -u) + ((h & 2) == 0 ? v : -v);
}