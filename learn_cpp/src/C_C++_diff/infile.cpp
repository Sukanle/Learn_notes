#include<iostream>
#include<fstream>

template<typename file_stream>
bool file_txt(const std::string& filename, std::ios::openmode mode = std::ios::in);
template<typename file_stream>
bool file_bin(const std::string& filename, std::ios::openmode mode = std::ios::in);

int main()
{

    std::string filename = "outfile.txt";
    file_bin<std::fstream>(filename, std::ios::in);

    return 0;
}

template<typename file_stream>
bool file_txt(const std::string& filename, std::ios::openmode mode)
{
    file_stream infile(filename, mode);
    if(!infile.is_open())
    {
        std::cerr << "Error: file could not be opened" << std::endl;
        return false;
    }

    std::string line;

    while(std::getline(infile, line))
    {
        std::cout << line << std::endl;
    }

    // 不建议，因为会丢失空格
    // while(infile >> line)
    // {
    //     std::cout << line << std::endl;
    // }

    infile.close();
    return true;
}

template<typename file_stream>
bool file_bin(const std::string& filename, std::ios::openmode mode)
{
    file_stream infile(filename, mode|std::ios::binary);
    if(!infile.is_open())
    {
        std::cerr << "Error: file could not be opened" << std::endl;
        return false;
    }

    std::string line;

    struct data
    {
        int date;
        float time;
        char str[80];
    };

    data txt;

    while(infile.read((char*)&txt, sizeof(txt)))
    {
        std::cout <<"The file changed time is:"<<
            txt.date << " " << txt.time << ".\n"
            << txt.str << std::endl;
    }
    infile.close();

    return true;
}
