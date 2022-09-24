//
// Created by William Ozeas on 9/24/22.
//

#include "TextRenderer.hpp"
#include "glm/ext.hpp"

GLubyte image[HEIGHT][WIDTH];

void draw_bitmap( FT_Bitmap*  bitmap,
                  FT_Int      x,
                  FT_Int      y)
{
    FT_Int  i, j, p, q;
    FT_Int  x_max = x + bitmap->width;
    FT_Int  y_max = y + bitmap->rows;


    /* for simplicity, we assume that `bitmap->pixel_mode' */
    /* is `FT_PIXEL_MODE_GRAY' (i.e., not a bitmap font)   */

    for ( i = x, p = 0; i < x_max; i++, p++ )
    {
        for ( j = y, q = 0; j < y_max; j++, q++ )
        {
            if ( i < 0      || j < 0       ||
                 i >= WIDTH || j >= HEIGHT )
                continue;

            image[j][i] |= bitmap->buffer[q * bitmap->width + p];
        }
    }
}

void
show_image( void )
{
    int  i, j;


    for ( i = 0; i < HEIGHT; i++ )
    {
        for ( j = 0; j < WIDTH; j++ )
            putchar( image[i][j] == 0 ? ' '
                                      : image[i][j] < 128 ? '+'
                                                          : '*' );
        putchar( '\n' );
    }
}

void TextRenderer::RenderText(std::string str, glm::vec2 position) {
    FT_Library library;
    FT_Face face;
    FT_Error error;

    const char *fontfile;
    const char *text;
    fontfile = "./Montserrat-Black.ttf";
    text = str.c_str();

    if ((error = FT_Init_FreeType (&library))) {
        std::cout << "error initialising freetype\n";
        abort();
    }
    if ((error = FT_New_Face (library, fontfile, 0, &face))) {
        std::cout << "error with ft new face\n";
        abort();
    }
    if ((error = FT_Set_Char_Size (face, FONT_SIZE * 64, FONT_SIZE * 64, 0, 0))) {
        std::cout << "error setting char size\n";
        abort();
    }

    hb_font_t *hb_font;
    hb_font = hb_ft_font_create (face, NULL);

    hb_buffer_t *buf = hb_buffer_create();
    hb_buffer_add_utf8 (buf, text, -1, 0, -1);
    hb_buffer_guess_segment_properties (buf);

    /* Shape it! */
    hb_shape (hb_font, buf, NULL, 0);

    /* Get glyph information and positions out of the buffer. */
//    unsigned int len = hb_buffer_get_length (buf);
    hb_glyph_info_t *info = hb_buffer_get_glyph_infos (buf, NULL);
    hb_glyph_position_t *pos = hb_buffer_get_glyph_positions (buf, NULL);

    //code from https://freetype.org/freetype2/docs/tutorial/example1.c
    FT_GlyphSlot  slot;
    FT_Matrix     matrix;                 /* transformation matrix */
    FT_Vector     pen;                    /* untransformed origin  */

    int num_chars     = strlen( text );
    float angle         = 0;
    double target_height = HEIGHT;

    /* cmap selection omitted;                                        */
    /* for simplicity we assume that the font contains a Unicode cmap */

    slot = face->glyph;

    /* set up matrix */
    matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
    matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
    matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L );
    matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );

    /* the pen position in 26.6 cartesian space coordinates; */
    /* start at (300,200) relative to the upper left corner  */
    pen.x = position.x;
    pen.y = position.y;

    for ( uint16_t n = 0; n < num_chars; n++ )
    {
        /* set transformation */
        FT_Set_Transform( face, &matrix, &pen );
        /* load glyph image into the slot (erase previous one) */
        error = FT_Load_Glyph( face, info[n].codepoint, FT_LOAD_RENDER );

        if ( error )
            continue;                 /* ignore errors */

        /* now, draw to our target surface (convert position) */
        draw_bitmap( &slot->bitmap,
                     slot->bitmap_left + position.x,
                     target_height - slot->bitmap_top - position.y);

        /* increment pen position */
        pen.x += pos[n].x_advance;
        pen.y += pos[n].y_advance;
    }

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexImage2D(
            GL_TEXTURE_2D,
            0,
            GL_RED,
            WIDTH,
            HEIGHT,
            0,
            GL_RED,
            GL_UNSIGNED_BYTE,
            image
    );
    // set texture options
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    GLuint program = gl_compile_program(
            //vertex shader:
            "#version 330 core\n"
            "layout (location = 0) in vec4 vertex; \n"
            "out vec2 TexCoords;\n"

            "uniform mat4 projection;\n"

            "void main() {\n"
            "	gl_Position = projection * vec4(vertex.xy, 0.0, 1.0);\n"
            "	TexCoords = vertex.zw;\n"
            "}\n"
            ,
            //fragment shader:
            "#version 330 core\n"
            "in vec2 TexCoords;\n"
            "out vec4 color;\n"

            "uniform sampler2D text;\n"
            "uniform vec3 textColor;\n"
            "void main() {\n"
            "vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, TexCoords).r);\n"
            "color = vec4(textColor, 1.0) * sampled;\n"
            "}\n"
    );

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glm::mat4 projection = glm::ortho(0.0f, (float)WIDTH, 0.0f, (float)HEIGHT);

// using code from https://learnopengl.com/In-Practice/Text-Rendering
// using code from PPU466.cpp
    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    glm::vec3 color(1.0f, 1.0f, 1.0f);
    glUseProgram(program);

    glUniformMatrix4fv(glGetUniformLocation(program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    glUniform3f(glGetUniformLocation(program, "textColor"), color.x, color.y, color.z);
    glActiveTexture(GL_TEXTURE0);
    glBindVertexArray(VAO);


        float xpos = 0;
        float ypos = 0;

        float w = WIDTH;
        float h = HEIGHT;
        // update VBO for each character
        float vertices[6][4] = {
                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos,     ypos,       0.0f, 1.0f },
                { xpos + w, ypos,       1.0f, 1.0f },

                { xpos,     ypos + h,   0.0f, 0.0f },
                { xpos + w, ypos,       1.0f, 1.0f },
                { xpos + w, ypos + h,   1.0f, 0.0f }
        };
        // render glyph texture over quad
        glBindTexture(GL_TEXTURE_2D, texture);
        // update content of VBO memory
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(vertices), vertices);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // render quad
        glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glBindTexture(GL_TEXTURE_2D, 0);


    GL_ERRORS();



    FT_Done_Face    ( face );
    FT_Done_FreeType( library );

    hb_buffer_destroy(buf);

}