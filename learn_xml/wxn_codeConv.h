#ifndef WXN_CODECONV_H
#define WXN_CODECONV_H 1

#include <fmt/color.h>
#include <fmt/format.h>
#include <iconv.h>
#include <string.h>

int code_convert(
    char* from_charset, char* to_charset, char* inbuf, int inlen, char* outbuf, int outlen);

#define UTF8_to_GD2312(inbuf) u2g(inbuf)
#define GD2312_to_UTF8(inbuf) g2u(inbuf)

char* u2g(char* inbuf);
char* g2u(char* inbuf);

#endif
