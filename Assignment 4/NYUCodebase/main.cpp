#ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <vector>
#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "FlareMap.h"

#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
#include "Entity.hpp"

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
std::vector<Entity> entities;
GLuint fontTexture;
int score = 0;
float bulletTimer = 1;
enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL};
GameMode mode = STATE_MAIN_MENU;
std::vector<Entity> tiles;


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

void DrawText(ShaderProgram &program, int fontTexture, std::string text, float x, float y, float size, float spacing) {
    float texture_size = 1.0 / 16.0f;
    std::vector<float> vertexData;
    std::vector<float> texCoordData;
    glm::mat4 textModelMatrix = glm::mat4(1.0f);
    textModelMatrix = glm::translate(textModelMatrix, glm::vec3(x, y, 1.0f));
    for (size_t i = 0; i < text.size(); i++) {
        float texture_x = (float)(((int)text[i]) % 16) / 16.0f;
        float texture_y = (float)(((int)text[i]) / 16) / 16.0f;
        vertexData.insert(vertexData.end(), {
            ((size + spacing) * i) + (-0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (-0.5f * size), -0.5f * size,
            ((size + spacing) * i) + (0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (0.5f * size), -0.5f * size,
            ((size + spacing) * i) + (0.5f * size), 0.5f * size,
            ((size + spacing) * i) + (-0.5f * size), -0.5f * size,
        });
        texCoordData.insert(texCoordData.end(), {
            texture_x, texture_y,
            texture_x, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x + texture_size, texture_y + texture_size,
            texture_x + texture_size, texture_y,
            texture_x, texture_y + texture_size,
        });
    }
    glUseProgram(program.programID);
    program.SetModelMatrix(textModelMatrix);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glVertexAttribPointer(program.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(program.positionAttribute);
    glVertexAttribPointer(program.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(program.texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, fontTexture);
    glDrawArrays(GL_TRIANGLES, 0, text.size() * 6);
    
    glDisableVertexAttribArray(program.positionAttribute);
    glDisableVertexAttribArray(program.texCoordAttribute);
}

class MainMenu {
public:
    void Render() {
        DrawText(texteredShader, fontTexture, "Space Invaders by Chris. Space to start!", -1.12,0,0.05, .01);
    }
    void Update() {
        glClear(GL_COLOR_BUFFER_BIT);
    }
    void ProcessEvents(){
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
            else if (event.type == SDL_KEYDOWN){
                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                    mode = STATE_GAME_LEVEL;
                    lastFrameTicks = (float)SDL_GetTicks()/1000.0f;
                }
            }
        }
    }
    void CleanUp(){
        
    }
};
    

class Game{
    public:

    void Setup(){
        SDL_Init(SDL_INIT_VIDEO);
        displayWindow = SDL_CreateWindow("My Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, SDL_WINDOW_OPENGL);
        SDL_GLContext context = SDL_GL_CreateContext(displayWindow);
        SDL_GL_MakeCurrent(displayWindow, context);
        #ifdef _WINDOWS
                glewInit();
        #endif
        glViewport(0, 0, 1280, 720);
        projectionMatrix = glm::ortho(-1.77f,1.77f, -1.0f, 1.0f, -1.0f, 1.0f); //Sets up orthographic projection.Ratio of 16:9
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        texteredShader.Load(RESOURCE_FOLDER"vertex_textured.glsl", RESOURCE_FOLDER"fragment_textured.glsl");
        untexteredShader.Load(RESOURCE_FOLDER"vertex.glsl", RESOURCE_FOLDER"fragment.glsl");
        untexteredShader.SetViewMatrix(viewMatrix);
        untexteredShader.SetProjectionMatrix(projectionMatrix);
        untexteredShader.SetModelMatrix(modelMatrix);
        texteredShader.SetViewMatrix(viewMatrix);
        texteredShader.SetProjectionMatrix(projectionMatrix);
        texteredShader.SetModelMatrix(modelMatrix);
        
        spriteSheetTexture = LoadTexture(RESOURCE_FOLDER"sheet.png");
        glBindTexture(GL_TEXTURE_2D, spriteSheetTexture);
        fontTexture = LoadTexture(RESOURCE_FOLDER"pixel_font.png");
        glClearColor(.364705, .737254, .823529, 1);
        FlareMap map;

        map.Load(RESOURCE_FOLDER"Platformer 1.txt");

        
        //Entity::Entity(float x, float y, float velocity_x, float velocity_y, int index , int sCountX, int sCountY, float r =1, float g =1, float b =1, float u = -4, float v = -4, int textureID = -4, float size = -4): position(x,y), velocity(velocity_x, velocity_y){
        for(int x=0; x < map.mapWidth; x++) {
            for(int y=0; y < map.mapHeight; y++) {
                // check map.mapData[y][x] for tile index
                int index = map.mapData[y][x];
<<<<<<< HEAD
                int sprite_count_x = 16;
                int sprite_count_y = 8;
                float tileSize = .1;
                float scale = .1;
//                float u = (float)(((int)index) % sprite_count_x) / (float) sprite_count_x;
//                float v = (float)(((int)index) / sprite_count_x) / (float) sprite_count_y;
                Entity tile = Entity(x*tileSize,-y*tileSize,0,0,index, sprite_count_x, sprite_count_y,0,0,0,spriteSheetTexture,scale);
=======
                float u = (float)(((int)index) % 30) / (float) 30;
                float v = (float)(((int)index) / 30) / (float) 16;
                Entity tile = Entity(x/3.34,-y/3.34,0,0,1.0/30,1.0/30,0,0,0,u,v,spriteSheetTexture,.3);
>>>>>>> parent of c059fe0... Halp
                tiles.push_back(tile);
            }
        }


    }
    void ProcessEvents(){
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
            else if (event.type == SDL_KEYDOWN){
                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE) {
                    
                }
            }
        }
    }
    void Update(){
        glClear(GL_COLOR_BUFFER_BIT);
        float ticks = (float)SDL_GetTicks()/1000.0f;
        elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        modelMatrix = glm::mat4(1.0f);
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        if(keys[SDL_SCANCODE_LEFT]) {
            entities[0].velocity.x = -2;
        } else if(keys[SDL_SCANCODE_RIGHT]) {
            entities[0].velocity.x = 2;
        }
        else{
            entities[0].velocity.x = 0;
        }
        if(keys[SDL_SCANCODE_UP]) {
            entities[0].velocity.y = 2;
        } else if(keys[SDL_SCANCODE_DOWN]) {
            entities[0].velocity.y = -2;
        }
        else{
            entities[0].velocity.y = 0;
        }
        entities[0].update(elapsed);
        viewMatrix = glm::mat4(1.0f);
        viewMatrix = glm::translate(viewMatrix, glm::vec3(-entities[0].position.x,-entities[0].position.y,0.0f));
        texteredShader.SetViewMatrix(viewMatrix);
        for(Entity tile: tiles){
            tile.Draw(texteredShader, elapsed);
        }
        
    }
    void Render(){
        DrawText(texteredShader, fontTexture, "Score: "+std::to_string(score), -1.73,.95,.05, .01);
        entities[0].Draw(texteredShader, elapsed);
    }
    void CleanUp(){
        
    }
};

MainMenu menu;
Game game;


void Setup(){
    game.Setup();
}
void ProcessEvents(){
    switch(mode){
        case STATE_MAIN_MENU:
            menu.ProcessEvents();
            break;
        case STATE_GAME_LEVEL:
            game.ProcessEvents();
            break;
    }
}
void Update(){
    switch(mode){
        case STATE_MAIN_MENU:
            menu.Update();
            break;
        case STATE_GAME_LEVEL:
            game.Update();
            break;
    }
}
void Render(){
    switch(mode){
        case STATE_MAIN_MENU:
            menu.Render();
            break;
        case STATE_GAME_LEVEL:
            game.Render();
            break;
    }
}
void Cleanup(){
    switch(mode){
        case STATE_MAIN_MENU:
            menu.CleanUp();
            break;
        case STATE_GAME_LEVEL:
            game.CleanUp();
            break;
    }
}
//"../Assets/house.png"
int main(int argc, char *argv[])
{
    
    Setup();
<<<<<<< HEAD
=======
    SheetSprite mySprite = SheetSprite(spriteSheetTexture,.635f, .01f, 1.0/30, 1.0/16, .2f);
>>>>>>> parent of c059fe0... Halp

//Entity::Entity(float x, float y, float velocity_x, float velocity_y, int index , int sCountX, int sCountY, float r =1, float g =1, float b =1, fint textureID = -4, float size = -4): position(x,y), velocity(velocity_x, velocity_y){
    entities.push_back(Entity(0,-.8,0,0,98,16,8,0,0,0,spriteSheetTexture, .1f));
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
