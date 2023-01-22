#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>
#include <tuple>

#include "utils.h"
#include "PerlinNoise.hpp"
#include "SPH.hpp"
#include "Voronoi.hpp"

using namespace std;

static int OUTPUT_WIDTH = 256;
static int OUTPUT_HEIGHT = 256;
static int NUM_COLORS = 255;

static bool normal = false;
static bool perlin = false;
static bool particle = false;
static bool combo = false;
static bool voronoi = false;

/*
	Helper function to put the rgb values to a ppm file
*/
static void put(ofstream& outf, float r, float g, float b)
{
	outf <<
		(int)r << " " <<
		(int)g << " " <<
		(int)b << "\n";
}

/*
	Uses a voronoi diagram to make the ppm.
	Not finished yet.
*/
static void voronoi_ppm(ofstream& outf, int seed, int node_count,
	float v_r, float v_g, float v_b, bool type, bool rand_c)
{
	outf << create_ppm_header("P3", OUTPUT_WIDTH, OUTPUT_HEIGHT, NUM_COLORS);

	Voronoi v(
		seed, 
		node_count, 
		OUTPUT_WIDTH, 
		OUTPUT_HEIGHT,
		v_r, v_g, v_b,
		type,
		rand_c
	);
	vector<tuple<float, float, float>> pixels = v.gen();

	for (auto& p : pixels)
	{
		put(
			outf, 
			get<0>(p),
			get<1>(p),
			get<2>(p)
		);
	}
}

