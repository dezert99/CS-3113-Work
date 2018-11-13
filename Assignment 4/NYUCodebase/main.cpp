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

#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

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
enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL};
GameMode mode = STATE_MAIN_MENU;
std::vector<Entity> coins;
std::vector<float> vertexData;
std::vector<float> texCoordData;
int sprite_count_x = 16;
int sprite_count_y = 8;
float tileSize = .1;
float scale = .1;
FlareMap map;
std::vector<int> solids;
float gravity = .01f;
float accumulator = 0.0f;
bool canJump = false;


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
void drawMap(){
    
    glUseProgram(texteredShader.programID);
    glm::mat4 mapModelMatrix = glm::mat4(1.0);
    texteredShader.SetModelMatrix(mapModelMatrix);
    glEnable(GL_BLEND);
    //glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    glVertexAttribPointer(texteredShader.positionAttribute, 2, GL_FLOAT, false, 0, vertexData.data());
    glEnableVertexAttribArray(texteredShader.positionAttribute);
    glVertexAttribPointer(texteredShader.texCoordAttribute, 2, GL_FLOAT, false, 0, texCoordData.data());
    glEnableVertexAttribArray(texteredShader.texCoordAttribute);
    glBindTexture(GL_TEXTURE_2D, spriteSheetTexture);
    glDrawArrays(GL_TRIANGLES, 0, vertexData.size()/2);
    
    glDisableVertexAttribArray(texteredShader.positionAttribute);
    glDisableVertexAttribArray(texteredShader.texCoordAttribute);
}

