//
//  Entity.hpp
//  NYUCodebase
//
//  Created by Chris Martinez on 10/17/18.
//  Copyright © 2018 Ivan Safrin. All rights reserved.
//

#ifndef Entity_hpp
#define Entity_hpp

#include <stdio.h>
#include "ShaderProgram.h"
#include "SheetSprite.hpp"

#endif /* Entity_hpp */
struct vec2 {
    vec2(float x, float y): x(x), y(y){}
    float x;
    float y;
};
class Entity {
    
public:
    Entity(float x, float y, float velocity_x, float velocity_y,int index , int sCountX, int sCountY, float r, float g, float b, int textureID, float size);
    void Draw(ShaderProgram &p, float elapsed);
    void update(float elapsed);
    bool collision(Entity &e);
    vec2 position;
    float rotation;
    int textureID;
    SheetSprite sprite;
    float width;
    float height;
    int index;
    int sCountX;
    int sCountY;
    vec2 velocity;
    float r;
    float g;
    float b;
};
