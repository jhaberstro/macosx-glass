//
//  main.cpp
//  macosx-glass
//
//  Created by Jedd Haberstro on 5/17/11.
//  Copyright 2011 Student. All rights reserved.
//

#include <SFML/Window.hpp>
#include "mesh.hpp"
#include "utilities.h"
#include "openctm.h"

#include "glm/glm.hpp"
#include "glm/ext.hpp"

#include "pez/pez.h"

enum
{
    WINDOW_WIDTH = PEZ_VIEWPORT_WIDTH,
    WINDOW_HEIGHT = PEZ_VIEWPORT_HEIGHT
};

enum
{
    ATTRIB_VERTEX,
    ATTRIB_TEXCOORD,
    ATTRIB_NORMAL,
    NUM_ATTRIBUTES
};


GLuint thickness_program;
GLuint quad_program;
GLuint normal_program;
GLuint fbo;
GLuint depth_texture;
GLuint quad_vbo;
float rotation = 0.0f;
mesh* dragon_mesh;


void create_fbo(GLuint& fbo, GLuint& depth_texture) {
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    
    glGenTextures(1, &depth_texture);
    glBindTexture(GL_TEXTURE_2D, depth_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F_ARB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_FLOAT, 0);
    //glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
    assert(GL_NO_ERROR == glGetError());
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, depth_texture, 0);
    
    GLuint depthRenderbuffer;
    glGenRenderbuffers(1, &depthRenderbuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, depthRenderbuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT16, WINDOW_WIDTH, WINDOW_HEIGHT);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthRenderbuffer);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if(status != GL_FRAMEBUFFER_COMPLETE) {
        printf("failed to make complete framebuffer object %x", status);
        //assert(false);
    }
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
}

GLuint create_quad(float left, float bottom, float right, float top)
{
    float quad[] = {
        left, bottom, 0, 0,
        left, top, 0, 1,
        right, top, 1, 1,
        right, top, 1, 1,
        right, bottom, 1, 0,
        left, bottom, 0, 0
    };
    
    GLuint handle;
    glGenBuffers(1, &handle);
    glBindBuffer(GL_ARRAY_BUFFER, handle);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), &quad[0], GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    
    return handle;
}

void draw_quad(GLuint vbo, GLuint pos_attrib) {
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(ATTRIB_VERTEX);
    glEnableVertexAttribArray(ATTRIB_TEXCOORD);
    glVertexAttribPointer(ATTRIB_VERTEX, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, 0);
    glVertexAttribPointer(ATTRIB_TEXCOORD, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 4, (GLvoid*)(sizeof(float) * 2));
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    glDisableVertexAttribArray(ATTRIB_VERTEX);
    glDisableVertexAttribArray(ATTRIB_TEXCOORD);
}

void draw_lighted_mesh(mesh* mesh, glm::mat4 const& mv, glm::mat4 const& p) {
    glUseProgram(normal_program);
    
    glm::mat3 normal_matrix = glm::transpose(glm::inverse(glm::mat3(mv)));
    
    int modelview_uniform = glGetUniformLocation(normal_program, "modelview");
    int projection_uniform = glGetUniformLocation(normal_program, "projection");
    int normal_matrix_uniform = glGetUniformLocation(normal_program, "normal_matrix");
    int light_pos_uniform = glGetUniformLocation(normal_program, "light_pos");
    int diffuse_uniform = glGetUniformLocation(normal_program, "diffuse");
    int ambient_uniform = glGetUniformLocation(normal_program, "ambient");
    int specular_uniform = glGetUniformLocation(normal_program, "specular");
    int shininess_uniform = glGetUniformLocation(normal_program, "shininess");
    
    glUniformMatrix4fv(modelview_uniform, 1, GL_FALSE, &mv[0][0]);
    glUniformMatrix4fv(projection_uniform, 1, GL_FALSE, &p[0][0]);
    glUniformMatrix3fv(normal_matrix_uniform, 1, GL_FALSE, &normal_matrix[0][0]);
    glUniform3f(light_pos_uniform, 0.25f, 0.25f, 1.0f);
    glUniform3f(diffuse_uniform, 0.0f, 0.45f, 0.75f);
    glUniform3f(ambient_uniform, 0.04f, 0.04f, 0.04f);
    glUniform3f(specular_uniform, 0.5f, 0.5f, 0.5f);
    glUniform1f(shininess_uniform, 50.0f);
    
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    
    draw_mesh(mesh, ATTRIB_VERTEX, ATTRIB_NORMAL);  
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glDisable(GL_DEPTH_TEST);
    glDisableVertexAttribArray(ATTRIB_VERTEX);
    glDisableVertexAttribArray(ATTRIB_NORMAL);
}

