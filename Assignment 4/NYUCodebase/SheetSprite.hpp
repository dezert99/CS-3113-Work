//
//  SpriteSheet.hpp
//  NYUCodebase
//
//  Created by Chris Martinez on 10/17/18.
//  Copyright © 2018 Ivan Safrin. All rights reserved.
//

#ifndef SpriteSheet_hpp
#define SpriteSheet_hpp

#include <stdio.h>
#include "ShaderProgram.h"
#endif /* SpriteSheet_hpp */

class SheetSprite {
public:
    SheetSprite(unsigned int textureID = 0, float width = -4, float height = -4, float size = -4){
        glBindTexture(GL_TEXTURE_2D, textureID);
        this -> textureID = textureID;
        this -> width = width;
        this -> height = height;
        this -> size = size;
    }
    void Draw(ShaderProgram &program, int index, int spriteCountX,
              int spriteCountY, int x, int y) const;
    //void setReconstruct(unsigned int textureID, float u, float v, float width, float height, float size);
    float size;
    unsigned int textureID;
    float width;
    float height;
};
