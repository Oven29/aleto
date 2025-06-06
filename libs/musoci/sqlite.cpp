#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "sqlite.hpp"

namespace sqlite {

SQLiteDB::SQLiteDB(const std::string& path) : dbPath(path) {
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        throw std::runtime_error("Failed to open database: " + std::string(sqlite3_errmsg(db)));
        db = nullptr;
    }
}

SQLiteDB::~SQLiteDB() {
    if (db)
        sqlite3_close(db);
}

bool SQLiteDB::executeQuery(const std::string& sql) {
    char* errMsg = nullptr;
    if (sqlite3_exec(db, sql.c_str(), nullptr, nullptr, &errMsg) != SQLITE_OK) {
        throw std::runtime_error("Failed to execute query: " + std::string(errMsg));
        sqlite3_free(errMsg);
        return false;
    }
    return true;
}

std::vector<types::TableSchema> SQLiteDB::getTables() {
    std::vector<types::TableSchema> tables;
    sqlite3_stmt* stmt;

    // Получаем список таблиц
    const char* sql = "SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%';";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        throw std::runtime_error("Failed to prepare statement for table list");
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string tableName = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        types::TableSchema table{tableName, {}};

        // Загружаем колонки этой таблицы
        std::string pragma = "PRAGMA table_info('" + tableName + "');";
        sqlite3_stmt* colStmt;
        if (sqlite3_prepare_v2(db, pragma.c_str(), -1, &colStmt, nullptr) != SQLITE_OK) {
            throw std::runtime_error("Failed to get columns for table: " + tableName);
        }

        while (sqlite3_step(colStmt) == SQLITE_ROW) {
            types::Column col;
            col.name = reinterpret_cast<const char*>(sqlite3_column_text(colStmt, 1));  // name
            col.type = reinterpret_cast<const char*>(sqlite3_column_text(colStmt, 2));  // type
            table.columns.push_back(col);
        }

        sqlite3_finalize(colStmt);
        tables.push_back(table);
    }

    sqlite3_finalize(stmt);
    return tables;
}

types::TableData SQLiteDB::select(const std::string& table, int offset, int limit) {
    types::TableData result;
    result.title = table;

    std::ostringstream query;
    query << "SELECT * FROM " << table << " LIMIT " << limit << " OFFSET " << offset << ";";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, query.str().c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        return result;
    }

    int colCount = sqlite3_column_count(stmt);
    for (int i = 0; i < colCount; ++i) {
        result.columns.push_back(types::Column(sqlite3_column_name(stmt, i), true, false, sqlite3_column_decltype(stmt, i)));
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::vector<std::string> row;
        for (int i = 0; i < colCount; ++i) {
            const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            row.emplace_back(text ? text : "");
        }
        result.data.push_back(std::move(row));
    }

    result.count = static_cast<int>(result.data.size());
    sqlite3_finalize(stmt);
    return result;
}

bool SQLiteDB::editRow(const std::string& table, const std::pair<std::string, std::string>& where,
                       const std::vector<std::pair<std::string, std::string>>& values) {
    std::ostringstream query;
    query << "UPDATE " << table << " SET ";
    for (size_t i = 0; i < values.size(); ++i) {
        query << values[i].first << " = ?";
        if (i + 1 < values.size())
            query << ", ";
    }
    query << " WHERE " << where.first << " = " << where.second << ";";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.str().c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    for (size_t i = 0; i < values.size(); ++i) {
        sqlite3_bind_text(stmt, static_cast<int>(i + 1), values[i].second.c_str(), -1, SQLITE_STATIC);
    }

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

bool SQLiteDB::addRow(const std::string& table, const std::vector<std::pair<std::string, std::string>>& values) {
    std::ostringstream query;
    query << "INSERT INTO " << table << " (";
    for (size_t i = 0; i < values.size(); ++i) {
        query << values[i].first;
        if (i + 1 < values.size())
            query << ", ";
    }
    query << ") VALUES (";
    for (size_t i = 0; i < values.size(); ++i) {
        query << "?";
        if (i + 1 < values.size())
            query << ", ";
    }
    query << ");";

    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.str().c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return false;

    for (size_t i = 0; i < values.size(); ++i) {
        sqlite3_bind_text(stmt, static_cast<int>(i + 1), values[i].second.c_str(), -1, SQLITE_STATIC);
    }

    bool success = sqlite3_step(stmt) == SQLITE_DONE;
    sqlite3_finalize(stmt);
    return success;
}

types::TableData SQLiteDB::search(const std::string& table, const std::string& column, const std::string& pattern, int limit) {
    types::TableData result;
    result.title = table;

    std::ostringstream query;
    query << "SELECT * FROM " << table << " WHERE " << column << " LIKE ? LIMIT " << limit << ";";
    sqlite3_stmt* stmt;

    if (sqlite3_prepare_v2(db, query.str().c_str(), -1, &stmt, nullptr) != SQLITE_OK)
        return result;

    std::string wildcard = "%" + pattern + "%";
    sqlite3_bind_text(stmt, 1, wildcard.c_str(), -1, SQLITE_STATIC);

    int colCount = sqlite3_column_count(stmt);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::vector<std::string> row;
        for (int i = 0; i < colCount; ++i) {
            const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            row.emplace_back(text ? text : "");
        }
        result.data.push_back(std::move(row));
    }

    result.count = static_cast<int>(result.data.size());
    sqlite3_finalize(stmt);
    return result;
}

bool SQLiteDB::createTable(const types::TableSchema& schema) {
    std::ostringstream query;
    query << "CREATE TABLE IF NOT EXISTS " << schema.title << " (";
    for (size_t i = 0; i < schema.columns.size(); ++i) {
        const auto& col = schema.columns[i];
        query << col.name << " " << col.type;
        if (!col.nullable)
            query << " NOT NULL";
        if (col.primary_key)
            query << " PRIMARY KEY";
        if (i + 1 < schema.columns.size())
            query << ", ";
    }
    query << ");";
    return executeQuery(query.str());
}

bool SQLiteDB::dropTable(const std::string& tableName) {
    return executeQuery("DROP TABLE IF EXISTS " + tableName + ";");
}

}  // namespace sqlite
