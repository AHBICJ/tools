#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <filesystem>

const size_t DEFAULT_SIZE_KB = 1000; // 默认1000KB

void splitCSV(const std::string& filename, size_t maxSizeKB) {
    std::ifstream infile(filename);
    if (!infile) {
        std::cerr << "无法打开文件: " << filename << std::endl;
        return;
    }
    char bom[3];
    infile.read(bom, 3);
    if (!(bom[0] == char(0xEF) && bom[1] == char(0xBB) && bom[2] == char(0xBF))) {
        infile.seekg(0); // 如果不是 BOM，则重置文件指针
    }

    std::filesystem::path filePath(filename);
    std::string fileNameWithoutExtension = filePath.stem().string();
    std::string parent_path = filePath.parent_path().string();
    std::string header;
    std::getline(infile, header); // 读取第一行
    std::istringstream header_stream(header);
    std::string token;
    int columnIndex = -1;

    // 找到device_id的位置
    int index = 0;
    while (std::getline(header_stream, token, ',')) {
        if (token == "device_id") {
            columnIndex = index;
            break;
        }
        ++index;
    }

    if (columnIndex == -1) {
        std::cerr << "没有找到列 'device_id'" << std::endl;
        return;
    }

    std::string line;
    int fileCount = 1;
    int currentSize = 0;

    std::ofstream outfile;
    outfile.open(parent_path + filePath.preferred_separator + fileNameWithoutExtension + "_" + std::to_string(fileCount) + ".txt");

    if (!outfile) {
        std::cerr << "无法创建输出文件" << std::endl;
        return;
    }

    while (std::getline(infile, line)) {
        std::istringstream line_stream(line);
        std::string value;
        int col = 0;
        
        // 找到device_id对应的值
        while (std::getline(line_stream, value, ',')) {
            if (col == columnIndex) {
                break;
            }
            ++col;
        }
        if (col != columnIndex) {
           std::cerr << "忽略错误的行:" << line << std::endl;
           continue;
        }

        // 检查写入是否会超出限制
        int lineSize = value.size() + 1; // +1 for newline character
        if (currentSize + lineSize > maxSizeKB * 1024) {
            // 文件达到限制，关闭当前文件并打开下一个
            outfile.close();
            fileCount++;
            outfile.open(parent_path + filePath.preferred_separator + fileNameWithoutExtension + "_" + std::to_string(fileCount) + ".txt");
            currentSize = 0; // 重置当前大小
        }

        // 写入数据
        outfile << value << "\n";
        currentSize += lineSize;
    }

    infile.close();
    outfile.close();
    std::cout << "分割完成，输出文件数: " << fileCount << std::endl;
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "用法: " << argv[0] << " filename [size_in_kb]" << std::endl;
        return 1;
    }

    std::string filename = argv[1];
    size_t sizeInKB = (argc == 3) ? std::stoul(argv[2]) : DEFAULT_SIZE_KB;

    splitCSV(filename, sizeInKB);
    return 0;
}