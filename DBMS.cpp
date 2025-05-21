#include "DBMS.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <iomanip>
#include <filesystem>
#include <ctime>

// ���캯��
DBMS::DBMS() {
    // ɨ���������ݿ�
    for (const auto& entry : std::filesystem::directory_iterator(".")) {
        if (std::filesystem::is_directory(entry)) {
            std::string dbName = entry.path().filename().string();
            databases[dbName] = std::vector<std::string>();
        }
    }
}

// ��������
DBMS::~DBMS() {
    // ��������״̬
    if (!currentDB.empty()) {
        for (const auto& table : tables) {
            saveTableInfo(table.second);
        }
    }
}

// ���ݿ����
bool DBMS::createDatabase(const std::string& name) {
    if (databases.find(name) != databases.end()) {
        std::cout << "Error: Database '" << name << "' already exists." << std::endl;
        return false;
    }

    try {
        if (std::filesystem::create_directory(name)) {
            databases[name] = std::vector<std::string>();
            std::cout << "Database created successfully." << std::endl;
            return true;
        }
    }
    catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    return false;
}

bool DBMS::useDatabase(const std::string& name) {
    if (databases.find(name) == databases.end()) {
        std::cout << "Error: Database '" << name << "' does not exist." << std::endl;
        return false;
    }

    currentDB = name;
    loadTables();
    std::cout << "Database changed to '" << name << "'." << std::endl;
    return true;
}

bool DBMS::dropDatabase(const std::string& name) {
    if (databases.find(name) == databases.end()) {
        std::cout << "Error: Database '" << name << "' does not exist." << std::endl;
        return false;
    }

    try {
        std::filesystem::remove_all(name);
        databases.erase(name);
        if (currentDB == name) {
            currentDB.clear();
            tables.clear();
        }
        std::cout << "Database dropped successfully." << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return false;
    }
}

void DBMS::showDatabases() {
    std::cout << "Databases:" << std::endl;
    std::cout << "--------------------" << std::endl;
    for (const auto& db : databases) {
        std::cout << db.first << std::endl;
    }
    std::cout << "--------------------" << std::endl;
    std::cout << databases.size() << " database(s)" << std::endl;
}

// �����
bool DBMS::createTable(const std::string& name, const std::string& columnDefs) {
    if (currentDB.empty()) {
        std::cout << "Error: No database selected." << std::endl;
        return false;
    }

    std::string tablePath = getTablePath(name);
    
    if (std::filesystem::exists(tablePath)) {
        std::cout << "Error: Table '" << name << "' already exists." << std::endl;
        return false;
    }

    // �����ж���
    std::vector<Column> columns;
    std::vector<std::string> colDefs = splitString(columnDefs, ',');
    
    for (const auto& colDef : colDefs) {
        std::istringstream colss(colDef);
        std::string colName, colType;
        colss >> colName >> colType;
        
        Column col;
        col.name = colName;
        
        // ����Ƿ��д�Сָ��
        size_t pos = colType.find('(');
        if (pos != std::string::npos) {
            std::string baseType = colType.substr(0, pos);
            std::string sizeStr = colType.substr(pos + 1);
            sizeStr = sizeStr.substr(0, sizeStr.length() - 1);  // �Ƴ�������
            
            col.type = (baseType == "CHAR") ? ColumnType::CHAR : ColumnType::INT;
            col.size = std::stoi(sizeStr);
        } else {
            col.type = (colType == "CHAR") ? ColumnType::CHAR : ColumnType::INT;
            col.size = (col.type == ColumnType::INT) ? sizeof(int) : 255;
        }
        
        columns.push_back(col);
    }

    // �������ļ�
    std::ofstream tableFile(tablePath);
    if (!tableFile) {
        std::cout << "Error: Failed to create table file." << std::endl;
        return false;
    }
    tableFile.close();

    // �����ṹ
    Table table;
    table.name = name;
    table.columns = columns;

    // �����ڴ��еı���Ϣ
    tables[currentDB + "." + name] = table;
    databases[currentDB].push_back(name);

    // �����ṹ���ļ�
    saveTableInfo(table);

    std::cout << "Table created successfully." << std::endl;
    return true;
}

