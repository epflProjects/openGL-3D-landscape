#pragma once
#include "icg_helper.h"

#define MAX_OCTAVES 10

class ScreenQuad {

    private:
        GLuint vertex_array_id_;        // vertex array object
        GLuint program_id_;             // GLSL shader program ID
        GLuint vertex_buffer_object_;   // memory buffer
        //GLuint texture_id_;             // texture ID
        GLuint permutation_texture_id_;

        float screenquad_width_;
        float screenquad_height_;

        //variables relative of fractional brownian motion realised on the frag shader.
        float fBm_H_;
        float fBm_lacunarity_;
        float fBm_exponent_array_[MAX_OCTAVES];

        int permutation_[256];

        GLuint gen_permutation_table() {
            /// Pseudo-randomly generate the permutation table.
            const int size(256);
            GLfloat permutationTable[size];
            for(int k=0; k<size; ++k)
                permutationTable[k] = k;

            /// Seed the pseudo-random generator for reproductability.
            std::srand(10);

            /// Fisher-Yates / Knuth shuffle.
            for(int k=size-1; k>0; --k) {
                GLuint idx = int(float(k) * std::rand() / RAND_MAX);
                GLfloat tmp = permutationTable[k];
                permutationTable[k] = permutationTable[idx];
                permutationTable[idx] = tmp;
            }

            /// Create the texture.
            GLuint permTableTexID;
            glGenTextures(1, &permTableTexID);
            glBindTexture(GL_TEXTURE_1D, permTableTexID);
            glTexImage1D(GL_TEXTURE_1D, 0, GL_R32F, size, 0, GL_RED, GL_FLOAT, permutationTable);
            glTexParameteri(GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

            return permTableTexID;
        }

    public:
        void Init(float screenquad_width, float screenquad_height,
                  GLuint texture) {

            // set screenquad size
            this->screenquad_width_ = screenquad_width;
            this->screenquad_height_ = screenquad_height;

            this->permutation_texture_id_ = gen_permutation_table();

            // load/Assign heightmap texture
            glBindTexture(GL_TEXTURE_1D, permutation_texture_id_);
            glActiveTexture(GL_TEXTURE0);
            GLuint perm_id = glGetUniformLocation(program_id_, "permutation_tex");
            glUniform1i(perm_id, 0 /*GL_TEXTURE2*/);
            glBindTexture(GL_TEXTURE_1D, GL_TEXTURE0);

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

            // to avoid the current object being polluted
            glBindVertexArray(0);
            glUseProgram(0);
        }

        /**
        * Precomputes the exponent array used in the fractal brownian motion
        * computation on the frag shader. And then set those values for the shader.
        * Should be called only once.
        *
        * @param H : represent 1 minus the fractal incrment.
        *            Makes the fBm smoother if close to 1, rough if close to 0
        * @param lacunarity :
        *
        */
        void fBmExponentPrecompAndSet(float H, float lacunarity){
            float frequency = 1.0f;
            for(int i = 0; i < MAX_OCTAVES; ++i){
                fBm_exponent_array_[i] = pow(frequency, -H);
                frequency *= lacunarity;
            }
            fBm_H_ = H;
            fBm_lacunarity_ = lacunarity;

            glUseProgram(program_id_);
            glBindVertexArray(vertex_array_id_);
            glUniform1f(glGetUniformLocation(program_id_, "lacunarity"),
                        this->fBm_lacunarity_);
            glUniform1f(glGetUniformLocation(program_id_, "H"),
                        this->fBm_H_);
            glUniform1fv(glGetUniformLocation(program_id_, "exponent_array"),
                        MAX_OCTAVES, this->fBm_exponent_array_);
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
