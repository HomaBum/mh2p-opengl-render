#ifndef TEXTRENDER_H
#define TEXTRENDER_H

#include <GLES2/gl2.h>  // Для OpenGL ES 2.0
#include "stb_truetype.h"

struct FontData {
    stbtt_fontinfo info;               // Информация о шрифте
    unsigned char* bitmap;             // Растровые данные атласа
    GLuint texture;                    // OpenGL-текстура атласа
    int atlas_width, atlas_height;     // Размеры атласа

    // Данные для разных диапазонов символов
    stbtt_packedchar cdata_ascii[95];     // ASCII 32-126
    stbtt_packedchar cdata_cyrillic[64];  // А-Яа-я
    stbtt_packedchar cdata_yo[2];         // Ёё
    int windowWidth;
    int windowHeight;
};

bool InitFont(FontData* font, const char* font_path, float font_size);
unsigned int CreateTextShaderProgram();  // Создание шейдеров внутри кода
void RenderText(FontData* font, const char* text, float x, float y, float r, float g, float b);

#endif
