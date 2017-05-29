// glew must be before glfw
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// contains helper functions such as shader compiler
#include "icg_helper.h"
#include "glm/gtc/matrix_transform.hpp"

#include "framebuffer.h"

#include "grid/grid.h"
#include "quad/quad.h"
#include "screenquad/screenquad.h"
#include "trackball.h"
#include "sky/sky.h"
#include "water/water.h"

//for memset (yes I'm an old fashioned C programmer, get over it.)
#include <string.h>

Grid terrain;
Water water;
Trackball trackball;
Sky sky;

int window_width = 800;
int window_height = 600;
const int tex_width = 2048;

FrameBuffer framebuffer;
FrameBuffer mirrorBuffer;
ScreenQuad heightmap;

using namespace glm;

mat4 projection_matrix;
mat4 view_matrix;
mat4 trackball_matrix;
mat4 old_trackball_matrix;

vec3 cam_pos;
vec3 cam_look;
vec3 cam_up;

float move_coeff = 0.005;
bool FPS_mode = false;

float total;

float last_y;

float margin = 0.08f;
float FPS_coeff = 2.5f;

//to store globally the height of the terrain.
GLfloat heightmap_data[tex_width * tex_width];


//debug function, to fflush the error queue to be sure the next error recieved afetr this is the one from the function trialed.
void flushErrorQueue(){
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR);
}
//debug function, to print what error is in the head of of the error queue
void displayErrEnum(){
    string errEnum;
    switch(glGetError()){
        case GL_NO_ERROR:
            errEnum = "GL_NO_ERROR";
            break;
        case GL_INVALID_ENUM:
            errEnum = "GL_INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            errEnum = "GL_INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            errEnum = "GL_INVALID_OPERATION";
            break;
        case GL_STACK_OVERFLOW:
            errEnum = "GL_STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            errEnum = "GL_STACK_UNDERFLOW";
            break;
        case GL_OUT_OF_MEMORY:
            errEnum = "GL_OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            errEnum = "GL_INVALID_FRAMEBUFFER_OPERATION";
            break;
        case GL_CONTEXT_LOST:
            errEnum = "GL_CONTEXT_LOST";
            break;
        case GL_TABLE_TOO_LARGE:
            errEnum = "GL_TABLE_TOO_LARGE";
            break;
        default:
            errEnum = "unknown error";
            break;
    } 
    cout << errEnum;
}

void framebufferHMRender(){
    // render to framebuffer
    framebuffer.Bind();
    {   
        glViewport(0,0,tex_width, tex_width);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        heightmap.Draw();
        memset(heightmap_data, (float) 0, tex_width*tex_width);
        //stocking the result of the heightmap to a global array, in order to read height values
        glReadPixels(0, 0, tex_width, tex_width, GL_RED, GL_FLOAT, heightmap_data);

    }
    framebuffer.Unbind();
}

//return the height of the terrain according to the x and y coordinates given as parameters.
float getHeight(float x, float y, int terrain_w){
    if(abs(x) > 1.0f || abs(y) > 1.0f){
        return 0.0f;
    }
    int center = terrain_w/2;
    int newY = (x + 1.0f) * center;
    int newX = (-y + 1.0f) * center;
    return heightmap_data[newX * terrain_w + newY];
}

void Init(GLFWwindow* window) {
    glClearColor(0.0, 0.0, 0.0 /*black*/, 1.0 /*solid*/);
    glEnable(GL_DEPTH_TEST);
    //added for floor reflection, not sure if need to be kept.
    glEnable(GL_MULTISAMPLE);

    sky.Init();

    // setup view and projection matrices
    cam_pos = vec3(0.0f, 1.0f, 2.0f);
    cam_look = vec3(0.0f, 0.0f, 0.0f);
    cam_up = vec3(0.0f, 1.0f, 0.0f);
    view_matrix = lookAt(cam_pos, cam_look, cam_up);
    float ratio = window_width / (float) window_height;
    projection_matrix = perspective(45.0f, ratio, 0.1f, 10.0f);

    trackball_matrix = IDENTITY_MATRIX;

    // on retina/hidpi displays, pixels != screen coordinates
    // this unsures that the framebuffer has the same size as the window
    // (see http://www.glfw.org/docs/latest/window.html#window_fbsize)
    glfwGetFramebufferSize(window, &window_width, &window_height);
    GLuint heightmap_tex_id = framebuffer.Init(tex_width, tex_width);
    terrain.Init(heightmap_tex_id);

    GLuint mirror_tex_id = mirrorBuffer.Init(tex_width, tex_width);
    water.Init(mirror_tex_id);
    heightmap.Init(tex_width, tex_width, heightmap_tex_id);
    heightmap.fBmExponentPrecompAndSet(1, 1.54);

    framebufferHMRender();

    //cout << "test of Read pixels : " << heightmap_data[(tex_width - 1) * (tex_width - 1)] << " and "<< heightmap_data[1] << endl;
    //enable transparency
    glEnable (GL_BLEND); 
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}


