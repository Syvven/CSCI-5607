// This template code was originally written by Matt Overby while a TA for CSci5607

// The loaders are included by glfw3 (glcorearb.h) if we are not using glew.
#include "glad/glad.h"
#include "GLFW/glfw3.h"

// Includes
#include "trimesh.hpp"
#include "shader.hpp"
#include <cstring> // memcpy

#include <iostream>
#include <math.h>
#include <iomanip>
#include <sstream>

// Constants
#define WIN_WIDTH 1080
#define WIN_HEIGHT 1080
#define PI 3.14159265358979323846f  /* pi */

class Mat4x4 {
public:

	float m[16];

	Mat4x4(){ // Default: Identity
		m[0] = 1.f;  m[4] = 0.f;  m[8]  = 0.f;  m[12] = 0.f;
		m[1] = 0.f;  m[5] = 1.f;  m[9]  = 0.f;  m[13] = 0.f;
		m[2] = 0.f;  m[6] = 0.f;  m[10] = 1.f;  m[14] = 0.f;
		m[3] = 0.f;  m[7] = 0.f;  m[11] = 0.f;  m[15] = 1.f;
	}

	void make_identity(){
		m[0] = 1.f;  m[4] = 0.f;  m[8]  = 0.f;  m[12] = 0.f;
		m[1] = 0.f;  m[5] = 1.f;  m[9]  = 0.f;  m[13] = 0.f;
		m[2] = 0.f;  m[6] = 0.f;  m[10] = 1.f;  m[14] = 0.f;
		m[3] = 0.f;  m[7] = 0.f;  m[11] = 0.f;  m[15] = 1.f;
	}

	void print(){
		printf(
			"| % 6.2f % 6.2f % 6.2f % 6.2f |\n| % 6.2f % 6.2f % 6.2f % 6.2f |\n| % 6.2f % 6.2f % 6.2f % 6.2f |\n| % 6.2f % 6.2f % 6.2f % 6.2f |\n",
			m[0], m[1], m[2], m[3],
			m[4], m[5], m[6], m[7],
			m[8], m[9], m[10], m[11],
			m[12], m[13], m[14], m[15]
		);
	}

	void make_scale(float x, float y, float z){
		make_identity();
		m[0] = x; m[5] = y; m[10] = z;
	}
};

static inline const Vec3f operator*(const Mat4x4 &m, const Vec3f &v){
	Vec3f r( m.m[0]*v[0]+m.m[4]*v[1]+m.m[8]*v[2],
		m.m[1]*v[0]+m.m[5]*v[1]+m.m[9]*v[2],
		m.m[2]*v[0]+m.m[6]*v[1]+m.m[10]*v[2] );
	return r;
}


class Cam3d {
public:
	/* camera state variables */
	Mat4x4 view, projection, rotation;

	Vec3f n, u, v, d, eye, up, forward, 
		  negMove, posMove, negTurn, /* x = theta, y = phi */ posTurn,
		  up_dir, right_dir, velocity, init_up;

	float near, far, left, right, top, bottom, 
		  yaw, pitch, roll, boost_speed, 
		  move_speed, turn_speed, dt;

	bool moving, boosting;
	/* easier than having a bunch of ands */
	int moving_flags   = 0b000000,
		rotating_flags = 0b0000;

	Cam3d() 
	{
		float near = 0.0001f; float far = 1000.f;
		init_params(
			Vec3f(
				/*min[0] + (max[0] - min[0])*0.5f,
				min[1] + 3.0f,
				min[2] + (max[2] - min[2])*0.5f*/
				0, 0, 0
			), /* eye */
			Vec3f(0.f, 0.f, -1.f), /* dir */
			Vec3f(0.f, 1.f, 0.f), /* up */
			near, far, /* near, far */
			-near * 0.5, near * 0.5, near * 0.5, -near * 0.5, /* left, right, top, bottom */
			0.05, 0.05, 0.05 /* speed in X Y Z */
		);
	}

	Cam3d(Vec3f& eye_, Vec3f& dir_, Vec3f& up_,
			float near, float far, 
			float left, float right, float top, float bottom,
			float s_speed, float a_speed, float g_speed) 
	{
		init_params(
			eye_, dir_, up_, near, far, 
			left, right, top, bottom, s_speed, 
			a_speed, g_speed
		);
	}

