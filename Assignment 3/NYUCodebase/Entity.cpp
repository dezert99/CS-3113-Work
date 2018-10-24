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
    if(sprite.u == -4){
        std::cout << "Untextered";
        float vertices[] = {x,y,x+width,y,x+width,y+height, x,y,x+width,y+height,x,y+height};
        p.SetColor(r, g, b, 1);
        glm::mat4 modelMatrix = glm::mat4(1.0f);
        p.SetModelMatrix(modelMatrix);
        glUseProgram(p.programID);
        glVertexAttribPointer(p.positionAttribute, 2, GL_FLOAT, false, 0, vertices);
        glEnableVertexAttribArray(p.positionAttribute);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        glDisableVertexAttribArray(p.positionAttribute);
        }
        else{
            sprite.Draw(p, x, y);
        }
    }
Entity::Entity(float x, float y, float velocity_x, float velocity_y, float width, float height , float r =1, float g =1, float b =1, float u = -4, float v = -4, int textureID = -4, float size = -4){
    this->x = x;
    this->y = y;
    this-> velocity_x = velocity_x;
    this->velocity_y = velocity_y;
    this->width = width;
    this->height = height;
    this->r = r;
    this->g = g;
    this->b = b;
    this -> sprite = SheetSprite(textureID, u, v ,width, height, size);
}
void Entity::update(float elapsed){
    x += velocity_x *elapsed;
    y += velocity_y * elapsed;
}
bool Entity::collision(Entity &e){
    float x_distance = abs(e.x-x)-((e.width+width));
    float y_distance = abs(e.y-y)-((e.height+height));
    if(x_distance< 0 && y_distance < 0){
        return true;
    }
    return false;
}
