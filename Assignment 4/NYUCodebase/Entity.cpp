//
//  Entity.cpp
//  NYUCodebase
//
//  Created by Chris Martinez on 10/17/18.
//  Copyright © 2018 Ivan Safrin. All rights reserved.
//

#include "Entity.hpp"
#include "ShaderProgram.h"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"

void Entity::Draw(ShaderProgram &p, float elapsed){
    
    sprite.Draw(p, index, sCountX, sCountY, position.x, position.y);
}
Entity::Entity(float x, float y, float velocity_x, float velocity_y, int index , int sCountX, int sCountY, float r =1, float g =1, float b =1, int textureID = -4, float size = -4): position(x,y), velocity(velocity_x, velocity_y){
    this-> index = index;
    this-> sCountX = sCountX;
    this -> sCountY = sCountY;
    this->r = r;
    this->g = g;
    this->b = b;
    this -> sprite = SheetSprite(textureID,width, height, size);
}
void Entity::update(float elapsed){
    position.x += velocity.x *elapsed;
    position.y += velocity.y * elapsed;
}
bool Entity::collision(Entity &e){
    float x_distance = abs(e.position.x-position.x)-((e.width+width));
    float y_distance = abs(e.position.y-position.y)-((e.height+height));
    if(x_distance< 0 && y_distance < 0){
        return true;
    }
    return false;
}
