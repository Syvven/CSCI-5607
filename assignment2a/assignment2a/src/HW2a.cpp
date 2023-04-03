// Skeleton code for hw2a
// Based on example code from: Interactive Computer Graphics: A Top-Down Approach with Shader-Based OpenGL (6th Edition), by Ed Angel


#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <assert.h>

#define DEBUG_ON 1  // repetitive of the debug flag in the shader loading code, included here for clarity only
#define ALLOW_FILES 1 

// This file contains the code that reads the shaders from their files and compiles them
#include "ShaderStuff.hpp"

//----------------------------------------------------------------------------

static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
static void cursor_pos_callback(GLFWwindow* window, double xpos, double ypos);
static void error_callback(int error, const char* decription);
static void multiply_matrices(GLfloat* left, GLfloat* right);
static void print_matrix(char* mat_name, GLfloat* mat);
static void read_vertices_from_file(char* filename);
static void update_transformation_matrix();
static void reset();


// initialize some basic structure types
typedef struct Vector2D {
    GLfloat x, y;
    Vector2D(GLfloat _x, GLfloat _y) : x(_x), y(_y) {}
    Vector2D() : x(0), y(0) {}
} Vector2D;

typedef struct Color3D {
    GLfloat r, g, b;
    Color3D(GLfloat _r, GLfloat _g, GLfloat _b) : r(_r), g(_g), b(_b) {}
    Color3D() : r(0), g(0), b(0) {}
} Color3D;

static GLint m_location;
static GLFWcursor* hand_cursor, * arrow_cursor; // some different cursors

const GLint window_width = 1500,
            window_height = 1500;

const GLdouble ww_inv = 1.0f / window_width,
               wh_inv = 1.0f / window_height;

const GLdouble pi = 4.0 * atan(1.0);

static GLdouble prev_x, prev_y;
static GLdouble mouse_x, mouse_y;

// define some assorted global variables, to make life easier
static GLfloat M[16]; // general transformation matrix

/* translation matrix */
static GLfloat T[16]{
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
}; 

/* translation globals */
static int T_X_IND   =  3,
           T_Y_IND   =  7,
           T_Z_IND   =  11;

static GLfloat move_x = 0.0f,
               move_y = 0.0f,
               move_z = 0.0f;

static bool translate = false;

/* scale matrix */
static GLfloat S[16]{
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};

/* scale globals */

static bool up_state    = false, 
            down_state  = false,
            left_state  = false,
            right_state = false;

static int S_X_IND = 0,
           S_Y_IND = 5,
           S_Z_IND = 10;

static GLfloat S_POS_MOD =  0.05f,
               S_NEG_MOD = -0.05f;


/* rotation matrix */
static GLfloat R[16]{
    1, 0, 0, 0,
    0, 1, 0, 0,
    0, 0, 1, 0,
    0, 0, 0, 1
};

/* rotation globals */
static bool rot = false;
static GLfloat rot_ang       = 0.0f,
               rot_speed     = 0.0f,
               rot_dir       = 0.0f,
               rot_max_speed = 0.1f,
               rot_accel     = 0.0005,
               rot_decel     = 0.0005,
               R_SPEED_MOD   = 0.1f;

/* values for extra credit */
std::vector<Vector2D> vertices_vec;
Vector2D* vertices;
std::vector<Color3D> colors_vec;
Color3D* colors;
std::vector<unsigned int> inds_vec;
unsigned int* indices;
bool read_from_file = false,
     read_kiwi_file = false;
GLint num_verts = 0;

static GLfloat Ms[3][16],
               Rs[3][16];

static GLfloat sub_rot_speed_1 = 0.0f,
               sub_rot_speed_2 = 0.0f,
               sub_rot_speed_3 = 0.0f,
               sub_rot_ang_1   = 0.0f,
               sub_rot_ang_2   = 0.0f,
               sub_rot_ang_3   = 0.0f,
               sub_rot_dir_1   = 0.0f,
               sub_rot_dir_2   = 0.0f,
               sub_rot_dir_3   = 0.0f;

