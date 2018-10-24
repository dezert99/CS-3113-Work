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

class Entity {
    
public:
    Entity(float x, float y, float velocity_x, float velocity_y, float width, float height , float r, float g, float b, float u, float v , int textureID, float size);
    void Draw(ShaderProgram &p, float elapsed);
    void update(float elapsed);
    bool collision(Entity &e);
    float x;
    float y;
    float rotation;
    int textureID;
    SheetSprite sprite;
    float width;
    float height;
    float velocity_x;
    float velocity_y;
    float r;
    float g;
    float b;
};
