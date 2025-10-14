#include <stdio.h>

unsigned int CRC;                 // int的大小是32位，作32位CRC寄存器
unsigned int CRC_32_Table[256];   // 用来保存CRC码表
void GenerateCRC32_Table() {
    for (int i = 0; i < 256; ++i)   // 用++i以提高效率
    {
        CRC = i;
        for (int j = 0; j < 8; ++j) {
            if (CRC & 1)                         // LSM为1
                CRC = (CRC >> 1) ^ 0xEDB88320;   // 采取反向校验
            else   // 0xEDB88320就是CRC-32多项表达式的reversed值
                CRC >>= 1;
        }
        CRC_32_Table[i] = CRC;
    }
}

int main() {
    FILE* pfile = fopen("crc32.txt", "w+");
    GenerateCRC32_Table();
    for (int i = 0; i + 3 < 256 - 4; i += 4)
        fprintf(pfile, "%#0.8x, %#0.8x, %#0.8x, %#0.8x,\n", CRC_32_Table[i], CRC_32_Table[i + 1],
                CRC_32_Table[i + 2], CRC_32_Table[i + 3]);
    fprintf(pfile, "%#0.8x, %#0.8x, %#0.8x, %#0.8x\n", CRC_32_Table[252], CRC_32_Table[253],
            CRC_32_Table[254], CRC_32_Table[255]);
    return 0;
}