bool DBMS::dropTable(const std::string& name) {
    if (currentDB.empty()) {
        std::cout << "Error: No database selected." << std::endl;
        return false;
    }

    std::string tablePath = getTablePath(name);
    if (!tableExists(name)) {
        std::cout << "Error: Table '" << name << "' does not exist." << std::endl;
        return false;
    }

    try {
        std::filesystem::remove(tablePath);
        std::filesystem::remove(tablePath + ".info");
        
        auto& dbTables = databases[currentDB];
        dbTables.erase(std::remove(dbTables.begin(), dbTables.end(), name), dbTables.end());
        tables.erase(currentDB + "." + name);

        std::cout << "Table dropped successfully." << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
        return false;
    }
}

void DBMS::showTables() {
    if (currentDB.empty()) {
        std::cout << "Error: No database selected." << std::endl;
        return;
    }

    std::cout << "Tables in database '" << currentDB << "':" << std::endl;
    std::cout << "--------------------" << std::endl;
    for (const auto& tableName : databases[currentDB]) {
        std::cout << tableName << std::endl;
    }
    std::cout << "--------------------" << std::endl;
    std::cout << databases[currentDB].size() << " table(s)" << std::endl;
}

// ���ݲ���
bool DBMS::insertInto(const std::string& tableName, 
                     const std::string& columnList, 
                     const std::string& valueList) {
    if (currentDB.empty()) {
        std::cout << "Error: No database selected." << std::endl;
        return false;
    }

    if (!tableExists(tableName)) {
        std::cout << "Error: Table '" << tableName << "' does not exist." << std::endl;
        return false;
    }

    // ��ȡ��ṹ
    const Table& table = tables[currentDB + "." + tableName];
    
    // ����ֵ�б�
    std::vector<std::string> values = splitString(valueList, ',');
    
    // ����ṩ�������б���֤������ֵ������ƥ��
    if (!columnList.empty()) {
        std::vector<std::string> columns = splitString(columnList, ',');
        if (columns.size() != values.size()) {
            std::cout << "Error: Column count doesn't match value count" << std::endl;
            return false;
        }
    } else {
        // ���û���ṩ�����б���ֵ֤�������Ƿ�ƥ��������
        if (values.size() != table.columns.size()) {
            std::cout << "Error: Value count doesn't match column count" << std::endl;
            return false;
        }
    }

    // д���¼
    if (!writeRecord(tableName, values)) {
        std::cout << "Error: Failed to write record" << std::endl;
        return false;
    }

    std::cout << "1 row inserted successfully." << std::endl;
    return true;
}

bool DBMS::selectFrom(const std::string& tableName, 
                     const std::string& columnList, 
                     const std::string& whereClause) {
    if (currentDB.empty()) {
        std::cout << "Error: No database selected." << std::endl;
        return false;
    }

    if (!tableExists(tableName)) {
        std::cout << "Error: Table '" << tableName << "' does not exist." << std::endl;
        return false;
    }

    // ��ȡ��ṹ
    const Table& table = tables[currentDB + "." + tableName];
    
    // ��ȡ���м�¼
    auto records = readRecords(tableName);
    
    // ȷ��Ҫ��ʾ����
    std::vector<std::string> columnsToShow;
    if (columnList == "*") {
        for (const auto& col : table.columns) {
            columnsToShow.push_back(col.name);
        }
    } else {
        columnsToShow = splitString(columnList, ',');
    }

    // ��ȡ�е�����
    std::map<std::string, size_t> columnIndices;
    for (size_t i = 0; i < table.columns.size(); ++i) {
        columnIndices[table.columns[i].name] = i;
    }

    // ��ӡ����
    for (const auto& colName : columnsToShow) {
        std::cout << std::setw(15) << std::left << colName;
    }
    std::cout << std::endl;

    // ��ӡ�ָ���
    for (size_t i = 0; i < columnsToShow.size(); ++i) {
        std::cout << "---------------";
    }
    std::cout << std::endl;

    // ��ӡ��¼
    int matchCount = 0;
    for (const auto& record : records) {
        if (evaluateCondition(record, whereClause, columnIndices)) {
            for (const auto& colName : columnsToShow) {
                auto it = columnIndices.find(colName);
                if (it != columnIndices.end()) {
                    std::cout << std::setw(15) << std::left << record[it->second];
                }
            }
            std::cout << std::endl;
            matchCount++;
        }
    }

    std::cout << matchCount << " row(s) in set" << std::endl;
    return true;
}

