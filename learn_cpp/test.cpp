#include<filesystem>
#include<fmt/ostream.h>
#include<fstream>

namespace fs = std::filesystem;
int main() {
    std::string filename = "demo.txt";
    std::string dirname = "demo";
    fs::path dir_path(dirname);
    // 使用 / 运算符连接路径
    fs::path file_path = dir_path / filename;

    // 检测路径是否存在
    if(!fs::exists(dir_path)){
    // 创建一个目录（封装了系统调用，如unix的mkdir, windows的CreateDirectory）
        fs::create_directory(dir_path);
    }
    std::ofstream file(file_path);
    file << "Hello, World!" << std::endl;
    file.close();

    // 检测文件是否存在（封装了系统调用，如unix的access, windows的GetFileAttributes）
    if(fs::exists(file_path)){
        fmt::print("File created successfully!\n");
    }

    fmt::print("File size: {}\n", fs::file_size(file_path));

    // 重名文件（封装了系统调用，如unix的rename, windows的MoveFile）
    fs::path new_file_path = dir_path / "new_demo.txt";
    fs::rename(file_path, new_file_path);

    // 遍历目录（封装了系统调用，如unix的opendir, readdir, closedir, windows的FindFirstFile, FindNextFile, FindClose）
    fmt::print("Contents of directory :\n");
    for(const auto& entry : fs::directory_iterator(dir_path)){
        fmt::print("{}\n", entry.path());
    }

    // 删除文件与目录（封装了系统调用，如unix的unlink, rmdir, windows的DeleteFile, RemoveDirectory）
    // fs::remove(new_file_path);
    // fs::remove(dir_path);

    return 0;
}
