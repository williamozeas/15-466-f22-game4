//
// Created by William Ozeas on 9/24/22.
//

#ifndef INC_15_466_F22_GAME4_TEXTRENDERER_HPP
#define INC_15_466_F22_GAME4_TEXTRENDERER_HPP

#include <ft2build.h>
#include FT_FREETYPE_H

#include <string>
#include <glm/glm.hpp>
#include "glm/glm.hpp"
#include "GL.hpp"
#include "gl_errors.hpp"
#include "gl_compile_program.hpp"
#include <hb.h>
#include <hb-ft.h>

#define FONT_SIZE 100
#define MARGIN (FONT_SIZE * .5)
#define WIDTH   1280
#define HEIGHT  960

struct TextRenderer {
    TextRenderer() = default;
    void RenderText(std::string str, glm::vec2 position);
};
#endif //INC_15_466_F22_GAME4_TEXTRENDERER_HPP
