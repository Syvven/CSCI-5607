// This template code was originally written by Matt Overby while a TA for CSci5607

// The loaders are included by glfw3 (glcorearb.h) if we are not using glew.
#include "glad/glad.h"
#include "GLFW/glfw3.h"

// Includes
#include "trimesh.hpp"
#include "shader.hpp"
#include <cstring> // memcpy

// Constants
#define WIN_WIDTH 1080
#define WIN_HEIGHT 1080

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
		std::cout << m[0] << ' ' <<  m[4] << ' ' <<  m[8]  << ' ' <<  m[12] << "\n";
		std::cout << m[1] << ' ' <<   m[5] << ' ' <<  m[9]  << ' ' <<   m[13] << "\n";
		std::cout << m[2] << ' ' <<   m[6] << ' ' <<  m[10] << ' ' <<   m[14] << "\n";
		std::cout << m[3] << ' ' <<   m[7] << ' ' <<  m[11] << ' ' <<   m[15] << "\n";
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
	Mat4x4 view, projection;
	Vec3f n, u, v, d, eye;
	float near, far, left, right, top, bottom;

	/* camera state variables */
	bool ascending, strafing, going; /* Y, X, Z */
	float ascend_speed, strafe_speed, go_speed;
	float ascend_dir, strafe_dir, go_dir;

	Cam3d() { near = far = left = right = top = bottom = 0; }

	Cam3d(Vec3f& eye_, Vec3f& dir, Vec3f& up,
			float near, float far, 
			float left, float right, float top, float bottom,
			float s_speed, float a_speed, float g_speed) {

		/* movement state variables */
		ascending = strafing = going = false;
		ascend_dir = strafe_dir = go_dir = 0;
		ascend_speed = a_speed;
		strafe_speed = s_speed;
		go_speed     = g_speed;
		
		/* define the frustrum and projection matrix */
		this->near  = near;  this->far    = far;
		this->top   = top;   this->bottom = bottom;
		this->right = right; this->left   = left;

		projection.m[0] = (2 * near) / (right - left);   projection.m[12] = -near*(right + left) / (right - left);
		projection.m[5] = (2 * near) / (top - bottom);   projection.m[13] = -near*(top + bottom) / (top   - bottom);
		projection.m[10] = -(far + near) / (far - near); projection.m[14] = (2 * far * near)     / (near  - far);
		projection.m[11] = -1;

		/* define the viewing matrix*/

		eye = eye_;
		n = dir; n *= -1.f; n.normalize();
		u = up.cross(n); u.normalize();
		v = n.cross(u); v.normalize();

		d = Vec3f(
			-(eye.dot(u)),
			-(eye.dot(v)),
			-(eye.dot(n))
		);

		this->view.m[0] = u[0];  this->view.m[4] = u[1]; this->view.m[8] = u[2];
		this->view.m[12] = d[0];

		this->view.m[1] = v[0];  this->view.m[5] = v[1]; this->view.m[9] = v[2];
		this->view.m[13] = d[1];

		this->view.m[2] = n[0]; this->view.m[6] = n[1]; this->view.m[10] = n[2];
		this->view.m[14] = d[2];

		this->view.m[15] = 1;
	}

	void update() { update_eye(); }

	void update_eye()
	{
		float sum = abs(strafe_dir) + abs(ascend_dir) + abs(go_dir) + 9e-5;

		this->eye += this->n * (
			-1 * go_dir * go_speed / sum
		);
		this->eye += this->u * (
			-1 * strafe_dir * strafe_speed / sum
		);
		this->eye += this->v * (
			ascend_dir * ascend_speed / sum
		);

		d = Vec3f(
			-(eye.dot(u)),
			-(eye.dot(v)),
			-(eye.dot(n))
		);

		this->view.m[12] = d[0];
		this->view.m[13] = d[1];
		this->view.m[14] = d[2];
	}
	
	/*void update_dir();
	void update_up();*/
};

