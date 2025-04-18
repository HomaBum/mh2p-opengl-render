#define STB_TRUETYPE_IMPLEMENTATION
#include "text_render.h"
#include <stdio.h>
#include <vector>
#include "libgl2.h"

#define ATLAS_WIDTH 1024
#define ATLAS_HEIGHT 1024

// Встроенные шейдеры (GLSL ES 1.00 для OpenGL ES 2.0)
const char* vertex_shader_source =
"attribute vec2 position;\n"
"attribute vec2 uv;\n"
"varying vec2 frag_uv;\n"
"void main() {\n"
"    gl_Position = vec4(position, 0.0, 1.0);\n"
"    frag_uv = uv;\n"
"}\n";

const char* fragment_shader_source =
"precision mediump float;\n"
"varying vec2 frag_uv;\n"
"uniform sampler2D tex;\n"
"uniform vec3 textColor;\n"
"void main() {\n"
"    float alpha = texture2D(tex, frag_uv).a;\n"
"    gl_FragColor = vec4(textColor, alpha);\n"
"}\n";

// Компиляция шейдера
unsigned int CompileShader(GLenum type, const char* source) {
	LibGl2* libGl2 = LibGl2::GetInstance();

    unsigned int shader = libGl2->glCreateShader(type);
    libGl2->glShaderSource(shader, 1, &source, NULL);
    libGl2->glCompileShader(shader);

    // Проверка ошибок
    int success;
    libGl2->glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char info_log[512];
        libGl2->glGetShaderInfoLog(shader, 512, NULL, info_log);
        printf("ERROR: glCompileShader() %s\n", info_log);
    }
    return shader;
}

// Создание шейдерной программы
unsigned int CreateTextShaderProgram() {
	LibGl2* libGl2 = LibGl2::GetInstance();

    unsigned int vertex_shader = CompileShader(GL_VERTEX_SHADER, vertex_shader_source);
    unsigned int fragment_shader = CompileShader(GL_FRAGMENT_SHADER, fragment_shader_source);

    unsigned int program = libGl2->glCreateProgram();
    libGl2->glAttachShader(program, vertex_shader);
    libGl2->glAttachShader(program, fragment_shader);
    libGl2->glLinkProgram(program);

    // Проверка ошибок линковки
    int success;
    libGl2->glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char info_log[512];
        libGl2->glGetProgramInfoLog(program, 512, NULL, info_log);
        printf("ERROR: glLinkProgram() %s\n", info_log);
    }

    libGl2->glDeleteShader(vertex_shader);
    libGl2->glDeleteShader(fragment_shader);

    return program;
}

