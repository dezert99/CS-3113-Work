 #ifdef _WINDOWS
#include <GL/glew.h>
#endif
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_image.h>
#include <SDL_mixer.h>
#include <vector>
#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "FlareMap.h"
#include "Enemy.hpp"


#ifdef _WINDOWS
#define RESOURCE_FOLDER ""
#else
#define RESOURCE_FOLDER "NYUCodebase.app/Contents/Resources/"
#endif
#include "Entity.hpp"

#define FIXED_TIMESTEP 0.0166666f
#define MAX_TIMESTEPS 6

//GLobals
bool done = false;
SDL_Event event;

glm::mat4 projectionMatrix = glm::mat4(1.0f);
glm::mat4 modelMatrix = glm::mat4(1.0f);
glm::mat4 viewMatrix = glm::mat4(1.0f);

ShaderProgram untexteredShader;

GLuint spriteSheetTexture;
ShaderProgram texteredShader;
std::vector<Entity> entities;
std::vector<Enemy> enemies;
std::vector<Entity> bullets;
std::vector<Entity> healthUp;

//Game
int bulletCount = 0;
int health = 3;
int score = 0;
float hitTimer = 3.0;
int level = 1;


GLuint fontTexture;

enum GameMode { STATE_MAIN_MENU, STATE_GAME_LEVEL};
GameMode mode = STATE_MAIN_MENU;

std::vector<Entity> power;
std::vector<float> vertexData;
std::vector<float> texCoordData;

//Sprite Sheet Drawing
int sprite_count_x = 16;
int sprite_count_y = 8;
float tileSize = .1;
float scale = .1;
FlareMap map;
std::vector<int> solids = {17, 33, 34, 35, 32, 1, 3, 100, 101};

//Physics
float gravity = .017f;
bool canJump = false;
float jumpPower = .9;
int jumps = 2;

//Time
float elapsed;
float accumulator = 0.0f;
float lastFrameTicks = 0.0f;

//Sound
Mix_Chunk *hurt;
Mix_Chunk *shoot;
Mix_Chunk *jump;
Mix_Chunk *enemyHit;
Mix_Chunk *heal;
Mix_Chunk *pickup;

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

