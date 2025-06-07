#include <sstream>

#include "postgresql.hpp"

namespace postgresql {

PostgreSqlDB::PostgreSqlDB(const std::string& host, int port, const std::string& user, const std::string& password, const std::string& database)
    : conn(std::make_unique<pqxx::connection>("host=" + host + " port=" + std::to_string(port) + " dbname=" + database + " user=" + user +
                                              " password=" + password)) {
}

PostgreSqlDB::~PostgreSqlDB() = default;

bool PostgreSqlDB::executeQuery(const std::string& sql) {
    pqxx::work txn(*conn);
    txn.exec(sql);
    txn.commit();
    return true;
}

std::vector<types::TableSchema> PostgreSqlDB::getTables() {
    std::vector<types::TableSchema> result;
    pqxx::work txn(*conn);
    auto res = txn.exec("SELECT table_name FROM information_schema.tables WHERE table_schema = 'public';");

    for (const auto& row : res) {
        std::string table = row[0].as<std::string>();
        std::vector<types::Column> columns;

        auto colRes = txn.exec(
            "SELECT column_name, is_nullable, data_type "
            "FROM information_schema.columns "
            "WHERE table_name = " +
            txn.quote(table) + ";");

        for (const auto& col : colRes) {
            std::string name = col[0].as<std::string>();
            bool nullable = col[1].as<std::string>() == "YES";
            std::string type = col[2].as<std::string>();
            bool primary_key = false;

            auto pk = txn.exec(
                "SELECT kcu.column_name "
                "FROM information_schema.table_constraints tc "
                "JOIN information_schema.key_column_usage kcu "
                "ON tc.constraint_name = kcu.constraint_name "
                "WHERE tc.table_name = " +
                txn.quote(table) + " AND tc.constraint_type = 'PRIMARY KEY';");

            for (const auto& pkcol : pk) {
                if (pkcol[0].as<std::string>() == name) {
                    primary_key = true;
                    break;
                }
            }

            columns.emplace_back(name, nullable, primary_key, type);
        }

        result.emplace_back(table, columns);
    }

    return result;
}

types::TableData PostgreSqlDB::select(const std::string& table, int offset, int limit) {
    auto schema = getTables();
    pqxx::work txn(*conn);
    std::vector<types::Column> columns;

    for (const auto& t : schema) {
        if (t.title == table) {
            columns = t.columns;
            break;
        }
    }

    std::stringstream ss;
    ss << "SELECT * FROM " << txn.quote_name(table) << " OFFSET " << offset << " LIMIT " << limit << ";";

    auto res = txn.exec(ss.str());
    std::vector<std::vector<std::string>> rows;

    for (const auto& row : res) {
        std::vector<std::string> r;
        for (const auto& field : row) {
            r.push_back(field.is_null() ? "NULL" : field.c_str());
        }
        rows.push_back(std::move(r));
    }

    return types::TableData(table, columns, rows, offset / limit, res.size());
}

bool PostgreSqlDB::editRow(const std::string& table, const std::pair<std::string, std::string>& where,
                           const std::vector<std::pair<std::string, std::string>>& values) {
    pqxx::work txn(*conn);
    std::stringstream ss;
    ss << "UPDATE " << txn.quote_name(table) << " SET ";
    for (size_t i = 0; i < values.size(); ++i) {
        ss << txn.quote_name(values[i].first) << " = " << txn.quote(values[i].second);
        if (i < values.size() - 1)
            ss << ", ";
    }
    ss << " WHERE " << txn.quote_name(where.first) << " = " << txn.quote(where.second) << ";";
    txn.exec(ss.str());
    txn.commit();
    return true;
}

bool PostgreSqlDB::addRow(const std::string& table, const std::vector<std::pair<std::string, std::string>>& values) {
    pqxx::work txn(*conn);
    std::stringstream cols, vals;
    for (size_t i = 0; i < values.size(); ++i) {
        cols << txn.quote_name(values[i].first);
        vals << txn.quote(values[i].second);
        if (i < values.size() - 1) {
            cols << ", ";
            vals << ", ";
        }
    }

    std::string sql = "INSERT INTO " + txn.quote_name(table) + " (" + cols.str() + ") VALUES (" + vals.str() + ");";

    txn.exec(sql);
    txn.commit();
    return true;
}

bool PostgreSqlDB::removeRow(const std::string& table, const std::pair<std::string, std::string>& where) {
    pqxx::work txn(*conn);

    std::string sql = "DELETE FROM " + txn.quote_name(table) + " WHERE " + txn.quote_name(where.first) + " = " + txn.quote(where.second);

    txn.exec(sql);
    txn.commit();
    return true;
}

types::TableData PostgreSqlDB::search(const std::string& table, const std::string& column, const std::string& pattern, int limit) {
    auto schemas = getTables();
    std::vector<types::Column> columns;

    pqxx::work txn(*conn);
    std::string sql = "SELECT * FROM " + txn.quote_name(table) + " WHERE CAST(" + txn.quote_name(column) + " AS TEXT) ILIKE " +
                      txn.quote('%' + pattern + '%') + " LIMIT " + std::to_string(limit) + ";";
    auto res = txn.exec(sql);

    for (const auto& s : schemas) {
        if (s.title == table) {
            columns = s.columns;
            break;
        }
    }

    std::vector<std::vector<std::string>> rows;
    for (const auto& row : res) {
        std::vector<std::string> r;
        for (const auto& field : row) {
            r.push_back(field.is_null() ? "NULL" : field.c_str());
        }
        rows.push_back(std::move(r));
    }

    return types::TableData(table, columns, rows, 0, res.size());
}

bool PostgreSqlDB::createTable(const types::TableSchema& schema) {
    pqxx::work txn(*conn);
    std::stringstream ss;
    ss << "CREATE TABLE " << txn.quote_name(schema.title) << " (";
    for (size_t i = 0; i < schema.columns.size(); ++i) {
        const auto& col = schema.columns[i];
        ss << txn.quote_name(col.name) << " " << col.type;
        if (!col.nullable)
            ss << " NOT NULL";
        if (col.primary_key)
            ss << " PRIMARY KEY";
        if (i < schema.columns.size() - 1)
            ss << ", ";
    }
    ss << ");";
    txn.exec(ss.str());
    txn.commit();
    return true;
}

bool PostgreSqlDB::dropTable(const std::string& tableName) {
    pqxx::work txn(*conn);
    txn.exec("DROP TABLE IF EXISTS " + txn.quote_name(tableName) + ";");
    txn.commit();
    return true;
}

}  // namespace postgresql
