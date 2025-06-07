#pragma once

#include <memory>
#include <pqxx/pqxx>

#include "base.hpp"

namespace postgresql {

class PostgreSqlDB : public base::Database {
 public:
    explicit PostgreSqlDB(const std::string& host, int port, const std::string& user, const std::string& password, const std::string& database);
    ~PostgreSqlDB() override;

    bool executeQuery(const std::string& sql) override;
    std::vector<types::TableSchema> getTables() override;
    types::TableData select(const std::string& table, int offset, int limit) override;
    bool editRow(const std::string& table, const std::pair<std::string, std::string>& where,
                 const std::vector<std::pair<std::string, std::string>>& values) override;
    bool addRow(const std::string& table, const std::vector<std::pair<std::string, std::string>>& values) override;
    bool removeRow(const std::string& table, const std::pair<std::string, std::string>& where) override;
    types::TableData search(const std::string& table, const std::string& column, const std::string& pattern, int limit) override;
    bool createTable(const types::TableSchema& schema) override;
    bool dropTable(const std::string& tableName) override;

 private:
    std::unique_ptr<pqxx::connection> conn;
};

}  // namespace postgresql