static bool sub_rot_1 = false,
            sub_rot_2 = false,
            sub_rot_3 = false,
            one_press = false,
            two_press = false,
            thr_press = false;

//----------------------------------------------------------------------------
// function that is called whenever an error occurs
static void
error_callback(int error, const char* description){
    fputs(description, stderr);  // write the error description to stderr
}

//----------------------------------------------------------------------------
// function that is called whenever a keyboard event occurs; defines how keyboard input will be handled
static void
key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    int p_or_r = (action == GLFW_PRESS || action == GLFW_RELEASE);

    /* close window if escape pressed */
    if ((key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q) && action == GLFW_RELEASE) 
        glfwSetWindowShouldClose(window, GL_TRUE);  

    /* elongate along Y axis */
    if (key == GLFW_KEY_UP && p_or_r)
        up_state = !up_state;

    /* elongate along X axis */
    if (key == GLFW_KEY_DOWN && p_or_r)
        down_state = !down_state;

    if (key == GLFW_KEY_LEFT && p_or_r)
        left_state = !left_state;

    if (key == GLFW_KEY_RIGHT  && p_or_r)
        right_state = !right_state;

    if (key == GLFW_KEY_1 && p_or_r)
        one_press = !one_press;

    if (key == GLFW_KEY_2 && p_or_r)
        two_press = !two_press;

    if (key == GLFW_KEY_3 && p_or_r)
        thr_press = !thr_press;

    if (key == GLFW_KEY_R && action == GLFW_RELEASE)
        reset();
}

//----------------------------------------------------------------------------
// function that is called whenever a mouse or trackpad button press event occurs
static void
mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    glfwGetCursorPos(window, &mouse_x, &mouse_y);
    // Check which mouse button triggered the event, e.g. GLFW_MOUSE_BUTTON_LEFT, etc.
    // and what the button action was, e.g. GLFW_PRESS, GLFW_RELEASE, etc.
    // (Note that ordinary trackpad click = mouse left button)
    // Also check if any modifier keys were active at the time of the button press, e.g. GLFW_MOD_ALT, etc.
    // Take the appropriate action, which could (optionally) also include changing the cursor's appearance
    if (button == GLFW_MOUSE_BUTTON_LEFT)
    {
        if (action == GLFW_PRESS)
        {
            if (!rot && !sub_rot_1 && !sub_rot_2 && !sub_rot_3 && mods == GLFW_MOD_CONTROL)
            {
                move_x = 2 * mouse_x * ww_inv - 1.0f;
                move_y = 2 * -mouse_y * wh_inv + 1.0f;
                translate = true;
            }
            else if (!translate && !one_press && !two_press && !thr_press)
                rot = true;
            else
            {
                if (one_press) sub_rot_1 = true;
                if (two_press) sub_rot_2 = true;
                if (thr_press) sub_rot_3 = true;
            }

        }

        else if (action == GLFW_RELEASE)
        {
            translate = false;
            rot = false;
            sub_rot_1 = false;
            sub_rot_2 = false;
            sub_rot_3 = false;
        }
    }

    prev_x = mouse_x;
    prev_y = mouse_y;
}

