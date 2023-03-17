#include <iostream>
#include <fstream>
#include <iomanip>
#include <vector>
#include <typeinfo>
#include <sstream>
#include <chrono>
#include <sys/stat.h>

#include "../include/utils.h"
#include "../include/vec3.h"
#include "../include/ray.h"
#include "../include/sphere.h"
#include "../include/cylinder.h"
#include "../include/triangle.h"
#include "../include/material.h"
#include "../include/color.h"
#include "../include/light.h"
#include "../include/texture.h"

#include "../include/RayTracer.h"

using namespace std;

template <typename T>
inline string ToString(T value)
{
    stringstream out;
    out << scientific;
    out << value;
    return out.str();
}

/*
    Sorry for not commenting this too much.
    Its really just a bunch of file manip and error handling
     so idk what there is much to say. 
    Any argument specifications is in the readme. 
    Hope you're having a good day!
    If you're not, hopefully tomorrow will be better!
*/

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
    Color& bkgcolor, vector<Object*>* objects,
    bool parallel, int threads, vector<Light*>* lights, 
    bool cueing, float amax, float amin, float dmax, 
    float dmin, Color& cueingcolor, ofstream& outf)
{
    RayTracer r(
        eye,
        view,
        up,
        hfov,
        width,
        height,
        bkgcolor,
        objects,
        lights,
        parallel,
        cueing,
        amin,
        amax,
        dmin,
        dmax,
        cueingcolor,
        threads
    );

    vector<Color> pixels;
    int tests = 15;
    auto start = chrono::high_resolution_clock::now();
    // for (int i = 0; i < tests; i++)
    // {
    //     r.gen(pixels);
    // }

    tests = 1;
    r.gen(pixels);
    
    auto stop = chrono::high_resolution_clock::now();

    auto duration = chrono::duration_cast<chrono::milliseconds>(stop - start);

    cout << duration.count() / tests << "ms" << endl;

    outf << create_ppm_header("P3", width, height, 255);

    put_all_normalized(outf, pixels, 255);
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
		err_msg("Input file does not exist.");
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
    bool texture_applied = false;
    bool normal_map_applied = false;
    vector<Material*> materials;
    vector<Texture*> textures;
    vector<NormalMap*> normals;
    vector<Object*> objects;
    vector<Light*> lights;
    vector<Triangle*> triangles;

    /* for triangles, starting flat */
    vector<vec3> vertices;
    vector<vec3> vertex_normals;
    vector<vec3> vertex_texture_coords;

    vector<vector<vec3*>> triangle_defs;
    vector<vector<vec3*>> vertex_normal_defs;
    vector<vector<vec3*>> vertex_texture_coord_defs;
    vector<vector<Texture*>> texture_defs;
    vector<vector<NormalMap*>> normal_defs;

    bool eye_present = false;
    bool view_present = false;
    bool up_present = false;
    bool bkg_or_cueing_present = false;
    bool hfov_present = false;
    bool size_present = false;

    bool quad = false;

    bool cueing = false;
    float amax = -1;
    float amin = -1;
    float dmax = -1;
    float dmin = -1;
    Color cueingcolor;

    int threads = -1;

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
                    if (r < zeros.epsilon)
                    {
                        string msg = string("");
                        msg = msg + "sphere radius must be non-zero\n" +
                            "Tolerance: " + ToString(zeros.epsilon) + "\n";
                        err_msg(msg);
                    }
                }
                catch (invalid_argument& e)
                {
                    inf.close();
                    err_msg("Invalid argument to stof().\n");
                }
                tokens.erase(tokens.end());

                vec3 cent = extract_only_vector(keyword, tokens, inf);

                

                Texture* text = nullptr;
                if (texture_applied) text = textures[textures.size()-1];

                NormalMap* nmap = nullptr;
                if (normal_map_applied) nmap = normals[normals.size()-1];

                objects.push_back(new Sphere(
                    cent, r, materials[materials.size()-1], text, nmap
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

                    if (r < zeros.epsilon)
                    {
                        string msg = string("");
                        msg = msg + "cylinder radius must be non-zero\n" +
                            "Tolerance: " + ToString(zeros.epsilon) + "\n";
                        err_msg(msg);
                    }

                    if (l < zeros.epsilon)
                    {
                        string msg = string("");
                        msg = msg + "cylinder length must be non-zero\n" +
                            "Tolerance: " + ToString(zeros.epsilon) + "\n";
                        err_msg(msg);
                    }
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

                // Texture* text = nullptr;
                // if (texture_applied) text = textures[textures.size()-1];
                objects.push_back(new Cylinder(
                    c, d, r, l, materials[materials.size()-1]/*, text*/
                ));
                continue;
            }
            else if (material_applied && keyword == "f")
            {
                if (tokens.size() != 3 && tokens.size() != 4) 
                {
                    inf.close();
                    string msg = string(
                        ((tokens.size() > 3) ? "Too many" : "Too few")
                    ) + " values following keyword 'f'.\n"
                      + "Syntax: f <v1> <v2> <v3>\n";
                    err_msg(msg);
                }

                vector<string> toks = split_vector(tokens, "//");
                vector<string> slash_toks = split_vector(tokens, "/");

                vec3* tri_d = nullptr;
                vec3* vert_d = nullptr;
                vec3* text_d = nullptr;

                if (toks.size() == 6)
                {
                    vector<string> tri = {toks[0], toks[2], toks[4]};
                    vector<string> nor = {toks[1], toks[3], toks[5]};

                    tri_d = new vec3(extract_only_vector(keyword, tri, inf));
                    vert_d = new vec3(extract_only_vector(keyword, nor, inf));
                }
                else if (slash_toks.size() == 6)
                {
                    vector<string> tri = {
                        slash_toks[0], slash_toks[2], slash_toks[4]
                    };
                    vector<string> tex = {
                        slash_toks[1], slash_toks[3], slash_toks[5]
                    };

                    tri_d = new vec3(extract_only_vector(keyword, tri, inf));
                    text_d = new vec3(extract_only_vector(keyword, tex, inf));
                }
                else if (slash_toks.size() == 9)
                {
                    vector<string> tri = {
                        slash_toks[0], slash_toks[3], slash_toks[6]
                    };
                    vector<string> tex = {
                        slash_toks[1], slash_toks[4], slash_toks[7]
                    };
                    vector<string> nor = {
                        slash_toks[2], slash_toks[5], slash_toks[8]
                    };

                    tri_d = new vec3(extract_only_vector(keyword, tri, inf));
                    text_d = new vec3(extract_only_vector(keyword, tex, inf));
                    vert_d = new vec3(extract_only_vector(keyword, nor, inf));
                }
                /* special case because kiwi was in quads */
                else if (slash_toks.size() == 12)
                {
                    vector<string> tri = {
                        slash_toks[0], slash_toks[3], slash_toks[6]
                    };
                    // vector<string> tex = {
                    //     slash_toks[1], slash_toks[4], slash_toks[7]
                    // };
                    vector<string> nor = {
                        slash_toks[2], slash_toks[5], slash_toks[8]
                    };

                    tri_d = new vec3(extract_only_vector(keyword, tri, inf));
                    // text_d = new vec3(extract_only_vector(keyword, tex, inf));
                    vert_d = new vec3(extract_only_vector(keyword, nor, inf));

                    triangle_defs[triangle_defs.size()-1]
                        .push_back(tri_d);
                    vertex_normal_defs[vertex_normal_defs.size()-1]
                        .push_back(vert_d);
                    vertex_texture_coord_defs[vertex_texture_coord_defs.size()-1]
                        .push_back(text_d);

                    tri = {
                        slash_toks[6], slash_toks[9], slash_toks[0]
                    };
                    // tex = {
                    //     slash_toks[7], slash_toks[10], slash_toks[1]
                    // };
                    nor = {
                        slash_toks[8], slash_toks[11], slash_toks[2]
                    };

                    tri_d = new vec3(extract_only_vector(keyword, tri, inf));
                    // text_d = new vec3(extract_only_vector(keyword, tex, inf));
                    vert_d = new vec3(extract_only_vector(keyword, nor, inf));

                    triangle_defs[triangle_defs.size()-1]
                        .push_back(tri_d);
                    vertex_normal_defs[vertex_normal_defs.size()-1]
                        .push_back(vert_d);
                    vertex_texture_coord_defs[vertex_texture_coord_defs.size()-1]
                        .push_back(text_d);

                    Texture* texture_d = nullptr;
                    if (texture_applied)
                        texture_d = textures[textures.size()-1];

                    texture_defs[texture_defs.size()-1]
                        .push_back(texture_d);
                    texture_defs[texture_defs.size()-1]
                        .push_back(texture_d);

                    NormalMap* nmap = nullptr;
                    if (normal_map_applied)
                        nmap = normals[normals.size()-1];
                    normal_defs[normal_defs.size()-1]
                        .push_back(nmap);
                    normal_defs[normal_defs.size()-1]
                        .push_back(nmap);

                    quad = true;

                    continue;
                }
                else 
                {
                    tri_d = new vec3(extract_only_vector(keyword, tokens, inf));
                }

                Texture* texture_d = nullptr;
                if (texture_applied)
                    texture_d = textures[textures.size()-1];

                texture_defs[texture_defs.size()-1]
                    .push_back(texture_d);

                NormalMap* nmap = nullptr;
                if (normal_map_applied)
                    nmap = normals[normals.size()-1];

                normal_defs[normal_defs.size()-1]
                    .push_back(nmap);

                triangle_defs[triangle_defs.size()-1]
                    .push_back(tri_d);
                vertex_normal_defs[vertex_normal_defs.size()-1]
                    .push_back(vert_d);
                vertex_texture_coord_defs[vertex_texture_coord_defs.size()-1]
                    .push_back(text_d);
                
                continue;
            }
            else if (keyword != "//" && 
                     keyword != "v" && 
                     keyword != "vn" &&
                     keyword != "vt" &&
                     keyword != "texture" &&
                     keyword != "bump")
            {
                material_applied = false;
                texture_applied = false;
                normal_map_applied = false;
            }

            if (!material_applied && keyword == "circle")
            {
                inf.close();
                err_msg("circle must follow a material specification\n");
            }
            else if (!material_applied && keyword == "cylinder")
            {
                inf.close();
                err_msg("cylinder must follow a material specification\n");
            }
            else if (keyword == "eye")
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
                    if (out_width < 1)
                        err_msg("width must be greater than 0\n");
                    if (out_height < 1)
                        err_msg("height must be greater than 0\n");
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
                bkg_or_cueing_present = true;
            }
            else if (keyword == "mtlcolor")
            {
                if (tokens.size() != 10)
                {
                    inf.close();
                    string msg = string(
                        ((tokens.size() > 3) ? "Too many" : "Too few")
                    ) + " values following keyword 'mtlcolor'.\n"
                      + "Syntax: mtlcolor <Odr> <Odg> <Odb> <Osr> <Osg> "
                      + "<Osb> <ka> <kd> <ks> <n>\n";
                    err_msg(msg);
                }

                vector<string> vdiffuse = {tokens[0], tokens[1], tokens[2]};
                vector<string> vspecular = {tokens[3], tokens[4], tokens[5]};
                vector<string> coefficients = {
                    tokens[6], tokens[7], tokens[8], tokens[9]
                };

                Color diffuse = extract_only_color(keyword, vdiffuse, inf);
                Color specular = extract_only_color(keyword, vspecular, inf);

                vector<char> valid{'.'};
                for (auto& t : coefficients)
                {
                    if (!is_number(t, valid))
                    {
                        inf.close();
                        err_msg(
                            "mtlcolor coeffs must be floats/ints.\n"
                        );
                    }
                }

                float ka, kd, ks, n;
                try
                {
                    ka = stof(coefficients[0]);
                    kd = stof(coefficients[1]);
                    ks = stof(coefficients[2]);
                    n = stof(coefficients[3]);
                    

                }
                catch (invalid_argument& e)
                {
                    inf.close();
                    err_msg(
                        "Invalid argument to stof().\n"
                    );
                }
                
                Material* m = new Material(
                    diffuse,
                    specular, 
                    ka,
                    kd,
                    ks,
                    n
                );
                materials.push_back(m);
                material_applied = true;

                triangle_defs.push_back(vector<vec3*>());
                vertex_normal_defs.push_back(vector<vec3*>());
                vertex_texture_coord_defs.push_back(vector<vec3*>());
                texture_defs.push_back(vector<Texture*>());
                normal_defs.push_back(vector<NormalMap*>());
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
            else if (keyword == "threadcount")
            {
                if (tokens.size() != 1)
                {
                    inf.close();
                    string msg = string(
                        ((tokens.size() > 1) ? "Too many" : "Too few")
                    ) + " values following keyword 'threadcount'.\n"
                      + "Syntax: threadcount <num>\n";
                    err_msg(msg);
                }

                vector<char> valid;
                for (auto& tok : tokens)
                {
                    if (!is_number(tok, valid))
                    {
                        inf.close();
                        err_msg(
                            "threadcount width must be positive, nonzero, integer.\n"
                        );
                    }
                }

                try
                {
                    threads = stoi(tokens[0]);
                    if (out_width < 1)
                        err_msg("threadcount must be greater than 0\n");
                }
                catch (invalid_argument& e)
                {
                    inf.close();
                    err_msg(
                        "Invalid argument to stoi().\n"
                    );
                }
            }
            else if (keyword == "light")
            {
                if (tokens.size() != 7)
                {
                    inf.close();
                    string msg = string(
                        ((tokens.size() > 1) ? "Too many" : "Too few")
                    ) + " values following keyword 'light'.\n"
                      + "Syntax: light <x> <y> <z> <w> <r> <g> <b>\n";
                    err_msg(msg);
                }

                vector<string> vpos{tokens[0], tokens[1], tokens[2]};
                string type_light = tokens[3];
                vector<string> vcolor{tokens[4], tokens[5], tokens[6]};
                
                vec3 pos = extract_only_vector(keyword, vpos, inf);
                Color col = extract_only_color(keyword, vcolor, inf);

                vector<char> valid;
                if (!is_number(type_light, valid))
                {
                    inf.close();
                    err_msg(
                        "light w must be 1 (point) or 0 (directional).\n"
                    );
                }

                int w;
                try
                {
                    w = stoi(type_light);
                }
                catch (invalid_argument& e)
                {
                    inf.close();
                    err_msg(
                        "Invalid argument to stoi().\n"
                    );
                }

                if (w == 0)
                {
                    if (pos == zeros)
                    {
                        inf.close();
                        err_msg(
                            "directional light direction must be non-zero\n"
                        );
                    }
                    lights.push_back(
                        new DirectionalLight(
                            pos,
                            col
                        )
                    );
                }
                else if (w == 1)
                {
                    lights.push_back(
                        new PointLight(
                            pos,
                            col
                        )
                    );
                }
                else 
                {
                    inf.close();
                    err_msg(
                        "light w must be 1 (point) or 0 (directional).\n"
                    );
                }
            }
            else if (keyword == "attlight")
            {
                if (tokens.size() != 10)
                {
                    inf.close();
                    string msg = string(
                        ((tokens.size() > 1) ? "Too many" : "Too few")
                    ) + " values following keyword 'attenlight'.\n"
                      + "Syntax: attlight <x> <y> <z> <w> <r> <g> <b> <c1> <c2> <c3>\n";
                    err_msg(msg);
                }

                vector<string> vpos{tokens[0], tokens[1], tokens[2]};
                string type_light = tokens[3];
                vector<string> vcolor{tokens[4], tokens[5], tokens[6]};
                vector<string> vfalloff{tokens[7], tokens[8], tokens[9]};
                
                vec3 pos = extract_only_vector(keyword, vpos, inf);
                Color col = extract_only_color(keyword, vcolor, inf);
                vec3 falloff = extract_only_vector(keyword, vfalloff, inf);
                if (falloff.x < 0 || falloff.y < 0 || falloff.z < 0)
                {
                    inf.close();
                    err_msg(
                        "falloff constants must be positive or zero.\n"
                    );
                }
                     

                vector<char> valid;
                if (!is_number(type_light, valid))
                {
                    inf.close();
                    err_msg(
                        "attlight w must be 1 (point).\n"
                    );
                }

                int w;
                try
                {
                    w = stoi(type_light);
                }
                catch (invalid_argument& e)
                {
                    inf.close();
                    err_msg(
                        "Invalid argument to stoi().\n"
                    );
                }

                if (w == 1)
                {
                    lights.push_back(
                        new AttenPointLight(
                            pos,
                            col,
                            falloff.x, 
                            falloff.y,
                            falloff.z
                        )
                    );
                }
                else 
                {
                    inf.close();
                    err_msg(
                        "attenuated light must be point light (0).\n"
                    );
                }
            }
            else if (keyword == "depthcueing")
            {
                if (tokens.size() != 7)
                {
                    inf.close();
                    string msg = string(
                        ((tokens.size() > 1) ? "Too many" : "Too few")
                    ) + " values following keyword 'depthcueing'.\n"
                      + "Syntax: depthcueing <dcr> <dcg> <dcb> "
                      + "<amax> <amin> <dmax> <dmin>\n";
                    err_msg(msg);
                }
                vector<string> depthc{tokens[0], tokens[1], tokens[2]};
                vector<string> coefs{tokens[3], tokens[4], tokens[5], tokens[6]};

                cueingcolor = extract_only_color(keyword, depthc, inf);
                vector<char> valid{'.'};
                for (auto& tok : tokens)
                {
                    if (!is_number(tok, valid))
                    {
                        inf.close();
                        err_msg(
                            "threadcount width must be positive, nonzero, integer.\n"
                        );
                    }
                }

                try
                {
                    amax = stof(coefs[0]);
                    amin = stof(coefs[1]);
                    dmax = stof(coefs[2]);
                    dmin = stof(coefs[3]);
                }
                catch (invalid_argument& e)
                {
                    inf.close();
                    err_msg(
                        "Invalid argument to stoi().\n"
                    );
                }

                bkg_or_cueing_present = true;
                cueing = true;
            }
            else if (keyword == "v")
            {
                if (tokens.size() != 3) 
                {
                    inf.close();
                    string msg = string(
                        ((tokens.size() > 3) ? "Too many" : "Too few")
                    ) + " values following keyword 'v'.\n"
                      + "Syntax: v <x> <y> <z>\n";
                    err_msg(msg);
                }

                vertices.push_back(
                    extract_only_vector(keyword, tokens, inf)
                );
            }
            else if (keyword == "vn")
            {
                if (tokens.size() != 3) 
                {
                    inf.close();
                    string msg = string(
                        ((tokens.size() > 3) ? "Too many" : "Too few")
                    ) + " values following keyword 'v'.\n"
                      + "Syntax: v <x> <y> <z>\n";
                    err_msg(msg);
                }

                vertex_normals.push_back(
                    extract_only_vector(keyword, tokens, inf)
                );
            }
            else if (keyword == "vt")
            {
                if (tokens.size() != 2) 
                {
                    inf.close();
                    string msg = string(
                        ((tokens.size() > 3) ? "Too many" : "Too few")
                    ) + " values following keyword 'vt'.\n"
                      + "Syntax: vt <u> <v>\n";
                    err_msg(msg);
                }

                tokens.push_back("0");

                vertex_texture_coords.push_back(
                    extract_only_vector(keyword, tokens, inf)
                );
            }
            else if (keyword == "texture")
            {
                if (tokens.size() != 1)
                {
                    inf.close();
                    string msg = string(
                        ((tokens.size() > 1) ? "Too many" : "Too few")
                    ) + " values following keyword 'texture'.\n"
                      + "Syntax: texture <ppmfile>\n";
                    err_msg(msg);
                }
                
                textures.push_back(read_ppm_to_texture(tokens[0]));
                
                texture_applied = true;
            }
            else if (keyword == "bump")
            {
                if (tokens.size() != 1)
                {
                    inf.close();
                    string msg = string(
                        ((tokens.size() > 1) ? "Too many" : "Too few")
                    ) + " values following keyword 'bump'.\n"
                      + "Syntax: bump <ppmfile>\n";
                    err_msg(msg);
                }
                
                normals.push_back(read_ppm_to_normal(tokens[0]));
                normal_map_applied = true;
            }
            else if (keyword != "//")
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
        !up_present || !bkg_or_cueing_present ||
        !hfov_present || !size_present)
    {
        string msg = string("");
        msg = msg + "All following keywords must be present:\n" +
            "imsize, eye, viewdir, hfov, updir, bkgcolor/depthcueing\n";
        err_msg(msg);
    }

    /* create triangles and check that all vertices were specified properly */
    for (int i = 0; i < triangle_defs.size(); i++)
    {
        for (int j = 0; j < triangle_defs[i].size(); j++)
        {
            vec3* norm1=nullptr; vec3* norm2=nullptr; vec3* norm3=nullptr;
            vec3* text1=nullptr; vec3* text2=nullptr; vec3* text3=nullptr;
            Texture* t_texture = nullptr;
            NormalMap* t_normal = nullptr;

            if (triangle_defs[i][j] == nullptr)
                err_msg("This shouldn't have happened.\n");

            int v1 = (int)triangle_defs[i][j]->x;
            int v2 = (int)triangle_defs[i][j]->y;
            int v3 = (int)triangle_defs[i][j]->z;

            delete triangle_defs[i][j];

            int vcnt = vertices.size();
            if (v1 <= 0 || v1 > vcnt) 
                err_msg("Invalid vertex index specified\n");
            if (v2 <= 0 || v2 > vcnt) 
                err_msg("Invalid vertex index specified\n");
            if (v3 <= 0 || v3 > vcnt) 
                err_msg("Invalid vertex index specified\n");

            if (vertex_normal_defs[i][j] != nullptr)
            {
                int vn1 = (int)vertex_normal_defs[i][j]->x;
                int vn2 = (int)vertex_normal_defs[i][j]->y;
                int vn3 = (int)vertex_normal_defs[i][j]->z;

                int vncnt = vertex_normals.size();
                if (vn1 <= 0 || vn1 > vncnt) 
                    err_msg("Invalid vertex normal index specified\n");
                if (vn2 <= 0 || vn2 > vncnt) 
                    err_msg("Invalid vertex normal index specified\n");
                if (vn3 <= 0 || vn3 > vncnt) 
                    err_msg("Invalid vertex normal index specified\n");

                norm1 = &vertex_normals[vn1-1];
                norm2 = &vertex_normals[vn2-1];
                norm3 = &vertex_normals[vn3-1];

                delete vertex_normal_defs[i][j];
            }

            if (vertex_texture_coord_defs[i][j] != nullptr) {
                int vt1 = (int)vertex_texture_coord_defs[i][j]->x;
                int vt2 = (int)vertex_texture_coord_defs[i][j]->y;
                int vt3 = (int)vertex_texture_coord_defs[i][j]->z;

                int vtcnt = vertex_texture_coords.size();
                if (vt1 <= 0 || vt1 > vtcnt) 
                    err_msg("Invalid vertex texture index specified\n");
                if (vt2 <= 0 || vt2 > vtcnt) 
                    err_msg("Invalid vertex texture index specified\n");
                if (vt3 <= 0 || vt3 > vtcnt) 
                    err_msg("Invalid vertex texture index specified\n");

                text1 = &vertex_texture_coords[vt1-1];
                text2 = &vertex_texture_coords[vt2-1];
                text3 = &vertex_texture_coords[vt3-1];

                delete vertex_texture_coord_defs[i][j];

                assert(
                    normal_defs[i][j] != nullptr || 
                    texture_defs[i][j] != nullptr
                );

                if (texture_defs[i][j] != nullptr)
                    t_texture = texture_defs[i][j];
                if (normal_defs[i][j] != nullptr)
                    t_normal = normal_defs[i][j];   
            }

            objects.push_back(
                new Triangle(
                    &vertices[v1-1],
                    &vertices[v2-1],
                    &vertices[v3-1],
                    norm1, norm2, norm3,
                    text1, text2, text3,
                    materials[i], t_texture, t_normal
                )
            );
        }
    }



	/*
		Creates output ppm file
		Truncates any existing file with the same name
	*/
	vector<string> in_file_tokens = split((string)argv[1], ".");
    vector<string> file_tokens = split(in_file_tokens[0], "/");
	if (in_file_tokens[in_file_tokens.size() - 1] != "txt")
	{
        err_msg("Input file must be a .txt file.\n");
	}

    string dirname = "outputs";
    if (mkdir("outputs", 0777) == -1)
        cerr << "outputs directory already exists." << endl;

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
        &objects,
        parallel,
        threads,
        &lights,
        cueing,
        amax,
        amin,
        dmax,
        dmin,
        cueingcolor,
        outf
    );

    for (int i = 0; i < materials.size(); i++)
        delete materials[i];

    for (int i = 0; i < objects.size(); i++)
        delete objects[i];

    for (int i = 0; i < lights.size(); i++)
        delete lights[i];

    for (int i = 0; i < triangles.size(); i++)
        delete triangles[i];

    for (int i = 0; i < textures.size(); i++)
        delete textures[i];

    for (int i = 0; i < normals.size(); i++)
        delete normals[i];

    return 0;
}