// gets called for every frame.
void Display() {
    //render to frameBuffer of the mirror
    const float time = glfwGetTime();
    mirrorBuffer.Bind();
    {
        vec3 cam_down(cam_up.x, cam_up.y, -cam_up.z);
        vec3 mirrored_cam_pos(cam_pos.x, cam_pos.y, /*-*/cam_pos.z);
        //mat4 mirrored_view = lookAt(mirrored_cam_pos, cam_look, cam_down);
        mat4 mirrored_proj = scale(projection_matrix, vec3(-1.0f, 1.0f, 1.0f));
 
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        terrain.Draw(time, trackball_matrix * IDENTITY_MATRIX, view_matrix, mirrored_proj);
        //sky.Draw(trackball_matrix, mirrored_view, mirrored_proj);
    }
    mirrorBuffer.Unbind();

    // render to window
    glViewport(0, 0, window_width, window_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //terrain.Draw(time, trackball_matrix * IDENTITY_MATRIX, view_matrix, projection_matrix);
    //sky.Draw(trackball_matrix, view_matrix, projection_matrix);
    water.Draw(time, trackball_matrix * IDENTITY_MATRIX, view_matrix, projection_matrix);
    
    //update the lookAt position
    view_matrix = lookAt(cam_pos, cam_look, cam_up);
}

// gets called when the windows/framebuffer is resized.
void ResizeCallback(GLFWwindow* window, int width, int height) {
    window_width = width;
    window_height = height;

    float ratio = window_width / (float) window_height;
    projection_matrix = perspective(45.0f, ratio, 0.1f, 10.0f);

    glViewport(0, 0, window_width, window_height);

    // when the window is resized, the framebuffer and the screenquad
    // should also be resized
    framebuffer.Cleanup();
    framebuffer.Init(tex_width, tex_width);

    // render to framebuffer
    framebufferHMRender();
}

// transforms glfw screen coordinates into normalized OpenGL coordinates.
vec2 TransformScreenCoords(GLFWwindow* window, int x, int y) {
    // the framebuffer and the window doesn't necessarily have the same size
    // i.e. hidpi screens. so we need to get the correct one
    int width;
    int height;
    glfwGetWindowSize(window, &width, &height);
    return vec2(2.0f * (float)x / width - 1.0f,
                1.0f - 2.0f * (float)y / height);
}

void MouseButton(GLFWwindow* window, int button, int action, int mod) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        double x_i, y_i;
        glfwGetCursorPos(window, &x_i, &y_i);
        vec2 p = TransformScreenCoords(window, x_i, y_i);
        trackball.BeingDrag(p.x, p.y);
        old_trackball_matrix = trackball_matrix;
        // Store the current state of the model matrix.
    }
}

void dontExceedTerrainIfFPS() {
    if(FPS_mode) {
        if(cam_pos[0] > 1.0f) {
            cam_pos[0] = 1.0f;
        } else if (cam_pos[0] < -1.0f) {
            cam_pos[0] = -1.0f;
        }
        if(cam_pos[2] > 1.0f) {
            cam_pos[2] = 1.0f;
        } else if (cam_pos[2] < -1.0f) {
            cam_pos[2] = -1.0f;
        }
    }
}

void move(bool forward) {
    float coeff = -1.0f;
    if(forward) {
        coeff = 1.0f;
    }
    total = abs(cam_look[0]-cam_pos[0]) + abs(cam_look[1]-cam_pos[1]) + abs(cam_look[2]-cam_pos[2]);
    cam_pos[0] = cam_pos[0] + coeff * (((cam_look[0]-cam_pos[0])/total) * move_coeff);
    cam_pos[2] = cam_pos[2] + coeff * (((cam_look[2]-cam_pos[2])/total) * move_coeff);
    if(!FPS_mode) {
        cam_pos[1] = cam_pos[1] + coeff * (((cam_look[1]-cam_pos[1])/total) * move_coeff);
    } else {
        dontExceedTerrainIfFPS();
        cam_pos[1] = getHeight(cam_pos[0], cam_pos[2], tex_width)/FPS_coeff + margin;
    }
    cam_look[0] = cam_look[0] + coeff * (((cam_look[0]-cam_pos[0])/total) * move_coeff);
    if(!FPS_mode) {
        cam_look[1] = cam_look[1] + coeff * (((cam_look[1]-cam_pos[1])/total) * move_coeff);
    }
    cam_look[2] = cam_look[2] + coeff * (((cam_look[2]-cam_pos[2])/total) * move_coeff);
}

void moveForward() {
    move(true);
}

