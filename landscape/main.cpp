// glew must be before glfw
#include <GL/glew.h>
#include <GLFW/glfw3.h>

// contains helper functions such as shader compiler
#include "icg_helper.h"
#include "glm/gtc/matrix_transform.hpp"

#include "framebuffer.h"

#include "grid/grid.h"
#include "particles/snow.h"
#include "quad/quad.h"
#include "screenquad/screenquad.h"
#include "trackball.h"
#include "sky/sky.h"

//for memset (yes I'm an old fashioned C programmer, get over it.)
#include <string.h>

Grid terrain;
Grid water;
Trackball trackball; 
Snow snow;
Sky sky;

int window_width = 800;
int window_height = 600;
const int tex_width = 2048;

FrameBuffer framebuffer;
ScreenQuad heightmap;

using namespace glm;

mat4 projection_matrix;
mat4 view_matrix;
mat4 trackball_matrix;
mat4 old_trackball_matrix;

float last_y;

vec3 cam_pos;
vec3 cam_look;
vec3 cam_up;
float total;


float default_move_coeff = 0.01;
float default_rotation_move_coeff = 0.01;
float inertia_move_coeff;
bool inertia;
bool m_forward;
bool m_backward;
bool m_right;
bool m_left;
bool m_up;
bool m_down;

bool FPS_mode = false;
float margin = 0.1f;
float FPS_coeff = 2.5f;

bool bezier_mode = false;
float bezier_speed = 1.0f;
float bezier_frame_number = 0.0f;
const int numberOfPts = 8;
vec3 pos[numberOfPts] = {vec3(-0.96145, 0.334909, 0.98078),
                         vec3(-0.660958, 0.110372, 0.658177),
                         vec3(-0.572374, 0.0633491, 0.573775),
                         vec3(0.109945, 0.158341, -0.162833),
                         vec3(-0.0461158, 0.49035, -2.03308), //out
                         vec3(0.570173, 0.0930193, -0.492701),
                         vec3(0.696272, 0.340389, -0.151085),
                         vec3(0.873018, 0.423555, 0.982745)};

vec3 look[numberOfPts] = {vec3(0.41161, -0.600675, -0.401152),
                          vec3(0.755826, -0.255177, -0.642288),
                          vec3(0.816481, 0.0232589, -0.749613),
                          vec3(1.55104, 0.221676, -1.40737),
                          vec3(1.90209, 0.0565342, -2.10169), //out
                          vec3(1.0685, 0.16972, 1.39271),
                          vec3(-0.629362, 0.365474, 1.14593),
                          vec3(-0.157628, -0.810295, -0.529818)};



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

    snow.Init();
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
    terrain.Init(heightmap_tex_id, TERRAIN);
    water.Init(heightmap_tex_id, WATER);
    heightmap.Init(tex_width, tex_width, heightmap_tex_id);
    heightmap.fBmExponentPrecompAndSet(1, 1.54);

    framebufferHMRender();

    //cout << "test of Read pixels : " << heightmap_data[(tex_width - 1) * (tex_width - 1)] << " and "<< heightmap_data[1] << endl;
    //enable transparency
    glEnable (GL_BLEND);
    glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
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