//----------------------------------------------------------------------------
// function that is called whenever a cursor motion event occurs
static void
cursor_pos_callback(GLFWwindow* window, double xpos, double ypos) {
    // determine the direction of the mouse or cursor motion
    // update the current mouse or cursor location
    //  (necessary to quantify the amount and direction of cursor motion)
    // take the appropriate action

    GLfloat mouse_x_r = xpos - window_width * 0.5;
    GLfloat mouse_y_r = ypos - window_height * 0.5;

    if (rot)
    {
        rot_dir = atan2(
            mouse_x_r * prev_y - mouse_y_r * prev_x, 
            mouse_x_r * prev_x + mouse_y_r * prev_y
        );

        rot_dir = std::copysign(1.0f, rot_dir);
    }

    if (sub_rot_1)
    {
        sub_rot_dir_1 = atan2(
            mouse_x_r * prev_y - mouse_y_r * prev_x,
            mouse_x_r * prev_x + mouse_y_r * prev_y
        );

        sub_rot_dir_1 = std::copysign(1.0f, sub_rot_dir_1);
    }

    if (sub_rot_2)
    {
        sub_rot_dir_2 = atan2(
            mouse_x_r * prev_y - mouse_y_r * prev_x,
            mouse_x_r * prev_x + mouse_y_r * prev_y
        );

        sub_rot_dir_2 = std::copysign(1.0f, sub_rot_dir_2);
    }

    if (sub_rot_3)
    {
        sub_rot_dir_3 = atan2(
            mouse_x_r * prev_y - mouse_y_r * prev_x,
            mouse_x_r * prev_x + mouse_y_r * prev_y
        );

        sub_rot_dir_3 = std::copysign(1.0f, sub_rot_dir_3);
    }

    if (translate)
    {
        move_x = 2 * xpos * ww_inv - 1.0f;
        move_y = 2 * -ypos * wh_inv + 1.0f;
    }

    prev_x = mouse_x_r;
    prev_y = mouse_y_r;
}

//----------------------------------------------------------------------------

/* left is left and right is right. output stored in right */
static void multiply_matrices(GLfloat* left, GLfloat* right)
{
    for (int i = 0; i < 4; i++)
    {
        GLfloat aux[4]{ right[i], right[i+4], right[i+8], right[i+12] };
        right[i]    = aux[0]*left[0]  + aux[1]*left[1]  + aux[2]*left[2]  + aux[3]*left[3];
        right[i+4]  = aux[0]*left[4]  + aux[1]*left[5]  + aux[2]*left[6]  + aux[3]*left[7];
        right[i+8]  = aux[0]*left[8]  + aux[1]*left[9]  + aux[2]*left[10] + aux[3]*left[11];
        right[i+12] = aux[0]*left[12] + aux[1]*left[13] + aux[2]*left[14] + aux[3]*left[15];
    }
}

static void update_transformation_matrix()
{
    /**************** accumulate scale updates ****************/

    S[S_Y_IND] += up_state    * S_POS_MOD + down_state * S_NEG_MOD;
    S[S_X_IND] += right_state * S_POS_MOD + left_state * S_NEG_MOD;

    if (abs(S[S_Y_IND]) < 1e-15) S[S_Y_IND] = 0.0001;
    if (abs(S[S_X_IND]) < 1e-15) S[S_X_IND] = 0.0001;

    /**************** accumulate translation updates ****************/

    T[T_X_IND] = move_x;
    T[T_Y_IND] = move_y;

    /**************** accumulte rotation updates ****************/

    /* ramp up speed until it goes beyond max speed */
    rot_speed       += (rot_accel * rot       * (rot_speed       < rot_max_speed));

    sub_rot_speed_1 += (rot_accel * sub_rot_1 * (sub_rot_speed_1 < rot_max_speed)) + (rot_accel * rot * (rot_speed < rot_max_speed));
    sub_rot_speed_2 += (rot_accel * sub_rot_2 * (sub_rot_speed_2 < rot_max_speed)) + (rot_accel * rot * (rot_speed < rot_max_speed));
    sub_rot_speed_3 += (rot_accel * sub_rot_3 * (sub_rot_speed_3 < rot_max_speed)) + (rot_accel * rot * (rot_speed < rot_max_speed));

    /* decelerate if no longer holding mouse button */
    rot_speed       -= (rot_decel * !rot       * (rot_speed       > 0));

    sub_rot_speed_1 -= (rot_decel * !sub_rot_1 * (sub_rot_speed_1 > 0)) - (rot_decel * !rot * (rot_speed > 0));
    sub_rot_speed_2 -= (rot_decel * !sub_rot_2 * (sub_rot_speed_2 > 0)) - (rot_decel * !rot * (rot_speed > 0));
    sub_rot_speed_3 -= (rot_decel * !sub_rot_3 * (sub_rot_speed_3 > 0)) - (rot_decel * !rot * (rot_speed > 0));

    rot_ang       += rot_speed       * rot_dir;

    sub_rot_ang_1 += sub_rot_speed_1 * sub_rot_dir_1 + (rot_speed * rot_dir);
    sub_rot_ang_2 += sub_rot_speed_2 * sub_rot_dir_2 + (rot_speed * rot_dir);
    sub_rot_ang_3 += sub_rot_speed_3 * sub_rot_dir_3 + (rot_speed * rot_dir);

    if (read_from_file)
    {
        R[0] = cos(rot_ang); R[1] = -sin(rot_ang);
        R[4] = sin(rot_ang); R[5] = cos(rot_ang);

        /**************** apply transformation matrices ****************/
        multiply_matrices(S, M);
        multiply_matrices(R, M);
        multiply_matrices(T, M);
    }
    else
    {
        Rs[0][0] = cos(sub_rot_ang_1); Rs[0][1] = -sin(sub_rot_ang_1);
        Rs[0][4] = sin(sub_rot_ang_1); Rs[0][5] =  cos(sub_rot_ang_1);
        
        Rs[1][0] = cos(sub_rot_ang_2); Rs[1][1] = -sin(sub_rot_ang_2);
        Rs[1][4] = sin(sub_rot_ang_2); Rs[1][5] =  cos(sub_rot_ang_2);
        
        Rs[2][0] = cos(sub_rot_ang_3); Rs[2][1] = -sin(sub_rot_ang_3);
        Rs[2][4] = sin(sub_rot_ang_3); Rs[2][5] =  cos(sub_rot_ang_3);
        

        /**************** apply transformation matrices ****************/

        for (int i = 0; i < 3; i++)
        {
            multiply_matrices(S, Ms[i]);
            multiply_matrices(Rs[i], Ms[i]);
            multiply_matrices(T, Ms[i]);
        }
    }
}

