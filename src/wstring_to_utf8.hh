#ifndef WSTRINGTOUTF8_H
#define WSTRINGTOUTF8_H

void append_utf8_char(std::string& out, uint32_t codepoint) {
    if (codepoint <= 0x7F) {
        out += static_cast<char>(codepoint);
    }
    else if (codepoint <= 0x7FF) {
        out += static_cast<char>(0xC0 | (codepoint >> 6));
        out += static_cast<char>(0x80 | (codepoint & 0x3F));
    }
    else if (codepoint <= 0xFFFF) {
        out += static_cast<char>(0xE0 | (codepoint >> 12));
        out += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (codepoint & 0x3F));
    }
    else {
        out += static_cast<char>(0xF0 | (codepoint >> 18));
        out += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
        out += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        out += static_cast<char>(0x80 | (codepoint & 0x3F));
    }
}

// Преобразование std::wstring → char* (UTF-8), пользователь должен освободить память
char* wstring_to_utf8_cstr(const std::wstring& wstr) {
    std::string utf8;

    for (wchar_t wc : wstr) {
#if WCHAR_MAX > 0xFFFF  // 32-бит wchar_t (Linux/macOS)
        append_utf8_char(utf8, static_cast<uint32_t>(wc));
#else                   // 16-бит wchar_t (Windows)
        if (wc >= 0xD800 && wc <= 0xDBFF) {
            // high surrogate
            continue; // простая реализация, не обрабатываем суррогаты
        }
        else {
            append_utf8_char(utf8, static_cast<uint32_t>(wc));
        }
#endif
    }

    // Выделим char* и скопируем
    char* result = (char*)malloc(utf8.size() + 1);
    if (result) {
        std::copy(utf8.begin(), utf8.end(), result);
        result[utf8.size()] = '\0';
    }

    return result;
}

#endif