bool DBMS::update(const std::string& tableName, 
                 const std::string& setClause, 
                 const std::string& whereClause) {
    if (currentDB.empty()) {
        std::cout << "Error: No database selected." << std::endl;
        return false;
    }

    if (!tableExists(tableName)) {
        std::cout << "Error: Table '" << tableName << "' does not exist." << std::endl;
        return false;
    }

    // ��ȡ���м�¼
    std::vector<std::vector<std::string>> records = readRecords(tableName);
    const Table& table = tables[currentDB + "." + tableName];

    // ���� SET �Ӿ�
    std::vector<std::pair<std::string, std::string>> setValues;
    std::vector<std::string> setParts = splitString(setClause, ',');
    for (const auto& setPart : setParts) {
        size_t eqPos = setPart.find('=');
        if (eqPos != std::string::npos) {
            std::string colName = setPart.substr(0, eqPos);
            std::string value = setPart.substr(eqPos + 1);
            // ȥ��ǰ��ո�
            colName.erase(0, colName.find_first_not_of(" \t"));
            colName.erase(colName.find_last_not_of(" \t") + 1);
            value.erase(0, value.find_first_not_of(" \t"));
            value.erase(value.find_last_not_of(" \t") + 1);
            // ȥ��ֵ������
            if (!value.empty() && value.front() == '\'' && value.back() == '\'') {
                value = value.substr(1, value.length() - 2);
            }
            setValues.push_back({colName, value});
        }
    }

    // ��ȡ�е�����
    std::map<std::string, size_t> columnIndices;
    for (size_t i = 0; i < table.columns.size(); ++i) {
        columnIndices[table.columns[i].name] = i;
    }

    // ���¼�¼
    int updatedCount = 0;
    for (auto& record : records) {
        if (evaluateCondition(record, whereClause, columnIndices)) {
            for (const auto& setValue : setValues) {
                auto it = columnIndices.find(setValue.first);
                if (it != columnIndices.end()) {
                    record[it->second] = setValue.second;
                }
            }
            updatedCount++;
        }
    }

    // д���ļ�
    std::string tablePath = getTablePath(tableName);
    std::ofstream file(tablePath);
    if (!file) {
        std::cout << "Error: Failed to write back to table file." << std::endl;
        return false;
    }

    for (const auto& record : records) {
        for (size_t i = 0; i < record.size(); ++i) {
            if (i > 0) file << ",";
            file << record[i];
        }
        file << std::endl;
    }

    std::cout << updatedCount << " row(s) updated." << std::endl;
    return true;
}

bool DBMS::deleteFrom(const std::string& tableName, const std::string& whereClause) {
    if (currentDB.empty()) {
        std::cout << "Error: No database selected." << std::endl;
        return false;
    }

    if (!tableExists(tableName)) {
        std::cout << "Error: Table '" << tableName << "' does not exist." << std::endl;
        return false;
    }

    // ��ȡ���м�¼
    std::vector<std::vector<std::string>> records = readRecords(tableName);
    const Table& table = tables[currentDB + "." + tableName];

    // ��ȡ�е�����
    std::map<std::string, size_t> columnIndices;
    for (size_t i = 0; i < table.columns.size(); ++i) {
        columnIndices[table.columns[i].name] = i;
    }

    // ���Ҫɾ���ļ�¼
    std::vector<std::vector<std::string>> remainingRecords;
    int deletedCount = 0;

    for (const auto& record : records) {
        if (!evaluateCondition(record, whereClause, columnIndices)) {
            remainingRecords.push_back(record);
        } else {
            deletedCount++;
        }
    }

    // д���ļ�
    std::string tablePath = getTablePath(tableName);
    std::ofstream file(tablePath);
    if (!file) {
        std::cout << "Error: Failed to write back to table file." << std::endl;
        return false;
    }

    for (const auto& record : remainingRecords) {
        for (size_t i = 0; i < record.size(); ++i) {
            if (i > 0) file << ",";
            file << record[i];
        }
        file << std::endl;
    }

    std::cout << deletedCount << " row(s) deleted." << std::endl;
    return true;
}

