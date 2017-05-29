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

float last_y;

void Init(GLFWwindow* window) {
    glClearColor(0.0, 0.0, 0.0 /*black*/, 1.0 /*solid*/);
    glEnable(GL_DEPTH_TEST);
    //added for floor reflection, not sure if need to be kept.
    glEnable(GL_MULTISAMPLE);

    sky.Init();

    // setup view and projection matrices
    cam_pos = vec3(0.0f, 1.0f, 2.0f);
    cam_look = vec3(0.0f, 0.0f, 0.0f);
    cam_up = vec3(0.0f, 0.0f, -1.0f);
    view_matrix = lookAt(cam_pos, cam_look, cam_up);
    //view_matrix = translate(mat4(1.0f), vec3(0.0f, 0.0f, -4.0f));
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

    // render to framebuffer
    framebuffer.Bind();
    {   
        glViewport(0,0,tex_width,tex_width);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        heightmap.Draw();
    }
    framebuffer.Unbind();

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
    framebuffer.Bind();
    {   
        glViewport(0,0,tex_width,tex_width);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        heightmap.Draw();
    }
    framebuffer.Unbind();

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
          view_matrix[3][2] *= 1.01f;
        } else {
          view_matrix[3][2] /= 1.01f;
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

    // set the callback for escape key
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