static void reset()
{
    sub_rot_dir_1 = sub_rot_speed_1 = sub_rot_ang_1 = 0.0f;
    sub_rot_dir_2 = sub_rot_speed_2 = sub_rot_ang_2 = 0.0f;
    sub_rot_dir_3 = sub_rot_speed_3 = sub_rot_ang_3 = 0.0f;
    rot_dir       = rot_speed       = rot_ang       = 0.0f;

    move_x = move_y = move_z = prev_x = prev_y = 0.0f;

    sub_rot_1 = sub_rot_2 = sub_rot_3 = false;
    one_press = two_press = thr_press = false;

    up_state = down_state = right_state = left_state = translate = false;
    
    for (int i = 0; i < 16; i++)
    {
        Rs[0][i] = Rs[1][i] = Rs[2][i] = (i % 5 == 0);
        Ms[0][i] = Ms[1][i] = Ms[2][i] = (i % 5 == 0);
        M[i] = T[i] = S[i] = (i % 5 == 0);
    }
}

void init( void )
{
    GLuint vao[1], buffer, ebo, program, location1, location2;
    if (!read_from_file)
    {
        // set up some hard-coded colors and geometry
        // this part can be customized to read in an object description from a file

        num_verts = 9;
        
        colors_vec.clear();
        vertices_vec.clear();

        colors_vec.push_back(Color3D(1, 1, 1));  // white
        colors_vec.push_back(Color3D(1, 0, 0));  // red
        colors_vec.push_back(Color3D(1, 0, 0));  // red
        colors_vec.push_back(Color3D(1, 1, 1));  // white
        colors_vec.push_back(Color3D(0, 0, 1));  // blue
        colors_vec.push_back(Color3D(0, 0, 1));  // blue
        colors_vec.push_back(Color3D(1, 1, 1));  // white
        colors_vec.push_back(Color3D(0, 1, 1));  // cyan
        colors_vec.push_back(Color3D(0, 1, 1));  // cyan

        vertices_vec.push_back(Vector2D(0, 0.25));      // center
        vertices_vec.push_back(Vector2D(0.25, 0.5));    // upper right
        vertices_vec.push_back(Vector2D(-0.25, 0.5));   // upper left
        vertices_vec.push_back(Vector2D(0, 0.25));      // center (again)
        vertices_vec.push_back(Vector2D(0.25, -0.5));   // low-lower right
        vertices_vec.push_back(Vector2D(0.5, -0.25));   // mid-lower right
        vertices_vec.push_back(Vector2D(0, 0.25));      // center (again)
        vertices_vec.push_back(Vector2D(-0.5, -0.25));  // low-lower left
        vertices_vec.push_back(Vector2D(-0.25, -0.5));  // mid-lower left
    }

    if (!read_kiwi_file) for (int i = 0; i < num_verts; i++) inds_vec.push_back(i);

    size_t colors_size = colors_vec.size() * sizeof(Color3D);
    size_t verts_size = vertices_vec.size() * sizeof(Vector2D);
    size_t inds_size = inds_vec.size() * sizeof(unsigned int);

    colors = colors_vec.data();
    vertices = vertices_vec.data();
    indices = inds_vec.data();
        
    // Create and bind a vertex array object

    /* creates the vertex array object */
    glGenVertexArrays(1, vao);

    /* this gens the buffer for the vao for color/texture/vertices */
    glGenBuffers(1, &buffer);
    /* this gens the buffer for the vao for indices of the triangles */
    glGenBuffers(1, &ebo);

    /* binds the current vertex array (as you can have multiple) */
    glBindVertexArray(vao[0]);
    
    /* binds the current buffer */
    glBindBuffer(GL_ARRAY_BUFFER, buffer);
    /* sets how much data this buffer will be able to store */
    glBufferData(
        GL_ARRAY_BUFFER,
        colors_size + verts_size,
        nullptr, /* nullptr because we call subData right after */
        GL_STATIC_DRAW
    );

    /* some asserts just in case */
    assert(sizeof(GLfloat) * 2 == sizeof(Vector2D));
    assert(sizeof(GLfloat) * 3 == sizeof(Color3D));
    assert(verts_size == (sizeof(GLfloat) * 2 * num_verts));
    assert(colors_size == (sizeof(GLfloat) * 3 * num_verts));
    assert(inds_size == (sizeof(unsigned int) * inds_vec.size()));

    /* actually puts the data from verts into the first slots of the array buffer */
    glBufferSubData(
        GL_ARRAY_BUFFER, 
        0, verts_size, 
        vertices
    );
    /* actually puts the data from the colors into the rest of the array buffer */
    glBufferSubData(
        GL_ARRAY_BUFFER, 
        verts_size, colors_size, 
        colors
    );

    /* binds the next buffer for the triangle indices */
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    /* sets how much size the buffer will need */
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER, 
        inds_size, 
        nullptr, /* nullptr because we call subData right after */
        GL_STATIC_DRAW
    );
    /* actually puts the indices into the buffer object */
    glBufferSubData(
        GL_ELEMENT_ARRAY_BUFFER,
        0, inds_size,
        indices
    );

    // Define the names of the shader files
    std::stringstream vshader, fshader;
    vshader << SRC_DIR << "/vshader2a.glsl";
    fshader << SRC_DIR << "/fshader2a.glsl";

    // Load the shaders and use the resulting shader program
    program = InitShader(vshader.str().c_str(), fshader.str().c_str());

    // Determine locations of the necessary attributes and matrices used in the vertex shader
    location1 = glGetAttribLocation(program, "vertex_position");
    glEnableVertexAttribArray(location1);
    glVertexAttribPointer(location1, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
    location2 = glGetAttribLocation(program, "vertex_color");
    glEnableVertexAttribArray(location2);
    glVertexAttribPointer(location2, 3, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(sizeof(Vector2D)*num_verts));
    m_location = glGetUniformLocation(program, "M");

    // Define static OpenGL state variables
    glClearColor(1.0, 1.0, 1.0, 1.0); // white, opaque background

    // Define some GLFW cursors (in case you want to dynamically change the cursor's appearance)
    // If you want, you can add more cursors, or even define your own cursor appearance
    arrow_cursor = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    hand_cursor = glfwCreateStandardCursor(GLFW_HAND_CURSOR);

    for (int j = 0; j < 16; j++)
    {
        Rs[0][j] = (j % 5 == 0);
        Rs[1][j] = (j % 5 == 0);
        Rs[2][j] = (j % 5 == 0);
    }
}

