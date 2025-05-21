#ifndef DBMS_H
#define DBMS_H

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <sstream>
#include <iomanip>

// 定义列的数据类型
enum class ColumnType {
    INT,
    CHAR
};

// 定义列的结构
struct Column {
    std::string name;
    ColumnType type;
    int size;  // 对于 CHAR 类型使用
};

// 定义表的结构
struct Table {
    std::string name;
    std::vector<Column> columns;
};

class DBMS {
public:
    DBMS();
    ~DBMS();

    // 数据库操作
    bool createDatabase(const std::string& name);
    bool useDatabase(const std::string& name);
    bool dropDatabase(const std::string& name);
    void showDatabases();

    // 表操作
    bool createTable(const std::string& name, const std::string& columnDefs);
    bool dropTable(const std::string& name);
    void showTables();

    // 数据操作
    bool insertInto(const std::string& tableName, 
                   const std::string& columnList = "", 
                   const std::string& valueList = "");
    bool selectFrom(const std::string& tableName, 
                   const std::string& columnList = "*", 
                   const std::string& whereClause = "");
    bool update(const std::string& tableName, 
               const std::string& setClause, 
               const std::string& whereClause = "");
    bool deleteFrom(const std::string& tableName, 
                   const std::string& whereClause = "");

private:
    std::string currentDB;
    std::map<std::string, std::vector<std::string>> databases;  // 数据库名 -> 表名列表
    std::map<std::string, Table> tables;  // 完整表名(db.table) -> 表结构

    // 辅助函数
    std::string getTablePath(const std::string& tableName) const;
    bool tableExists(const std::string& tableName) const;
    void loadTables();
    void saveTableInfo(const Table& table);
    Table loadTableInfo(const std::string& tableName);
    
    // 字符串分割函数
    std::vector<std::string> splitString(const std::string& str, char delimiter) const;
    std::vector<std::string> splitString(const std::string& str, const std::string& delimiter) const;
    
    bool writeRecord(const std::string& tableName, const std::vector<std::string>& values);
    std::vector<std::vector<std::string>> readRecords(const std::string& tableName);
    bool evaluateCondition(const std::vector<std::string>& record,
                          const std::string& condition,
                          const std::map<std::string, size_t>& columnIndices);
};

#endif // DBMS_H