// ��������
std::string DBMS::getTablePath(const std::string& tableName) const {
    return currentDB + "/" + tableName + ".table";
}

bool DBMS::tableExists(const std::string& tableName) const {
    return tables.find(currentDB + "." + tableName) != tables.end();
}

void DBMS::loadTables() {
    tables.clear();
    if (currentDB.empty()) return;

    for (const auto& entry : std::filesystem::directory_iterator(currentDB)) {
        std::string path = entry.path().string();
        if (path.length() >= 6 && path.substr(path.length() - 6) == ".table") {
            std::string tableName = entry.path().stem().string();
            Table table = loadTableInfo(tableName);
            tables[currentDB + "." + tableName] = table;
            if (std::find(databases[currentDB].begin(), databases[currentDB].end(), tableName) 
                == databases[currentDB].end()) {
                databases[currentDB].push_back(tableName);
            }
        }
    }
}

void DBMS::saveTableInfo(const Table& table) {
    if (currentDB.empty()) return;

    std::string infoPath = currentDB + "/" + table.name + ".table.info";
    std::ofstream infoFile(infoPath);
    if (!infoFile) return;

    for (const auto& column : table.columns) {
        infoFile << column.name << " "
                << (column.type == ColumnType::INT ? "INT" : "CHAR") << " "
                << column.size << std::endl;
    }
}

Table DBMS::loadTableInfo(const std::string& tableName) {
    Table table;
    table.name = tableName;

    std::string infoPath = currentDB + "/" + tableName + ".table.info";
    std::ifstream infoFile(infoPath);
    if (!infoFile) return table;

    std::string line;
    while (std::getline(infoFile, line)) {
        std::istringstream iss(line);
        Column col;
        std::string typeStr;
        iss >> col.name >> typeStr >> col.size;
        col.type = (typeStr == "INT" ? ColumnType::INT : ColumnType::CHAR);
        table.columns.push_back(col);
    }

    return table;
}

std::vector<std::string> DBMS::splitString(const std::string& str, char delimiter) const {
    std::vector<std::string> result;
    std::istringstream iss(str);
    std::string token;

    while (std::getline(iss, token, delimiter)) {
        // ȥ��ǰ��ո�
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);
        result.push_back(token);
    }

    return result;
}

std::vector<std::string> DBMS::splitString(const std::string& str, const std::string& delimiter) const {
    std::vector<std::string> result;
    size_t start = 0;
    size_t end = str.find(delimiter);
    
    while (end != std::string::npos) {
        std::string token = str.substr(start, end - start);
        // ȥ��ǰ��ո�
        token.erase(0, token.find_first_not_of(" \t"));
        token.erase(token.find_last_not_of(" \t") + 1);
        result.push_back(token);
        
        start = end + delimiter.length();
        end = str.find(delimiter, start);
    }
    
    // �������һ��token
    std::string token = str.substr(start);
    token.erase(0, token.find_first_not_of(" \t"));
    token.erase(token.find_last_not_of(" \t") + 1);
    result.push_back(token);
    
    return result;
}

bool DBMS::writeRecord(const std::string& tableName, const std::vector<std::string>& values) {
    std::string tablePath = getTablePath(tableName);
    std::ofstream file(tablePath, std::ios::app);
    if (!file) return false;

    for (size_t i = 0; i < values.size(); ++i) {
        if (i > 0) file << ",";
        file << values[i];
    }
    file << std::endl;
    return true;
}