//----------------------------------------------------------------------------

static void read_kiwi_from_file(char* filename)
{
    std::cout << "Loading Secret Kiwi :)" << std::endl;

    FILE* fp;
    if ((fp = fopen(filename, "r")) == nullptr)
    {
        fprintf(stderr, "Failed To Open File: %s\n", filename);
        return;
    }

    char line[512];
    Color3D color;
    while (fgets(line, 512, fp) != nullptr)
    {
        if (line[0] == '\n') continue;

        if (line[0] == 'v')
        {
            Vector2D vertex;  float throwaway;
            int ret = sscanf(line, "v %f %f %f",
                &vertex.x, &vertex.y, &throwaway
            );

            if (ret == EOF) break;
            if (ret != 3)
            {
                fprintf(stderr, "Failed to read vertex from %s.\n", filename);
                exit(EXIT_FAILURE);
            }

            GLfloat newx = vertex.x * cos(-0.5*pi) + vertex.y * -sin(-0.5*pi);
            GLfloat newy = vertex.x * sin(-0.5*pi) + vertex.y *  cos(-0.5*pi);
            vertex.x = newx; vertex.y = newy;

            vertices_vec.push_back(vertex);
            colors_vec.push_back(color);
        }
        if (line[0] == 'c')
        {
            int ret = sscanf(line, "c %f %f %f",
                &color.r, &color.g, &color.b
            );

            if (ret == EOF) break;
            if (ret != 3)
            {
                fprintf(stderr, "Failed to read color from %s.\n", filename);
                exit(EXIT_FAILURE);
            }
        }
        if (line[0] == 'f')
        {
            unsigned int v0, v1, v2, v3;
            int ret = sscanf(line, "f %d %d %d %d",
                &v0, &v1, &v2, &v3
            );
            if (ret == EOF) break;
            if (ret != 4)
            {
                fprintf(stderr, "Failed to read face from %s.\n", filename);
                exit(EXIT_FAILURE);
            }

            /* break quad into two triangles */

            inds_vec.push_back(v0-1);
            inds_vec.push_back(v1-1);
            inds_vec.push_back(v2-1);

            inds_vec.push_back(v0-1);
            inds_vec.push_back(v2-1);
            inds_vec.push_back(v3-1);
        }
    }

    if (vertices_vec.size() != 0)
    {
        read_from_file = true;
        num_verts = vertices_vec.size();

        read_kiwi_file = true;
    }
}