void renderEntities(){
    for(FlareMapEntity ent: map.entities){
        if(ent.type == "enemy"){
            float u = (float)((102) % sprite_count_x) / (float) sprite_count_x;
            float v = (float)((102) / sprite_count_x) / (float) sprite_count_y;
            float spriteWidth = 1.0f/(float)sprite_count_x;
            float spriteHeight = 1.0f/(float)sprite_count_y;
            SheetSprite enemy = SheetSprite(spriteSheetTexture,u, v,spriteWidth, spriteHeight, tileSize);
            enemies.push_back(Enemy(ent.x*tileSize,ent.y*-tileSize, .07,0,enemy.width,enemy.height, u, v, enemy.textureID, enemy.size, &map, &solids, &entities[0]));
        }
        else if(ent.type == "power"){
            float u = (float)((38) % sprite_count_x) / (float) sprite_count_x;
            float v = (float)((38) / sprite_count_x) / (float) sprite_count_y;
            float spriteWidth = 1.0f/(float)sprite_count_x;
            float spriteHeight = 1.0f/(float)sprite_count_y;
            SheetSprite powerSprite = SheetSprite(spriteSheetTexture,u, v,spriteWidth, spriteHeight, tileSize);
            power.push_back(Entity(ent.x*tileSize,ent.y*-tileSize,0,0,powerSprite.width,powerSprite.height,0,0,0,powerSprite.u,powerSprite.v,powerSprite.textureID, powerSprite.size));
        }
        else if(ent.type == "health"){
            float u = (float)((27) % sprite_count_x) / (float) sprite_count_x;
            float v = (float)((27) / sprite_count_x) / (float) sprite_count_y;
            float spriteWidth = 1.0f/(float)sprite_count_x;
            float spriteHeight = 1.0f/(float)sprite_count_y;
            SheetSprite healthSprite = SheetSprite(spriteSheetTexture,u, v,spriteWidth, spriteHeight, tileSize);
            healthUp.push_back(Entity(ent.x*tileSize,ent.y*-tileSize,0,0,healthSprite.width,healthSprite.height,0,0,0,healthSprite.u,healthSprite.v,healthSprite.textureID, healthSprite.size));
        }
    }
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
    gridY = (int)((entities[0].position.y - (tileSize/ 2)) / -tileSize);
    //std::cout << "gridX: " << gridX << " gridY: " << gridY << " Postion x: "<< entities[0].position.x << " Postion y: "<< entities[0].position.y <<std::endl;
    if(gridX < map.mapWidth && gridY < map.mapHeight){
        for(int solidID: solids){
            if(map.mapData[gridY][gridX] == solidID){
                if(map.mapData[gridY][gridX] == 100 || map.mapData[gridY][gridX] == 101){
                    if(hitTimer<0){
                        health--;
                        hitTimer = 3;
                        Mix_PlayChannel( -1, hurt, 0);
                    }
                }
                entities[0].collidedBottom = true;
                entities[0].position.y += fabs((-tileSize * gridY) - (entities[0].position.y - tileSize/2))+.00001;
                jumps = 2;
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
    gridY = (int)((entities[0].position.y + (tileSize / 2)) / -tileSize);
    //std::cout << "gridX: " << gridX << " gridY: " << gridY << " Postion x: "<< entities[0].position.x << " Postion y: "<< entities[0].position.y <<std::endl;
    if(gridX < map.mapWidth && gridY < map.mapHeight){
        for(int solidID: solids){
            if(map.mapData[gridY][gridX] == solidID){
                entities[0].position.y -= fabs(((-tileSize * gridY) -tileSize) - (entities[0].position.y + tileSize/2))+.001;
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
    
    gridX = (int)((entities[0].position.x - (tileSize / 2))/ tileSize);
    gridY = (int)(entities[0].position.y / -tileSize);
    //std::cout << "gridX: " << gridX << " gridY: " << gridY << " Postion x: "<< entities[0].position.x << " Postion y: "<< entities[0].position.y <<std::endl;
    if(gridX < map.mapWidth && gridY < map.mapHeight){
        for(int solidID: solids){
            if(map.mapData[gridY][gridX] == solidID){
                entities[0].position.x += fabs(((tileSize * gridX) +tileSize) - (entities[0].position.x - tileSize/2))+.001;
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
    
    gridX = (int)((entities[0].position.x + (tileSize / 2))/ tileSize);
    gridY = (int)(entities[0].position.y / -tileSize);
    //std::cout << "gridX: " << gridX << " gridY: " << gridY << " Postion x: "<< entities[0].position.x << " Postion y: "<< entities[0].position.y <<std::endl;
    if(gridX < map.mapWidth && gridY < map.mapHeight){
        if(map.mapData[gridY][gridX] == 12){
            level++;
            enemies.clear();
            power.clear();
            healthUp.clear();
            map.entities.clear();
            if(level == 2){
                map.Load(RESOURCE_FOLDER"level 2.txt");
            }
            else{
                map.Load(RESOURCE_FOLDER"level 3.txt");
                entities[0].position.x = 48/entities[0].sprite.size;
                entities[0].position.y = 30/entities[0].sprite.size;
            }
            vertexData.clear();
            texCoordData.clear();
            for(int y=0; y < map.mapHeight; y++) {
                for(int x=0; x < map.mapWidth; x++) {
                    if(map.mapData[y][x] != 0){
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
                entities[0].position.x = 48*tileSize;
                entities[0].position.y = 11*(-tileSize);
            }
            renderEntities();
        }
        for(int solidID: solids){
            if(map.mapData[gridY][gridX] == solidID){
                entities[0].position.x -= fabs(((tileSize * gridX)) - (entities[0].position.x + tileSize/2))+.001;
                return true;
            }
        }
    }
    return false;
}

bool entityCollideBottom(Entity& ent){
    int gridX =0;
    int gridY = 0;
    
    gridX = (int)(ent.position.x / tileSize);
    gridY = (int)((ent.position.y - (tileSize/ 2)) / -tileSize);
    //std::cout << "gridX: " << gridX << " gridY: " << gridY << " Postion x: "<< entities[0].position.x << " Postion y: "<< entities[0].position.y <<std::endl;
    if(gridX < map.mapWidth && gridY < map.mapHeight){
        for(int solidID: solids){
            if(map.mapData[gridY][gridX] == solidID){
                ent.collidedBottom = true;
                ent.position.y += fabs((-tileSize * gridY) - (ent.position.y - tileSize/2))+.00001;
                return true;
            }
        }
    }
    ent.collidedBottom = false;
    return false;
}
bool entityCollideTop(Entity& ent){
    int gridX =0;
    int gridY = 0;
    
    gridX = (int)(ent.position.x / tileSize);
    gridY = (int)((ent.position.y + (tileSize / 2)) / -tileSize);
    //std::cout << "gridX: " << gridX << " gridY: " << gridY << " Postion x: "<< ent.position.x << " Postion y: "<< ent.position.y <<std::endl;
    if(gridX < map.mapWidth && gridY < map.mapHeight){
        for(int solidID: solids){
            if(map.mapData[gridY][gridX] == solidID){
                ent.collidedTop = true;
                ent.position.y -= fabs(((-tileSize * gridY) -tileSize) - (ent.position.y + tileSize/2))+.001;
                return true;
            }
        }
    }
    ent.collidedTop = false;
    return false;
}
bool entityCollideLeft(Entity& ent){
    int gridX =0;
    int gridY = 0;
    
    gridX = (int)((ent.position.x - (tileSize / 2))/ tileSize);
    gridY = (int)(ent.position.y / -tileSize);
    //std::cout << "gridX: " << gridX << " gridY: " << gridY << " Postion x: "<< ent.position.x << " Postion y: "<< ent.position.y <<std::endl;
    if(gridX < map.mapWidth && gridY < map.mapHeight){
        for(int solidID: solids){
            if(map.mapData[gridY][gridX] == solidID){
                ent.collidedLeft = true;
                ent.position.x += fabs(((tileSize * gridX) +tileSize) - (ent.position.x - tileSize/2))+.001;
                return true;
            }
        }
    }
    ent.collidedLeft = false;
    return false;
}
bool entityCollideRight(Entity& ent){
    int gridX =0;
    int gridY = 0;
    
    gridX = (int)((ent.position.x + (tileSize / 2))/ tileSize);
    gridY = (int)(ent.position.y / -tileSize);
    //std::cout << "gridX: " << gridX << " gridY: " << gridY << " Postion x: "<< ent.position.x << " Postion y: "<< ent.position.y <<std::endl;
    if(gridX < map.mapWidth && gridY < map.mapHeight){
        for(int solidID: solids){
            if(map.mapData[gridY][gridX] == solidID){
                ent.collidedRight = true;
                ent.position.x -= fabs(((tileSize * gridX)) - (ent.position.x + tileSize/2))+.001;
                return true;
            }
        }
    }
    ent.collidedRight = false;
    return false;
}
// Test of test points on player
void testingPoints(){
    int gridX =0;
    int gridY = 0;
    //Bottom, bottom right, top left, top right
    std::vector<bool> testPoints = {false, false, false, false};
    //Bottom left
    gridX = (int)((entities[0].position.x -.01 - (tileSize/ 2)) / tileSize);
    gridY = (int)((entities[0].position.y -.01 - (tileSize/ 2)) / -tileSize);
    if(gridX < map.mapWidth && gridY < map.mapHeight){
        for(int solidID: solids){
            if(map.mapData[gridY][gridX] == solidID){
                //std::cout << "In" << std::endl;
                testPoints[0] = true;
                break;
            }
            
        }
    }
    
    //Bottom right
    gridX = (int)((entities[0].position.x +.01 + (tileSize/2)) / tileSize);
    gridY = (int)((entities[0].position.y  - .001 - (tileSize/ 2)) / -tileSize);
    if(gridX < map.mapWidth && gridY < map.mapHeight){
        for(int solidID: solids){
            //std::cout << "gridX: " << gridX << " gridY: " << gridY << " Postion x: "<< entities[0].position.x << " Postion y: "<< entities[0].position.y  << " Map " << map.mapData[gridY][gridX] << std::endl;
            if(map.mapData[gridY][gridX] == solidID){
                testPoints[1] = true;
                break;
            }
            
        }
    }
    
    //Top left
    gridX = (int)((entities[0].position.x -.05 - (tileSize/2)) / tileSize);
    gridY = (int)((entities[0].position.y + .01 + (tileSize/ 2)) / -tileSize);
    if(gridX < map.mapWidth && gridY < map.mapHeight){
        for(int solidID: solids){
            if(map.mapData[gridY][gridX] == solidID){
                testPoints[2] = true;
                break;
            }
            
        }
    }
    
    //Top right
    gridX = (int)((entities[0].position.x +.01 + (tileSize/2)) / tileSize);
    gridY = (int)((entities[0].position.y + .01 + (tileSize/ 2)) / -tileSize);
    if(gridX < map.mapWidth && gridY < map.mapHeight){
        for(int solidID: solids){
            if(map.mapData[gridY][gridX] == solidID){
                testPoints[3] = true;
                break;
            }
            
        }
    }
    
    
    //std::cout << testPoints[0] << " "<< testPoints[1] << " " << testPoints[2] << " " << testPoints[3] << std::endl;
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
        Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 4096 );
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
        

        map.Load(RESOURCE_FOLDER"level 1.txt");

        
        //Entity::Entity(float x, float y, float velocity_x, float velocity_y, int index , int sCountX, int sCountY, float r =1, float g =1, float b =1, float u = -4, float v = -4, int textureID = -4, float size = -4): position(x,y), velocity(velocity_x, velocity_y){
        
                for(int y=0; y < map.mapHeight; y++) {
                    for(int x=0; x < map.mapWidth; x++) {
                        if(map.mapData[y][x] != 0){
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
    }
    void ProcessEvents(){
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT || event.type == SDL_WINDOWEVENT_CLOSE) {
                done = true;
            }
            else if (event.type == SDL_KEYDOWN){
                if(event.key.keysym.scancode == SDL_SCANCODE_SPACE &&  jumps > 0) {
                    entities[0].position.y+=.02;
                    Mix_PlayChannel( -1, jump, 0);
                    entities[0].velocity.y = jumpPower;
                    jumps--;
                }
                if(event.key.keysym.scancode == SDL_SCANCODE_DOWN){
                    bullets[bulletCount].position.x = entities[0].position.x;
                    bullets[bulletCount].position.y = entities[0].position.y+.01;
                    bullets[bulletCount].velocity.y = -.5;
                    bulletCount++;
                    if(bulletCount > 29){
                        bulletCount = 0;
                    }
                    Mix_PlayChannel( -1, shoot, 0);
                }
                else if(event.key.keysym.scancode == SDL_SCANCODE_LEFT){
                    bullets[bulletCount].position.x = entities[0].position.x;
                    bullets[bulletCount].position.y = entities[0].position.y+.01;
                    bullets[bulletCount].velocity.x = -.5;
                    bulletCount++;
                    if(bulletCount > 29){
                        bulletCount = 0;
                    }
                    Mix_PlayChannel( -1, shoot, 0);
                }
                else if(event.key.keysym.scancode == SDL_SCANCODE_RIGHT){
                    bullets[bulletCount].position.x = entities[0].position.x;
                    bullets[bulletCount].position.y = entities[0].position.y+.01;
                    bullets[bulletCount].velocity.x = .5;
                    bulletCount++;
                    if(bulletCount > 29){
                        bulletCount = 0;
                    }
                    Mix_PlayChannel( -1, shoot, 0);
                }
                else if(event.key.keysym.scancode == SDL_SCANCODE_UP){
                    bullets[bulletCount].position.x = entities[0].position.x;
                    bullets[bulletCount].position.y = entities[0].position.y+.01;
                    bullets[bulletCount].velocity.y = .5;
                    bulletCount++;
                    if(bulletCount > 29){
                        bulletCount = 0;
                    }
                    Mix_PlayChannel( -1, shoot, 0);
                }
            }
        }
    }
    void Update(float elapsedUpdate = elapsed){
        glClear(GL_COLOR_BUFFER_BIT);
        
        //Movement
        modelMatrix = glm::mat4(1.0f);
        const Uint8 *keys = SDL_GetKeyboardState(NULL);
        if(keys[SDL_SCANCODE_A]) {
            entities[0].velocity.x = -.3;
        } else if(keys[SDL_SCANCODE_D]) {
            entities[0].velocity.x = .3;
        }
        else{
            entities[0].velocity.x = 0;
        }
        
        //Physics on player
        entities[0].velocity.y -= gravity;
        if(playerCollideBottom()||playerCollideTop()){
            entities[0].velocity.y = 0;
        }
        playerCollideLeft();
        playerCollideRight();
        
        //Physics on enemys
        for(Enemy& e: enemies){
            e.velocity.y -= gravity;
            if(entityCollideBottom(e)||entityCollideTop(e)){
                //std::cout << "In22" << std::endl;
               e.velocity.y = 0;
            }
            entityCollideLeft(e);
            entityCollideRight(e);
        }
        
        //entities[0].velocity.y = 0;
//        if(keys[SDL_SCANCODE_UP]) {
//            entities[0].velocity.y = 2;
//        } else if(keys[SDL_SCANCODE_DOWN]) {
//            entities[0].velocity.y = -2;
//        }
//        else{
//            entities[0].velocity.y = 0;
//        }
        for(Entity& power: power){
            if(entities[0].collision(power)){
                power.position.y = -100;
                score +=5;
                Mix_PlayChannel( -1, pickup, 0);
            }
        }
        for(Entity& h: healthUp){
            if(entities[0].collision(h)){
                h.position.y = -100;
                health +=2;
                Mix_PlayChannel( -1, heal, 0);
            }
        }
        testingPoints();
//        if(entities[0].collision(coins[0])){
//            coins[0].position.y = -100;
//        }
        
        entities[0].update(elapsedUpdate);
        for(Enemy& e: enemies){
            if(e.collision() && hitTimer < 0){
                std::cout << "Hit" << std::endl;
                health--;
                hitTimer = 3.0;
                e.position.x=-10000;
                Mix_PlayChannel( -1, hurt, 0);
            }
            e.update(elapsedUpdate);
        }
        hitTimer -= elapsedUpdate;
        for(Entity& b: bullets){
            entityCollideLeft(b);
            entityCollideRight(b);
            if(b.collidedRight || b.collidedLeft){
                b.position.x = -1000;
                b.position.y = -1000;
                b.velocity.x = 0;
                b.velocity.y = 0;
            }
            b.update(elapsedUpdate);
        }
        
        
        for(Entity& b: bullets){
            if(b.velocity.x !=0 || b.velocity.y != 0){
                for(Enemy& e: enemies){
                    if(b.collision(e)){
                        score++;
                        e.position.x=-1000;
                        b.position.x = -1000;
                        b.velocity.x = 0;
                        b.velocity.y = 0;
                        Mix_PlayChannel( -1, enemyHit, 0);
                    }
                }
            }
        }
        viewMatrix = glm::mat4(1.0f);
        viewMatrix = glm::translate(viewMatrix, glm::vec3(-entities[0].position.x,-entities[0].position.y,0.0f));
        texteredShader.SetViewMatrix(viewMatrix);
        
    }
    void Render(){
        drawMap();
        for(Entity& e: entities){
            e.Draw(texteredShader, elapsed);
        }
        for(Entity& power: power){
            power.Draw(texteredShader, elapsed);
        }
        for(Enemy& enemy: enemies){
            enemy.Draw(texteredShader, elapsed);
        }
        for(Entity& b: bullets){
            b.Draw(texteredShader, elapsed);
        }
        for(Entity& h: healthUp){
            h.Draw(texteredShader, elapsed);
        }
        DrawText(texteredShader, fontTexture, "Health: "+ std::to_string(health), entities[0].position.x-1.7,entities[0].position.y+.9,.05, .01);
        DrawText(texteredShader, fontTexture, "Score: "+ std::to_string(score), entities[0].position.x-1.7,entities[0].position.y+.8,.05, .01);

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
    SheetSprite mySprite = SheetSprite(spriteSheetTexture,u, v,spriteWidth, spriteHeight, tileSize);
    entities.push_back(Entity(49*tileSize,4*(-tileSize),0,0,mySprite.width,mySprite.height,0,0,0,mySprite.u,mySprite.v,mySprite.textureID, mySprite.size));
    renderEntities();
    //Enemy
    //float x, float y, float velocity_x, float velocity_y, float width, float height, float u, float v, int textureID, float size, FlareMap* map, std::vector<int>* Solids
    u = (float)(((int)102) % sprite_count_x) / (float) sprite_count_x;
    v = (float)(((int)102) / sprite_count_x) / (float) sprite_count_y;

//    SheetSprite enemy = SheetSprite(spriteSheetTexture,u, v,spriteWidth, spriteHeight, tileSize);
//    enemies.push_back(Enemy(2.5,-.8, .07,0,enemy.width,enemy.height, u, v, enemy.textureID, enemy.size, &map, &solids, &entities[0]));
//    enemies.push_back(Enemy(2,-.6, .07,0,enemy.width,enemy.height, u, v, enemy.textureID, enemy.size, &map, &solids, &entities[0]));
//    enemies.push_back(Enemy(1.5,-3, .07,0,enemy.width,enemy.height, u, v, enemy.textureID, enemy.size, &map, &solids, &entities[0]));
//    enemies.push_back(Enemy(3,-3, .07,0,enemy.width,enemy.height, u, v, enemy.textureID, enemy.size, &map, &solids, &entities[0]));
//    enemies.push_back(Enemy(2,-3, .07,0,enemy.width,enemy.height, u, v, enemy.textureID, enemy.size, &map, &solids, &entities[0]));
//    enemies.push_back(Enemy(1.8,-2.3, .07,0,enemy.width,enemy.height, u, v, enemy.textureID, enemy.size, &map, &solids, &entities[0]));
//    enemies.push_back(Enemy(3,-2.3, .07,0,enemy.width,enemy.height, u, v, enemy.textureID, enemy.size, &map, &solids, &entities[0]));
//    enemies.push_back(Enemy(1.2,-2.3, .07,0,enemy.width,enemy.height, u, v, enemy.textureID, enemy.size, &map, &solids, &entities[0]));
//    enemies.push_back(Enemy(2.2,-2.3, .07,0,enemy.width,enemy.height, u, v, enemy.textureID, enemy.size, &map, &solids, &entities[0]));
//    enemies.push_back(Enemy(3.5,-2.7, .07,0,enemy.width,enemy.height, u, v, enemy.textureID, enemy.size, &map, &solids, &entities[0]));
//    enemies.push_back(Enemy(2.8,-2.7, .07,0,enemy.width,enemy.height, u, v, enemy.textureID, enemy.size, &map, &solids, &entities[0]));
//    enemies.push_back(Enemy(4,-2.7, .07,0,enemy.width,enemy.height, u, v, enemy.textureID, enemy.size, &map, &solids, &entities[0]));
    
    u = (float)(((int)52) % sprite_count_x) / (float) sprite_count_x;
    v = (float)(((int)52) / sprite_count_x) / (float) sprite_count_y;
    SheetSprite coin = SheetSprite(spriteSheetTexture,u, v,spriteWidth , spriteHeight, tileSize);
    
    u = (float)(((int)21) % sprite_count_x) / (float) sprite_count_x;
    v = (float)(((int)21) / sprite_count_x) / (float) sprite_count_y;
    SheetSprite bullet = SheetSprite(spriteSheetTexture,u, v,spriteWidth , spriteHeight, tileSize-.04);
    
    for(int i = 0; i < 30; i++){
        bullets.push_back(Entity(300,300,0,0,bullet.width,bullet.height,0,0,0,bullet.u,bullet.v,bullet.textureID, bullet.size));
    }
    
//Entity::Entity(float x, float y, float velocity_x, float velocity_y, int index , int sCountX, int sCountY, float r =1, float g =1, float b =1, fint textureID = -4, float size = -4): position(x,y), velocity(velocity_x, velocity_y){
    Mix_Music *music;
    
    music = Mix_LoadMUS("bg.mp3");
    hurt = Mix_LoadWAV("hurt.wav");
    heal = Mix_LoadWAV("heal.wav");
    //Mix_Chunk *shoot;
    //Mix_Chunk *jump;
    //Mix_Chunk *enemyHit;
   // Mix_Chunk *heal;
    enemyHit = Mix_LoadWAV("hit.wav");
    shoot = Mix_LoadWAV("shoot.wav");
    pickup = Mix_LoadWAV("pickup.wav");
    jump = Mix_LoadWAV("jump.wav");
    
    Mix_VolumeMusic(30);
    Mix_PlayMusic(music, -1);
    
    while (!done && health > 0) {
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
