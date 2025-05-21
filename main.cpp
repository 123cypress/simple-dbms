#include <iostream>
#include <string>
#include <ctime>
#include <windows.h>
#include "DBMS.h"

extern FILE* yyin;
extern int yyparse();
DBMS* g_dbms = nullptr;

std::string getCurrentTime() {
    time_t now = time(0);
    struct tm timeinfo;
    char buffer[80];
    localtime_s(&timeinfo, &now);
    strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return std::string(buffer);
}

// 获取临时文件的完整路径
std::string getTempFilePath() {
    char tempPath[MAX_PATH];
    char tempFileName[MAX_PATH];
    DWORD result;
    
    // 获取临时文件夹路径
    result = GetTempPathA(MAX_PATH, tempPath);
    if(result == 0 || result > MAX_PATH) {
        throw std::runtime_error("Failed to get temp path");
    }
    
    // 生成唯一的临时文件名
    if(GetTempFileNameA(tempPath, "sql", 0, tempFileName) == 0) {
        throw std::runtime_error("Failed to generate temp file name");
    }
    
    return std::string(tempFileName);
}

int main() {
    g_dbms = new DBMS();
    std::string input;
    FILE* temp = nullptr;
    std::string tempFilePath;
    std::cout << "Simple SQL Database Management System" << std::endl;
    std::cout << "Current Time: " << getCurrentTime() << std::endl;
    std::cout << "\nEnter SQL commands (type 'EXIT' to quit):" << std::endl;

    while(true) {
        std::cout << "\nSQL> ";
        std::getline(std::cin, input);

        if(input.empty()) continue;
        if(input == "EXIT" || input == "exit") break;

        if(input.back() != ';') input += ";";

        try {
            // 获取临时文件路径
            tempFilePath = getTempFilePath();

            // 将输入写入临时文件
            if(fopen_s(&temp, tempFilePath.c_str(), "w") != 0 || !temp) {
                std::cerr << "Error creating temporary file: " << tempFilePath << std::endl;
                continue;
            }
            fprintf(temp, "%s", input.c_str());
            fclose(temp);

            // 解析SQL
            if(fopen_s(&yyin, tempFilePath.c_str(), "r") != 0 || !yyin) {
                std::cerr << "Error opening temporary file: " << tempFilePath << std::endl;
                remove(tempFilePath.c_str());
                continue;
            }

            if(yyparse() != 0) {
                std::cerr << "Error parsing SQL statement" << std::endl;
            }

            fclose(yyin);
            remove(tempFilePath.c_str());
        }
        catch(const std::exception& e) {
            std::cerr << "Error: " << e.what() << std::endl;
            if(!tempFilePath.empty()) {
                remove(tempFilePath.c_str());
            }
        }
    }

    delete g_dbms;
    return 0;
}