#include "wxn_codeConv.h"

int code_convert(const char* from_charset, const char* to_charset, char* inbuf, size_t inlen,
    char* outbuf, size_t outlen)
{
    iconv_t cd;

    cd = iconv_open(to_charset, from_charset);

    if (cd == 0)
        return -1;

    memset(outbuf, 0, outlen);

#ifdef _WIN32
    const char **pin = (const char **)&inbuf;
    char** pout = &outbuf;
#else
    char** pin = &inbuf;
    char** pout = &outbuf;
#endif

    if (iconv(cd, pin, &inlen, pout, &outlen) == -1)
        return -1;

    iconv_close(cd);
    return 0;
}

char* u2g(char* inbuf)
{
    size_t nOutLen = 2 * strlen(inbuf) - 1;
    char* pOut = (char*)malloc(nOutLen);
    if (code_convert("utf-8", "gb2312", inbuf, strlen(inbuf), pOut, nOutLen) == -1) {
        free(pOut);
        return nullptr;
    }
    return pOut;
}

char* g2u(char* inbuf)
{
    size_t nOutLen = 2 * strlen(inbuf) - 1;
    char* pOut = (char*)malloc(nOutLen);
    if (code_convert("gb2312", "utf-8", inbuf, strlen(inbuf), pOut, nOutLen) == -1) {
        free(pOut);
        return nullptr;
    }
    return pOut;
}