void worldToTileCoordinates(float worldX, float worldY, int *gridX, int *gridY) {
    *gridX = (int)(worldX / tileSize);
    *gridY = (int)(worldY / -tileSize);
}
bool playerCollideBottom(){
    int gridX =0;
    int gridY = 0;
    
    gridX = (int)(entities[0].position.x / tileSize);
    gridY = (int)((entities[0].position.y - (entities[0].height / 2)) / -tileSize);
    //std::cout << "gridX: " << gridX << " gridY: " << gridY << " Postion x: "<< entities[0].position.x << " Postion y: "<< entities[0].position.y <<std::endl;
    if(gridX < map.mapWidth && gridY < map.mapHeight){
        for(int solidID: solids){
            if(map.mapData[gridY][gridX] == solidID){
                entities[0].collidedBottom = true;
                entities[0].position.y += fabs((-tileSize * gridY) - (entities[0].position.y - entities[0].height/2))+.001;
                canJump = true;
                return true;
            }
        }
    }
    entities[0].collidedBottom = false;
    return false;
}
bool playerCollideTop(){
    int gridX =0;
    int gridY = 0;
    
    gridX = (int)(entities[0].position.x / tileSize);
    gridY = (int)((entities[0].position.y + (entities[0].height / 2)) / -tileSize);
    //std::cout << "gridX: " << gridX << " gridY: " << gridY << " Postion x: "<< entities[0].position.x << " Postion y: "<< entities[0].position.y <<std::endl;
    if(gridX < map.mapWidth && gridY < map.mapHeight){
        for(int solidID: solids){
            if(map.mapData[gridY][gridX] == solidID){
                entities[0].position.y -= fabs(((-tileSize * gridY) -tileSize) - (entities[0].position.y + entities[0].height/2))+.001;
                return true;
            }
        }
    }
    entities[0].collidedBottom = false;
    return false;
}
bool playerCollideLeft(){
    int gridX =0;
    int gridY = 0;
    
    gridX = (int)((entities[0].position.x - (entities[0].width / 2))/ tileSize);
    gridY = (int)(entities[0].position.y / -tileSize);
    std::cout << "gridX: " << gridX << " gridY: " << gridY << " Postion x: "<< entities[0].position.x << " Postion y: "<< entities[0].position.y <<std::endl;
    if(gridX < map.mapWidth && gridY < map.mapHeight){
        for(int solidID: solids){
            if(map.mapData[gridY][gridX] == solidID){
                entities[0].position.x += fabs(((tileSize * gridX) - tileSize) - (entities[0].position.x - entities[0].width/2))+.001;
                return true;
            }
        }
    }
    entities[0].collidedBottom = false;
    return false;
}
bool playerCollideRight(){
    int gridX =0;
    int gridY = 0;
    
    gridX = (int)((entities[0].position.x + (entities[0].width / 2))/ tileSize);
    gridY = (int)(entities[0].position.y / -tileSize);
    std::cout << "gridX: " << gridX << " gridY: " << gridY << " Postion x: "<< entities[0].position.x << " Postion y: "<< entities[0].position.y <<std::endl;
    if(gridX < map.mapWidth && gridY < map.mapHeight){
        for(int solidID: solids){
            if(map.mapData[gridY][gridX] == solidID){
                entities[0].position.x -= fabs(((tileSize * gridX) + tileSize) - (entities[0].position.x + entities[0].width/2))+.001;
                return true;
            }
        }
    }
    return false;
}
class MainMenu {
public:
    void Render() {
        DrawText(texteredShader, fontTexture, "Jumpy bug, by Christopher Martinez", -1.12,0,0.05, .01);
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
        //glClearColor(.364705, .737254, .823529, 1);
        

        map.Load(RESOURCE_FOLDER"Platformer 2.txt");

        
        //Entity::Entity(float x, float y, float velocity_x, float velocity_y, int index , int sCountX, int sCountY, float r =1, float g =1, float b =1, float u = -4, float v = -4, int textureID = -4, float size = -4): position(x,y), velocity(velocity_x, velocity_y){
        
                for(int y=0; y < map.mapHeight; y++) {
                    for(int x=0; x < map.mapWidth; x++) {
                        
                        if(map.mapData[y][x] != 0 && map.mapData[y][x] != 12){
                            float u = (float)(((int)map.mapData[y][x]) % sprite_count_x) / (float) sprite_count_x;
                            float v = (float)(((int)map.mapData[y][x]) / sprite_count_x) / (float) sprite_count_y;
                            float spriteWidth = 1.0f/(float)sprite_count_x;
                            float spriteHeight = 1.0f/(float)sprite_count_y;
                            vertexData.insert(vertexData.end(), {
                                tileSize * x, -tileSize * y,
                                tileSize * x, (-tileSize * y)-tileSize,
                                (tileSize * x)+tileSize, (-tileSize * y)-tileSize,
                                tileSize * x, -tileSize * y,
                                (tileSize * x)+tileSize, (-tileSize * y)-tileSize,
                                (tileSize * x)+tileSize, -tileSize * y
                            });
                            texCoordData.insert(texCoordData.end(), {
                                u, v,
                                u, v+(spriteHeight),
                                u+spriteWidth, v+(spriteHeight),
                                u, v,
                                u+spriteWidth, v+(spriteHeight),
                                u+spriteWidth, v
                            });
                        }
                    }
                }
        //Setup solids
        solids.push_back(17);

    }
    void ProcessEvents(){
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
            else if (event.type == SDL_KEYDOWN){
                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE && canJump) {
                    entities[0].velocity.y = .8;
                    canJump = false;
                }
            }
        }
    }
    void Update(float elapsedUpdate = elapsed){
        glClear(GL_COLOR_BUFFER_BIT);
        
        modelMatrix = glm::mat4(1.0f);
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        if(keys[SDL_SCANCODE_LEFT]) {
            entities[0].velocity.x = -.3;
        } else if(keys[SDL_SCANCODE_RIGHT]) {
            entities[0].velocity.x = .3;
        }
        else{
            entities[0].velocity.x = 0;
        }
        //std::cout << playerCollideBottom() << std::endl;
        entities[0].velocity.y -= gravity;
        if(playerCollideBottom()||playerCollideTop()){
            entities[0].velocity.y = 0;
        }
        playerCollideLeft();
        //entities[0].velocity.y = 0;
//        if(keys[SDL_SCANCODE_UP]) {
//            entities[0].velocity.y = 2;
//        } else if(keys[SDL_SCANCODE_DOWN]) {
//            entities[0].velocity.y = -2;
//        }
//        else{
//            entities[0].velocity.y = 0;
//        }
        for(Entity& coin: coins){
            if(entities[0].collision(coin)){
                coin.position.y = -100;
            }
        }
//        if(entities[0].collision(coins[0])){
//            coins[0].position.y = -100;
//        }
        
        entities[0].update(elapsedUpdate);
        viewMatrix = glm::mat4(1.0f);
        viewMatrix = glm::translate(viewMatrix, glm::vec3(-entities[0].position.x,-entities[0].position.y,0.0f));
        texteredShader.SetViewMatrix(viewMatrix);
        
    }
    void Render(){
        DrawText(texteredShader, fontTexture, "Score: "+std::to_string(score), -1.73,.95,.05, .01);
        drawMap();
        for(Entity& e: entities){
            e.Draw(texteredShader, elapsed);
        }
        for(Entity& coin: coins){
            coin.Draw(texteredShader, elapsed);
        }

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
void Update(float elapsedUpdate = elapsed){
    switch(mode){
        case STATE_MAIN_MENU:
            menu.Update();
            break;
        case STATE_GAME_LEVEL:
            game.Update(elapsedUpdate);
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
    float u = (float)(((int)80) % sprite_count_x) / (float) sprite_count_x;
    float v = (float)(((int)80) / sprite_count_x) / (float) sprite_count_y;
    float spriteWidth = 1.0f/(float)sprite_count_x;
    float spriteHeight = 1.0f/(float)sprite_count_y;
    SheetSprite mySprite = SheetSprite(spriteSheetTexture,u, v,spriteWidth , spriteHeight, tileSize);
    u = (float)(((int)52) % sprite_count_x) / (float) sprite_count_x;
    v = (float)(((int)52) / sprite_count_x) / (float) sprite_count_y;
    SheetSprite coin = SheetSprite(spriteSheetTexture,u, v,spriteWidth , spriteHeight, tileSize);

//Entity::Entity(float x, float y, float velocity_x, float velocity_y, int index , int sCountX, int sCountY, float r =1, float g =1, float b =1, fint textureID = -4, float size = -4): position(x,y), velocity(velocity_x, velocity_y){
    entities.push_back(Entity(0,-.8,-.1,0,mySprite.width,mySprite.height,0,0,0,mySprite.u,mySprite.v,mySprite.textureID, mySprite.size));
    coins.push_back(Entity(.4,-1.25,-.1,0,coin.width,coin.height,0,0,0,coin.u,coin.v,coin.textureID, coin.size));
    coins.push_back(Entity(1.338,-1.14,-.1,0,coin.width,coin.height,0,0,0,coin.u,coin.v,coin.textureID, coin.size));
    while (!done) {
        float ticks = (float)SDL_GetTicks()/1000.0f;
        elapsed = ticks - lastFrameTicks;
        lastFrameTicks = ticks;
        elapsed += accumulator;
        if(elapsed < FIXED_TIMESTEP) {
            accumulator = elapsed;
            continue; }
        while(elapsed >= FIXED_TIMESTEP) {
            Update(FIXED_TIMESTEP);
            elapsed -= FIXED_TIMESTEP;
        }
        accumulator = elapsed;

        ProcessEvents();
        Update();
        Render();
        SDL_GL_SwapWindow(displayWindow);
    }
    Cleanup();
    
    SDL_Quit();
    return 0;
}
