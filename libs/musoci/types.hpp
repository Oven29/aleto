#pragma once

#include <iostream>
#include <string>
#include <vector>

namespace types {

class IData {
 public:
    virtual ~IData() = default;

    virtual std::string getString() const = 0;
};

class Column : public IData {
 public:
    std::string name;
    bool nullable = false;
    bool primary_key = false;
    std::string type;

    Column() = default;

    Column(std::string name, bool nullable, bool primary_key, std::string type)
        : name(std::move(name)), nullable(nullable), primary_key(primary_key), type(std::move(type)) {}

    std::string getString() const override {
        std::string s = "COLUMN " + name + " " + type;
        if (!nullable)
            s += " NOT NULL";
        if (primary_key)
            s += " PRIMARY KEY";
        return s;
    }
};

class TableSchema : public IData {
 public:
    std::string title;
    std::vector<Column> columns;

    TableSchema() = default;

    TableSchema(std::string title, std::vector<Column> columns) : title(std::move(title)), columns(std::move(columns)) {};

    std::string getString() const override {
        std::string s = "TABLE " + title + " (";
        for (const auto& column : columns) {
            s += column.getString() + ", ";
        }
        s += ");\n";
        return s;
    }
};

class TableData : public TableSchema {
 public:
    std::vector<std::vector<std::string>> data;
    int page = 0;
    int count = 0;

    TableData() = default;

    TableData(std::string title, std::vector<Column> columns, std::vector<std::vector<std::string>> data, int page, int count)
        : TableSchema(std::move(title), std::move(columns)), data(std::move(data)), page(page), count(count) {}
};

}  // namespace types
