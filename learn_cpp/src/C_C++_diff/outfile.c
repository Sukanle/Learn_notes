#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum file_value {
    NAME = 0,
    OPENMODE,
};
bool file_bin(const char* file[], void* buf, size_t type_size, int len);
bool file_txt(const char* file[], void* buf, int len);
int main() {
    const char* str[] = {"This file was created by the C program.\n",
                         "Using the C-style file I/O.\n", "Hello, World!\n",
                         "This is a test file.\n", "Goodbye, World!\n"};

    const char* file[] = {"outfile.txt", "w"};

    /* file_txt(file,str,5); */
    file_bin(file, str, sizeof(char), 5);

    return 0;
}

bool file_bin(const char* file[], void* buf, size_t type_size, int len) {
    // FILE结构体是C语言中的文件结构体，用于文件操作，常用指针类型
    FILE* fp;
    char tmp[10];
    strcpy(tmp, file[OPENMODE]);
    strcat(tmp, "b");

    // fopen函数用于打开文件，第一个参数是文件名，第二个参数是打开方式
    // r: 只读，文件必须存在
    // w: 只写，文件不存在则创建，存在则清空
    // a: 追加，文件不存在则创建，存在则在文件末尾追加
    // t: 文本模式，默认模式
    // b: 二进制模式
    // +: 读写模式
    // 成功返回文件指针，失败返回NULL，错误码保存在errno中
    fp = fopen(file[NAME], tmp);

    // 写入文件
    for (int i = 0; i < len; i++) {
        // fwrite()函数用于写入文件，第一个参数是要写入的数据，第二个参数是数据大小，第三个参数是数据个数，第四个参数是文件指针
        // 返回值是实际写入的数据个数
        // fwrite()函数是二进制写入，不会自动添加换行符
        fwrite(((const char**)buf)[i], strlen(((const char**)buf)[i]), 1, fp);
    }

    // 关闭文件，如不关闭，文件内容可能不会写入磁盘
    fclose(fp);

    return true;
}

bool file_txt(const char* file[], void* buf, int len) {
    FILE* fp;

    // 打开文件
    fp = fopen(file[NAME], file[OPENMODE]);

    int res = 0;
    for (int i = 0; i < len; ++i) {
        if (i % 2 == 0)
            // fprintf()函数用于写入文件，第一个参数是文件指针，第二个参数是要写入的数据
            // 返回值是成功返回写入的字符个数，失败返回负数
            fprintf(fp, "%s", ((const char**)buf)[i]);
        else
            // fputs()函数用于写入文件，第一个参数是要写入的数据，第二个参数是文件指针
            // 返回值是成功返回0，失败返回EOF
            fputs(((const char**)buf)[i], fp);
    }
    fclose(fp);

    return true;
}