/*
	This combines the SPH and perlin noise generation.
	The SPH sim is run, and any pixel where the pressure value
	 is under a certain threshold, the perlin noise is calculated for it
	 and colored based on that. 
	I think it results in a pretty cool looking image, especially when
	 gravity is a circle rather than normal.

*/
static void sph_perlin_combo(ofstream& outf, int seed, bool circle_grav)
{
	outf << create_ppm_header("P3", OUTPUT_WIDTH, OUTPUT_HEIGHT, NUM_COLORS);

	PerlinNoise pns(seed);

	SPH sim(500, 1000, OUTPUT_WIDTH, OUTPUT_HEIGHT, circle_grav);
	sim.run();

	vector<float> press_vals = sim.calc_pressure();
	vector<float> dens_vals = sim.calc_density(); /* ignore this */

	/* color scheme 1 */
	float max = -9e-15;
	float min = 9e15;
	for (auto& f : press_vals)
	{
		if (f < 0) f = 0;
		if (f > max) max = f;
		if (f < min) min = f;
	}

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

/*
	This one was fun and interesting.
	A SPH fluid simulation is run for a number of timesteps
	 and the pressure at each time step is recorded.
	After all timesteps have been gone through,
	 the pressure is then slightly diffused to surrounding pixels
	 and the log is taken in order to reduce it slightly to make it look better.
	The timesteps are combined and it is returned after which the values
	 are converted into a range between 0 and 255 and the blue channel is used.
*/
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

/*
	Generates a ppm based on perlin noise
	The r_val, g_val, and b_val number multipliers
	 determine the "look" of the perlin noise.
	The lower they are, the more "wood" like and smooth
	The higher they are, the more chaotic and disjointed
*/
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

	/* specific type values */
	int seed, node_count;
	bool shade_type, color_type;
	float v_r, v_g, v_b;
	bool circle_grav;
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

		if (line_count == 0) /* line 0 will always be the same */
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
				if (!isdigit(c) && c != '.')
				{
					cerr << "Error: Width and height must be integers." << endl;
					cerr << "Syntax: imsize <width> <height>" << endl;
					exit(EXIT_FAILURE);
				}
			}

			for (auto& c : tokens[2])
			{
				if (!isdigit(c) && c != '.')
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
		else if (line_count == 1) /* line 1 will always determine type */
		{
			if (tokens.size() != 1)
			{
				cerr << "Error: Invalid input." << endl;
				cerr << "Syntax: <flags: (none/perlin/particle/combo/voronoi)>" << endl;
				exit(EXIT_FAILURE);
			}

			/* self explanatory */

			if (tokens[0] == "none")
			{
				normal = true;
				input_file_line_count = 2;
			}
			else if (tokens[0] == "perlin")
			{
				perlin = true;
				input_file_line_count = 3;
			}
			else if (tokens[0] == "particle")
			{
				particle = true;
				input_file_line_count = 3;
			}
			else if (tokens[0] == "combo")
			{
				combo = true;
				input_file_line_count = 3;
			}
			else if (tokens[0] == "voronoi")
			{
				voronoi = true;
				input_file_line_count = 3;
			}
			else
			{
				cerr << "Error: Invalid input." << endl;
				cerr << "Syntax: <flags: (none/perlin/particle/combo/voronoi)>" << endl;
				exit(EXIT_FAILURE);
			}
		}
		else /* each type of synthesis will have own argument count */
		{
			/*
				Perlin: single seed integer
				Particle: gravity type (normal / circle)
				Combo: seed then gravity type
				Voronoi: seed

				For all, seed being 0 means no seed is used
			*/

			/*
				Honestly I could probably make this cleaner using 
				 variables for each case but uhhhhh I don't really care that much
				Also comments here will be a bit sparse as its pretty straight forward
			*/

			if ((perlin || particle) && tokens.size() != 1)
			{
				cerr << "Error: Invalid input." << endl;
				cerr << "Syntax: ";
				cerr << (particle ? "<grav_type: normal/circle>" : "<seed>") << endl;
				exit(EXIT_FAILURE);
			}

			if (combo && tokens.size() != 2)
			{
				cerr << "Error: Invalid input." << endl;
				cerr << "Syntax: <seed> <grav_type: normal/circle>" << endl;
				exit(EXIT_FAILURE);
			}

			if (voronoi && tokens.size() != 4 && tokens.size() != 7)
			{
				cerr << "Error: Incorrect number of tokens." << endl
					<< "Syntax: "
					<< "<seed> <node_count> <flat / gradient> "
					<< "<random / set> <r> <g> <b>" << endl
					<< "If random, <r>, <g>, <b> do not need to be included." << endl;
				exit(EXIT_FAILURE);
			}

			if (perlin || combo || voronoi)
			{
				for (auto& c : tokens[0])
				{
					if (!isdigit(c))
					{
						cerr << "Error: Seed must be a positive integer or zero." << endl;
						exit(EXIT_FAILURE);
					}
				}

				seed = stoi(tokens[0]);
				if (seed < 0)
				{
					cerr << "Error: Seed must be positive or zero." << endl;
					exit(EXIT_FAILURE);
				}
			}
			else
			{
				if (tokens[0] == "normal")
				{
					circle_grav = false;
				}
				else if (tokens[0] == "circle")
				{
					circle_grav = true;
				}
				else
				{
					cerr << "Error: Invalid Input" << endl;
					cerr << "Syntax: <grav_type: normal/circle>" << endl;
					exit(EXIT_FAILURE);
				}
			}

			if (combo)
			{
				if (tokens[1] == "normal")
				{
					circle_grav = false;
				}
				else if (tokens[1] == "circle")
				{
					circle_grav = true;
				}
				else
				{
					cerr << "Error: Invalid Input" << endl;
					cerr << "Syntax: <grav_type: normal/circle>" << endl;
					exit(EXIT_FAILURE);
				}
			}

			if (voronoi)
			{
				/* extract node count */
				for (auto& c : tokens[1])
				{
					if (!isdigit(c))
					{
						cerr << "Error: Node count must be a positive integer." << endl;
						exit(EXIT_FAILURE);
					}
				}

				node_count = stoi(tokens[1]);
				if (node_count <= 1)
				{
					cerr << "Error: Node count must be greater than 1" << endl;
					exit(EXIT_FAILURE);
				}

				/* extract shading type */
				if (tokens[2] == "flat")
				{
					shade_type = false;
				}
				else if (tokens[2] == "gradient")
				{
					shade_type = true;
				}
				else
				{
					cerr << "Error: shade must be flat or gradient." << endl
						<< "Syntax: "
						<< "<seed> <node_count> <flat / gradient> "
						<< "<random / set> <r> <g> <b>" << endl
						<< "If random, <r>, <g>, <b> do not need to be included." << endl;
					exit(EXIT_FAILURE);
				}

				/* extract color type */
				if (tokens[3] == "random")
				{
					color_type = true;
				}
				else if (tokens[3] == "set")
				{
					color_type = false;
				}
				else
				{
					cerr << "Error: color must be random or set." << endl
						<< "Syntax: "
						<< "<seed> <node_count> <flat / gradient> "
						<< "<random / set> <r> <g> <b>" << endl
						<< "If random, <r>, <g>, <b> do not need to be included." << endl;
					exit(EXIT_FAILURE);
				}

				v_r = 0; v_b = 0; v_g = 0;
				/* if set colors, extract r, g, b */
				if (!color_type)
				{
					if (tokens.size() != 7)
					{
						cerr << "Error: r g b must be included with set colors" << endl
							<< "Syntax: "
							<< "<seed> <node_count> <flat / gradient> "
							<< "<random / set> <r> <g> <b>" << endl
							<< "If random, <r>, <g>, <b> do not need to be included." << endl;
						exit(EXIT_FAILURE);
					}
					
					for (int i = 4; i < 7; i++)
					{
						for (auto& c : tokens[i])
						{
							if (!isdigit(c))
							{
								cerr << "Error: r g b must be positive integers "
									<< "between 0 and 255 inclusive." << endl;
								exit(EXIT_FAILURE);
							}
						}
					}

					v_r = stof(tokens[4]); v_g = stof(tokens[5]); v_b = stof(tokens[6]);
					if (v_r > 255 || v_g > 255 || v_b > 255)
					{
						cerr << "Error: r g b must be positive integers "
							<< "between 0 and 255 inclusive." << endl;
						exit(EXIT_FAILURE);
					}
				}
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

	/* I feel like the rest of this is pretty self evident */

	if (normal)
	{
		cout << "Generating Boring Image..." << endl;
		gradient_basic_ppm(outf);
		cout << "Enjoy your image :)" << endl;
	}

	if (perlin)
	{
		cout << "Commencing Perlin Noise Creation..." << endl;
		perlin_based_ppm(outf, seed);
		cout << "Enjoy your image :)" << endl;
	}

	if (particle)
	{
		cout << "Particle system initializing..." << endl;
		sph_based_ppm(outf, circle_grav);
		cout << "Enjoy your image :)" << endl;
	}

	if (combo)
	{
		cout << "Combo detected..." << endl;
		sph_perlin_combo(outf, seed, circle_grav);
		cout << "Enjoy your image :)" << endl;
	}

	if (voronoi)
	{
		cout << "Voronoi generation..." << endl;
		voronoi_ppm(
			outf,
			seed, 
			node_count, 
			v_r,
			v_g,
			v_b,
			shade_type,
			color_type
		);
		cout << "Enjoy your image :)" << endl;
	}

	outf.close();
}
