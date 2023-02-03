#include <iostream>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <typeinfo>
#include <sstream>
#include <chrono>
#include <random>

using namespace std;

int main(int argc, char** argv)
{
    ofstream inf{ "./inputs/kiwer.txt", std::ofstream::out | std::ofstream::app };
	if (!inf)
	{
		exit(EXIT_FAILURE);
	}

    float centerx = 0;
    float centery = 0;
    float centerz = 0;

    float rad = 600;
    float rad_a = 1;
    float r = 255;
    float g = 255;
    float b = 255;

    for (int i = 0; i < 200; i++)
    {
        float rx = rand() * 2 - 1;
        float ry = rand() * 2 - 0.5;
        float rz = rand() * 2 - 1;

        float mag = sqrt(rx*rx+ry*ry+rz*rz);

        float nx = rx/mag * rad;
        float ny = ry/mag * rad;
        float nz = rz/mag * rad;

        string s = "sphere ";
        s += to_string(nx) + 
             " " +
             to_string(ny) + 
             " " +
             to_string(nz) +
             " " +
             to_string(rad_a) + 
             "\n";

        inf << s;
    }



    inf.close();

    return 0;
}

/*
imsize 1024 1024

// eye 0 -5 15
// eye 10 -5 10
eye 10 0 10

// viewdir 0 0.3 -1
// viewdir -1 0.5 -1
viewdir -1 0 -1

hfov 90
updir 0 1 0
bkgcolor 0.031 0.011 0.188

threadcount 8
// 8 is ideal for csel-kh1262-13

// major coordinate axes

mtlcolor 1 0 0 
cylinder 1000 0 0 -1 0 0 .1 1000

mtlcolor 0 1 0 
cylinder 0 1000 0 0 -1 0 .1 1000

mtlcolor 0 0 1 
cylinder 0 0 1000 0 0 -1 .1 1000

mtlcolor 0.219 0.164 0.019
sphere 0 0 0 4
sphere -6 -2 0 6
cylinder -8 -3 -2  0 -1 .1  1  5
cylinder -8 -3  2  0 -1 .1  1  5

mtlcolor 0.01 0.01 0.01
sphere 3 1.7 1.5 1
sphere 3 1.7 -1.5 1

// after this make yellow cylinders

mtlcolor 0.988 0.819 0.349
cylinder -8 -8 -2.5  0 -1 .1  1  2
cylinder -8 -8  2.5  0 -1 .1  1  2
sphere -8.3 -10 2.6 1
sphere -8.3 -10 -2.5 1

cylinder 4 0 0 1 -0.1 0 0.5 3

// the ground he's standing on

mtlcolor 0.168 0.352 0.168
sphere 0 -1011 0 1000

// moon

mtlcolor 0.886 0.882 0.792
sphere -80 100 -220 10

mtlcolor 0.031 0.011 0.188
sphere -80 100 -210 9

*/