//
//	Global state variables
//
namespace Globals {
	double cursorX, cursorY; // cursor positions
	float win_width, win_height; // window size
	float aspect;
	GLuint verts_vbo[1], colors_vbo[1], normals_vbo[1], faces_ibo[1], tris_vao;
	TriMesh mesh;

	//  Model, view and projection matrices, initialized to the identity
	Mat4x4 model; // not used in this assignment; included for completeness only

	Cam3d camera;
}


//
//	Callbacks
//
static void error_callback(int error, const char* description){ fprintf(stderr, "Error: %s\n", description); }

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
	// Close on escape or Q
	if( action == GLFW_PRESS ){
		switch ( key ) {
			case GLFW_KEY_ESCAPE: glfwSetWindowShouldClose(window, GL_TRUE); break;
			case GLFW_KEY_Q:      glfwSetWindowShouldClose(window, GL_TRUE); break;
			case GLFW_KEY_W: {
				Globals::camera.go_dir += 1.f;
				break;
			}
			case GLFW_KEY_S: {
				Globals::camera.go_dir += -1.f;
				break;
			}
			case GLFW_KEY_A: {
				Globals::camera.strafe_dir += 1.f;
				break;
			}
			case GLFW_KEY_D: {
				Globals::camera.strafe_dir += -1.f;
				break;
			}
			case GLFW_KEY_LEFT_BRACKET: {
				Globals::camera.ascend_dir += -1.f;
				break;
			}
			case GLFW_KEY_RIGHT_BRACKET: {
				Globals::camera.ascend_dir += 1.f;
				break;
			}
			case GLFW_KEY_RIGHT
			{

			}
		}
	}
	if (action == GLFW_RELEASE)
	{
		switch (key) {
			case GLFW_KEY_W: {
				Globals::camera.go_dir += -1.f;
				break;
			}
			case GLFW_KEY_S: {
				Globals::camera.go_dir += 1.f;
				break;
			}
			case GLFW_KEY_A: {
				Globals::camera.strafe_dir += -1.f;
				break;
			}
			case GLFW_KEY_D: {
				Globals::camera.strafe_dir += 1.f;
				break;
			}
			case GLFW_KEY_LEFT_BRACKET: {
				Globals::camera.ascend_dir += 1.f;
				break;
			}
			case GLFW_KEY_RIGHT_BRACKET: {
				Globals::camera.ascend_dir += -1.f;
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

}


// Function to set up geometry
void init_scene();

void update()
{
	Globals::camera.update();
}

//
//	Main
//
int main(int argc, char *argv[]){

	// Load the mesh
	std::stringstream obj_file; obj_file << MY_DATA_DIR << "sibenik/sibenik.obj";
	if( !Globals::mesh.load_obj( obj_file.str() ) ){ return 0; }
	Globals::mesh.print_details();

	// Forcibly scale the mesh vertices so that the entire model fits within a (-1,1) volume: the code below is a temporary measure that is needed to enable the entire model to be visible in the template app, before the student has defined the proper viewing and projection matrices
    	// This code should eventually be replaced by the use of an appropriate projection matrix
    	// FYI: the model dimensions are: center = (0,0,0); height: 30.6; length: 40.3; width: 17.0
    // find the extremum of the vertex locations (this approach works because the model is known to be centered; a more complicated method would be required in the general case)
    Vec3f min, max, scale;
    min = Globals::mesh.vertices[0]; max = Globals::mesh.vertices[0];
	for( int i=0; i<Globals::mesh.vertices.size(); ++i ){
           if (Globals::mesh.vertices[i][0] < min[0]) min[0] = Globals::mesh.vertices[i][0];
           else if (Globals::mesh.vertices[i][0] > max[0]) max[0] = Globals::mesh.vertices[i][0];
           if (Globals::mesh.vertices[i][1] < min[1]) min[1] = Globals::mesh.vertices[i][1];
           else if (Globals::mesh.vertices[i][1] > max[1]) max[1] = Globals::mesh.vertices[i][1];
           if (Globals::mesh.vertices[i][2] < min[2]) min[2] = Globals::mesh.vertices[i][2];
           else if (Globals::mesh.vertices[i][2] > max[2]) max[2] = Globals::mesh.vertices[i][2];
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

	/* define the camera initial view params */

	float near = 0.1f; float far = 100.f;

	Globals::camera = Cam3d(
		Vec3f(
			min[0] + (max[0] - min[0])*0.5f,
			min[1] + 3.0f, 
			min[2] + (max[2] - min[2])*0.5f
		), /* eye */
		Vec3f(1.f, 0.f, 0.f), /* dir */
		Vec3f(0.f, 1.f, 0.f), /* up */
		near, far, /* near, far */
		-near*0.5, near*0.5, near*0.5, -near*0.5, /* left, right, top, bottom */
		0.05, 0.05, 0.05 /* speed in X Y Z */
	);

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
	framebuffer_size_callback(window, int(Globals::win_width), int(Globals::win_height)); 

	// Perform some OpenGL initializations
	glEnable(GL_DEPTH_TEST);  // turn hidden surfce removal on
	glClearColor(1.f,1.f,1.f,1.f);  // set the background to white

	// Enable the shader, this allows us to set uniforms and attributes
	shader.enable();

	// Bind buffers
	glBindVertexArray(Globals::tris_vao);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, Globals::faces_ibo[0]);
    
	// Game loop
	while( !glfwWindowShouldClose(window) ){

		// Clear the color and depth buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		update();

		// Send updated info to the GPU
		glUniformMatrix4fv( shader.uniform("model"), 1, GL_FALSE, Globals::model.m  ); // model transformation (always the identity matrix in this assignment)
		glUniformMatrix4fv( shader.uniform("view"), 1, GL_FALSE, Globals::camera.view.m  ); // viewing transformation
		glUniformMatrix4fv( shader.uniform("projection"), 1, GL_FALSE, Globals::camera.projection.m ); // projection matrix

		// Draw
		glDrawElements(GL_TRIANGLES, Globals::mesh.faces.size()*3, GL_UNSIGNED_INT, 0);

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


void init_scene(){

	using namespace Globals;

	// Create the buffer for vertices
	glGenBuffers(1, verts_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, verts_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, mesh.vertices.size()*sizeof(mesh.vertices[0]), &mesh.vertices[0][0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create the buffer for colors
	glGenBuffers(1, colors_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, mesh.colors.size()*sizeof(mesh.colors[0]), &mesh.colors[0][0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create the buffer for normals
	glGenBuffers(1, normals_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo[0]);
	glBufferData(GL_ARRAY_BUFFER, mesh.normals.size()*sizeof(mesh.normals[0]), &mesh.normals[0][0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	// Create the buffer for indices
	glGenBuffers(1, faces_ibo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, faces_ibo[0]);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh.faces.size()*sizeof(mesh.faces[0]), &mesh.faces[0][0], GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	// Create the VAO
	glGenVertexArrays(1, &tris_vao);
	glBindVertexArray(tris_vao);

	int vert_dim = 3;

	// location=0 is the vertex
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, verts_vbo[0]);
	glVertexAttribPointer(0, vert_dim, GL_FLOAT, GL_FALSE, sizeof(mesh.vertices[0]), 0);

	// location=1 is the color
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo[0]);
	glVertexAttribPointer(1, vert_dim, GL_FLOAT, GL_FALSE, sizeof(mesh.colors[0]), 0);

	// location=2 is the normal
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, normals_vbo[0]);
	glVertexAttribPointer(2, vert_dim, GL_FLOAT, GL_FALSE, sizeof(mesh.normals[0]), 0);

	// Done setting data for the vao
	glBindVertexArray(0);

}