std::vector<std::vector<std::string>> DBMS::readRecords(const std::string& tableName) {
    std::vector<std::vector<std::string>> records;
    std::string tablePath = getTablePath(tableName);
    std::ifstream file(tablePath);
    if (!file) return records;

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty()) continue;
        records.push_back(splitString(line, ','));
    }

    return records;
}

bool DBMS::evaluateCondition(const std::vector<std::string>& record,
                           const std::string& condition,
                           const std::map<std::string, size_t>& columnIndices) {
    if (condition.empty()) {
        return true;  // û��������ƥ�����м�¼
    }

    // ��������
    std::vector<std::string> andConditions = splitString(condition, " AND ");
    bool result = true;

    for (const auto& andCondition : andConditions) {
        bool orResult = false;
        std::vector<std::string> orConditions = splitString(andCondition, " OR ");

        for (const auto& orCondition : orConditions) {
            // ȥ�����źͿո�
            std::string cleanCondition = orCondition;
            cleanCondition.erase(std::remove(cleanCondition.begin(), cleanCondition.end(), '('), cleanCondition.end());
            cleanCondition.erase(std::remove(cleanCondition.begin(), cleanCondition.end(), ')'), cleanCondition.end());
            cleanCondition.erase(0, cleanCondition.find_first_not_of(" \t"));
            cleanCondition.erase(cleanCondition.find_last_not_of(" \t") + 1);

            // ���ұȽ������
            size_t opPos = std::string::npos;
            std::string op;
            if ((opPos = cleanCondition.find("!=")) != std::string::npos) op = "!=";
            else if ((opPos = cleanCondition.find("=")) != std::string::npos) op = "=";
            else if ((opPos = cleanCondition.find(">")) != std::string::npos) op = ">";
            else if ((opPos = cleanCondition.find("<")) != std::string::npos) op = "<";

            if (opPos != std::string::npos) {
                std::string colName = cleanCondition.substr(0, opPos);
                std::string value = cleanCondition.substr(opPos + op.length());
                
                // ����������ֵ
                colName.erase(0, colName.find_first_not_of(" \t"));
                colName.erase(colName.find_last_not_of(" \t") + 1);
                value.erase(0, value.find_first_not_of(" \t"));
                value.erase(value.find_last_not_of(" \t") + 1);
                
                // �ر����ַ����Ƚϣ�Ѱ�Ҵ����ŵ�ֵ
                if (!value.empty()) {
                    if (value.front() == '\'' && value.back() == '\'') {
                        value = value.substr(1, value.length() - 2);
                    }
                }

                auto it = columnIndices.find(colName);
                if (it != columnIndices.end()) {
                    size_t colIndex = it->second;
                    if (colIndex < record.size()) {
                        std::string recordValue = record[colIndex];
                        if (!recordValue.empty() && recordValue.front() == '\'' && recordValue.back() == '\'') {
                            recordValue = recordValue.substr(1, recordValue.length() - 2);
                        }

                        // �Ƚ�ֵ
                        bool compareResult = false;
                        if (op == "=") {
                            compareResult = (recordValue == value);
                        } else if (op == "!=") {
                            compareResult = (recordValue != value);
                        } else if (op == ">") {
                            // ������ֵ�Ƚ�
                            try {
                                int recordNum = std::stoi(recordValue);
                                int valueNum = std::stoi(value);
                                compareResult = (recordNum > valueNum);
                            } catch (...) {
                                compareResult = (recordValue > value);
                            }
                        } else if (op == "<") {
                            // ������ֵ�Ƚ�
                            try {
                                int recordNum = std::stoi(recordValue);
                                int valueNum = std::stoi(value);
                                compareResult = (recordNum < valueNum);
                            } catch (...) {
                                compareResult = (recordValue < value);
                            }
                        }

                        orResult = orResult || compareResult;
                    }
                }
            }
        }

        result = result && orResult;
    }

    return result;
}