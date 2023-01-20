#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>

#include "utils.h"
#include "PerlinNoise.hpp"
#include "SPH.hpp"

using namespace std;

static int OUTPUT_WIDTH = 256;
static int OUTPUT_HEIGHT = 256;
static int NUM_COLORS = 255;

static bool normal = false;
static bool perlin = false;
static bool particle = false;
static bool combo = false;

static void put(ofstream& outf, float r, float g, float b)
{
	outf <<
		(int)r << " " <<
		(int)g << " " <<
		(int)b << "\n";
}

static void sph_perlin_combo(ofstream& outf, int seed, bool circle_grav)
{
	outf << create_ppm_header("P3", OUTPUT_WIDTH, OUTPUT_HEIGHT, NUM_COLORS);

	PerlinNoise pns(seed);

	SPH sim(500, 1000, OUTPUT_WIDTH, OUTPUT_HEIGHT, circle_grav);
	sim.run();

	vector<float> press_vals = sim.calc_pressure();
	vector<float> dens_vals = sim.calc_density(); /* ignore this for now */

	/* color scheme 1 */
	float max = -9e-15;
	float min = 9e15;
	for (auto& f : press_vals)
	{
		if (f < 0) f = 0;
		if (f > max) max = f;
		if (f < min) min = f;
	}

	cout << min << endl;
	cout << max << endl;

	float oldRange = max - min;
	float newRange = 255 - 0;

	int count = 0;
	for (auto& f : press_vals)
	{
		f = (((f - min) * newRange) / oldRange);		

		/* 30 before */
		if (f < 30)
		{
			int row = count / OUTPUT_WIDTH;
			int col = count % OUTPUT_WIDTH;

			double x = (double)col / (double)OUTPUT_WIDTH;
			double y = (double)row / (double)OUTPUT_HEIGHT;
			double z = (double)(row + col) / (double)(OUTPUT_WIDTH + OUTPUT_HEIGHT);

			double val = pns.gen(10 * x, 10 * y, 0.8);

			double r_val = 10 * pns.gen(6 * x, 2 * y, 4 * z);
			double g_val = 15 * pns.gen(4 * x, 4 * y, 2 * z);
			double b_val = 30 * pns.gen(2 * x, 6 * y, 6 * z);

			r_val = r_val - floor(r_val);
			g_val = g_val - floor(g_val);
			b_val = b_val - floor(b_val);

			put(outf, floor(255 * r_val), floor(255 * g_val), floor(255 * b_val));
		}
		else
		{
			float r = (0.7 * 256) - f * 0.5;
			float g = (0.6 * 256) - f * 0.4;
			float b = (1.0 * 256) - f * 0.3;

			put(outf, 0, g, b);
			
		}
		count++;
	}
}

static void sph_based_ppm(ofstream& outf, bool circle_grav)
{
	outf << create_ppm_header("P3", OUTPUT_WIDTH, OUTPUT_HEIGHT, NUM_COLORS);

	SPH sim(500, 1000, OUTPUT_WIDTH, OUTPUT_HEIGHT, circle_grav);
	sim.run();

	vector<float> press_vals = sim.calc_pressure();
	vector<float> dens_vals = sim.calc_density(); /* ignore this for now */

	/* color scheme 1 */
	float max = -9e-15;
	float min = 9e15;
	for (auto& f : press_vals)
	{
		if (f < 0) f = 0;
		if (f > max) max = f;
		if (f < min) min = f;
	}

	cout << min << endl;
	cout << max << endl;

	float oldRange = max - min;
	float newRange = 255 - 0;

	for (auto& f : press_vals)
	{
		f = (((f - min) * newRange) / oldRange);

		float r = (0.7 * 256) - f * 0.5;
		float g = (0.6 * 256) - f * 0.4;
		float b = (1.0 * 256) - f * 0.2;

		put(outf, 0, 0, f);
	}

	/* color scheme 2 */
	/*for (auto& f : press_vals)
	{
		float r = (0.7 * 256) - f * 0.5;
		float g = (0.6 * 256) - f * 0.4;
		float b = (1.0 * 256) - f * 0.2;

		outf <<
			r << " " <<
			g << " " <<
			b << "\n";
	}*/
}

static void perlin_based_ppm(ofstream& outf, int seed)
{
	outf << create_ppm_header("P3", OUTPUT_WIDTH, OUTPUT_HEIGHT, NUM_COLORS);

	PerlinNoise pns(seed);
	for (int row = 0; row < OUTPUT_HEIGHT; row++)
	{
		for (int col = 0; col < OUTPUT_WIDTH; col++)
		{
			double x = (double)col / (double)OUTPUT_WIDTH;
			double y = (double)row / (double)OUTPUT_HEIGHT;
			double z = (double)(row + col) / (double)(OUTPUT_WIDTH + OUTPUT_HEIGHT);

			double val = pns.gen(10*x, 10*y, 0.8);

			double r_val = 200 * pns.gen(6*x, 2*y, 4*z);
			double g_val = 2 * pns.gen(4*x, 4*y, 2*z);
			double b_val = 70 * pns.gen(2*x, 6*y, 6*z);

			r_val = r_val - floor(r_val);
			g_val = g_val - floor(g_val);
			b_val = b_val - floor(b_val);

			put(outf, floor(255 * r_val), floor(255 * g_val), floor(255 * b_val));
		}
	}

}

