#pragma once
#include "icg_helper.h"

class ScreenQuad {

    private:
        GLuint vertex_array_id_;        // vertex array object
        GLuint program_id_;             // GLSL shader program ID
        GLuint vertex_buffer_object_;   // memory buffer
        //GLuint texture_id_;             // texture ID
        GLuint permutation_texture_id_;

        float screenquad_width_;
        float screenquad_height_;

        //int permutation_[256];
        int p_[512];

        // source: http://stackoverflow.com/questions/20734774/random-array-generation-with-no-duplicates
        void shuffle(int *arr, size_t n) {
          if (n > 1) {
            size_t i;
            srand(time(NULL));
            for (i = 0; i < n - 1; i++) {
              size_t j = i + rand() / (RAND_MAX / (n - i) + 1);
              int t = arr[j];
              arr[j] = arr[i];
              arr[i] = t;
            }
          }
        }

    public:
        void Init(float screenquad_width, float screenquad_height,
                  GLuint texture) {

            // set screenquad size
            this->screenquad_width_ = screenquad_width;
            this->screenquad_height_ = screenquad_height;

            //set perumutation table
            int permutation_ [256];
            for (int i = 0; i < 256; i++) {
              permutation_[i] = (float)i;
            }

            shuffle(permutation_, sizeof(permutation_)/sizeof(int));

  //           int permutation_ [] = { 151,160,137,91,90,15,
  //  131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
  //  190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
  //  88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
  //  77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
  //  102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
  //  135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
  //  5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
  //  223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
  //  129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
  //  251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
  //  49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
  //  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180
  //  };

            for (int i=0; i < 256 ; i++) {
                this->p_[i] = permutation_[i];
                this->p_[256+i] = this->p_[i];
              }

            for (int i = 0; i < 256; i++) {
              cout << permutation_[i] << endl;
            }

            // put the p array in texture
            {
                glGenTextures(1, &permutation_texture_id_);
                glBindTexture(GL_TEXTURE_1D, permutation_texture_id_);
                glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

                // create texture for the color attachment
                // see Table.2 on
                // khronos.org/opengles/sdk/docs/man3/docbook4/xhtml/glTexImage2D.xml
                glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, screenquad_width_, screenquad_height_,
                             GL_RED, GL_UNSIGNED_INT, this->p_);
                // how to load from buffer

            }

            // compile the shaders
            program_id_ = icg_helper::LoadShaders("screenquad_vshader.glsl",
                                                  "screenquad_fshader.glsl");
            if(!program_id_) {
                exit(EXIT_FAILURE);
            }

            glUseProgram(program_id_);

            // vertex one vertex Array
            glGenVertexArrays(1, &vertex_array_id_);
            glBindVertexArray(vertex_array_id_);

            // vertex coordinates
            {
                const GLfloat vertex_point[] = { /*V1*/ -1.0f, -1.0f, 0.0f,
                                                 /*V2*/ +1.0f, -1.0f, 0.0f,
                                                 /*V3*/ -1.0f, +1.0f, 0.0f,
                                                 /*V4*/ +1.0f, +1.0f, 0.0f};
                // buffer
                glGenBuffers(1, &vertex_buffer_object_);
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_point),
                             vertex_point, GL_STATIC_DRAW);

                // attribute
                GLuint vertex_point_id = glGetAttribLocation(program_id_, "vpoint");
                glEnableVertexAttribArray(vertex_point_id);
                glVertexAttribPointer(vertex_point_id, 3, GL_FLOAT, DONT_NORMALIZE,
                                      ZERO_STRIDE, ZERO_BUFFER_OFFSET);
            }

            // texture coordinates
            {
                const GLfloat vertex_texture_coordinates[] = { /*V1*/ 0.0f, 0.0f,
                                                               /*V2*/ 1.0f, 0.0f,
                                                               /*V3*/ 0.0f, 1.0f,
                                                               /*V4*/ 1.0f, 1.0f};

                // buffer
                glGenBuffers(1, &vertex_buffer_object_);
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_);
                glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_texture_coordinates),
                             vertex_texture_coordinates, GL_STATIC_DRAW);

                // attribute
                GLuint vertex_texture_coord_id = glGetAttribLocation(program_id_,
                                                                     "vtexcoord");
                glEnableVertexAttribArray(vertex_texture_coord_id);
                glVertexAttribPointer(vertex_texture_coord_id, 2, GL_FLOAT,
                                      DONT_NORMALIZE, ZERO_STRIDE,
                                      ZERO_BUFFER_OFFSET);
            }

            // // load/Assign texture
            // this->texture_id_ = texture;
            // glBindTexture(GL_TEXTURE_2D, texture_id_);
            // GLuint tex_id = glGetUniformLocation(program_id_, "heightmap_tex");
            // glUniform1i(tex_id, 0 /*GL_TEXTURE0*/);
            // glBindTexture(GL_TEXTURE_2D, 0);
            glBindTexture(GL_TEXTURE_1D, permutation_texture_id_);
            GLuint tex_id = glGetUniformLocation(program_id_, "permutation_tex");
            glUniform1i(tex_id, 0 /*GL_TEXTURE0*/);
            glBindTexture(GL_TEXTURE_1D, 0);

            // to avoid the current object being polluted
            glBindVertexArray(0);
            glUseProgram(0);
        }

        void Cleanup() {
            glBindVertexArray(0);
            glUseProgram(0);
            glDeleteBuffers(1, &vertex_buffer_object_);
            glDeleteProgram(program_id_);
            glDeleteVertexArrays(1, &vertex_array_id_);
            //glDeleteTextures(1, &texture_id_);
            glDeleteTextures(1, &permutation_texture_id_);
        }

        void UpdateSize(int screenquad_width, int screenquad_height) {
            this->screenquad_width_ = screenquad_width;
            this->screenquad_height_ = screenquad_height;
        }

        void Draw() {
            glUseProgram(program_id_);
            glBindVertexArray(vertex_array_id_);

            // window size uniforms
            glUniform1f(glGetUniformLocation(program_id_, "tex_width"),
                        this->screenquad_width_);
            glUniform1f(glGetUniformLocation(program_id_, "tex_height"),
                        this->screenquad_height_);

            // bind texture
            glActiveTexture(GL_TEXTURE0);
            //glBindTexture(GL_TEXTURE_2D, texture_id_);
            glBindTexture(GL_TEXTURE_1D, permutation_texture_id_);

            // draw
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

            glBindVertexArray(0);
            glUseProgram(0);
        }
};
