#ifndef TEXTRENDER_H
#define TEXTRENDER_H

#include <GLES2/gl2.h>  // ��� OpenGL ES 2.0
#include "stb_truetype.h"

struct FontData {
    stbtt_fontinfo info;               // ���������� � ������
    unsigned char* bitmap;             // ��������� ������ ������
    GLuint texture;                    // OpenGL-�������� ������
    int atlas_width, atlas_height;     // ������� ������

    // ������ ��� ������ ���������� ��������
    stbtt_packedchar cdata_ascii[95];     // ASCII 32-126
    stbtt_packedchar cdata_cyrillic[64];  // �-��-�
    stbtt_packedchar cdata_yo[2];         // ��
    int windowWidth;
    int windowHeight;
};

bool InitFont(FontData* font, const char* font_path, float font_size);
unsigned int CreateTextShaderProgram();  // �������� �������� ������ ����
void RenderText(FontData* font, const char* text, float x, float y, float r, float g, float b);

#endif