	void init_params(Vec3f& eye_, Vec3f& dir_, Vec3f& up_,
		float near, float far,
		float left, float right, float top, float bottom,
		float s_speed, float a_speed, float g_speed)
	{
		/* movement state variables */
		moving = boosting = false;

		/* assume 60fps */
		dt = 1.f/60.f;

		turn_speed = 1.f;
		move_speed = 2.f;
		boost_speed = 2.5f;

		/* define the frustrum and projection matrix */
		this->near = near;   this->far = far;
		this->top = top;     this->bottom = bottom;
		this->right = right; this->left = left;

		/* define the viewing matrix*/
		init_up = up_; forward = dir_; up = up_; eye = eye_;
		update_n(); update_u(); update_v(); update_d();

		pitch = asin(-forward[1]);
		yaw = asin(forward[0] / cos(pitch));

		setup_view(); setup_projection();
	}

	void adjust_perspective(float height, float width)
	{
		/* personal heuristic */
		float mod_h = height / std::min(height, width);
		float mod_w = width / std::min(height, width);

		float halfn = near * 0.5;

		left = halfn * -mod_w; right = halfn * mod_w;
		top = halfn * mod_h; bottom = halfn * -mod_h;

		setup_projection();
	}

	void setup_projection()
	{
		/* transposed from in slides */
		float ttn = 2 * near; 
		float tmb = top - bottom;
		projection.m[0]  = ttn / (right - left);            projection.m[5] = ttn / tmb; 
		projection.m[2]  = (right + left) / (right - left); projection.m[6] = (top + bottom) / tmb;
		projection.m[10] = -(far + near) / (far - near);    projection.m[14] = -1; 
		projection.m[11] = -(ttn*far) / (far-near);         projection.m[15] = 0.f;
	}

	void update_n()
	{ n = forward; n *= -1.f; n.normalize(); }

	void update_u()
	{ u = up.cross(n); u.normalize(); right_dir = u; }

	void update_v()
	{ v = n.cross(u); v.normalize(); }

	void update_d()
	{
		d[0] = -(eye.dot(u));
		d[1] = -(eye.dot(v));
		d[2] = -(eye.dot(n));
	}

	void setup_view()
	{
		view.m[0] = u[0]; view.m[4] = v[0]; view.m[8]  = n[0];
		view.m[1] = u[1]; view.m[5] = v[1]; view.m[9]  = n[1];
		view.m[2] = u[2]; view.m[6] = v[2]; view.m[10] = n[2];
		view.m[3] = d[0]; view.m[7] = d[1]; view.m[11] = d[2];
	}

	void update()
	{ 
		/* 
			uses euler angles, gimbal lock possible 
			I wanted to try quaternions but couldn't get them working
			unfortunte!
		*/
		if (rotating_flags)
		{
			float mod = turn_speed * dt;
			yaw += mod * (negTurn[0] + posTurn[0]);
			pitch += mod * (negTurn[1] + posTurn[1]);

			/* yaw pitch roll */
			float yup = yaw - PI * 0.5f;
			float cospitch = cos(pitch);

			forward = Vec3f(
				cospitch * sin(yaw),
				-sin(pitch),
				cospitch * cos(yaw)
			);

			right_dir = Vec3f(sin(yup), 0.0f, cos(yup));
			up = right_dir.cross(forward);

			update_n();
			update_u();
			update_v();
		}
			
		if (moving_flags)
		{
			negMove.normalize(); posMove.normalize();

			velocity = posMove;
			velocity += negMove;
			velocity *= (move_speed * dt * (2 * boosting + !boosting));

			eye += right_dir * velocity[0]; 
			eye[1] += velocity[1];
			eye += forward * velocity[2];
		}

		if (moving_flags || rotating_flags)
		{
			update_d();
			setup_view();
		}
	}
};

typedef struct GLmodel
{
	GLuint verts_vbo[1], colors_vbo[1], 
		normals_vbo[1], faces_ibo[1], tris_vao;
	std::vector<Mat4x4> model_mats;
	std::vector<Vec3f> scales, translates;
	std::vector<Vec3f> rotations;
	int model_count = 1;
	TriMesh mesh;
} GLmodel;

