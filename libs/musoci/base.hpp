#pragma once

#include "types.hpp"

namespace base {

class Database {
 public:
    virtual ~Database() = default;
    Database() = default;

    virtual bool executeQuery(const std::string& sql) = 0;
    virtual std::vector<types::TableSchema> getTables() = 0;
    virtual types::TableData select(const std::string& table, int offset, int limit) = 0;
    virtual bool editRow(const std::string& table, const std::pair<std::string, std::string>& where,
                         const std::vector<std::pair<std::string, std::string>>& values) = 0;
    virtual bool addRow(const std::string& table, const std::vector<std::pair<std::string, std::string>>& values) = 0;
    virtual bool removeRow(const std::string& table, const std::pair<std::string, std::string>& where) = 0;
    virtual types::TableData search(const std::string& table, const std::string& column, const std::string& pattern, int limit) = 0;
    virtual bool createTable(const types::TableSchema& schema) = 0;
    virtual bool dropTable(const std::string& tableName) = 0;
};

}  // namespace base
