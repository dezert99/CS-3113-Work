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

class Entity {
    
public:
    Entity(float x, float y, float velocity_x, float velocity_y, float width, float height , float r =1, float g =1, float b =1, int textureID = NULL){
        this->x = x;
        this->y = y;
        this-> velocity_x = velocity_x;
        this->velocity_y = velocity_y;
        this->width = width;
        this->height = height;
        this->r = r;
        this->g = g;
        this->b = b;
    }
    void Draw(ShaderProgram &p, float elapsed){
        if(textureID == NULL){
            this -> x += velocity_x *elapsed;
            this-> y += velocity_y * elapsed;
            float vertices[] = {x,y,x+width,y,x+width,y+height, x,y,x+width,y+height,x,y+height};
            p.SetColor(r, g, b, 1);
            glUseProgram(p.programID);
            glVertexAttribPointer(p.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
            glEnableVertexAttribArray(p.positionAttribute);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glDisableVertexAttribArray(p.positionAttribute);
        }
        else{
            this -> x += velocity_x *elapsed;
            this-> y += velocity_y * elapsed;
            float vertices[] = {x,y,x+width,y,x+width,y+height, x,y,x+width,y+height,x,y+height};
            p.SetColor(r, g, b, 1);
            glUseProgram(p.programID);
            glVertexAttribPointer(p.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
            glEnableVertexAttribArray(p.positionAttribute);
            float texCoords[] = {0.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 0.0};
            glVertexAttribPointer(p.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
            glEnableVertexAttribArray(p.texCoordAttribute);
            glDrawArrays(GL_TRIANGLES, 0, 6);
            glDisableVertexAttribArray(p.positionAttribute);
            glDisableVertexAttribArray(p.texCoordAttribute);
        }
    }
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    float x;
    float y;
    float rotation;
    int textureID;
    float width;
    float height;
    float velocity_x;
    float velocity_y;
    float r;
    float g;
    float b;
};
class SheetSprite {
public:
    SheetSprite(unsigned int textureID, float u, float v, float width, float height, float size){
        this -> textureID = textureID;
        this -> u = u;
        this -> v = v;
        this -> width = width;
        this -> height = height;
        this -> size = size;
    }
    void Draw(ShaderProgram &program);
    float size;
    unsigned int textureID;
    float u;
    float v;
    float width;
    float height;
};
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


void SheetSprite::Draw(ShaderProgram &program) {
    glBindTexture(GL_TEXTURE_2D, textureID);
    GLfloat texCoords[] = {
        u, v+height,
        u+width, v,
        u, v,
        u+width, v,
        u, v+height,
        u+width, v+height
    };
    float aspect = width / height;
    float vertices[] = {
        -0.5f * size * aspect, -0.5f * size,
        0.5f * size * aspect, 0.5f * size,
        -0.5f * size * aspect, 0.5f * size,
        0.5f * size * aspect, 0.5f * size,
        -0.5f * size * aspect, -0.5f * size ,
        0.5f * size * aspect, -0.5f * size};
    // draw our arrays
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoords);
    glEnableVertexAttribArray(program.texCoordAttribute);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}




//GLobals
glm::mat4 projectionMatrix = glm::mat4(1.0f);
SDL_Event event;
bool done = false;
glm::mat4 modelMatrix = glm::mat4(1.0f);
glm::mat4 viewMatrix = glm::mat4(1.0f);
float lastFrameTicks = 0.0f;
ShaderProgram untexteredShader;
float elapsed;
GLuint spriteSheetTexture;
//GLuint spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"house.png");
//SheetSprite mySprite = SheetSprite(spriteSheetTexture, 425.0f/1024.0f, 468.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, 0.2f);
ShaderProgram texteredShader;

SDL_Window* displayWindow;

void Setup(){
    SDL_Init(SDL_INIT_VIDEO);
    displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 360, SDL_WINDOW_OPENGL);
    SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
    SDL_GL_MakeCurrent(displayWindow, context);
    #ifdef _WINDOWS
        glewInit();
    #endif
    glViewport(0, 0, 640, 360);
    projectionMatrix = glm::ortho(-1.77f,1.77f, -1.0f, 1.0f, -1.0f, 1.0f); //Sets up orthographic projection.Ratio of 16:9
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    texteredShader.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
    texteredShader.SetViewMatrix(viewMatrix);
    texteredShader.SetProjectionMatrix(projectionMatrix);
    spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"sheet.png");
//
    
}
void ProcessEvents(){
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
            done = true;
        }
    }
}
void Update(){
    glClear(GL_COLOR_BUFFER_BIT);
    float ticks = (float)SDL_GetTicks()/1000.0f;
    elapsed = ticks - lastFrameTicks;
    lastFrameTicks = ticks;
    modelMatrix = glm::mat4(1.0f);

}
void Render(Entity mySprite){
    mySprite.Draw(texteredShader,.1);
}
void Cleanup(){
    
}
//"../Assets/house.png"
int main(int argc, char *argv[])
{
    
    Setup();
    SheetSprite mySprite = SheetSprite(spriteSheetTexture, 425.0f/1024.0f, 468.0f/1024.0f, 93.0f/1024.0f, 84.0f/1024.0f, 0.2f);
    Entity test = Entity(0, 0, 0, 0, 50, 50,1,1,1,spriteSheetTexture);
    while (!done) {
        ProcessEvents();
        Update();
        Render(test);
        SDL_GL_SwapWindow(displayWindow);
    }
    Cleanup();
    
    SDL_Quit();
    return 0;
}