bool InitFont(FontData* font, const char* path, float font_size) {
	LibGl2* libGl2 = LibGl2::GetInstance();
    // Загрузка файла шрифта
    FILE* font_file = fopen(path, "rb");
    if(!font_file) {
    	printf("Cant open file %s\n", path);
    	return false;
    }
    fseek(font_file, 0, SEEK_END);
    long font_size_bytes = ftell(font_file);
    fseek(font_file, 0, SEEK_SET);

    unsigned char* ttf_buffer = new unsigned char[font_size_bytes];
    fread(ttf_buffer, 1, font_size_bytes, font_file);
    fclose(font_file);

    printf("InitFont font file read\n");

    // 2. Инициализация шрифта
    if (!stbtt_InitFont(&font->info, ttf_buffer, 0)) {
        printf("Error: Failed to initialize font (possibly corrupt file)\n");
        free(ttf_buffer);
        return false;
    }

    // 3. Создание текстурного атласа
    font->atlas_width = 1024;
    font->atlas_height = 1024;
    font->bitmap = (unsigned char*)calloc(font->atlas_width * font->atlas_height, 1);

    // 4. Настройка упаковщика символов
    stbtt_pack_context pc;
    if (!stbtt_PackBegin(&pc, font->bitmap, font->atlas_width, font->atlas_height, 0, 1, NULL)) {
        printf("Error: Failed to initialize font packer\n");
        free(ttf_buffer);
        free(font->bitmap);
        return false;
    }

    // 5. Настройка диапазонов символов
    stbtt_pack_range ranges[3] = { 0 };

    // ASCII 32-126
    ranges[0].font_size = font_size;
    ranges[0].first_unicode_codepoint_in_range = 32;
    ranges[0].num_chars = 95;
    ranges[0].chardata_for_range = font->cdata_ascii;

    // Русские буквы А-Яа-я (0x0410-0x044F)
    ranges[1].font_size = font_size;
    ranges[1].first_unicode_codepoint_in_range = 0x0410;
    ranges[1].num_chars = 64;
    ranges[1].chardata_for_range = font->cdata_cyrillic;

    // Ё и ё через массив кодпоинтов
    static int yo_codepoints[] = { 0x0401, 0x0451 };
    ranges[2].font_size = font_size;
    ranges[2].array_of_unicode_codepoints = yo_codepoints;
    ranges[2].num_chars = 2;
    ranges[2].chardata_for_range = font->cdata_yo;

    // 6. Упаковка символов
    if (!stbtt_PackFontRanges(&pc, ttf_buffer, 0, ranges, 3)) {
        printf("Error: Failed to pack font ranges\n");
        stbtt_PackEnd(&pc);
        free(ttf_buffer);
        free(font->bitmap);
        return false;
    }
    stbtt_PackEnd(&pc);

    // 7. Создание OpenGL текстуры
    libGl2->glGenTextures(1, &font->texture);
    libGl2->glBindTexture(GL_TEXTURE_2D, font->texture);

    // Для OpenGL ES 2.0:
    libGl2->glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA,
        font->atlas_width, font->atlas_height,
        0, GL_ALPHA, GL_UNSIGNED_BYTE,
        font->bitmap);

    // Для OpenGL 3.3+ замените на:
    // glTexImage2D(GL_TEXTURE_2D, 0, GL_R8,
    //             font->atlas_width, font->atlas_height,
    //             0, GL_RED, GL_UNSIGNED_BYTE,
    //             font->bitmap);

    libGl2->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    libGl2->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    libGl2->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    libGl2->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // 8. Проверка ошибок OpenGL
    GLenum err = libGl2->glGetError();
    if (err != GL_NO_ERROR) {
        printf("OpenGL error during texture creation: 0x%04X\n", err);
    }

    free(ttf_buffer);

    printf("InitFont ok\n");

    return true;
}

stbtt_packedchar* GetCharData(FontData* font, int codepoint) {
    // ASCII
    if (codepoint >= 32 && codepoint <= 126) {
        return &font->cdata_ascii[codepoint - 32];
    }
    // Русские заглавные
    else if (codepoint >= 0x0410 && codepoint <= 0x042F) {
        return &font->cdata_cyrillic[codepoint - 0x0410];
    }
    // Русские строчные
    else if (codepoint >= 0x0430 && codepoint <= 0x044F) {
        return &font->cdata_cyrillic[32 + (codepoint - 0x0430)];
    }
    // Ё
    else if (codepoint == 0x0401) {
        return &font->cdata_yo[0];
    }
    // ё
    else if (codepoint == 0x0451) {
        return &font->cdata_yo[1];
    }

    return nullptr;
}

int utf8_to_codepoint(const char* utf8, uint32_t* codepoint) {
    const unsigned char* p = (const unsigned char*)utf8;

    if (p[0] < 0x80) {
        *codepoint = p[0];
        return 1;
    }
    else if ((p[0] & 0xE0) == 0xC0) {
        if (!p[1] || (p[1] & 0xC0) != 0x80) goto fail;
        uint32_t cp = ((p[0] & 0x1F) << 6) | (p[1] & 0x3F);
        if (cp < 0x80) goto fail; // overlong
        *codepoint = cp;
        return 2;
    }
    else if ((p[0] & 0xF0) == 0xE0) {
        if (!p[1] || !p[2] || (p[1] & 0xC0) != 0x80 || (p[2] & 0xC0) != 0x80) goto fail;
        uint32_t cp = ((p[0] & 0x0F) << 12) |
                      ((p[1] & 0x3F) << 6) |
                      (p[2] & 0x3F);
        if (cp < 0x800 || (cp >= 0xD800 && cp <= 0xDFFF)) goto fail;
        *codepoint = cp;
        return 3;
    }
    else if ((p[0] & 0xF8) == 0xF0) {
        if (!p[1] || !p[2] || !p[3] ||
            (p[1] & 0xC0) != 0x80 ||
            (p[2] & 0xC0) != 0x80 ||
            (p[3] & 0xC0) != 0x80)
            goto fail;
        uint32_t cp = ((p[0] & 0x07) << 18) |
                      ((p[1] & 0x3F) << 12) |
                      ((p[2] & 0x3F) << 6) |
                      (p[3] & 0x3F);
        if (cp < 0x10000 || cp > 0x10FFFF) goto fail;
        *codepoint = cp;
        return 4;
    }

fail:
    *codepoint = 0xFFFD;
    return 1;
}