//
//	Global state variables
//
namespace Globals {
	double cursorX, cursorY; // cursor positions
	float win_width, win_height, aspect; /* window size */
	GLmodel church, secret_kiwi; /* models for church and kiwi */
	Cam3d camera; /* camera class */
	bool kiwi_available;
}

//
//	Callbacks
//
static void error_callback(int error, const char* description){ fprintf(stderr, "Error: %s\n", description); }

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	// Close on escape or Q
	if( action == GLFW_PRESS ){
		switch (key) {
			case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GL_TRUE); break;
			case GLFW_KEY_Q:      glfwSetWindowShouldClose(window, GL_TRUE); break;
			case GLFW_KEY_W: {
				Globals::camera.posMove[2] = 1.f;
				Globals::camera.moving_flags |= 0b000001;
				break;
			}
			case GLFW_KEY_S: {
				Globals::camera.negMove[2] = -1.f;
				Globals::camera.moving_flags |= 0b000010;
				break;
			}
			case GLFW_KEY_A: {
				Globals::camera.negMove[0] = -1.f;
				Globals::camera.moving_flags |= 0b000100;
				break;
			}
			case GLFW_KEY_D: {
				Globals::camera.posMove[0] = 1.f;
				Globals::camera.moving_flags |= 0b001000;
				break;
			}
			case GLFW_KEY_LEFT_CONTROL: {
				Globals::camera.negMove[1] = -1.f;
				Globals::camera.moving_flags |= 0b010000;
				break;
			}
			case GLFW_KEY_SPACE: {
				Globals::camera.posMove[1] = 1.f;
				Globals::camera.moving_flags |= 0b100000;
				break;
			}
			case GLFW_KEY_RIGHT:
			{
				Globals::camera.posTurn[0] = -1.f;
				Globals::camera.rotating_flags |= 0b0001;
				break;
			}
			case GLFW_KEY_LEFT:
			{
				Globals::camera.negTurn[0] = 1.f;
				Globals::camera.rotating_flags |= 0b0010;
				break;
			}
			case GLFW_KEY_UP:
			{
				Globals::camera.posTurn[1] = -1.f;
				Globals::camera.rotating_flags |= 0b0100;
				break;
			}
			case GLFW_KEY_DOWN:
			{
				Globals::camera.negTurn[1] = 1.f;
				Globals::camera.rotating_flags |= 0b1000;
				break;
			}
			case GLFW_KEY_LEFT_SHIFT:
			{
				Globals::camera.boosting = true;
				break;
			}
		}
	}
	if (action == GLFW_RELEASE)
	{
		switch (key) {
			case GLFW_KEY_W: {
				Globals::camera.posMove[2] = 0.f;
				Globals::camera.moving_flags ^= 0b000001;
				break;
			}
			case GLFW_KEY_S: {
				Globals::camera.negMove[2] = 0.f;
				Globals::camera.moving_flags ^= 0b000010;
				break;
			}
			case GLFW_KEY_A: {
				Globals::camera.negMove[0] = 0.f;
				Globals::camera.moving_flags ^= 0b000100;
				break;
			}
			case GLFW_KEY_D: {
				Globals::camera.posMove[0] = 0.f;
				Globals::camera.moving_flags ^= 0b001000;
				break;
			}
			case GLFW_KEY_LEFT_CONTROL: {
				Globals::camera.negMove[1] = 0.f;
				Globals::camera.moving_flags ^= 0b010000;
				break;
			}
			case GLFW_KEY_SPACE: {
				Globals::camera.posMove[1] = 0.f;
				Globals::camera.moving_flags ^= 0b100000;
				break;
			}
			case GLFW_KEY_RIGHT:
			{
				Globals::camera.posTurn[0] = 0.f;
				Globals::camera.rotating_flags ^= 0b0001;
				break;
			}
			case GLFW_KEY_LEFT:
			{
				Globals::camera.negTurn[0] = 0.f;
				Globals::camera.rotating_flags ^= 0b0010;
				break;
			}
			case GLFW_KEY_UP:
			{
				Globals::camera.posTurn[1] = 0.f;
				Globals::camera.rotating_flags ^= 0b0100;
				break;
			}
			case GLFW_KEY_DOWN:
			{
				Globals::camera.negTurn[1] = 0.f;
				Globals::camera.rotating_flags ^= 0b1000;
				break;
			}
			case GLFW_KEY_LEFT_SHIFT:
			{
				Globals::camera.boosting = false;
				break;
			}
		}
	}
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height){
	Globals::win_width = float(width);
	Globals::win_height = float(height);
    Globals::aspect = Globals::win_width/Globals::win_height;
	
    glViewport(0,0,width,height);

	// ToDo: update the perspective matrix as the window size changes
	Globals::camera.adjust_perspective(Globals::win_height, Globals::win_width);
}

