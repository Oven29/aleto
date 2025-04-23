#pragma once

#include <iostream>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "json.hpp"

using json = nlohmann::json;

namespace types {

class IData {
public:
  virtual ~IData() = default;
  virtual json to_json() const = 0;
  virtual std::string type_name() const = 0;
};

inline std::ostream &operator<<(std::ostream &os, const IData &data) {
  os << data.to_json().dump(2);
  return os;
}

class Column : public IData {
public:
  std::string name;
  bool nullable = false;
  bool primary_key = false;
  std::string type;

  Column() = default;

  Column(std::string name, bool nullable, bool primary_key, std::string type)
      : name(std::move(name)), nullable(nullable), primary_key(primary_key),
        type(std::move(type)) {}

  json to_json() const override {
    return {{"name", name},
            {"nullable", nullable},
            {"primary_key", primary_key},
            {"type", type}};
  }

  std::string type_name() const override { return "Column"; }

  static std::shared_ptr<Column> from_json(const json &j) {
    auto col = std::make_shared<Column>();
    col->name = j.at("name");
    col->nullable = j.value("nullable", false);
    col->primary_key = j.value("primary_key", false);
    col->type = j.at("type");
    return col;
  }
};

class TableSchema : public IData {
public:
  std::string title;
  std::vector<std::shared_ptr<Column>> columns;

  json to_json() const override {
    json j;
    j["title"] = title;
    j["columns"] = json::array();
    for (const auto &col : columns)
      j["columns"].push_back(col->to_json());
    return j;
  }

  std::string type_name() const override { return "TableSchema"; }

  static std::shared_ptr<TableSchema> from_json(const json &j) {
    auto ts = std::make_shared<TableSchema>();
    ts->title = j.at("title");
    for (const auto &item : j["columns"]) {
      ts->columns.push_back(Column::from_json(item));
    }
    return ts;
  }
};

class TableData : public TableSchema {
public:
  std::vector<std::vector<std::string>> data;
  int page = 0;
  int count = 0;

  json to_json() const override {
    json j = TableSchema::to_json();
    j["data"] = data;
    j["page"] = page;
    j["count"] = count;
    return j;
  }

  std::string type_name() const override { return "TableData"; }

  static std::shared_ptr<TableData> from_json(const json &j) {
    auto td = std::make_shared<TableData>();
    td->title = j.at("title");
    for (const auto &item : j["columns"]) {
      td->columns.push_back(Column::from_json(item));
    }
    td->data = j.value("data", std::vector<std::vector<std::string>>{});
    td->page = j.value("page", 0);
    td->count = j.value("count", 0);
    return td;
  }
};

class SchemaListData : public IData {
public:
  std::vector<TableSchema> items;

  json to_json() const override {
    json arr = json::array();
    for (const auto &item : items)
      arr.push_back(item.to_json());
    return arr;
  }

  std::string type_name() const override { return "SchemaListData"; }

  static std::shared_ptr<SchemaListData> from_json(const json &j) {
    auto ptr = std::make_shared<SchemaListData>();
    for (const auto &item : j["items"])
      ptr->items.push_back(*TableSchema::from_json(item));
    return ptr;
  }
};

class Answer {
public:
  bool ok;
  std::shared_ptr<IData> data = nullptr;
  std::optional<std::string> error_message;

  json to_json() const {
    json j;
    j["ok"] = ok;
    j["error_message"] =
        error_message.has_value() ? json(error_message.value()) : nullptr;
    j["data_type"] = data ? data->type_name() : "None";
    j["data"] = data ? data->to_json() : nullptr;
    return j;
  }

  static Answer from_json(const json &j) {
    Answer a;
    a.ok = j.at("ok").get<bool>();
    if (!j["error_message"].is_null())
      a.error_message = j["error_message"].get<std::string>();

      
    if (j.contains("data") && !j["data"].is_null()) {
      std::string type = j["data_type"];
      if (type == "TableData") {
        a.data = TableData::from_json(j["data"]);
      } else if (type == "TableSchema") {
        a.data = TableSchema::from_json(j["data"]);
      } else if (type == "SchemaListData") {
        a.data = SchemaListData::from_json(j["data"]);
      }
    }

    return a;
  }
};

typedef std::shared_ptr<types::IData> IDataPtr;

} // namespace types
