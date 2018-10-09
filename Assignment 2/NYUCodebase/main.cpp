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
    Entity(float x, float y, float directionX, float directionY, float width, float height , float r =1, float g =1, float b =1){
        this->x = x;
        this->y = y;
        this-> direction_x = directionX;
        this->direction_y = directionY;
        this->width = width;
        this->height = height;
        this->r = r;
        this->g = g;
        this->b = b;
    }
    void Draw(ShaderProgram &p, float elapsed){
        this -> x += direction_x *elapsed;
        this-> y += direction_y * elapsed;
        float vertices[] = {x,y,x+width,y,x+width,y+height, x,y,x+width,y+height,x,y+height};
        p.SetColor(r, g, b, 1);
        glUseProgram(p.programID);
        glVertexAttribPointer(p.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(p.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(p.positionAttribute);
    }
    glm::mat4 modelMatrix = glm::mat4(1.0f);
    float x;
    float y;
    float rotation;
    int textureID;
    float width;
    float height;
    float velocity;
    float direction_x;
    float direction_y;
    //int score = 0;
    float r;
    float g;
    float b;
};
//GLobals
glm::mat4 projectionMatrix = glm::mat4(1.0f);
SDL_Event event;
bool done = false;
glm::mat4 modelMatrix = glm::mat4(1.0f);
glm::mat4 viewMatrix = glm::mat4(1.0f);
float lastFrameTicks = 0.0f;
ShaderProgram untexteredShader;
float paddleDirctionX = 0;
    float x = 0;
Entity player = Entity(1.67,-.3,0,0,.1,.6,0, 1, 0);
Entity AI = Entity(-1.77,-.3,0,0,.1,.6, 1, 0, 0);
Entity ball = Entity(-.025,-.025,.5,.5,.05,.05);
float elapsed;
int score = 0;

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


    untexteredShader.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
    player.y += player.direction_y * elapsed;
    modelMatrix = glm::translate(modelMatrix, glm::vec3(0.0f,x,0.0f));

    untexteredShader.SetModelMatrix(modelMatrix);
    untexteredShader.SetViewMatrix(viewMatrix);
    untexteredShader.SetProjectionMatrix(projectionMatrix);
    glClearColor(0, 0, 0, 1);
    const Uint8 *keys = SDL_GetKeyboardState(NULL);
    if(keys[SDL_SCANCODE_UP] && player.y+player.height <=1) {
        player.direction_y = 1;
    } else if(keys[SDL_SCANCODE_DOWN] && player.y >=-1) {
        player.direction_y = -1;
    }
    else{
        player.direction_y = 0;
    }
    if(keys[SDL_SCANCODE_W]&& AI.y+AI.height <=1){
        AI.direction_y = 1;
    }
    else if(keys[SDL_SCANCODE_S]&& AI.y >=-1){
        AI.direction_y = -1;
    }
    else{
        AI.direction_y = 0;
    }
    
    if(ball.y+ball.height >= 1){
        ball.direction_y *= -1;
    }
    else if(ball.y <= -1){
        ball.direction_y *= -1;
    }
    else if(ball.x+ball.width >= player.x && ball.y >= player.y && ball.y <= player.y+player.height|| ball.x <= AI.x+AI.width && ball.y >= AI.y && ball.y <= AI.y+AI.height){
        ball.direction_x *= -1;
    }
    
    if(ball.x+ball.width >= 1.77){
        //AI.score ++;
        ball.x = -.025;
        ball.y = -.025;
        std::cout << "AI Scores" << std::endl;
        glClearColor(1, 0, 0, 1);
    }
    else if(ball.x <= -1.77){
        //player.score++;
        ball.x = -.025;
        ball.y = -.025;
        std::cout << "Player Scores" << std::endl;
        glClearColor(0, 1, 0, 1);
    }
    

}
void Render(){
    player.Draw(untexteredShader, elapsed);
    AI.Draw(untexteredShader, elapsed);
    ball.Draw(untexteredShader, elapsed);
}
void Cleanup(){
    
}
//"../Assets/house.png"
int main(int argc, char *argv[])
{
    Setup();
    while (!done) {
        ProcessEvents();
        Update();
        Render();
        SDL_GL_SwapWindow(displayWindow);
    }
    Cleanup();
    
    SDL_Quit();
    return 0;
}