// Function to set up geometry
void init_scene();

static void set_matrix(GLmodel& mod, int model)
{
	Mat4x4* m = &mod.model_mats[model];
	Vec3f* r = &mod.rotations[model];
	Vec3f* t = &mod.translates[model];

	float crz = cos((*r)[2]);
	float srz = sin((*r)[2]);

	float cry = cos((*r)[1]);
	float sry = sin((*r)[1]);

	float crx = cos((*r)[0]);
	float srx = sin((*r)[0]);

	m->m[0] = crz * cry - srz * srx * sry;
	m->m[1] = -srz * crx;
	m->m[2] = crz * sry + srz * srx * cry;

	m->m[4] = srz * cry + crz * srx * sry;
	m->m[5] = crz * crx;
	m->m[6] = srz * sry - crz * srx * cry;

	m->m[8] = -crx * sry;
	m->m[9] = srx;
	m->m[10] = crx * cry;

	m->m[3] = (*t)[0]; m->m[7] = (*t)[1]; m->m[11] = (*t)[2];
}

void update()
{ 
	Globals::camera.update(); 

	if (Globals::kiwi_available)
	{
		for (int i = 0; i < Globals::secret_kiwi.model_count; i++)
		{
			Globals::secret_kiwi.rotations[i][1] += 0.01;
			set_matrix(Globals::secret_kiwi, i);
		}
	}
}

