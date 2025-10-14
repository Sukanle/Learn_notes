#pragma once

#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <arpa/inet.h>
#include <unistd.h>
#include <wait.h>

typedef uint16_t ptlv_t;
typedef struct pack_ft {
#define FT_NOR (1 << 0)
#define FT_ZIP (1 << 1)   //
#define FT_EXE (1 << 2)
#define FT_DIR (1 << 3)
#define PACK_FTYPE_SIZE 4
    uint8_t ftype;
    uint8_t fcount[PACK_FTYPE_SIZE];
} pack_ft;
typedef struct {
    uint32_t packet_magic;                    // 魔数
    uint32_t packet_size;                     // 包大小
    uint16_t packet_seq;                      // 包序列
    uint8_t packet_ftype;                     // 包内部文件类型
    uint8_t packet_fcount[PACK_FTYPE_SIZE];   // 包内部文件类型数量
    uint8_t flags;                            // 标志位, 1-发送端, 0-接受端
    ptlv_t version;                           // 协议版本
    uint32_t reserved;                        // 保留字段
    uint32_t check;                           // 校验码
    time_t timestamp;                         // 时间戳
} hpack_t;

#define HPACK_MAGIC 0x0d000721
#define HPACK_SIZE sizeof(hpack_t)

typedef enum {
    Nd2N = (ptlv_t)0xa6138275,
    SoLA = (ptlv_t)0x3aa5de5a,
    NeNA = (ptlv_t)0x1c8e2e1d,
    MgLU = (ptlv_t)0x8d8f3fb6,
    FoNA = (ptlv_t)0x4a27fb08,
} WebPtl;

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

typedef bool (*dencrypt_optfn)(const hpack_t* const);

/*
 * @brief CRC32
 * @param data
 * @param length
 * @param crc32
 * @return
 * */
uint32_t crc32(const void* __restrict __data, size_t __length,
               uint32_t __crc32);
uint16_t encrypt(WebPtl __ptl, ...);
ptlv_t dencrypt(WebPtl __ptl, ...);
int swrite(int __fd, const char* __restrict __msg, int __size);
int sread(int __fd, char* const __restrict __msg, int __size);
int hsend(int __cfd, const char* const __restrict __msg,
          hpack_t* const __restrict __hd);
int hrecv(int __cfd, char* const __restrict __msg,
          hpack_t* const __restrict __hd, dencrypt_optfn __fn);
int set_hpack_pft(uint8_t __ftype, uint32_t __fcount,
                hpack_t* const __restrict __hd);

#ifdef __cplusplus
}
#endif
