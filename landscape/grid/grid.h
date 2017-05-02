#pragma once
#include "icg_helper.h"
#include <glm/gtc/type_ptr.hpp>

using namespace std;

enum GridType {
    TERRAIN,
    WATER
};

class Grid {

    private:
        GLuint vertex_array_id_;                // vertex array object
        GLuint vertex_buffer_object_position_;  // memory buffer for positions
        GLuint vertex_buffer_object_index_;     // memory buffer for indices
        GLuint program_id_;                     // GLSL shader program ID
        GLuint heightmap_tex_id_;
        GLuint grass_texture_id_;
        GLuint rock_texture_id_;
        GLuint sand_texture_id_;
        GLuint snow_texture_id_;
        GLuint num_indices_;                    // number of vertices to render
        GLuint MVP_id_;                         // model, view, proj matrix ID

        void loadTexture(string filename, GLuint* texture_id, string texture_name_shader, int texture_number) {
            int width;
            int height;
            int nb_component;
            // set stb_image to have the same coordinates as OpenGL
            stbi_set_flip_vertically_on_load(1);
            unsigned char* image = stbi_load(filename.c_str(), &width,
                                             &height, &nb_component, 0);

            if (image == nullptr) {
                throw(string("Failed to load texture"));
            }

            glGenTextures(1, texture_id);
            glBindTexture(GL_TEXTURE_2D, *texture_id);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

            if (nb_component == 3) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                             GL_RGB, GL_UNSIGNED_BYTE, image);
            } else if (nb_component == 4) {
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0,
                           GL_RGBA, GL_UNSIGNED_BYTE, image);
            }

            GLuint tex_id = glGetUniformLocation(program_id_, texture_name_shader.c_str());
            glUniform1i(tex_id, texture_number - GL_TEXTURE0);

            // cleanup
            glBindTexture(GL_TEXTURE_2D, 0);
            stbi_image_free(image);
        }

    public:
        void Init(GLuint heightmap_tex_id, enum GridType gT) {
            // compile the shaders.
            string prefix;
            switch(gT){
                case TERRAIN:
                    prefix = "terrain";
                    break;
                case WATER:
                    prefix = "water";
                    break;
                default:
                    prefix = "terrain";
            }
            program_id_ = icg_helper::LoadShaders((prefix + "_vshader.glsl").c_str(),
                                                  (prefix + "_fshader.glsl").c_str());
            if(!program_id_) {
                exit(EXIT_FAILURE);
            }

            glUseProgram(program_id_);



            // vertex one vertex array
            glGenVertexArrays(1, &vertex_array_id_);
            glBindVertexArray(vertex_array_id_);

            // vertex coordinates and indices
            {
                std::vector<GLfloat> vertices;
                std::vector<GLuint> indices;
                // make a triangle grid with dimension 1024x1024.
                // always two subsequent entries in 'vertices' form a 2D vertex position.
                int grid_dim = 1024;

                // the given code below are the vertices for a simple quad.
                // your grid should have the same dimension as that quad, i.e.,
                // reach from [-1, -1] to [1, 1].

                // vertex position of the triangles.
                for(int i = 0; i < grid_dim; ++i) {
                    for(int j = 0; j < grid_dim; ++j) {
                        // map the index to the position [-1, 1]
                        float x = 2.0/(grid_dim-1.0) * i - 1;
                        float y = 2.0/(grid_dim-1.0) * j - 1;
                        vertices.push_back(x); vertices.push_back(y);
                    }
                }

                // and indices.
                indices.push_back(0);
                indices.push_back(grid_dim);
                size_t idx = 1;
                int increment = 1;
                while(idx < (grid_dim * grid_dim - grid_dim)){
                    if(idx % grid_dim == 0){
                        if(increment < 0){
                            indices.push_back(idx);
                        }
                        idx += grid_dim;
                        increment = -increment;
                        if(increment < 0){
                            idx--;
                        }
                    }

                    indices.push_back(idx);
                    indices.push_back(idx+grid_dim);

                    idx = idx + increment;
                }


                num_indices_ = indices.size();

                // position buffer
                glGenBuffers(1, &vertex_buffer_object_position_);
                glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer_object_position_);
                glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(GLfloat),
                             &vertices[0], GL_STATIC_DRAW);

                // vertex indices
                glGenBuffers(1, &vertex_buffer_object_index_);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vertex_buffer_object_index_);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
                             &indices[0], GL_STATIC_DRAW);

                // position shader attribute
                GLuint loc_position = glGetAttribLocation(program_id_, "position");
                glEnableVertexAttribArray(loc_position);
                glVertexAttribPointer(loc_position, 2, GL_FLOAT, DONT_NORMALIZE,
                                      ZERO_STRIDE, ZERO_BUFFER_OFFSET);
            }

            // load texture
            loadTexture("grass_texture.tga", &grass_texture_id_, "grass_tex", GL_TEXTURE0+1);
            loadTexture("sand_texture.tga", &sand_texture_id_, "sand_tex", GL_TEXTURE0+2);
            loadTexture("snow_texture.tga", &snow_texture_id_, "snow_tex", GL_TEXTURE0+3);
            loadTexture("rock_texture.tga", &rock_texture_id_, "rock_tex", GL_TEXTURE0+4);

            // heightmap texture
            this->heightmap_tex_id_ = heightmap_tex_id;
            glBindTexture(GL_TEXTURE_2D, this->heightmap_tex_id_);
            GLuint heightmap_id = glGetUniformLocation(program_id_, "heightmap_tex");
            glUniform1i(heightmap_id, 0 /*GL_TEXTURE0*/);
            glBindTexture(GL_TEXTURE_2D, GL_TEXTURE0);

            // other uniforms
            MVP_id_ = glGetUniformLocation(program_id_, "MVP");

            // to avoid the current object being polluted
            glBindVertexArray(0);
            glUseProgram(0);
        }

        void Cleanup() {
            glBindVertexArray(0);
            glUseProgram(0);
            glDeleteBuffers(1, &vertex_buffer_object_position_);
            glDeleteBuffers(1, &vertex_buffer_object_index_);
            glDeleteVertexArrays(1, &vertex_array_id_);
            glDeleteProgram(program_id_);
            glDeleteTextures(1, &heightmap_tex_id_);
            glDeleteTextures(1, &grass_texture_id_);
            glDeleteTextures(1, &sand_texture_id_);
            glDeleteTextures(1, &snow_texture_id_);
            glDeleteTextures(1, &rock_texture_id_);
        }

        void Draw(float time, const glm::mat4 &model = IDENTITY_MATRIX,
                  const glm::mat4 &view = IDENTITY_MATRIX,
                  const glm::mat4 &projection = IDENTITY_MATRIX) {
            glUseProgram(program_id_);
            glBindVertexArray(vertex_array_id_);

            // bind textures
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, heightmap_tex_id_);

            glActiveTexture(GL_TEXTURE0+1);
            glBindTexture(GL_TEXTURE_2D, grass_texture_id_);

            glActiveTexture(GL_TEXTURE0+2);
            glBindTexture(GL_TEXTURE_2D, sand_texture_id_);

            glActiveTexture(GL_TEXTURE0+3);
            glBindTexture(GL_TEXTURE_2D, snow_texture_id_);

            glActiveTexture(GL_TEXTURE0+4);
            glBindTexture(GL_TEXTURE_2D, rock_texture_id_);

            // setup MVP
            glm::mat4 MVP = projection*view*model;
            glUniformMatrix4fv(MVP_id_, ONE, DONT_TRANSPOSE, glm::value_ptr(MVP));

            // pass the current time stamp to the shader.
            glUniform1f(glGetUniformLocation(program_id_, "time"), time);

            // draw
            glDrawElements(GL_TRIANGLE_STRIP, num_indices_, GL_UNSIGNED_INT, 0);

            glBindVertexArray(0);
            glUseProgram(0);
        }
};
