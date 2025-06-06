#pragma once

#include "../sqlite3/sqlite3.h"

#include "base.hpp"

namespace sqlite {

class SQLiteDB : public base::Database {
 private:
    sqlite3* db = nullptr;
    std::string dbPath;

 public:
    explicit SQLiteDB(const std::string& path);
    ~SQLiteDB() override;

    bool executeQuery(const std::string& sql) override;
    std::vector<types::TableSchema> getTables() override;
    types::TableData select(const std::string& table, int offset, int limit) override;
    bool editRow(const std::string& table, const std::pair<std::string, std::string>& where,
                 const std::vector<std::pair<std::string, std::string>>& values) override;
    bool addRow(const std::string& table, const std::vector<std::pair<std::string, std::string>>& values) override;
    types::TableData search(const std::string& table, const std::string& column, const std::string& pattern, int limit) override;
    bool createTable(const types::TableSchema& schema) override;
    bool dropTable(const std::string& tableName) override;
};

}  // namespace sqlite
