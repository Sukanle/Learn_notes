#include <iostream>
#include <fstream>
#include <chrono>
#include <thread>
#include <fmt/format.h>

#define TEXT_OUT_BUF 100000 // 测试输出缓冲区对于输入输出的影响，设置为100000（样本数）
template <typename file_stream,typename data_t>
bool file_txt(const std::string &filename, std::ios::openmode mode,void *buf,int len);
template <typename file_stream,typename data_t>
bool file_bin(const std::string &filename, std::ios::openmode mode,void *buf);

// 无缓冲区输出,并计时
bool file_nobuf(const std::string &filename, std::ios::openmode mode);
// 有缓冲区输出,并计时
bool file_buf(const std::string &filename, std::ios::openmode mode);
int main()
{
    // std::ios::sync_with_stdio(false),std::cin.tie(0),std::cout.tie(0),std::cerr.tie(0);

    std::string filename = R"(outfile.txt)";
    std::string str[] = {
        "This file was created by the C++ program.\n",
        "Using the fstream class, it not C-style file I/O.\n",
        "Hello, World!\n",
        "This is a test file.\n",
        "Goodbye, World!\n"
    };
    // file_txt<std::fstream,std::string *>(filename,std::ios::out,str,5);

    struct data
    {
        int date;
        float time;
        char str[80];
    };

    data d = { 20241029,16.07 , "This is a test file.\nWrite by C++ program(binary mode)." };
    file_bin<std::fstream,data>(filename,std::ios::out,&d);

    // file_nobuf(filename,std::ios::out);
    // file_buf(filename,std::ios::out);

    return 0;
}

template <typename file_stream,typename data_t>
bool file_txt(const std::string &filename, std::ios::openmode mode,void *buf,int len)
{
    file_stream file(filename,mode);
    if (!file.is_open())
    {
        std::cerr << "Error: file could not be opened.\n";
        return false;
    }
    for(int i=0;i<len;i++)
    {
        file<<static_cast<data_t>(buf)[i];
    }

    file.close();
    return true;
}

template <typename file_stream,typename data_t>
bool file_bin(const std::string &filename, std::ios::openmode mode,void *buf)
{
    file_stream file(filename,mode|std::ios::binary);
    if (!file.is_open())
    {
        std::cerr << "Error: file could not be opened.\n";
        return false;
    }

    file.write(static_cast<const char*>(buf),sizeof(data_t));
    file.close();
    return true;
}

bool file_nobuf(const std::string &filename, std::ios::openmode mode)
{
    std::ofstream file(filename,mode);
    if (!file.is_open())
    {
        std::cerr << "Error: file could not be opened.\n";
        return false;
    }

    auto start = std::chrono::system_clock::now();
    // 1. 使用flush()函数,强制将缓冲区的数据写入文件
    puts("This is a test no buffer. (by member function: flush())");
    for(int i=1;i<=TEXT_OUT_BUF;i++)
    {
        file<<"Number: "<<i<<"\n";
        file.flush();
    }
    auto end = std::chrono::system_clock::now();
    auto time_span = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);

    fmt::print("Time: {}ms\n",time_span.count());

    // 2. 使用std::endl,在输出流中插入一个换行符并刷新缓冲区
    // std::cout<<"This is a test no buffer. (by std::endl)\n";
    // for(int i=1;i<=TEXT_OUT_BUF;i++)
    // {
    //     file<<"Number: "<<i<<std::endl;
    //     std::this_thread::sleep_for(std::chrono::milliseconds(500));
    // }
    // 3. 使用std::unitbuf,设置流的内部缓冲区为无缓冲区,std::nounitbuf恢复缓冲区
    // std::cout<<"This is a test no buffer. (by std::unitbuf)\n";
    // file<<std::unitbuf;
    // for(int i=1;i<=TEXT_OUT_BUF;i++)
    // {
    //     file<<"Number: "<<i<<"\n";
    //     std::this_thread::sleep_for(std::chrono::milliseconds(500));
    // }
    // file<<std::nounitbuf;

    file.close();
    return true;
}
bool file_buf(const std::string &filename, std::ios::openmode mode)
{
    std::ofstream file(filename,mode);
    if (!file.is_open())
    {
        std::cerr << "Error: file could not be opened.\n";
        return false;
    }

    auto start = std::chrono::system_clock::now();
    puts("This is a test buffer.");
    for(int i=1;i<=TEXT_OUT_BUF;i++)
    {
        file<<"Number: "<<i<<"\n";
    }
    auto end = std::chrono::system_clock::now();
    auto time_span = std::chrono::duration_cast<std::chrono::milliseconds>(end-start);
    fmt::print("Time: {}ms\n",time_span.count());

    file.close();
    return true;
}