//
//	Main
//
int main(int argc, char *argv[]){

	// Load the mesh
	std::stringstream obj_file; obj_file << MY_DATA_DIR << "sibenik/sibenik.obj";
	std::stringstream kiwi_file; kiwi_file << MY_DATA_DIR << "biwer/kiwi1.obj";

	if( !Globals::church.mesh.load_obj(obj_file.str())){ return EXIT_FAILURE; }
	Globals::church.mesh.print_details();

	Globals::kiwi_available = true;
	if (!Globals::secret_kiwi.mesh.load_obj(kiwi_file.str())) { 
		Globals::kiwi_available = false; 
		std::cout << "The church is safe..." << std::endl;
	}
	else
	{
		std::cout << "The exits are blocked by kiwis!!" << std::endl;
	}
	if (Globals::kiwi_available) Globals::secret_kiwi.mesh.print_details();

	// Forcibly scale the mesh vertices so that the entire model fits within a (-1,1) volume: the code below is a temporary measure that is needed to enable the entire model to be visible in the template app, before the student has defined the proper viewing and projection matrices
    	// This code should eventually be replaced by the use of an appropriate projection matrix
    	// FYI: the model dimensions are: center = (0,0,0); height: 30.6; length: 40.3; width: 17.0
    // find the extremum of the vertex locations (this approach works because the model is known to be centered; a more complicated method would be required in the general case)
    Vec3f min, max, scale;
    min = Globals::church.mesh.vertices[0]; max = Globals::church.mesh.vertices[0];
	for( int i=0; i<Globals::church.mesh.vertices.size(); ++i ){
           if (Globals::church.mesh.vertices[i][0] < min[0]) min[0] = Globals::church.mesh.vertices[i][0];
           else if (Globals::church.mesh.vertices[i][0] > max[0]) max[0] = Globals::church.mesh.vertices[i][0];
           if (Globals::church.mesh.vertices[i][1] < min[1]) min[1] = Globals::church.mesh.vertices[i][1];
           else if (Globals::church.mesh.vertices[i][1] > max[1]) max[1] = Globals::church.mesh.vertices[i][1];
           if (Globals::church.mesh.vertices[i][2] < min[2]) min[2] = Globals::church.mesh.vertices[i][2];
           else if (Globals::church.mesh.vertices[i][2] > max[2]) max[2] = Globals::church.mesh.vertices[i][2];
    }
    // work with positive numbers
    /*if (min < 0) min = -min;*/
    // scale so that the component that is most different from 0 is mapped to 1 (or -1); all other values will then by definition fall between -1 and 1
    /*if (max > min) scale = 1/max; else scale = 1/min;*/
    	
	// scale the model vertices by brute force
    /*Mat4x4 mscale; mscale.make_scale( scale, scale, scale );
	for( int i=0; i<Globals::mesh.vertices.size(); ++i ){
           Globals::mesh.vertices[i] = mscale*Globals::mesh.vertices[i];
    }*/
    // The above can be removed once a proper projection matrix is defined

	float near = 0.0001f; float far = 1000.f;
	Globals::camera = Cam3d(
		Vec3f(
			min[0] + (max[0] - min[0])*0.5f,
			min[1] + 2.5f,
			min[2] + (max[2] - min[2])*0.5f
		), /* eye */
		Vec3f(1.f, 0.f, 0.f), /* dir */
		Vec3f(0.f, 1.f, 0.f), /* up */
		near, far, /* near, far */
		-near * 0.5, near * 0.5, near * 0.5, -near * 0.5, /* left, right, top, bottom */
		0.05, 0.05, 0.05 /* speed in X Y Z */
	);

	/* define the camera initial view params */

	// Set up the window variable
	GLFWwindow* window;
    
    // Define the error callback function
	glfwSetErrorCallback(&error_callback);

	// Initialize glfw
	if( !glfwInit() ){ return EXIT_FAILURE; }

	// Ask for OpenGL 3.3
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Create the glfw window
	Globals::win_width = WIN_WIDTH;
	Globals::win_height = WIN_HEIGHT;
	window = glfwCreateWindow(int(Globals::win_width), int(Globals::win_height), "HW2b", NULL, NULL);
	if( !window ){ glfwTerminate(); return EXIT_FAILURE; }

	// Define callbacks to handle user input and window resizing
	glfwSetKeyCallback(window, &key_callback);
	glfwSetFramebufferSizeCallback(window, &framebuffer_size_callback);

	// More setup stuff
	glfwMakeContextCurrent(window); // Make the window current
    glfwSwapInterval(1); // Set the swap interval

	// make sure the openGL and GLFW code can be found
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		std::cout << "Failed to gladLoadGLLoader" << std::endl;
		glfwTerminate();
		return EXIT_FAILURE;
	}

	// Initialize the shaders
	// MY_SRC_DIR was defined in CMakeLists.txt
	// it specifies the full path to this project's src/ directory.
	mcl::Shader shader;
	std::stringstream ss; ss << MY_SRC_DIR << "shader.";
	shader.init_from_files( ss.str()+"vert", ss.str()+"frag" );

	// Initialize the scene
	init_scene();


	if (Globals::kiwi_available)
	{
		Globals::secret_kiwi.translates.push_back(Vec3f(
			min[0] + 3.f,
			min[1] + 0.8f,
			min[2] + (max[2] - min[2]) * 0.5f
		));

		Globals::secret_kiwi.translates.push_back(Vec3f(
			min[0] + 3.f,
			min[1] + 0.8f,
			min[2] + (max[2] - min[2]) * 0.5f + 2.f
		));

		Globals::secret_kiwi.translates.push_back(Vec3f(
			min[0] + 3.f,
			min[1] + 0.8f,
			min[2] + (max[2] - min[2]) * 0.5f - 2.f
		));

		/* 10 -12 -2 */

		Globals::secret_kiwi.translates.push_back(Vec3f(10.5, -13, -2));

		Globals::secret_kiwi.translates.push_back(Vec3f(10.5, -13, 2));

		set_matrix(Globals::secret_kiwi, 0);
	}

	framebuffer_size_callback(window, int(Globals::win_width), int(Globals::win_height)); 

	// Perform some OpenGL initializations
	glEnable(GL_DEPTH_TEST);  // turn hidden surfce removal on
	glClearColor(1.f,1.f,1.f,1.f);  // set the background to white

	// Enable the shader, this allows us to set uniforms and attributes
	shader.enable();

	// Bind buffers

	// Game loop
	while(!glfwWindowShouldClose(window)){

		// Clear the color and depth buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		update();

		// Send updated info to the GPU
		glUniformMatrix4fv(shader.uniform("projection"), 1, GL_TRUE, Globals::camera.projection.m); // projection matrix
		glUniformMatrix4fv(shader.uniform("view"), 1, GL_TRUE, Globals::camera.view.m); // viewing transformation

		/* draw church */
		glBindVertexArray(Globals::church.tris_vao);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Globals::church.faces_ibo[0]);
		glUniformMatrix4fv(shader.uniform("model"), 1, GL_FALSE, Globals::church.model_mats[0].m); // model transformation (always the identity matrix in this assignment)
		glDrawElements(GL_TRIANGLES, Globals::church.mesh.faces.size()*3, GL_UNSIGNED_INT, 0);

		/* draw kiwi */
		if (Globals::kiwi_available)
		{
			glBindVertexArray(Globals::secret_kiwi.tris_vao);
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Globals::secret_kiwi.faces_ibo[0]);
			for (int i = 0; i < Globals::secret_kiwi.model_count; i++)
			{
				glUniformMatrix4fv(shader.uniform("model"), 1, GL_TRUE, Globals::secret_kiwi.model_mats[i].m); // model transformation (always the identity matrix in this assignment)
				glDrawElements(GL_TRIANGLES, Globals::secret_kiwi.mesh.faces.size() * 3, GL_UNSIGNED_INT, 0);
			}
		}
		

		// Finalize
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // end game loop

	// Unbind
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindVertexArray(0);

	// Disable the shader, we're done using it
	shader.disable();
    
	return EXIT_SUCCESS;
}

