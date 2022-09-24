//
// Created by William Ozeas on 9/24/22.
//

#ifndef INC_15_466_F22_GAME4_TEXTRENDERER_HPP
#define INC_15_466_F22_GAME4_TEXTRENDERER_HPP

#include <string>
#include <glm/glm.hpp>

struct TextRenderer {
    TextRenderer() = default;
    void RenderText(std::string str, glm::vec2 position);
};
#endif //INC_15_466_F22_GAME4_TEXTRENDERER_HPP
