#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <typeinfo>
#include <sstream>

#include "utils.h"
#include "vec3.h"
#include "ray.h"
#include "sphere.h"
#include "cylinder.h"
#include "material.h"
#include "color.h"

#include "RayTracer.h"

using namespace std;

template <typename T>
inline string ToString(T value)
{
    stringstream out;
    out << scientific;
    out << value;
    return out.str();
}

Color extract_only_color(string keyword, vector<string>& tokens, ifstream& inf)
{
    if (tokens.size() != 3)
    {
        inf.close();
        string msg = string(
            ((tokens.size() > 3) ? "Too many" : "Too few")
        ) + " values following keyword '" + keyword + "'.\n"
            + "Syntax: " + keyword + " <r> <g> <b>\n";
        err_msg(msg);
    }

    vector<char> valid{'.'};
    for (auto& tok : tokens)
    {
        if (!is_number(tok, valid))
        {
            inf.close();
            err_msg(
                keyword + " r g b must be positive integers or floats.\n"
            );
        }
    }

    try
    {
        float r = stof(tokens[0]);
        float g = stof(tokens[1]);
        float b = stof(tokens[2]);

        if (r > 1 || r < 0)
        {
            inf.close();
            err_msg(keyword + " r value must be between 0 and 1.\n");
        }
        if (g > 1 || g < 0)
        {
            inf.close();
            err_msg(keyword + " g value must be between 0 and 1.\n");
        }
        if (b > 1 || b < 0)
        {
            inf.close();
            err_msg(keyword + " b value must be between 0 and 1.\n");
        }

        return (Color(r, g, b));
    }   
    catch (invalid_argument& e)
    {
        inf.close();
        err_msg("Invalid argument to stof().\n");

        /* this wont be reached ever */
        return Color(0,0,0);
    }
}

vec3 extract_only_vector(string keyword, vector<string>& tokens, ifstream& inf)
{
    if (tokens.size() != 3)
    {
        inf.close();
        string msg = string(
            ((tokens.size() > 3) ? "Too many" : "Too few")
        ) + " values following keyword '" + keyword + "'.\n"
            + "Syntax: " + keyword + " <x> <y> <z>\n";
        err_msg(msg);
    }

    vector<char> valid{'.', '-'};
    for (auto& tok : tokens)
    {
        if (!is_number(tok, valid))
        {
            inf.close();
            err_msg(
                keyword + " x,y,z must be integers or floats.\n"
            );
        }
    }

    try
    {
        return (vec3(
            stof(tokens[0]),
            stof(tokens[1]), 
            stof(tokens[2])
        ));
    }   
    catch (invalid_argument& e)
    {
        inf.close();
        err_msg("Invalid argument to stof().\n");

        /* this wont be reached ever */
        return vec3(0,0,0);
    }
}