/* 
	Generates a very basic pattern ppm
	This gradient has a magenta lower left and cyan upper right
	Upper left is black and lower right is white
*/
static void gradient_basic_ppm(ofstream& outf)
{
	outf << create_ppm_header("P3", OUTPUT_WIDTH, OUTPUT_HEIGHT, NUM_COLORS);

	for (int row = 0; row < OUTPUT_HEIGHT; row++) {
		for (int col = 0; col < OUTPUT_WIDTH; col++) {
			put(outf,
				row * (NUM_COLORS + 1.0) / OUTPUT_HEIGHT,
				col * (NUM_COLORS + 1.0) / OUTPUT_WIDTH,
				(row + col) * (NUM_COLORS + 1.0) / (OUTPUT_WIDTH + OUTPUT_HEIGHT)
			);
		}
	}
}

int main(int argc, char** argv)
{
	/* 
	 * arguments should be in this pattern:
	 *		<executable> <input_file_name>.txt
	 * any more or less arguments is error
	 */

	if (argc != 2)
	{
		cerr << "Incorrect number of command line arguments." << endl;
		cerr << "Syntax: <executable> <input_file_name>.txt" << endl;
		exit(EXIT_FAILURE);
	}

	/* open input file and check for existence */
	ifstream inf{ argv[1] };
	if (!inf)
	{
		cerr << "Input file does not exist." << endl;
		exit(EXIT_FAILURE);
	}

	/*
		At this point, both input and output file should be set
		The following lines of code will read the input file
			and make sure the format is proper
	*/
	int line_count = 0;
	int input_file_line_count = 2;
	string in_line;
	while (!inf.eof())
	{
		/* 
			Ensure the syntax of input file is correct
			Change this condition if adding more params to input file
		*/
		if (line_count >= input_file_line_count)
		{
			cerr << "Error: Too many lines in input file." << endl;
			cerr << "Syntax: imsize <width> <height>" << endl;
			exit(EXIT_FAILURE);
		}

		/* gets line and splits into tokens */
		getline(inf, in_line);
		vector<string> tokens = split(in_line, " ");

		if (line_count == 0)
		{
			/* checks that proper number of tokesn in input file */
			if (tokens.size() != 3 || tokens[0] != "imsize")
			{
				cerr << "Error: Invalid input." << endl;
				cerr << "Syntax: imsize <width> <height>" << endl;
				exit(EXIT_FAILURE);
			}

			/* check that width and height are numbers with no chars */
			for (auto& c : tokens[1])
			{
				if (!isdigit(c) && c != '-' && c != '.')
				{
					cerr << "Error: Width and height must be integers." << endl;
					cerr << "Syntax: imsize <width> <height>" << endl;
					exit(EXIT_FAILURE);
				}
			}

			for (auto& c : tokens[2])
			{
				if (!isdigit(c) && c != '-' && c != '.')
				{
					cerr << "Error: Width and height must be integers." << endl;
					cerr << "Syntax: imsize <width> <height>" << endl;
					exit(EXIT_FAILURE);
				}
			}

			/*
				Width and height should now be numbers
				Still error handling just in case
				Also, I'm allowing floats to be inputs
				They will just be rounded to the nearest int by stoi()
			*/
			try
			{
				OUTPUT_WIDTH = stoi(tokens[1]);
				OUTPUT_HEIGHT = stoi(tokens[2]);
			}
			catch (invalid_argument& e)
			{
				cerr << "Invalid argument to stoi()." << endl;
				exit(EXIT_FAILURE);
			}
		}
		else if (line_count == 1)
		{
			if (tokens.size() != 1)
			{
				cerr << "Error: Invalid input." << endl;
				cerr << "Syntax: <flags: (none/synth)>" << endl;
				exit(EXIT_FAILURE);
			}

			if (tokens[0] == "none")
			{
				normal = true;
			}
			if (tokens[0] == "perlin")
			{
				perlin = true;
			}
			if (tokens[0] == "particle")
			{
				particle = true;
			}
			if (tokens[0] == "combo")
			{
				combo = true;
			}
		}

		line_count++;
	}

	/* accounts for too few lines in file */
	if (line_count < input_file_line_count)
	{
		cerr << "Error: Too few lines in input file." << endl;
		cerr << "Syntax: imsize <width> <height>" << endl;
		exit(EXIT_FAILURE);
	}

	cout << "Width: " << OUTPUT_WIDTH << endl;
	cout << "Height: " << OUTPUT_HEIGHT << endl << endl;

	/* in file is done being used so close */
	inf.close();

	/*
		Creates output ppm file
		Truncates any existing file with the same name
	*/
	vector<string> in_file_tokens = split((string)argv[1], ".");
	if (in_file_tokens[in_file_tokens.size() - 1] != "txt")
	{
		cerr << "Input file must be a .txt file." << endl;
		exit(EXIT_FAILURE);
	}

	string out_file_name = in_file_tokens[0] + ".ppm";
	ofstream outf{ out_file_name, ios_base::trunc };
	if (!outf)
	{
		cerr << "Error in creating '" << out_file_name << "' output file." << endl;
		exit(EXIT_FAILURE);
	}

	if (normal)
	{
		cout << "Generating Boring Image..." << endl;
		gradient_basic_ppm(outf);
		cout << "Enjoy your image :)" << endl;
	}

	if (perlin)
	{
		cout << "Commencing Perlin Noise Creation..." << endl;
		perlin_based_ppm(outf, 8008);
		cout << "Enjoy your image :)" << endl;
	}

	if (particle)
	{
		cout << "Particle system initializing..." << endl;
		sph_based_ppm(outf, false);
		cout << "Enjoy your image :)" << endl;
	}

	if (combo)
	{
		cout << "Combo detected..." << endl;
		sph_perlin_combo(outf, 8008, false);
		cout << "Enjoy your image :)" << endl;
	}

	outf.close();
}
