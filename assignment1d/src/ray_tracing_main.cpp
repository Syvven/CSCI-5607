#include <type_traits>
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
#include "../include/mesh.h"
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

static ifstream inf;

static Color extract_only_color(string keyword, vector<string> tokens);
static vec3 extract_only_vector(string keyword, vector<string> tokens);
static float custom_stof(string tok);
static int custom_stoi(string tok);
static void validate_size(int got, int required, string key);
static void end_condition(int cond, string msg);
static void validate_tokens(vector<string> toks, vector<char> valids, string msg);

static Color extract_only_color(string keyword, vector<string> tokens)
{
    validate_size(tokens.size(), 3, keyword);

    validate_tokens(tokens, {'.'}, keyword + " r,g,b ");

    float r = custom_stof(tokens[0]);
    float g = custom_stof(tokens[1]);
    float b = custom_stof(tokens[2]);

    end_condition(
        r > 1 || r < 0 || g > 1 || g < 0 || b > 1 || b < 0,
        keyword + " r,g,b must be between 0 and 1.\n"
    );

    return Color(r, g, b);
}

static vec3 extract_only_vector(string keyword, vector<string> tokens)
{
    validate_size(tokens.size(), 3, keyword);

    validate_tokens(tokens, {'.', '-'}, keyword + " x,y,z ");

    return vec3(
        custom_stof(tokens[0]),
        custom_stof(tokens[1]),
        custom_stof(tokens[2])
    );
}

static float custom_stof(string tok)
{
    try
    {
        return stof(tok);
    }   
    catch (invalid_argument& e)
    {
        end_condition(1, "Invalid argument to stof().\n");
        return 0.0f;
    }
}

static int custom_stoi(string tok)
{
    try
    {
        return stoi(tok);
    }
    catch (invalid_argument& e)
    {
        end_condition(1, "Invalid argument to stoi().\n");
        return 0;
    }
}

static void validate_size(int got, int required, string key)
{
    end_condition(
        required != got,
        string(((got > required) ? "Too many" : "Too few"))
             + " values following keyword " + key + ".\n" +
             "Refer to README for more specific input syntax.\n"
    );
}

static void end_condition(int cond, string msg)
{
    if (cond)
    {
        inf.close();
        err_msg(msg);
    }
}