void initialize() {    
    attribute thickness_attribs[] = { { ATTRIB_VERTEX, "position" }, { ATTRIB_NORMAL, "normal" } };
    thickness_program = load_shader("thickness.vsh", "thickness.fsh", thickness_attribs, 2);

    attribute quad_attribs[] = { { ATTRIB_VERTEX, "position" }, { ATTRIB_TEXCOORD, "tex_coord" } };
    quad_program = load_shader("quad.vsh", "quad.fsh", quad_attribs, 2);
    
    attribute normal_attribs[] = { { ATTRIB_VERTEX, "position" }, { ATTRIB_NORMAL, "normal" } };
    normal_program = load_shader("normal.vsh", "normal.fsh", normal_attribs, 2);
    
    create_fbo(fbo, depth_texture);
    quad_vbo = create_quad(-1.0f, -1.0f, 1.0f, 1.0f);
    dragon_mesh = load_mesh_ctm("dragon.ctm");        
}

void draw() {
    float x = 0.6f;
    float y = x * WINDOW_HEIGHT / WINDOW_WIDTH;
    glm::mat4 perspective = glm::frustum(-x, x, -y, y, 1.0f, 100.0f);
    glm::mat4 modelview = glm::translate(0.0f, 0.1f, -2.0f);
    modelview = glm::rotate(modelview, 25.0f + rotation, 0.0f, 1.0f, 0.0f);
    modelview = glm::rotate(modelview, 270.0f, 1.0f, 0.0f, 0.0f);
    
    // Draw depth into depth_texture
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    glUseProgram(thickness_program);
    int modelview_uniform = glGetUniformLocation(thickness_program, "modelview");
    int projection_uniform = glGetUniformLocation(thickness_program, "projection");
    glUniformMatrix4fv(modelview_uniform, 1, GL_FALSE, &modelview[0][0]);
    glUniformMatrix4fv(projection_uniform, 1, GL_FALSE, &perspective[0][0]);
    
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    draw_mesh(dragon_mesh, ATTRIB_VERTEX, ATTRIB_NORMAL);
    glDisable(GL_BLEND);
    //save_framebuffer("out.raw");
    
    
    // Render full screen quad
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // 0 is the default buffer
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(quad_program);
    int depth_texture_uniform = glGetUniformLocation(quad_program, "depth_texture");
    int color_uniform = glGetUniformLocation(quad_program, "color");

    glEnable(GL_TEXTURE_2D);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, depth_texture);
    glUniform1i(depth_texture_uniform, 0);
    glUniform3f(color_uniform, 73.0f / 255.0f, 174.0f / 255.0f, 255.0f / 255.0f);
    
    draw_quad(quad_vbo, ATTRIB_VERTEX);
    glBindTexture(GL_TEXTURE_2D, 0);
}

const char* PezInitialize(int width, int height) {
    initialize();
    return "Glass";
}

void PezRender() {
    draw();
}

void PezUpdate(unsigned int milliseconds) {
    rotation += 0.25f;
}

void PezHandleMouse(int x, int y, int action) { }

