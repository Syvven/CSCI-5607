#ifndef VEC2_H_
#define VEC2_H_

#include <iostream>
#include <cmath>

/*
	Vector class useful for the SPH simulation.
	Contains various functions on vectors
	 including distance and normalization.
*/

class vec2
{
public:
	vec2()
	{
		this->x = 0;
		this->y = 0;
	}

	vec2(float x_, float y_)
	{
		this->x = x_;
		this->y = y_;
	}

	vec2(const vec2& ovec)
	{
		this->x = ovec.x;
		this->y = ovec.y;
	}

	void sub(vec2& ovec)
	{
		this->x -= ovec.x;
		this->y -= ovec.y;
	}

	void add(vec2& ovec)
	{
		this->x += ovec.x;
		this->y += ovec.y;
	}

	void multiplyScalar(float a)
	{
		this->x *= a;
		this->y *= a;
	}

	float distanceTo(vec2& ovec)
	{
		float dx = ovec.x - this->x;
		float dy = ovec.y - this->y;
		return sqrt(dx * dx + dy * dy);
	}

	void normalize()
	{
		float mag = sqrt(x * x + y * y);
		if (mag == 0) return;
		this->x /= mag;
		this->y /= mag;
	}

	vec2& clone()
	{
		vec2* nv = new vec2(*this);
		return *nv;
	}

	void setToLength(float newL)
	{
		float mag = sqrt(x * x + y * y);
		if (mag == 0) return;
		x *= newL / mag;
		y *= newL / mag;
	}

	float x;
	float y;

private:
	
};

#endif