void init_buffers(GLmodel& model)
{
	int vert_dim = 3;

	// Create the buffer for vertices
	glGenBuffers(1, model.verts_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, model.verts_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, model.mesh.vertices.size()*sizeof(model.mesh.vertices[0]), &model.mesh.vertices[0][0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create the buffer for colors
	glGenBuffers(1, model.colors_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, model.colors_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, model.mesh.colors.size()*sizeof(model.mesh.colors[0]), &model.mesh.colors[0][0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create the buffer for normals
	glGenBuffers(1, model.normals_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, model.normals_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, model.mesh.normals.size()*sizeof(model.mesh.normals[0]), &model.mesh.normals[0][0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create the buffer for indices
	glGenBuffers(1, model.faces_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, model.faces_ibo[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, model.mesh.faces.size()*sizeof(model.mesh.faces[0]), &model.mesh.faces[0][0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Create the VAO
	glGenVertexArrays(1, &model.tris_vao);
	glBindVertexArray(model.tris_vao);

	// location=0 is the vertex
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, model.verts_vbo[0]);
	glVertexAttribPointer(0, vert_dim, GL_FLOAT, GL_FALSE, sizeof(model.mesh.vertices[0]), 0);

	// location=1 is the color
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, model.colors_vbo[0]);
	glVertexAttribPointer(1, vert_dim, GL_FLOAT, GL_FALSE, sizeof(model.mesh.colors[0]), 0);

	// location=2 is the normal
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, model.normals_vbo[0]);
	glVertexAttribPointer(2, vert_dim, GL_FLOAT, GL_FALSE, sizeof(model.mesh.normals[0]), 0);

	// Done setting data for the vao
	glBindVertexArray(0);
}

void init_scene(){
	init_buffers(Globals::church);
	Globals::church.model_mats.push_back(Mat4x4());

	if (Globals::kiwi_available)
	{
		init_buffers(Globals::secret_kiwi);
		Globals::secret_kiwi.model_count = 5;
		for (int i = 0; i < Globals::secret_kiwi.model_count; i++)
		{
			Globals::secret_kiwi.model_mats.push_back(Mat4x4());
			Globals::secret_kiwi.rotations.push_back(Vec3f(0.f, 0.f, PI));
		}
	}
}

