//
//  Enemy.hpp
//  NYUCodebase
//
//  Created by Chris Martinez on 12/5/18.
//  Copyright © 2018 Ivan Safrin. All rights reserved.
//
#pragma once
#ifndef Enemy_hpp
#define Enemy_hpp

#include <stdio.h>
#include "ShaderProgram.h"
#include "SheetSprite.hpp"
#include "Entity.hpp"
#include "FlareMap.h"
#include <vector>


//float x, float y, float velocity_x, float velocity_y, float width, float height , float r =1, float g =1, float b =1, float u = -4, float v = -4, int textureID = -4, float size = -4
class Enemy : public Entity{
    // 0 = patrolling; 1 = chasing
    int state = 0;
    std::vector<int>* solids;
    FlareMap* map;
    Entity* player;
public:
    Enemy(float x, float y, float velocity_x, float velocity_y, float width, float height, float u, float v, int textureID, float size, FlareMap* map, std::vector<int>* solids, Entity* player);
    void checkState();
    void update(float elapsed);
    bool collision();
    int travelDirection = 1;
    void testingPoints();
    std::vector<bool> testPoints {false, false, false, false};
};

#endif /* Enemy_hpp */