static void validate_tokens(vector<string> toks, vector<char> valids, string msg)
{
    for (auto& tok : toks)
    {
        end_condition(
            !is_number(tok, valids),
            msg + "must be integers or floats.\n" +
            "Refer to README for more specific input syntax.\n"
        );
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
	inf.open(argv[1]);
	if (!inf) err_msg("Input file does not exist.");

	/*
		At this point, both input and output file should be set
		The following lines of code will read the input file
			and make sure the format is proper
	*/
	string in_line;

    vec3 eye_pos, view_dir, up_dir;
    Color bkgcolor;
    float hfov, out_width, out_height, bkg_eta;
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

    bool block_applied = false;

    bool meshing = false;
    vector<vector<int>> mesh_face_indices;
    vector<int> mesh_levels;
    int mesh_face_counter = 0;

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
                validate_size(tokens.size(), 4, keyword);

                end_condition(
                    !is_number(tokens[3], {'.'}),
                    "circle radius must be positive integer or float.\n"
                );

                float r = custom_stof(tokens[3]);
                end_condition(
                    r < zeros.epsilon,
                    string("") + "sphere radius must be non-zero\n" +
                        "Tolerance: " + ToString(zeros.epsilon) + "\n"
                );
                
                tokens.erase(tokens.end());

                vec3 cent = extract_only_vector(keyword, tokens);

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
                validate_size(tokens.size(), 8, keyword);

                validate_tokens(
                    v_slice(tokens, 6, 7, 1), {'.'}, 
                    "cylinder radius and length "
                );

                float r = custom_stof(tokens[6]);
                float l = custom_stof(tokens[7]);

                end_condition(
                    r < zeros.epsilon || l < zeros.epsilon,
                    string("cylinder radius/length must be non-zero\n") +
                        "Tolerance: " + ToString(zeros.epsilon) + "\n"
                );
    
                tokens.erase(tokens.end());
                tokens.erase(tokens.end());

                vec3 c = extract_only_vector(keyword, v_slice(tokens, 0, 2, 1));
                vec3 d = extract_only_vector(keyword, v_slice(tokens, 3, 5, 1));

                end_condition(
                    d == zeros,
                    string("cylinder direction must be non-zero\n") +
                        "Tolerance: " + ToString(zeros.epsilon) + "\n"
                );

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
                    tri_d = new vec3(extract_only_vector(
                        keyword, v_slice(toks, 0, 4, 2)
                    ));
                    vert_d = new vec3(extract_only_vector(
                        keyword, v_slice(toks, 1, 5, 2)
                    ));
                }
                else if (slash_toks.size() == 6)
                {
                    tri_d = new vec3(extract_only_vector(
                        keyword, v_slice(slash_toks, 0, 4, 2)
                    ));
                    text_d = new vec3(extract_only_vector(
                        keyword, v_slice(slash_toks, 1, 5, 2)
                    ));
                }
                else if (slash_toks.size() == 9)
                {
                    tri_d = new vec3(extract_only_vector(
                        keyword, v_slice(slash_toks, 0, 6, 3)
                    ));
                    text_d = new vec3(extract_only_vector(
                        keyword, v_slice(slash_toks, 1, 7, 3)
                    ));
                    vert_d = new vec3(extract_only_vector(
                        keyword, v_slice(slash_toks, 2, 8, 3)
                    ));
                }
                /* special case because kiwi was in quads */
                else if (slash_toks.size() == 12)
                {
                    tri_d = new vec3(extract_only_vector(
                        keyword, v_slice(slash_toks, 0, 6, 3)
                    ));
                    /* text_d = new vec3(extract_only_vector(
                        keyword, v_slice(slash_toks, 1, 7, 3)
                    )); */
                    vert_d = new vec3(extract_only_vector(
                        keyword, v_slice(slash_toks, 2, 8, 3)
                    ));

                    triangle_defs[triangle_defs.size()-1]
                        .push_back(tri_d);
                    vertex_normal_defs[vertex_normal_defs.size()-1]
                        .push_back(vert_d);
                    vertex_texture_coord_defs[vertex_texture_coord_defs.size()-1]
                        .push_back(text_d);

                    if (meshing) mesh_face_indices[mesh_face_indices.size()-1]
                                    .push_back(mesh_face_counter++);

                    /* cant do slicing here because last is before first */

                    vector<string> tri = {
                        slash_toks[6], slash_toks[9], slash_toks[0]
                    };
                    // tex = {
                    //     slash_toks[7], slash_toks[10], slash_toks[1]
                    // };
                    vector<string> nor = {
                        slash_toks[8], slash_toks[11], slash_toks[2]
                    };

                    tri_d = new vec3(extract_only_vector(keyword, tri));
                    // text_d = new vec3(extract_only_vector(keyword, tex));
                    vert_d = new vec3(extract_only_vector(keyword, nor));

                    triangle_defs[triangle_defs.size()-1]
                        .push_back(tri_d);
                    vertex_normal_defs[vertex_normal_defs.size()-1]
                        .push_back(vert_d);
                    vertex_texture_coord_defs[vertex_texture_coord_defs.size()-1]
                        .push_back(text_d);

                    if (meshing) mesh_face_indices[mesh_face_indices.size()-1]
                                    .push_back(mesh_face_counter++);

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
                    tri_d = new vec3(extract_only_vector(keyword, tokens));
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

                if (meshing) mesh_face_indices[mesh_face_indices.size()-1]
                                .push_back(mesh_face_counter++);
                
                continue;
            }
            else if (keyword != "//" && 
                     keyword != "v" && 
                     keyword != "vn" &&
                     keyword != "vt" &&
                     keyword != "texture" &&
                     keyword != "bump" && 
                     keyword != "mesh")
            {
                material_applied = false;
                texture_applied = false;
                normal_map_applied = false;
            }

            /* end if an object is specified with no material provided */
            if (!material_applied)
            {
                end_condition(
                    keyword == "circle" ||
                    keyword == "cylinder" ||
                    keyword == "f",
                    keyword + " must follow a material specification\n"
                );
            }
            
            if (keyword == "eye")
            {
                eye_pos = extract_only_vector(keyword, tokens);
                eye_present = true;
            }
            else if (keyword == "viewdir")
            {
                view_dir = extract_only_vector(keyword, tokens);
                view_present = true;
                end_condition(
                    view_dir == zeros,
                    string("viewdir must be non-zero\n") +
                        "Tolerance: " + ToString(zeros.epsilon) + "\n"
                );
            }
            else if (keyword == "updir")
            {
                up_dir = extract_only_vector(keyword, tokens);
                up_present = true;

                end_condition(
                    up_dir == zeros,
                    string("updir must be non-zero\n") +
                        "Tolerance: " + ToString(zeros.epsilon) + "\n"
                );  
            }
            else if (keyword == "hfov")
            {
                validate_size(tokens.size(), 1, keyword);

                end_condition(
                    !is_number(tokens[0], {'.'}),
                    "hfov must be a positive integer or float.\n"
                );

                hfov = custom_stof(tokens[0]);
                hfov_present = true;

                end_condition(
                    hfov > 360 || hfov < 9e-13,
                    "hfov must be between 9e-13 and 360 degrees.\n"
                );
            }
            else if (keyword == "imsize")
            {
                /* values to extract: width and height */
                validate_size(tokens.size(), 2, keyword);

                validate_tokens(tokens, {'.'}, "imsize width ");

                /*
                    Width and height should now be numbers
                    Still error handling just in case
                    Also, I'm allowing floats to be inputs
                    They will just be rounded to the nearest int by stoi()
                */
                out_width = custom_stoi(tokens[0]);
                out_height = custom_stoi(tokens[1]);

                end_condition(
                    out_width < 1 || out_height < 1,
                    "width and height must be greater than 0\n"
                );
                
                size_present = true;
            }
            else if (keyword == "bkgcolor")
            {
                validate_size(tokens.size(), 4, keyword);

                bkgcolor = extract_only_color(
                    keyword, v_slice(tokens, 0, 2, 1)
                );

                validate_tokens({tokens[3]}, {'.'}, "bkgcolor eta ");
                bkg_eta = custom_stof(tokens[3]);

                bkg_or_cueing_present = true;
            }
            else if (keyword == "mtlcolor")
            {
                end_condition(
                    tokens.size() != 14 && tokens.size() != 12,
                    "Incorrect number of tokens to 'mtlcolor'\n"
                );

                if (tokens.size() == 14)
                {
                    vector<string> coefficients = v_slice(tokens, 6, 13, 1);

                    Color diffuse = extract_only_color(
                        keyword, v_slice(tokens, 0, 2, 1)
                    );

                    Color specular = extract_only_color(
                        keyword, v_slice(tokens, 3, 5, 1)
                    );

                    Color alpha = extract_only_color(
                        keyword, v_slice(tokens, 10, 12, 1)
                    );

                    validate_tokens(coefficients, {'.'}, "mtlcolor coeffs ");
                    
                    Material* m = new Material(
                        diffuse,
                        specular, 
                        custom_stof(coefficients[0]), /* ka */
                        custom_stof(coefficients[1]), /* kd */
                        custom_stof(coefficients[2]), /* ks */
                        custom_stof(coefficients[3]), /* n */
                        alpha.r,
                        alpha.g, 
                        alpha.b,
                        true,
                        custom_stof(coefficients[7]) /* index of refraction (eta) */
                    );

                    materials.push_back(m);
                    material_applied = true;
                }
                else 
                {
                    vector<string> coefficients = v_slice(tokens, 6, 11, 1);

                    Color diffuse = extract_only_color(
                        keyword, v_slice(tokens, 0, 2, 1)
                    );

                    Color specular = extract_only_color(
                        keyword, v_slice(tokens, 3, 5, 1)
                    );

                    validate_tokens(coefficients, {'.'}, "mtlcolor coeffs ");
                    
                    Material* m = new Material(
                        diffuse,
                        specular, 
                        custom_stof(coefficients[0]), /* ka */
                        custom_stof(coefficients[1]), /* kd */
                        custom_stof(coefficients[2]), /* ks */
                        custom_stof(coefficients[3]), /* n */
                        custom_stof(coefficients[4]),
                        0, 0, false,
                        custom_stof(coefficients[5]) /* index of refraction (eta) */
                    );

                    materials.push_back(m);
                    material_applied = true;
                }

                triangle_defs.push_back(vector<vec3*>());
                vertex_normal_defs.push_back(vector<vec3*>());
                vertex_texture_coord_defs.push_back(vector<vec3*>());
                texture_defs.push_back(vector<Texture*>());
                normal_defs.push_back(vector<NormalMap*>());
            }
            else if (keyword == "projection")
            {
                validate_size(tokens.size(), 1, keyword);

                end_condition(
                    tokens[0] != "parallel",
                    string("Token following keyword ") + 
                        "'projection' must be 'parallel'.\n"
                );

                parallel = true;
            }
            else if (keyword == "threadcount")
            {
                validate_size(tokens.size(), 1, keyword);

                validate_tokens(tokens, {}, "threadcount ");

                threads = custom_stoi(tokens[0]);
                end_condition(
                    out_width < 1,
                    "threadcount must be greater than 0\n"
                );
            }
            else if (keyword == "light")
            {
                validate_size(tokens.size(), 7, keyword);

                string type_light = tokens[3];
                
                vec3 pos = extract_only_vector(
                    keyword, v_slice(tokens, 0, 2, 1)
                );
                Color col = extract_only_color(
                    keyword, v_slice(tokens, 4, 6, 1)
                );

                end_condition(
                    !is_number(type_light, {}),
                    "light w must be 1 (point) or 0 (directional).\n"
                );

                int w = custom_stoi(type_light);

                if (w == 0)
                {
                    end_condition(
                        pos == zeros,
                        "directional light direction must be non-zero\n"
                    );

                    lights.push_back(
                        new DirectionalLight(
                            pos,
                            col
                        )
                    );
                    continue;
                }
                
                if (w == 1)
                {
                    lights.push_back(
                        new PointLight(
                            pos,
                            col
                        )
                    );
                    continue;
                }

                end_condition(
                    1, 
                    "light w must be 1 (point) or 0 (directional).\n"
                );
            }
            else if (keyword == "attlight")
            {
                validate_size(tokens.size(), 10, keyword);

                string type_light = tokens[3];
                
                vec3 pos = extract_only_vector(
                    keyword, v_slice(tokens, 0, 2, 1)
                );
                Color col = extract_only_color(
                    keyword, v_slice(tokens, 4, 6, 1)
                );
                vec3 falloff = extract_only_vector(
                    keyword, v_slice(tokens, 7, 9, 1)
                );

                end_condition(
                    falloff.x < 0 || falloff.y < 0 || falloff.z < 0,
                    "falloff constants must be positive or zero.\n"
                );   

                end_condition(
                    !is_number(type_light, {}),
                    "attlight w must be 1 (point).\n"
                );

                int w = custom_stoi(type_light);

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
                    continue;
                }
                
                end_condition(1, "attlight must be point light (0).\n");
            }
            else if (keyword == "depthcueing")
            {
                validate_size(tokens.size(), 7, keyword);

                vector<string> coefs = v_slice(tokens, 3, 6, 1);

                cueingcolor = extract_only_color(
                    keyword, v_slice(tokens, 0, 2, 1)
                );

                validate_tokens(coefs, {'.'}, "depthcueing coefs ");

                amax = custom_stof(coefs[0]);
                amin = custom_stof(coefs[1]);
                dmax = custom_stof(coefs[2]);
                dmin = custom_stof(coefs[3]);
                
                bkg_or_cueing_present = true;
                cueing = true;
            }
            else if (keyword == "v")
            {
                validate_size(tokens.size(), 3, keyword);

                vertices.push_back(
                    extract_only_vector(keyword, tokens)
                );
            }
            else if (keyword == "vn")
            {
                validate_size(tokens.size(), 3, keyword);

                vertex_normals.push_back(
                    extract_only_vector(keyword, tokens)
                );
            }
            else if (keyword == "vt")
            {
                validate_size(tokens.size(), 2, keyword);

                tokens.push_back("0");

                vertex_texture_coords.push_back(
                    extract_only_vector(keyword, tokens)
                );
            }
            else if (keyword == "texture")
            {
                validate_size(tokens.size(), 1, keyword);
                
                textures.push_back(read_ppm_to_texture(tokens[0]));

                texture_applied = true;
            }
            else if (keyword == "bump")
            {
                validate_size(tokens.size(), 1, keyword);
                
                normals.push_back(read_ppm_to_normal(tokens[0]));
                normal_map_applied = true;
            }
            else if (keyword == "mesh")
            {
                end_condition(
                    tokens.size() != 2 && tokens.size() != 1,
                    "Incorrect token amount for keyword 'mesh'.\n"
                );

                if (tokens[0] == "start")
                {
                    end_condition(
                        meshing,
                        "cannot start mesh before stopping currently applied mesh\n"
                    );

                    meshing = true;
                    mesh_face_indices.push_back(vector<int>());

                    int p = 0;
                    if (tokens.size() == 2)
                        p = custom_stoi(tokens[1]);
                    mesh_levels.push_back(p);
                }
                else if (tokens[0] == "stop")
                {
                    end_condition(
                        !meshing,
                        "cannot stop mesh when no mesh in progress\n"
                    );

                    meshing = false;
                }
                else 
                {
                    end_condition(
                        1, 
                        string("value") + 
                        "following keyword " +
                        "'mesh' must be 'stop' or 'start'\n"
                    );
                }
            }
            else
            {
                end_condition(
                    keyword.length() < 2 ||
                    (keyword[0] != '/' || keyword[1] != '/'),
                    string("Invalid Keyword: '") + keyword + "'\n"
                );
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
    int first_triangle_index = objects.size();
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
            end_condition(
                v1 <= 0 || v1 > vcnt || v2 <= 0 || v2 > vcnt
                || v3 <= 0 || v3 > vcnt,
                "Invalid vertex index specified\n"
            );

            if (vertex_normal_defs[i][j] != nullptr)
            {
                int vn1 = (int)vertex_normal_defs[i][j]->x;
                int vn2 = (int)vertex_normal_defs[i][j]->y;
                int vn3 = (int)vertex_normal_defs[i][j]->z;

                int vncnt = vertex_normals.size();

                end_condition(
                    vn1 <= 0 || vn1 > vncnt || 
                    vn2 <= 0 || vn2 > vncnt ||
                    vn3 <= 0 || vn3 > vncnt,
                    "Invalid vertex normal index specified\n"
                );

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

                end_condition(
                    vt1 <= 0 || vt1 > vtcnt ||
                    vt2 <= 0 || vt2 > vtcnt ||
                    vt3 <= 0 || vt3 > vtcnt,
                    "Invalid vertex texture index specified\n"
                );

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

    cout << "piss" << endl;

    vector<Object*> polys;
    for (int i = mesh_face_indices.size()-1; i >= 0; i--)
    {
        for (int j = mesh_face_indices[i].size()-1; j >= 0; j--)
        {   
            int t_offset = mesh_face_indices[i][j];

            polys.push_back(objects[first_triangle_index + t_offset]);
            objects.erase(objects.begin() + first_triangle_index + t_offset);
        }

        objects.push_back(new Mesh(polys, mesh_levels[i]));
        polys.clear();
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

    RayTracer r(
        eye_pos,
        view_dir,
        up_dir,
        hfov,
        out_width,
        out_height,
        bkgcolor,
        bkg_eta,
        &objects,
        &lights,
        parallel,
        cueing,
        amin,
        amax,
        dmin,
        dmax,
        cueingcolor,
        threads
    );

    /* actually run the raytracer */

    vector<Color> pixels;
    int tests = 20;
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
    outf << create_ppm_header("P3", out_width, out_height, 255);

    put_all_normalized(outf, pixels, 255);

    /* clean up dynamicall allocated things */

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