static void initiate_ray_casting(vec3& eye, vec3& view, 
    vec3& up, float hfov, float width, float height,  
    Color& bkgcolor, vector<Material*>& materials,
    vector<Object*>& objects, bool parallel, ofstream& outf)
{
    RayTracer r(
        eye,
        view,
        up,
        hfov,
        width,
        height,
        bkgcolor,
        &materials,
        &objects,
        parallel
    );

    vector<Color> pixels = r.gen();

    outf << create_ppm_header("P3", width, height, 256);
    for (auto& c : pixels)
    {
        put(outf, c.r*255, c.g*255, c.b*255);
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
        string msg = string(
            "Incorrect number of command line arguments.\n"
        ) + "Syntax: <executable> <input_file_name>.txt\n";
		err_msg(msg);
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
	string in_line;

    vec3 eye_pos, view_dir, up_dir;
    Color bkgcolor;
    float hfov, out_width, out_height;
    bool parallel = false;
    bool material_applied = false;
    vector<Material*> materials;
    vector<Object*> objects;

    bool eye_present = false;
    bool view_present = false;
    bool up_present = false;
    bool bkg_present = false;
    bool hfov_present = false;
    bool size_present = false;

    vec3 zeros;

	while (!inf.eof())
	{
		/* gets line and splits into tokens */
		getline(inf, in_line);
		vector<string> tokens = split(in_line, " ");

        if (tokens.size() != 0)
        {
            string keyword = tokens[0];
            tokens.erase(tokens.begin());
            if (material_applied && keyword == "sphere")
            {
                if (tokens.size() != 4)
                {
                    inf.close();
                    string msg = string(
                        ((tokens.size() > 4) ? "Too many" : "Too few")
                    ) + " values following keyword 'sphere'.\n"
                      + "Syntax: sphere <cx> <cy> <cz> <rad>\n";
                    err_msg(msg);
                }

                vector<char> valid{'.'};
                if (!is_number(tokens[3], valid))
                {
                    inf.close();
                    err_msg(
                        "circle radius must be positive integer or float.\n"
                    );
                }

                float r = 0.0;
                try 
                {
                    r = stof(tokens[3]);
                }
                catch (invalid_argument& e)
                {
                    inf.close();
                    err_msg("Invalid argument to stof().\n");
                }
                tokens.erase(tokens.end());

                vec3 cent = extract_only_vector(keyword, tokens, inf);

                objects.push_back(new Sphere(
                    cent, r, materials[materials.size()-1]
                ));

                continue;
            }
            else if (material_applied && keyword == "cylinder")
            {
                if (tokens.size() != 8)
                {
                    inf.close();
                    string msg = string(
                        ((tokens.size() > 8) ? "Too many" : "Too few")
                    ) + " values following keyword 'cylinder'.\n"
                      + "Syntax: cylinder <cx> <cy> <cz> <dx> <dy> "
                      + "<dz> <rad> <len>\n";
                    err_msg(msg);
                }

                vector<char> valid{'.'};
                for (int i = 6; i < 8; i++)
                {
                    if (!is_number(tokens[i], valid))
                    {
                        inf.close();
                        string msg = string(
                            "cylinder radius and length "
                        ) + "must be positive integers or floats.\n";
                        err_msg(msg);
                    }
                }

                float r = 0.0;
                float l = 0.0;
                try 
                {
                    r = stof(tokens[6]);
                    l = stof(tokens[7]);
                }
                catch (invalid_argument& e)
                {
                    inf.close();
                    err_msg("Invalid argument to stof().\n");
                }
    
                tokens.erase(tokens.end());
                tokens.erase(tokens.end());

                vector<string> cv{tokens[0], tokens[1], tokens[2]};
                vector<string> dv{tokens[3], tokens[4], tokens[5]};

                vec3 c = extract_only_vector(keyword, cv, inf);
                vec3 d = extract_only_vector(keyword, dv, inf);

                if (d == zeros)
                {
                    string msg = string("");
                    msg = msg + "cylinder direction must be non-zero\n" +
                        "Tolerance: " + ToString(zeros.epsilon) + "\n";
                    err_msg(msg);
                }

                objects.push_back(new Cylinder(
                    c, d, r, l, materials[materials.size()-1]
                ));

                continue;
            }
            else 
            {
                material_applied = false;
            }

            if (keyword == "eye")
            {
                eye_pos = extract_only_vector(keyword, tokens, inf);
                eye_present = true;
            }
            else if (keyword == "viewdir")
            {
                view_dir = extract_only_vector(keyword, tokens, inf);
                view_present = true;
                if (view_dir == zeros)
                {
                    string msg = string("");
                    msg = msg + "viewdir must be non-zero\n" +
                        "Tolerance: " + ToString(zeros.epsilon) + "\n";
                    err_msg(msg);
                }
            }
            else if (keyword == "updir")
            {
                up_dir = extract_only_vector(keyword, tokens, inf);
                up_present = true;
                if (up_dir == zeros)
                {
                    string msg = string("");
                    msg = msg + "updir must be non-zero\n" +
                        "Tolerance: " + ToString(zeros.epsilon) + "\n";
                    err_msg(msg);
                }
                    
            }
            else if (keyword == "hfov")
            {
                if (tokens.size() != 1)
                {
                    inf.close();
                    string msg = string(
                        ((tokens.size() > 1) ? "Too many" : "Too few")
                    ) + " values following keyword 'hfov'.\n"
                      + "Syntax: hfov <horizontal_field_of_view>\n";
                    err_msg(msg);
                }

                vector<char> valid{'.'};
                if (!is_number(tokens[0], valid))
                {
                    inf.close();
                    err_msg("hfov must be a positive integer or float.\n");
                }

                try
                {
                    hfov = stof(tokens[0]);
                }
                catch (invalid_argument& e)
                {
                    inf.close();
                    err_msg(
                        "Invalid argument to stof().\n"
                    );
                }
                hfov_present = true;

                if (hfov > 360 || hfov < 9e-13)
                    err_msg("hfov must be between 9e-13 and 360 degrees.\n");
            }
            else if (keyword == "imsize")
            {
                /* values to extract: width and height */
                if (tokens.size() != 2)
                {
                    inf.close();
                    string msg = string(
                        ((tokens.size() > 3) ? "Too many" : "Too few")
                    ) + " values following keyword 'imsize'.\n"
                      + "Syntax: imsize <width> <height>\n";
                    err_msg(msg);
                }

                vector<char> valid{'.'};
                for (auto& tok : tokens)
                {
                    if (!is_number(tok, valid))
                    {
                        inf.close();
                        err_msg(
                            "imsize width must be positive integer or float.\n"
                        );
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
                    out_width = stoi(tokens[0]);
                    out_height = stoi(tokens[1]);
                }
                catch (invalid_argument& e)
                {
                    inf.close();
                    err_msg(
                        "Invalid argument to stoi().\n"
                    );
                }
                size_present = true;
            }
            else if (keyword == "bkgcolor")
            {
                bkgcolor = extract_only_color(keyword, tokens, inf);
                bkg_present = true;
            }
            else if (keyword == "mtlcolor")
            {
                Color mc = extract_only_color(keyword, tokens, inf);
                Material* m = new Material(mc);
                materials.push_back(m);
                material_applied = true;
            }
            else if (keyword == "projection")
            {
                if (tokens.size() != 1)
                {
                    inf.close();
                    string msg = string(
                        ((tokens.size() > 1) ? "Too many" : "Too few")
                    ) + " values following keyword 'projection'.\n"
                      + "Syntax: projection parallel\n";
                    err_msg(msg);
                }

                if (tokens[0] == "parallel")
                {
                    parallel = true;
                }
                else 
                {
                    inf.close();
                    err_msg(
                        string(
                            "Token following keyword "
                        ) + "'projection' must be 'parallel'.\n"
                    );
                }
            }
            else 
            {
                inf.close();
                string msg = string("Invalid Keyword: '") + keyword + "'\n";
                err_msg(msg);
            }
        }
	}

	/* in file is done being used so close */
	inf.close();

    if (!eye_present || !view_present ||
        !up_present || !bkg_present ||
        !hfov_present || !size_present)
    {
        string msg = string("");
        msg = msg + "All following keywords must be present:\n" +
            "imsize, eye, viewdir, hfov, updir, bkgcolor\n";
        err_msg(msg);
    }

	/*
		Creates output ppm file
		Truncates any existing file with the same name
	*/
	vector<string> in_file_tokens = split((string)argv[1], ".");
    vector<string> file_tokens = split(in_file_tokens[0], "/");
	if (in_file_tokens[in_file_tokens.size() - 1] != "txt")
	{
        err_msg("Input file must be a .txt file.");
	}
    
	string out_file_name = "./outputs/" + file_tokens[file_tokens.size()-1] + ".ppm";
	ofstream outf{ out_file_name, ios_base::trunc };
	if (!outf)
	{
        string msg = string("Error in creating '") + 
                        out_file_name + "' output file.\n";
		err_msg(msg);
	}

    initiate_ray_casting(
        eye_pos,
        view_dir,
        up_dir,
        hfov,
        out_width,
        out_height,
        bkgcolor,
        materials,
        objects,
        parallel,
        outf
    );

    for (int i = 0; i < materials.size(); i++)
        delete materials[i];
    for (int i = 0; i < objects.size(); i++)
        delete objects[i];

    return 0;
}