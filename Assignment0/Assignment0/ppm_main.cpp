#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <string>
#include <sstream>

#include "utils.h"

using namespace std;

static int OUTPUT_WIDTH = 256;
static int OUTPUT_HEIGHT = 256;
static int NUM_COLORS = 255;

static string create_ppm_header(string type, int width, int height, int colors)
{
	ostringstream s;
	s <<
		type << "\n" <<
		width << "\n" <<
		height << "\n" <<
		colors << "\n";

	string out(s.str());
	
	return out;
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
			outf <<
				row * (NUM_COLORS + 1) / OUTPUT_HEIGHT << " " <<
				col * (NUM_COLORS + 1) / OUTPUT_WIDTH << " " <<
				(row + col) * (NUM_COLORS + 1) / (OUTPUT_WIDTH + OUTPUT_HEIGHT) << "\n";
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
	int input_file_line_count = 1;
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

	gradient_basic_ppm(outf);

	outf.close();
}