void moveBackward() {
    move(false);
}

void MousePos(GLFWwindow* window, double x, double y) {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        vec2 p = TransformScreenCoords(window, x, y);

        mat4 rotation = trackball.Drag(p.x, p.y);
        trackball_matrix = rotation * old_trackball_matrix;
    }

    // zoom
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        if (y > last_y) {
            moveBackward();
        } else {
            moveForward();
        }
    }
    last_y = y;
}

void ErrorCallback(int error, const char* description) {
    fputs(description, stderr);
}

void rotate2D(float position_0, float position_1, float& look0, float& look1, float angle, bool vertical) {
    if(!vertical) {
        look0 = position_0 + (look0 - position_0) * cos(angle) - (look1 - position_1) * sin(angle);
    }
    look1 = position_1 + (look0 - position_0) * sin(angle) + (look1 - position_1) * cos(angle);
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    switch(key) {
        case 'W':
            //Forward
            if(action == GLFW_RELEASE) {
                moveForward();
            } else {
                moveForward();
            }
            break;
        case 'S':
            //Backward
            if(action == GLFW_RELEASE) {
                moveBackward();
            } else {
                moveBackward();
            }
            break;
        case 'A':
            //left
            if(action == GLFW_RELEASE) {
                rotate2D(cam_pos[0], cam_pos[2], cam_look[0], cam_look[2], -0.05, false);
            } else {
                rotate2D(cam_pos[0], cam_pos[2], cam_look[0], cam_look[2], -0.05, false);
            }
            break;
        case 'D':
            //right
            if(action == GLFW_RELEASE) {
                rotate2D(cam_pos[0], cam_pos[2], cam_look[0], cam_look[2], +0.05, false);
            } else {
                rotate2D(cam_pos[0], cam_pos[2], cam_look[0], cam_look[2], +0.05, false);
            }
            break;
        case 'Q':
            //up
            if(action == GLFW_RELEASE) {
                rotate2D(0,0, cam_look[0], cam_look[1], -0.1f, true);
            } else {
                rotate2D(0,0, cam_look[0], cam_look[1], -0.1f, true);
            }
            break;
        case 'E':
            //down
            if(action == GLFW_RELEASE) {
                rotate2D(0,0, cam_look[0], cam_look[1], 0.1f, true);
            } else {
                rotate2D(0,0, cam_look[0], cam_look[1], 0.1f, true);
            }
            break;
        case 'F':
            // only act on release
            if(action != GLFW_RELEASE) {
                return;
            }
            FPS_mode = !FPS_mode;
            dontExceedTerrainIfFPS();
            if(FPS_mode) {
                //set cam height !
                cam_pos[1] = getHeight(cam_pos[0], cam_pos[2], tex_width)/FPS_coeff + margin;
            }
            break;
        default:
            break;
    }
}

int main(int argc, char *argv[]) {
    // GLFW Initialization
    if(!glfwInit()) {
        fprintf(stderr, "Failed to initialize GLFW\n");
        return EXIT_FAILURE;
    }

    glfwSetErrorCallback(ErrorCallback);

    // hint GLFW that we would like an OpenGL 3 context (at least)
    // http://www.glfw.org/faq.html#how-do-i-create-an-opengl-30-context
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // attempt to open the window: fails if required version unavailable
    // note some Intel GPUs do not support OpenGL 3.2
    // note update the driver of your graphic card
    GLFWwindow* window = glfwCreateWindow(window_width, window_height,
                                          "framebuffer", NULL, NULL);
    if(!window) {
        glfwTerminate();
        return EXIT_FAILURE;
    }

    // makes the OpenGL context of window current on the calling thread
    glfwMakeContextCurrent(window);

    // set the callback for escape key and camera movements
    glfwSetKeyCallback(window, KeyCallback);

    // set the framebuffer resize callback
    glfwSetFramebufferSizeCallback(window, ResizeCallback);

    // set the mouse press and position callback
    glfwSetMouseButtonCallback(window, MouseButton);
    glfwSetCursorPosCallback(window, MousePos);

    // GLEW Initialization (must have a context)
    // https://www.opengl.org/wiki/OpenGL_Loading_Library
    glewExperimental = GL_TRUE; // fixes glew error (see above link)
    if(glewInit() != GLEW_NO_ERROR) {
        fprintf( stderr, "Failed to initialize GLEW\n");
        return EXIT_FAILURE;
    }

    cout << "OpenGL" << glGetString(GL_VERSION) << endl;

    // initialize our OpenGL program
    Init(window);

    // render loop
    while(!glfwWindowShouldClose(window)){
        Display();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // cleanup
    terrain.Cleanup();
    framebuffer.Cleanup();
    sky.Cleanup();
    water.Cleanup();

    // close OpenGL window and terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
