#pragma once

#include <iostream>
#include <unordered_map>
#include <vector>

#include "types.hpp"

namespace musoci {

namespace base {

class Base {
 private:
    types::IDataPtr execute(const std::string& command, const std::unordered_map<std::string, std::string>& params,
                            const std::vector<std::string>& args);

 public:
    Base() = default;
    virtual ~Base() = default;

    virtual std::unordered_map<std::string, std::string> getDefaultParams() = 0;

    std::shared_ptr<types::SchemaListData> connect();
    std::shared_ptr<types::TableData> get(const std::string& tablename, int page = 1);
    std::shared_ptr<types::TableSchema> createTable(const types::TableSchema& data);
    std::shared_ptr<types::TableSchema> dropTable(const std::string& tablename);
    std::shared_ptr<types::TableSchema> alterTable(const types::TableSchema& data);
    std::shared_ptr<types::TableSchema> addRow(const std::string& tablename, const std::vector<std::string>& data = {},
                                               const std::unordered_map<std::string, std::string>& namedData = {});
    std::shared_ptr<types::TableSchema> addColumn(const std::string& tablename, const types::Column& data);
};

}  // namespace base

namespace sqlite {

class Sqlite : public base::Base {
 private:
    std::string path;

 public:
    Sqlite(std::string path) : path(path) {};
    ~Sqlite() override = default;

    std::unordered_map<std::string, std::string> getDefaultParams() override {
        std::unordered_map<std::string, std::string> params;
        params["db"] = "sqlite";
        params["path"] = path;
        return params;
    };
};

}  // namespace sqlite

namespace postgresql {

class Postgresql : public base::Base {
 private:
    std::string host;
    int port;
    std::string user;
    std::string password;
    std::string database;

 public:
    Postgresql(std::string host, int port, std::string user, std::string password, std::string database)
        : host(host), port(port), user(user), password(password), database(database) {};
    ~Postgresql() override = default;

    std::unordered_map<std::string, std::string> getDefaultParams() override {
        std::unordered_map<std::string, std::string> params;
        params["db"] = "postgresql";
        params["host"] = host;
        params["port"] = std::to_string(port);
        params["user"] = user;
        params["password"] = password;
        params["database"] = database;
        return params;
    };
};

}  // namespace postgresql

namespace mysql {

class Mysql : public base::Base {
 private:
    std::string host;
    int port;
    std::string user;
    std::string password;
    std::string database;

 public:
    Mysql(std::string host, int port, std::string user, std::string password, std::string database)
        : host(host), port(port), user(user), password(password), database(database) {};
    ~Mysql() override = default;

    std::unordered_map<std::string, std::string> getDefaultParams() override {
        std::unordered_map<std::string, std::string> params;
        params["db"] = "mysql";
        params["host"] = host;
        params["port"] = std::to_string(port);
        params["user"] = user;
        params["password"] = password;
        params["database"] = database;
        return params;
    };
};

}  // namespace mysql

}  // namespace musoci
