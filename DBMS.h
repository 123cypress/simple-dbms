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

// �����е���������
enum class ColumnType {
    INT,
    CHAR
};

// �����еĽṹ
struct Column {
    std::string name;
    ColumnType type;
    int size;  // ���� CHAR ����ʹ��
};

// �����Ľṹ
struct Table {
    std::string name;
    std::vector<Column> columns;
};

class DBMS {
public:
    DBMS();
    ~DBMS();

    // ���ݿ����
    bool createDatabase(const std::string& name);
    bool useDatabase(const std::string& name);
    bool dropDatabase(const std::string& name);
    void showDatabases();

    // �����
    bool createTable(const std::string& name, const std::string& columnDefs);
    bool dropTable(const std::string& name);
    void showTables();

    // ���ݲ���
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
    std::map<std::string, std::vector<std::string>> databases;  // ���ݿ��� -> �����б�
    std::map<std::string, Table> tables;  // ��������(db.table) -> ��ṹ

    // ��������
    std::string getTablePath(const std::string& tableName) const;
    bool tableExists(const std::string& tableName) const;
    void loadTables();
    void saveTableInfo(const Table& table);
    Table loadTableInfo(const std::string& tableName);
    
    // �ַ����ָ��
    std::vector<std::string> splitString(const std::string& str, char delimiter) const;
    std::vector<std::string> splitString(const std::string& str, const std::string& delimiter) const;
    
    bool writeRecord(const std::string& tableName, const std::vector<std::string>& values);
    std::vector<std::vector<std::string>> readRecords(const std::string& tableName);
    bool evaluateCondition(const std::vector<std::string>& record,
                          const std::string& condition,
                          const std::map<std::string, size_t>& columnIndices);
};

#endif // DBMS_H