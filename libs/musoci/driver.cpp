#include <array>
#include <stdexcept>

#include <stdexcept>
#include <string>
#include "driver.hpp"

#if defined(_WIN32)
#include <windows.h>
#include <algorithm>
#elif defined(__APPLE__)
#include <limits.h>
#include <mach-o/dyld.h>
#else
#include <limits.h>
#include <unistd.h>
#endif

namespace {

const char* SQLDB_DRIVER_RELATIVE_PATH = "bin/sqldb-driver";

std::string escapeAndWrap(const std::string& input) {
    std::string result = "\"";  // начинаем с кавычки

    for (char ch : input) {
        if (ch == '"') {
            result += "\\\"";  // экранируем кавычку
        } else {
            result += ch;  // остальные символы добавляем как есть
        }
    }

    result += "\"";  // заканчиваем кавычкой
    return result;
};

std::string getExecutablePath(const std::string& relative = "") {
    char path[PATH_MAX];

#if defined(_WIN32)
    if (GetModuleFileNameA(NULL, path, MAX_PATH) == 0) {
        throw std::runtime_error("Failed to get executable path");
    }
    std::string fullPath(path);
    std::replace(fullPath.begin(), fullPath.end(), '\\', '/');  // нормализуем
#elif defined(__APPLE__)
    uint32_t size = sizeof(path);
    if (_NSGetExecutablePath(path, &size) != 0) {
        throw std::runtime_error("Buffer too small for executable path");
    }
    std::string fullPath(path);
#else
    ssize_t count = readlink("/proc/self/exe", path, sizeof(path) - 1);
    if (count == -1) {
        throw std::runtime_error("Failed to read /proc/self/exe");
    }
    path[count] = '\0';
    std::string fullPath(path);
#endif

    std::string dir = fullPath.substr(0, fullPath.find_last_of('/'));

    if (relative.empty())
        return dir;
    else
        return dir + "/" + relative;
}

std::string getDriverPath() {
    return getExecutablePath(SQLDB_DRIVER_RELATIVE_PATH);
};

std::string execWithOutput(const std::string& cmd) {
    std::array<char, 128> buffer;
    std::string result;

    // Открываем pipe на чтение
    using FileCloser = int (*)(FILE*);
    std::unique_ptr<FILE, FileCloser> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe) {
        throw std::runtime_error("popen() failed!");
    }

    // Считываем вывод
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
        result += buffer.data();
    }

    return result;
};

}  // namespace

namespace musoci::base {

types::IDataPtr Base::execute(const std::string& command, const std::unordered_map<std::string, std::string>& params,
                              const std::vector<std::string>& args) {
    std::unordered_map<std::string, std::string> namedArgs = this->getDefaultParams();
    for (const auto& [key, value] : params) {
        namedArgs[key] = value;
    }

    std::string cmd = "\"" + getDriverPath() + "\"";

    for (const auto& [key, value] : namedArgs) {
        cmd += " --" + key + "=" + escapeAndWrap(value);
    }

    cmd += " " + command;
    for (const auto& arg : args) {
        cmd += " " + escapeAndWrap(arg);
    }

    std::cout << cmd << std::endl;
    std::string resp = execWithOutput(cmd.c_str());
    std::cout << resp << std::endl;
    types::Answer res = types::Answer::from_json(json::parse(resp));
    if (!res.ok) {
        throw std::runtime_error(res.error_message);
    }
    return res.data;
};

std::shared_ptr<types::SchemaListData> Base::connect() {
    return std::dynamic_pointer_cast<types::SchemaListData>(this->execute("connect", {}, {}));
};

std::shared_ptr<types::TableData> Base::get(const std::string& tablename, int page) {
    std::vector args = {tablename, std::to_string(page)};
    return std::dynamic_pointer_cast<types::TableData>(this->execute("get", {}, args));
};

std::shared_ptr<types::TableSchema> Base::createTable(const types::TableSchema& data) {
    std::vector args = {data.to_json().dump()};
    return std::dynamic_pointer_cast<types::TableSchema>(this->execute("create_table", {}, args));
};

std::shared_ptr<types::TableSchema> Base::dropTable(const std::string& tablename) {
    std::vector args = {tablename};
    return std::dynamic_pointer_cast<types::TableSchema>(this->execute("drop_table", {}, args));
};

std::shared_ptr<types::TableSchema> Base::alterTable(const types::TableSchema& data) {
    std::vector args = {data.to_json().dump()};
    return std::dynamic_pointer_cast<types::TableSchema>(this->execute("alter_table", {}, args));
};

std::shared_ptr<types::TableSchema> Base::addRow(const std::string& tablename, const std::vector<std::string>& data,
                                                 const std::unordered_map<std::string, std::string>& namedData) {
    std::vector args = {tablename};
    for (const auto& item : data) {
        args.push_back(item);
    }
    return std::dynamic_pointer_cast<types::TableSchema>(this->execute("add_row", namedData, args));
};

std::shared_ptr<types::TableSchema> Base::addColumn(const std::string& tablename, const types::Column& data) {
    std::vector args = {tablename, data.to_json().dump()};
    return std::dynamic_pointer_cast<types::TableSchema>(this->execute("add_column", {}, args));
};

}  // namespace musoci::base
