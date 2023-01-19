#include <iostream>

#include 

#include "opencv2/highgui.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"

using namespace std;
using namespace cv;

class GaussianPyramid
{
public:
	GaussianPyramid(string img_src, int levels);
	~GaussianPyramid();

private:
	Mat orig_img;
	int levels;
};