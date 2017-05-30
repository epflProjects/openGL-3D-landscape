#pragma once
#include "icg_helper.h"
#include <glm/gtc/type_ptr.hpp>

using namespace std;

struct Snowflake {
  glm::vec3 pos;
  glm::vec3 speed;
  unsigned char r, g, b, a; // COLOR
  float size;
  float angle;
  float weight;
  float life; // Remaining life of the particle. If <0: dead and unused.
  float cameradistance; // Squared distance to the camera.

  bool operator<(const Snowflake &that) const {
    // Sort in reverse order : far particles drawn first.
    return this->cameradistance > that.cameradistance;
  }
};

static const int MaxSnowFlakes = 100000;
static GLfloat *g_snowflake_position_size_data = new GLfloat[MaxSnowFlakes * 4];
static GLubyte *g_snowflake_color_data = new GLubyte[MaxSnowFlakes * 4];
Snowflake SnowFlakesContainer[MaxSnowFlakes];
GLuint billboard_vertex_buffer;
GLuint snowflakes_position_buffer;
GLuint snowflakes_color_buffer;
int SnowflakesCount;
int LastUsedSnowFlake = 0;
GLuint CameraRight_worldspace_ID;
GLuint CameraUp_worldspace_ID;
GLuint ViewProjMatrixID;

class Snow {

private:
  // Finds a Snowflake in SnowFlakesContainer which isn't used yet.
  int FindUnusedSnowflake() {
    for (int i = LastUsedSnowFlake; i < MaxSnowFlakes; i++) {
      if (SnowFlakesContainer[i].life < 0) {
        LastUsedSnowFlake = i;
        return i;
      }
    }
    for (int i = 0; i < LastUsedSnowFlake; i++) {
      if (SnowFlakesContainer[i].life < 0) {
        LastUsedSnowFlake = i;
        return i;
      }
    }
    return 0;
  }

  void sortSnowflakes() {
    std::sort(&SnowFlakesContainer[0], &SnowFlakesContainer[MaxSnowFlakes]);
  }

public:
  float snowCount = 0;

  void Init() {
    static GLfloat *g_snowflake_position_size_data =
        new GLfloat[MaxSnowFlakes * 4];
    static GLubyte *g_snowflake_color_data = new GLubyte[MaxSnowFlakes * 4];

    GLuint program_id_ =
        icg_helper::LoadShaders("snow_vshader.glsl", "snow_fshader.glsl");

    // Vertex shader
    CameraRight_worldspace_ID =
        glGetUniformLocation(program_id_, "CameraRight_worldspace");
    CameraUp_worldspace_ID =
        glGetUniformLocation(program_id_, "CameraUp_worldspace");
    ViewProjMatrixID = glGetUniformLocation(program_id_, "VP");

    // fragment shader
    GLuint TextureID = glGetUniformLocation(program_id_, "myTextureSampler");

    for (int i = 0; i < MaxSnowFlakes; i++) {
      SnowFlakesContainer[i].life = -1.0f;
      SnowFlakesContainer[i].cameradistance = -1.0f;
    }

    // VBO containing the 4 vertices of the snowflakes
    static const GLfloat g_vertex_buffer_data[] = {
        -0.5f, -0.5f, 0.0f, 0.5f, -0.5f, 0.0f,
        -0.5f, 0.5f,  0.0f, 0.5f, 0.5f,  0.0f,
    };
    GLuint billboard_vertex_buffer;
    glGenBuffers(1, &billboard_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data),
                 g_vertex_buffer_data, GL_STATIC_DRAW);

    // The VBO containing the positions and sizes of the particles
    GLuint snowflakes_position_buffer;
    glGenBuffers(1, &snowflakes_position_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, snowflakes_position_buffer);
    glBufferData(GL_ARRAY_BUFFER, MaxSnowFlakes * 4 * sizeof(GLfloat), NULL,
                 GL_STREAM_DRAW);