void move(bool forward, float move_coeff) {
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

void moveForward(float move_coeff) {
    move(true, move_coeff);
}

void moveBackward(float move_coeff) {
    move(false, move_coeff);
}

void rotate2D(float position_0, float position_1, float& look0, float& look1, float angle, bool vertical) {
    if(!vertical) {
        look0 = position_0 + (look0 - position_0) * cos(angle) - (look1 - position_1) * sin(angle);
    }
    look1 = position_1 + (look0 - position_0) * sin(angle) + (look1 - position_1) * cos(angle);
}

void displayMove() {
    if(m_forward && !inertia) {
        moveForward(default_move_coeff);
    }
    if (m_forward && inertia) {
        moveForward(inertia_move_coeff);
        inertia_move_coeff /= 1.5;
        if(inertia_move_coeff < 0.00001f) {
            m_forward = false;
            inertia = false;
        }
    }
    if(m_backward && !inertia) {
        moveBackward(default_move_coeff);
    }
    if (m_backward && inertia) {
        moveBackward(inertia_move_coeff);
        inertia_move_coeff /= 1.5;
        if(inertia_move_coeff < 0.00001f) {
            m_backward = false;
            inertia = false;
        }
    }
    if(m_right && !inertia) {
        rotate2D(cam_pos[0], cam_pos[2], cam_look[0], cam_look[2], default_rotation_move_coeff, false);
    }
    if (m_right && inertia) {
        rotate2D(cam_pos[0], cam_pos[2], cam_look[0], cam_look[2], inertia_move_coeff, false);
        inertia_move_coeff /= 1.5;
        if(inertia_move_coeff < 0.00001f) {
            m_right = false;
            inertia = false;
        }
    }
    if(m_left && !inertia) {
        rotate2D(cam_pos[0], cam_pos[2], cam_look[0], cam_look[2], (-1 * default_rotation_move_coeff), false);
    }
    if (m_left && inertia) {
        rotate2D(cam_pos[0], cam_pos[2], cam_look[0], cam_look[2], (-1 * inertia_move_coeff), false);
        inertia_move_coeff /= 1.5;
        if(inertia_move_coeff < 0.00001f) {
            m_left = false;
            inertia = false;
        }
    }
    if(m_up && !inertia) {
        rotate2D(0,0, cam_look[0], cam_look[1], (-1 * default_rotation_move_coeff), true);
    }
    if (m_up && inertia) {
        rotate2D(0,0, cam_look[0], cam_look[1], (-1 * inertia_move_coeff), true);
        inertia_move_coeff /= 1.5;
        if(inertia_move_coeff < 0.00001f) {
            m_up = false;
            inertia = false;
        }
    }
    if(m_down && !inertia) {
        rotate2D(0,0, cam_look[0], cam_look[1], default_rotation_move_coeff, true);
    }
    if (m_down && inertia) {
        rotate2D(0,0, cam_look[0], cam_look[1], inertia_move_coeff, true);
        inertia_move_coeff /= 1.5;
        if(inertia_move_coeff < 0.00001f) {
            m_down = false;
            inertia = false;
        }
    }

    if(FPS_mode) {
        //set cam height !
        cam_pos[1] = getHeight(cam_pos[0], cam_pos[2], tex_width)/FPS_coeff + margin;
    }
}

vec3* pos_points() {
    vec3* pts = new vec3[numberOfPts];
    for(int i=0; i<numberOfPts; ++i) {
        pts[i] = pos[i];
    }
    return pts;
}

vec3* look_points() {
    vec3* pts = new vec3[numberOfPts];
    for(int i=0; i<numberOfPts; ++i) {
        pts[i] = look[i];
    }
    return pts;
}

vec3 getBezierPositionForFrame(vec3* pts, int size, float bezier_frame_number){
    if (size == 1) {
        vec3 pt = pts[0];
        delete [] pts;
        return pt;
    } else {
        vec3* newPts = new vec3[size - 1];
        for(int i = 0; i<size-1; ++i) {
            newPts[i] = (1 - bezier_frame_number)*pts[i] + bezier_frame_number*pts[i+1];
        }
        delete [] pts;
        return getBezierPositionForFrame(newPts, size - 1, bezier_frame_number);
    }
}

// gets called for every frame.
void Display() {
    // render to window
    glViewport(0, 0, window_width, window_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    const float time = glfwGetTime();
    terrain.Draw(time, trackball_matrix * IDENTITY_MATRIX, view_matrix, projection_matrix);
    snow.Draw(cam_pos, projection_matrix, time);
    sky.Draw(trackball_matrix, view_matrix, projection_matrix);
    water.Draw(time, trackball_matrix * IDENTITY_MATRIX, view_matrix, projection_matrix);

    if(bezier_mode) {
        bezier_frame_number += 0.0005 * bezier_speed;
        if(bezier_frame_number > 1.0f) {
            bezier_mode = false;
        } else if (bezier_frame_number < 0.0f) {
            bezier_mode = false;
        } else {
            cam_pos = getBezierPositionForFrame(pos_points(), numberOfPts, bezier_frame_number);
            cam_look = getBezierPositionForFrame(look_points(), numberOfPts, bezier_frame_number);
        }
    } else {
        displayMove();
    }

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

void MousePos(GLFWwindow* window, double x, double y) {
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS) {
        vec2 p = TransformScreenCoords(window, x, y);

        mat4 rotation = trackball.Drag(p.x, p.y);
        trackball_matrix = rotation * old_trackball_matrix;
    }

    // zoom
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS) {
        if (y > last_y) {
            moveBackward(default_move_coeff);
        } else {
            moveForward(default_move_coeff);
        }
    }
    last_y = y;
}

void ErrorCallback(int error, const char* description) {
    fputs(description, stderr);
}

void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    switch(key) {
        case 'W':
            //Forward
            if(bezier_mode) {
                bezier_speed += 0.1;
                cout << "speed : " << bezier_speed << endl;
            } else {
                if(action == GLFW_RELEASE) {
                    inertia_move_coeff = default_move_coeff;
                    inertia = true;
                } else {
                    m_forward = true;
                }
            }
            break;
        case 'S':
            //Backward
            if(bezier_mode) {
                bezier_speed -= 0.1;
                cout << "speed : " << bezier_speed << endl;
            } else {
                if(action == GLFW_RELEASE) {
                    inertia_move_coeff = default_move_coeff;
                    inertia = true;
                } else {
                    m_backward = true;
                }
            }
            break;
        case 'A':
            //left
            if(!bezier_mode) {
                if(action == GLFW_RELEASE) {
                    inertia_move_coeff = default_rotation_move_coeff;
                    inertia = true;
                } else {
                    m_left = true;
                }
            }
            break;
        case 'D':
            //right
            if(!bezier_mode) {
                if(action == GLFW_RELEASE) {
                    inertia_move_coeff = default_rotation_move_coeff;
                    inertia = true;
                } else {
                    m_right = true;
                }
            }
            break;
        case 'Q':
            //up
            if(!bezier_mode) {
                if(action == GLFW_RELEASE) {
                    inertia_move_coeff = default_rotation_move_coeff;
                    inertia = true;
                } else {
                    m_up = true;
                }
            }
            break;
        case 'E':
            //down
            if(!bezier_mode) {
                if(action == GLFW_RELEASE) {
                    inertia_move_coeff = default_rotation_move_coeff;
                    inertia = true;
                } else {
                    m_down = true;
                }
            }
            break;
        case 'F':
            // only act on release
            if(action != GLFW_RELEASE) {
                if(!bezier_mode) {
                    FPS_mode = !FPS_mode;
                    if(FPS_mode) {
                        default_move_coeff = 0.0025f;
                        dontExceedTerrainIfFPS();
                    } else {
                        default_move_coeff = 0.01f;
                    }
                }
            }
            break;
        case 'B':
            // only act on release
            if(action != GLFW_RELEASE) {
                bezier_mode = !bezier_mode;
                if(bezier_mode) {
                    if(FPS_mode) {
                        FPS_mode != FPS_mode;
                    }
                    bezier_frame_number = 0.0f;
                }
                return;
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
    snow.Cleanup();
    sky.Cleanup();

    // close OpenGL window and terminate GLFW
    glfwDestroyWindow(window);
    glfwTerminate();
    return EXIT_SUCCESS;
}
