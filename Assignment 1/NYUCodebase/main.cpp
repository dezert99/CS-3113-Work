#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif

SDL_Window* displayWindow;

GLuint LoadTexture(const char *filePath) {
    int w,h,comp;
    unsigned char* image = stbi_load(filePath, &w, &h, &comp, STBI_rgb_alpha);
    if(image == NULL) {
        std::cout << "Unable to load image. Make sure the path is correct\n";
        assert(false);
    }
    GLuint retTexture;
    glGenTextures(1, &retTexture);
    glBindTexture(GL_TEXTURE_2D, retTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    stbi_image_free(image);
    return retTexture;
}
//"../Assets/house.png"
int main(int argc, char *argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    glViewport(0, 0, 640, 360);
    //Untextured Shader
    ShaderProgram untexteredShader;
    untexteredShader.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    
    ShaderProgram texteredShader;
    texteredShader.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    
    GLuint house = LoadTexture(RESOURCE_FOLDER"house.png");
    GLuint sun = LoadTexture(RESOURCE_FOLDER"sun.png");
    GLuint tree = LoadTexture(RESOURCE_FOLDER"tree.png");
    
    //Matrices
    glm::mat4 projectionMatrix = glm::mat4(1.0f);
    projectionMatrix = glm::ortho(-1.77f,1.77f, -1.0f, 1.0f, -1.0f, 1.0f); //Sets up orthographic projection.Ratio of 19:9
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    glm::mat4 sunModelMatrix = glm::mat4(1.0f);
    sunModelMatrix = glm::translate(sunModelMatrix, glm::vec3(-1.5f,0.75f,1.0f));
    glm::mat4 treeModelMatrix = glm::mat4(1.0f);
    treeModelMatrix = glm::translate(treeModelMatrix, glm::vec3(1.20f,0.0f,1.0f));
    glm::mat4 viewMatrix = glm::mat4(1.0f);
    glClearColor(0.2f, 0.6f, 1.0f, 1.0f);
    
    glEnable(GL_BLEND);
    
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    float lastFrameTicks = 0.0f;
    SDL_Event event;
    bool done = false;
    
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
        }
        glClear(GL_COLOR_BUFFER_BIT);
        float ticks = (float)SDL_GetTicks()/1000.0f;
        float elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        
        //Untextured
        modelMatrix = glm::mat4(1.0f);
        untexteredShader.SetModelMatrix(modelMatrix);
        untexteredShader.SetViewMatrix(viewMatrix);
        untexteredShader.SetProjectionMatrix(projectionMatrix);
        
        float vertices[] = {1.77f, -1.0f, 1.77f, -0.2f, -1.77f, -0.2f,1.77f, -1.0f,-1.77f,-1.0f, -1.77f, -0.2f};
        untexteredShader.SetColor(0.3f, 0.825f, 0.3f, 1.0f);
        glVertexAttribPointer(untexteredShader.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(untexteredShader.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(untexteredShader.positionAttribute);
        
        //Textered
        glUseProgram(texteredShader.programID);
        
        texteredShader.SetModelMatrix(modelMatrix);
        texteredShader.SetProjectionMatrix(projectionMatrix);
        texteredShader.SetViewMatrix(viewMatrix);
        glBindTexture(GL_TEXTURE_2D, house);
        float houseVertices[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5};
        glVertexAttribPointer(texteredShader.positionAttribute, 2, GL_FLOAT, false, 0, houseVertices);
        glEnableVertexAttribArray(texteredShader.positionAttribute);
        float texCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
        glVertexAttribPointer(texteredShader.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(texteredShader.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(texteredShader.positionAttribute);
        glDisableVertexAttribArray(texteredShader.texCoordAttribute);
        //Sun
        
        glBindTexture(GL_TEXTURE_2D, sun);
        
        texteredShader.SetModelMatrix(sunModelMatrix);
        float sunVertices[] = {-0.25, -0.25, 0.25, -0.25, 0.25, 0.25, -0.25, -0.25, 0.25, 0.25, -0.25, 0.25}; //{-1.77f, 1.52f, -1.52f, 1.52f, -1.52f, 1.77f, -1.77f, 1.52f, -1.52f, 1.77f, -1.77f, 1.77f} {-0.25, -0.25, 0.25, -0.25, 0.25, 0.25, -0.25, -0.25, 0.25, 0.25, -0.25, 0.25}
        float angle = elapsed * 15 * (3.14159/180);
        sunModelMatrix = glm::rotate(sunModelMatrix, angle, glm::vec3(0.0f, 0.0f, 1.0));
        glVertexAttribPointer(texteredShader.positionAttribute, 2, GL_FLOAT, false, 0, sunVertices);
        glEnableVertexAttribArray(texteredShader.positionAttribute);
        glVertexAttribPointer(texteredShader.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(texteredShader.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(texteredShader.positionAttribute);
        glDisableVertexAttribArray(texteredShader.texCoordAttribute);
        
        
        //Tree
        
        glBindTexture(GL_TEXTURE_2D, tree);
        
        texteredShader.SetModelMatrix(treeModelMatrix);
        float treeVertices[] = {-0.5, -0.5, 0.5, -0.5, 0.5, 0.5, -0.5, -0.5, 0.5, 0.5, -0.5, 0.5}; //{-1.77f, 1.52f, -1.52f, 1.52f, -1.52f, 1.77f, -1.77f, 1.52f, -1.52f, 1.77f, -1.77f, 1.77f} {-0.25, -0.25, 0.25, -0.25, 0.25, 0.25, -0.25, -0.25, 0.25, 0.25, -0.25, 0.25}
        glVertexAttribPointer(texteredShader.positionAttribute, 2, GL_FLOAT, false, 0, treeVertices);
        glEnableVertexAttribArray(texteredShader.positionAttribute);
        glVertexAttribPointer(texteredShader.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
        glEnableVertexAttribArray(texteredShader.texCoordAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(texteredShader.positionAttribute);
        glDisableVertexAttribArray(texteredShader.texCoordAttribute);
        
        SDL_GL_SwapWindow(displayWindow);
    }
    
    SDL_Quit();
    return 0;
}