static void read_vertices_from_file(char *filename)
{
    FILE* fp;
    if ((fp = fopen(filename, "r")) == nullptr)
    {
        fprintf(stderr, "Failed To Open File: %s\n", filename);
        return;
    }

    char line[512];
    while (fgets(line, 512, fp) != nullptr)
    {
        if (line[0] == '\n') continue;
        Vector2D vertex; Color3D color; float throwaway;
        int ret = sscanf(line, "v %f %f %f %f %f %f",
            &vertex.x, &vertex.y, &throwaway,
            &color.r, &color.g, &color.b);
        if (ret == EOF) break;
        if (ret != 6)
        {
            fprintf(stderr, "Failed to read from %s.\n", filename);
            exit(EXIT_FAILURE);
        }

        vertices_vec.push_back(vertex);
        colors_vec.push_back(color);
    }

    if (vertices_vec.size() != 0)
    {
        read_from_file = true;
        num_verts = vertices_vec.size();
    }
}

static void print_matrix(char *mat_name, GLfloat* mat)
{
    printf(
        "%s = [%f %f %f %f\n     %f %f %f %f\n     %f %f %f %f\n     %f %f %f %f]\n\n", mat_name,
                    mat[0], mat[4], mat[8], mat[12], 
                    mat[1], mat[5], mat[9], mat[13], 
                    mat[2], mat[6], mat[10], mat[14], 
                    mat[3], mat[7], mat[11], mat[15]
    );
}