void RenderText(
    FontData* font,
    const char* text,
    float x, float y,
    float r, float g, float b
) {
	printf("RenderText start\n");
	LibGl2* libGl2 = LibGl2::GetInstance();

    static unsigned int shader_program = 0;
    if (shader_program == 0) {
        shader_program = CreateTextShaderProgram();
    }

    libGl2->glUseProgram(shader_program);
    libGl2->glUniform3f(libGl2->glGetUniformLocation(shader_program, "textColor"), r, g, b);
    libGl2->glActiveTexture(GL_TEXTURE0);
    libGl2->glBindTexture(GL_TEXTURE_2D, font->texture);

    stbtt_aligned_quad q;
    const char* p = text;
    while(*p) {

        unsigned int codepoint = 0;
        p += utf8_to_codepoint(p, &codepoint); // Ваша функция декодирования UTF-8

        stbtt_packedchar* ch = nullptr;

        // Выбор нужного диапазона
        if (codepoint >= 32 && codepoint <= 126) {
            ch = &font->cdata_ascii[codepoint - 32];
        }
        else if (codepoint >= 0x0410 && codepoint <= 0x044F) {
            ch = &font->cdata_cyrillic[codepoint - 0x0410];
        }
        else if (codepoint == 0x0401) { // Ё
            ch = &font->cdata_yo[0];
        }
        else if (codepoint == 0x0451) { // ё
            ch = &font->cdata_yo[1];
        }

        if (!ch) {
            continue;
        }

        stbtt_GetPackedQuad(
            ch,
            ATLAS_WIDTH, ATLAS_HEIGHT,
            0, &x, &y, &q, 1
        );

        // Пиксели → NDC
        float to_ndc_x = 2.0f / font->windowWidth;
        float to_ndc_y = 2.0f / font->windowHeight;

        float vertices[] = {
            q.x0 * to_ndc_x - 1.0f,  1.0f - q.y0 * to_ndc_y,  q.s0, q.t0, // Левый нижний
            q.x1 * to_ndc_x - 1.0f,  1.0f - q.y0 * to_ndc_y,  q.s1, q.t0, // Правый нижний
            q.x1 * to_ndc_x - 1.0f,  1.0f - q.y1 * to_ndc_y,  q.s1, q.t1, // Правый верхний
            q.x0 * to_ndc_x - 1.0f,  1.0f - q.y1 * to_ndc_y,  q.s0, q.t1  // Левый верхний
        };

        unsigned int indices[] = { 0, 1, 2, 0, 2, 3 };

        unsigned int vbo, ebo;
        libGl2->glGenBuffers(1, &vbo);
        libGl2->glGenBuffers(1, &ebo);

        libGl2->glBindBuffer(GL_ARRAY_BUFFER, vbo);
        libGl2->glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        libGl2->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        libGl2->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        libGl2->glEnableVertexAttribArray(0);
        libGl2->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);

        libGl2->glEnableVertexAttribArray(1);
        libGl2->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));

        libGl2->glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

        libGl2->glDisableVertexAttribArray(0);
        libGl2->glDisableVertexAttribArray(1);
        libGl2->glDeleteBuffers(1, &vbo);
        libGl2->glDeleteBuffers(1, &ebo);
    }

    printf("RenderText ok\n");
}
