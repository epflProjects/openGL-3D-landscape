#pragma once
#include "icg_helper.h"
#include <glm/gtc/type_ptr.hpp>

class Snow {

private:
  struct Snowflake {
    glm::vec3 pos;
    glm::vec3 speed;
    unsigned char r, g, b, a; // COLOR
    float size;
    float angle;
    float weight;
    float life; // Remaining life of the particle. If <0: dead and unused.
    float cameradistance; // Squared distance to the camera.

    bool operator<(const Snow &that) const {
      // Sort in reverse order : far particles drawn first.
      return this->cameradistance > that.cameradistance;
    }
  };

  static const int MaxSnowFlakes = 100000;
  Snowflake SnowFlakesContainer[MaxSnowFlakes];
  int LastUsedSnowFlake = 0;

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

  void sortSnowFlakes() {
    std::sort(&SnowFlakesContainer[0], &sortSnowFlakesContainer[MaxSnowFlakes]);
  }

public:
  static GLfoat *g_snowflake_position_size_data =
      new GLfloat[MaxSnowFlakes * 4];
  static GLubyte *g_snowflake_color_data = new GLubyte[MaxSnowFlakes * 4];
  Init() {
    GLuint program_id_ = LoadShaders("snow_vshader.glsl", "snow_fshader.glsl");

    for (int i = 0; i < MaxSnowFlakes; i++) {
      SnowFlakesContainer[i].life = -1.0f;
      SnowFlakesContainer[i].cameradistance = -1.0f;
    }

    // VBO
    static const GLfloat g_vertex_buffer_data[] = {
        -0.5f, -0.5f, 0.0f, 0.05f, -0.5f, 0.0f,
        -0.5f, 0.5f,  0.0f, 0.5f,  0.5f,  0.0f,
    };
    GLuint billboard_vertex_buffer;
    glGenBuffers(1, &billboard_vertex_buffer);
    glBindBuffer(GL_ARRAY_BUFFER, billboard_vertex_buffer);
    glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data),
                 g_vertex_buffer_data, GL_STATIC_DRAW);

    // The VBO containing the positions and sizes of the particles
    GLuint snowflakes_position_buffer;
    glGenBuffers(1, &particles_position_buffer);
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

  void Draw() {
    // TODO compléter dans le main.cpp
    double currentTime = glwGetTime();
    double delta = currentTime - lastTime;
    lastTime = currentTime;

    int newsnowflakes = (int)(delta * 10000.0);
    if (newsnowflakes > (int)(0.016f * 10000.0)) {
      newsnowflakes = (int)(0.016f * 10000.0);
    }

    for (int i = 0; i < newsnowflakes; i++) {
      int snowflakeIndex = FindUnusedSnowflake();
      SnowFlakesContainer[snowflakeIndex].life =
          5.0f; // this snowflake will live 5 seconds
      SnowFlakesContainer[snowflakeIndex].pos = glm::vec3(0, 0, -20.0f);

      float spread = 1.5f;
      glm::vec3 maindir = glm::vec3(0.0f, 10.0f, 0.0f);

      // bad way to create random direction. See:
      // http://stackoverflow.com/questions/5408276/python-uniform-spherical-distribution
      glm::vec3 randomdir = glm::vec3((rand() % 2000 - 1000.0f) / 1000.0f,
                                      (rand() % 2000 - 1000.0f) / 1000.0f,
                                      (rand() % 2000 - 1000.0f) / 1000.0f);

      SnowFlakesContainer[snowflakeIndex].speed = maindir + randomdir * spread;

      SnowFlakesContainer[snowflakeIndex].r = 0;
      SnowFlakesContainer[snowflakeIndex].g = 0;
      SnowFlakesContainer[snowflakeIndex].b = 0;
      SnowFlakesContainer[snowflakeIndex].a = (rand() % 256) / 3;

      SnowFlakesContainer[snowflakeIndex].size =
          (rand() % 1000) / 2000.0f + 0.1f;
    }

    int SnowflakesCount = 0;
    for (int i = 0; i < MaxSnowFlakes; i++) {
      Snowflake &s = SnowFlakesContainer[i];

      if (s.life > 0.0f) {
        s.life -= delta;
        if (s.life > 0.0f) {
          s.speed += glm::vec3(0.0f, -9.81f, 0.0f) * (float)delta * 0.5f;
          s.pos += s.speed * (float)delta;
          s.cameradistance = glm::length2(s.pos - CameraPosition);

          g_snowflake_position_size_data[4 * SnowflakesCount + 0] = s.pos.x;
          g_snowflake_position_size_data[4 * SnowflakesCount + 1] = s.pos.y;
          g_snowflake_position_size_data[4 * SnowflakesCount + 2] = s.pos.z;
          g_snowflake_position_size_data[4 * SnowflakesCount + 3] = s.size;

          g_snowflake_color_data[4 * SnowflakesCount + 0] = s.r;
          g_snowflake_color_data[4 * SnowflakesCount + 1] = s.g;
          g_snowflake_color_data[4 * SnowflakesCount + 2] = s.b;
          g_snowflake_color_data[4 * SnowflakesCount + 3] = s.a;
        } else {
          p.cameradistance = -1.0f;
        }
        SnowflakesCount++;
      }
    }
    sortSnowFlakes();
  }
}