int main(int argc, char** argv) {

    int i;
    GLFWwindow* window;

    prev_x = 0.0;
    prev_y = 0.0;

    // Initialize GLFW (performs platform-specific initialization)
    if (!glfwInit()) exit(EXIT_FAILURE);

    // Define the error callback function
    glfwSetErrorCallback(error_callback);
    
    // Ask for OpenGL 3.2
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Use GLFW to open a window within which to display your graphics
	window = glfwCreateWindow(window_width, window_height, "assignment2a", NULL, NULL);
	
    // Verify that the window was successfully created; if not, print error message and terminate
    if (!window)
	{
        printf("GLFW failed to create window; terminating\n");
        glfwTerminate();
        exit(EXIT_FAILURE);
	}
    
	glfwMakeContextCurrent(window); // makes the newly-created context current
    
    // Load all OpenGL functions (needed if using Windows)
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		printf("gladLoadGLLoader failed; terminating\n");
		glfwTerminate();
		exit(EXIT_FAILURE);
	}
    
	glfwSwapInterval(1);  // tells the system to wait for the rendered frame to finish updating before swapping buffers; can help to avoid tearing

    glfwSetInputMode(window, GLFW_STICKY_MOUSE_BUTTONS, GLFW_TRUE);

    // Define the keyboard callback function
    glfwSetKeyCallback(window, key_callback);
    // Define the mouse button callback function
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    // Define the mouse motion callback function
    glfwSetCursorPosCallback(window, cursor_pos_callback);

    /* read in vertices from file if file given */
    if (argc == 2) read_vertices_from_file(argv[1]);
    if (argc == 3 && strcmp(argv[2], "secret_kiwi") == 0) read_kiwi_from_file(argv[1]);

	// Create the shaders and perform other one-time initializations
	init();
	// event loop
    while (!glfwWindowShouldClose(window)) {

        // fill/re-fill the window with the background color
        glClear(GL_COLOR_BUFFER_BIT);

        // define/re-define the modelview matrix. In this template, we define M to be the identity matrix; you will need define M according to the user's actions.
        for (i = 0; i < 16; i++) {
            M[i] = (i % 5 == 0);
            Ms[0][i] = (i % 5 == 0);
            Ms[1][i] = (i % 5 == 0);
            Ms[2][i] = (i % 5 == 0);
        }

        /* update the transformation matrix */
        update_transformation_matrix();

        // sanity check that your matrix contents are what you expect them to be
        if (DEBUG_ON)
        {
            print_matrix("M", M);
            print_matrix("S", S);
            print_matrix("R", R);
            print_matrix("T", T);
        }

        if (read_from_file)
        {
            glUniformMatrix4fv(m_location, 1, GL_TRUE, M);   // send the updated model transformation matrix to the GPU
            glDrawElements(GL_TRIANGLES, inds_vec.size(), GL_UNSIGNED_INT, 0);
        }
        else
        {
            glUniformMatrix4fv(m_location, 1, GL_TRUE, Ms[0]);   // send the updated model transformation matrix to the GPU
            glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, 0);    // draw a triangle between the first vertex and each successive vertex pair in the hard-coded model

            glUniformMatrix4fv(m_location, 1, GL_TRUE, Ms[1]);   // send the updated model transformation matrix to the GPU
            glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(3 * sizeof(GLfloat)));

            glUniformMatrix4fv(m_location, 1, GL_TRUE, Ms[2]);   // send the updated model transformation matrix to the GPU
            glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_INT, (void*)(6 * sizeof(GLfloat)));
        }

		glFlush();	// ensure that all OpenGL calls have executed before swapping buffers

        glfwSwapBuffers(window);  // swap buffers
        glfwPollEvents(); // wait for a new event before re-drawing

	} // end graphics loop

	// Clean up
	glfwDestroyWindow(window);
	glfwTerminate();  // destroys any remaining objects, frees resources allocated by GLFW
	exit(EXIT_SUCCESS);

} // end main