    // The VBO containing the colors of the particles
    GLuint snowflakes_color_buffer;
    glGenBuffers(1, &snowflakes_color_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, snowflakes_color_buffer);
    glBufferData(GL_ARRAY_BUFFER, MaxSnowFlakes * 4 * sizeof(GLubyte), NULL,
                 GL_STREAM_DRAW);
  }

  void Draw(glm::vec3 cameraPos, glm::mat4 viewProjectionMatrix,
            float deltaTime) {
    // TODO je tente de mettre devant:
    glm::vec3 CameraPosition = cameraPos;
    glm::mat4 ViewProjectionMatrix = viewProjectionMatrix;

    int newsnowflakes = 1; //(int)(deltaTime * 10000.0);
    if (newsnowflakes > (int)(0.016f * 10000.0)) {
      newsnowflakes = (int)(0.016f * 10000.0);
    }

    // for (int i = 0; i < newsnowflakes; i++) {
    //   int snowflakeIndex = FindUnusedSnowflake();
    //   SnowFlakesContainer[snowflakeIndex].life =
    //       5.0f; // this snowflake will live 5 seconds
    //   SnowFlakesContainer[snowflakeIndex].pos = glm::vec3(0, 0, 0.0f);
    //
    //   float spread = 0.2f;
    //   glm::vec3 maindir = glm::vec3(0.0f, 0.0f, -10.0f);
    //
    //   // bad way to create random direction. See:
    //   //
    //   http://stackoverflow.com/questions/5408276/python-uniform-spherical-distribution
    //   // glm::vec3 randomdir = glm::vec3((rand() % 2000 - 1000.0f) / 1000.0f,
    //   //                                 (rand() % 2000 - 1000.0f) / 1000.0f,
    //   //                                 (rand() % 2000 - 1000.0f) /
    //   1000.0f);
    //
    //   SnowFlakesContainer[snowflakeIndex].speed =
    //       glm::vec3(0.0f, 0.0f, 0.0f); // maindir /*+ randomdir*/ * spread;
    //
    //   // // Uniform Spherical Distribution
    //   // unsigned seed =
    //   //     std::chrono::system_clock::now().time_since_epoch().count();
    //   // std::mt19937 generator(seed);
    //   // std::uniform_real_distribution<double> uniform01(0.0, 1.0);
    //   // double theta = 2 * M_PI * uniform01(generator);
    //   // double phi = acos(1 - 2 * uniform01(generator));
    //   // double x = sin(phi) * cos(theta);
    //   // double y = sin(phi) * sin(theta);
    //   // double z = cos(phi);
    //   // glm::vec3 USD(x, y, z);
    //   // ParticlesContainer[particleIndex].speed = maindir + USD * spread;
    //
    //   // random color
    //   SnowFlakesContainer[snowflakeIndex].r = 200;
    //   SnowFlakesContainer[snowflakeIndex].g = 200;
    //   SnowFlakesContainer[snowflakeIndex].b = 200;
    //   SnowFlakesContainer[snowflakeIndex].a = 1.0;
    //
    //   SnowFlakesContainer[snowflakeIndex].size =
    //       1.0f /*(rand() % 1000) / 2000.0f + 0.1f*/;
    // }

    // Simulate all particles
    // int SnowflakesCount = 0;
    // for (int i = 0; i < MaxSnowFlakes; i++) {
    //   Snowflake &s = SnowFlakesContainer[i];
    //
    //   if (s.life > 0.0f) {
    //     s.life -= deltaTime;
    //     if (s.life > 0.0f) {
    //       // s.speed = glm::vec3(0.0f, -9.81f, 0.0f) * (float)deltaTime *
    //       0.5f; s.pos += s.speed * (float)deltaTime; s.cameradistance =
    //       glm::length(s.pos - CameraPosition);
    //
    //       g_snowflake_position_size_data[4 * SnowflakesCount + 0] = s.pos.x;
    //       g_snowflake_position_size_data[4 * SnowflakesCount + 1] = s.pos.y;
    //       g_snowflake_position_size_data[4 * SnowflakesCount + 2] = s.pos.z;
    //       g_snowflake_position_size_data[4 * SnowflakesCount + 3] = s.size;
    //
    //       g_snowflake_color_data[4 * SnowflakesCount + 0] = s.r;
    //       g_snowflake_color_data[4 * SnowflakesCount + 1] = s.g;
    //       g_snowflake_color_data[4 * SnowflakesCount + 2] = s.b;
    //       g_snowflake_color_data[4 * SnowflakesCount + 3] = s.a;
    //
    //     } else {
    //       // Dead particles to sort buffer
    //       s.cameradistance = -1.0f;
    //     }
    //
    //     SnowflakesCount++;
    //   }
    // }

    // sortSnowflakes();

    // snowCount = SnowflakesCount;

    // Mise à jour des tampons qu'OpenGL utilise pour le rendu.
    // Il y a des façons bien plus sophistiquées pour envoyer des données du
    // CPU au GPU, mais c'est en dehors du champ de ce tutoriel.
    // http://www.opengl.org/wiki/Buffer_Object_Streaming

    // Same as the billboards tutorial
    glUniform3f(CameraRight_worldspace_ID, ViewProjectionMatrix[0][0],
                ViewProjectionMatrix[1][0], ViewProjectionMatrix[2][0]);
    glUniform3f(CameraUp_worldspace_ID, ViewProjectionMatrix[0][1],
                ViewProjectionMatrix[1][1], ViewProjectionMatrix[2][1]);

    glUniformMatrix4fv(ViewProjMatrixID, 1, GL_FALSE,
                       &ViewProjectionMatrix[0][0]);

    glBindBuffer(GL_ARRAY_BUFFER, snowflakes_position_buffer);
    glBufferData(GL_ARRAY_BUFFER, MaxSnowFlakes * 4 * sizeof(GLfloat), NULL,
                 GL_STREAM_DRAW); // Tampon orphelin, une méthode commune pour
                                  // améliorer les performances de streaming.
                                  // Voir le lien ci-dessus pour plus de
                                  // détails.
    glBufferSubData(GL_ARRAY_BUFFER, 0, SnowflakesCount * sizeof(GLfloat) * 4,
                    g_snowflake_position_size_data);

    glBindBuffer(GL_ARRAY_BUFFER, snowflakes_color_buffer);
    glBufferData(GL_ARRAY_BUFFER, MaxSnowFlakes * 4 * sizeof(GLubyte), NULL,
                 GL_STREAM_DRAW); // Tampon orphelin, une méthode commune pour
                                  // améliorer les performances de streaming.
                                  // Voir le lien ci-dessus pour plus de
                                  // détails.
    glBufferSubData(GL_ARRAY_BUFFER, 0, SnowflakesCount * sizeof(GLubyte) * 4,
                    g_snowflake_color_data);

    // Premier tampon d'attribut : sommets
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
    glVertexAttribPointer(
        0,        // attribut. Aucune raison précise pour 0, mais cela doit
                  // correspondre à la disposition dans le shader.
        3,        // taille
        GL_FLOAT, // type
        GL_FALSE, // normalisé ?
        0,        // nombre d'octets séparant deux sommets dans le tampon
        (void *)0 // décalage du tableau de tampon
        );

    // Second tampon d'attribut : position et centre des particules
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, snowflakes_position_buffer);
    glVertexAttribPointer(
        1,        // attribut. Aucune raison précise pour 1, mais cela doit
                  // correspondre à la disposition dans le shader.
        4,        // taille : x + y + z + taille => 4
        GL_FLOAT, // type
        GL_FALSE, // normalisé ?
        0,        // nombre d'octets séparant deux sommets dans le tampon
        (void *)0 // décalage du tableau de tampon
        );

    // 3e tampon d'attributs : couleurs des particules
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, snowflakes_color_buffer);
    glVertexAttribPointer(
        2, // attribut. Aucune raison précise pour 2, mais cela doit
           // correspondre à la disposition dans le shader.
        4, // taille : r + g + b + a => 4
        GL_UNSIGNED_BYTE, // type
        GL_TRUE,  // normalisé ? *** OUI, cela signifie que le unsigned char[4]
                  // sera accessible avec un vec4 (float) dans le shader ***
        0,        // nombre d'octets séparant deux sommets dans le tampon
        (void *)0 // décalage du tableau de tampon
        );

    glVertexAttribDivisor(
        0, 0); // particles vertices : always reuse the same 4 vertices -> 0
    glVertexAttribDivisor(1, 1); // positions : one per quad (its center) -> 1
    glVertexAttribDivisor(2, 1); // color : one per quad -> 1

    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, SnowflakesCount);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
  }

  void Cleanup() {
    glDeleteBuffers(1, &snowflakes_color_buffer);
    glDeleteBuffers(1, &snowflakes_position_buffer);
    glDeleteBuffers(1, &billboard_vertex_buffer);
    delete[] g_snowflake_position_size_data;
  }
};
