#include "../include/cppminidb/MiniDB.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <set>

MiniDB::MiniDB(const std::string &tableName) : tableName_(tableName) {}

void MiniDB::setColumns(const std::vector<std::string> &columnNames)
{
    columns_ = columnNames;
}

void MiniDB::insertRow(const std::vector<std::string> &values)
{
    if (columns_.empty())
    {
        throw std::runtime_error("Columns must be defined before inserting rows.");
    }

    if (values.size() != columns_.size())
    {
        throw std::invalid_argument("Number of values must match the number of columns.");
    }

    rows_.push_back(values);
}

std::string MiniDB::getTableFilePath() const
{
    return "./data/" + tableName_ + ".tbl";
}

std::string MiniDB::getTempFilePath() const
{
    return "./data/" + tableName_ + "_temp.tbl";
}

void MiniDB::save() const
{
    std::filesystem::create_directories("data");
    std::ofstream outFile(getTableFilePath(), std::ios::trunc); // overwrite;
    if (!outFile.is_open())
    {
        throw std::runtime_error("Failed to open file for writing.");
    }

    for (size_t i = 0; i < columns_.size(); ++i)
    {
        outFile << columns_[i];
        if (i != columns_.size() - 1)
            outFile << ",";
    }
    outFile << "\n";

    for (const auto &row : rows_)
    {
        for (size_t i = 0; i < columns_.size(); ++i)
        {
            if (i < row.size())
                outFile << row[i];
            if (i != row.size() - 1)
                outFile << ",";
        }
        outFile << "\n";
    }
}

std::vector<std::map<std::string, std::string>> MiniDB::loadFromDisk() const
{
    std::vector<std::map<std::string, std::string>> result;

    std::ifstream inFile(getTableFilePath());
    if (!inFile.is_open())
    {
        // throw std::runtime_error("Failed to open file for reading:" + getTableFilePath());
        return {};
    }

    std::string line;

    // read column headers
    std::getline(inFile, line);
    std::vector<std::string> fileColumns;
    std::stringstream headerStream(line);
    std::string header;
    while (std::getline(headerStream, header, ','))
    {
        fileColumns.push_back(header);
    }

    // read data rows
    while (std::getline(inFile, line))
    {
        std::vector<std::string> values;
        std::stringstream rowStream(line);
        std::string value;

        while (std::getline(rowStream, value, ','))
        {
            values.push_back(value);
        }

        // Pad missing values with empty strings if necessary
        while (values.size() < fileColumns.size())
        {
            values.push_back("");
        }

        std::map<std::string, std::string> rowMap;
        for (size_t i = 0; i < fileColumns.size(); ++i)
        {
            rowMap[fileColumns[i]] = values[i];
        }

        result.push_back(rowMap);
    }

    return result;
}

std::vector<std::map<std::string, std::string>> MiniDB::selectAll() const
{
    std::vector<std::map<std::string, std::string>> result;

    for (const auto &row : rows_)
    {
        if (row.size() != columns_.size())
            continue;

        std::map<std::string, std::string> rowMap;
        for (size_t i = 0; i < columns_.size(); ++i)
        {
            rowMap[columns_[i]] = row[i];
        }
        result.push_back(rowMap);
    }

    return result;
}

void MiniDB::clear()
{
    // Clear the in-memory rows
    rows_.clear();

    // Clear the disk file by opening with trunc mode
    std::ofstream outFile(getTableFilePath(), std::ios::trunc);

    if (!outFile.is_open())
    {
        throw std::runtime_error("Failed to open file for writing.");
    }

    // Since the file is empty after opening with trunc mode, re-write only the header row
    for (size_t i = 0; i < columns_.size(); ++i)
    {
        outFile << columns_[i];
        if (i != columns_.size() - 1)
            outFile << ",";
    }
    outFile << "\n";
}

std::string MiniDB::exportToJsonLegacy() const
{
    std::ostringstream oss;

    oss << "[";
    for (const auto &row : rows_)
    {
        oss << "{";
        for (size_t i = 0; i < row.size(); ++i)
        {
            oss << "\"" << columns_[i] << "\":\"" << row[i] << "\"";
            if (i != row.size() - 1)
                oss << ",";
        }
        oss << "}";
        if (&row != &rows_.back())
            oss << ",";

        oss << "\n";
    }

    oss << "]";

    return oss.str();
}

bool MiniDB::compare(int a, const std::string &op, int b) const
{
    if (op == "==")
        return a == b;
    if (op == "!=")
        return a != b;
    if (op == ">")
        return a > b;
    if (op == ">=")
        return a >= b;
    if (op == "<")
        return a < b;
    if (op == "<=")
        return a <= b;

    throw std::invalid_argument("Unsupported operator for integer: " + op);
}

bool MiniDB::compare(std::string a, const std::string &op, std::string b) const
{
    if (op == "==")
        return a == b;
    if (op == "!=")
        return a != b;

    throw std::invalid_argument("Unsupported operator for string comparison: " + op);
}

std::vector<std::map<std::string, std::string>> MiniDB::selectWhereFromMemory(
    const std::string &column,
    const std::string &op,
    const std::string &value) const
{
    std::vector<std::map<std::string, std::string>> result;

    bool columnExists = std::find(columns_.begin(), columns_.end(), column) != columns_.end();
    if (!columnExists)
        throw std::invalid_argument("Column not found: " + column);

    auto it = std::find(columns_.begin(), columns_.end(), column);
    size_t colIndex = std::distance(columns_.begin(), it);

    for (const auto &row : rows_)
    {
        if (columns_.size() != row.size())
            continue;

        const std::string &cell = row[colIndex];
        if (NumberValidator::isPureInteger(cell) && NumberValidator::isPureInteger(value))
        {
            int rowValue = std::stoi(row[colIndex]);
            int targetValue = std::stoi(value);

            if (MiniDB::compare(rowValue, op, targetValue))
            {
                std::map<std::string, std::string> rowMap;
                for (size_t i = 0; i < columns_.size(); ++i)
                {
                    rowMap[columns_[i]] = row[i];
                }
                result.push_back(rowMap);
            }
        }
        else if (op == "=" && op == "!=")
        {
            if (MiniDB::compare(cell, op, value))
            {
                std::map<std::string, std::string> rowMap;
                for (size_t i = 0; i < columns_.size(); ++i)
                {
                    rowMap[columns_[i]] = row[i];
                }
                result.push_back(rowMap);
            }
        }
        else
        {
            continue;
        }
    }
    return result;
}

std::vector<std::map<std::string, std::string>> MiniDB::selectWhereFromDisk(
    const std::string &column,
    const std::string &op,
    const std::string &value) const
{
    std::vector<std::map<std::string, std::string>> result;

    std::ifstream inFile(getTableFilePath());
    if (!inFile.is_open())
    {
        throw std::runtime_error("Failed to open file for reading.");
    }

    std::string line;

    // read column headers
    std::getline(inFile, line);
    std::vector<std::string> fileColumns;
    std::stringstream headerStream(line);
    std::string header;
    while (std::getline(headerStream, header, ','))
    {
        fileColumns.push_back(header);
    }

    // read data rows
    while (std::getline(inFile, line))
    {
        std::vector<std::string> values;
        std::stringstream rowStream(line);
        std::string cell;

        while (std::getline(rowStream, cell, ','))
        {
            values.push_back(cell);
        }

        // Pad missing values with empty strings if necessary
        while (values.size() < fileColumns.size())
        {
            values.push_back("");
        }

        auto it = std::find(fileColumns.begin(), fileColumns.end(), column);
        if (it != fileColumns.end())
        {
            size_t colIndex = std::distance(fileColumns.begin(), it);

            if (NumberValidator::isPureInteger(values[colIndex]) && NumberValidator::isPureInteger(value))
            {
                int rowValue = std::stoi(values[colIndex]);
                int targetValue = std::stoi(value);

                if (MiniDB::compare(rowValue, op, targetValue))
                {
                    std::map<std::string, std::string> rowMap;
                    for (size_t i = 0; i < fileColumns.size(); ++i)
                    {
                        rowMap[fileColumns[i]] = values[i];
                    }
                    result.push_back(rowMap);
                }
            }
            else if (op == "=" || op == "!=")
            {
                if (MiniDB::compare(values[colIndex], op, value))
                {
                    std::map<std::string, std::string> rowMap;
                    for (size_t i = 0; i < fileColumns.size(); ++i)
                    {
                        rowMap[fileColumns[i]] = values[i];
                    }
                    result.push_back(rowMap);
                }
            }
        }
    }
    return result;
}

void MiniDB::updateWhereFromMemory(const std::string &column,
                                   const std::string &op,
                                   const std::string &value,
                                   const std::map<std::string, std::string> &updateMap)
{

    // Check if the target column exists
    if (std::find(columns_.begin(), columns_.end(), column) == columns_.end())
    {
        throw std::invalid_argument("Target column not found: " + column);
    }

    // Check if update keys exist in columns
    for (const auto &[key, _] : updateMap)
    {
        if (std::find(columns_.begin(), columns_.end(), key) == columns_.end())
        {
            throw std::invalid_argument("Update column not found: " + key);
        }
    }

    auto it = std::find(columns_.begin(), columns_.end(), column);
    size_t colIndex = std::distance(columns_.begin(), it);

    for (auto &row : rows_)
    {
        if (row.size() != columns_.size())
        {
            continue;
        }

        const std::string &cell = row[colIndex];

        if (NumberValidator::isPureInteger(cell) && NumberValidator::isPureInteger(value))
        {
            int rowValue = std::stoi(cell);
            int targetValue = std::stoi(value);

            if (!MiniDB::compare(rowValue, op, targetValue))
            {
                continue;
            }
        }
        else if (op == "=" || op == "!=")
        {
            if (!MiniDB::compare(cell, op, value))
            {
                continue;
            }
        }
        else
        {
            continue;
        }

        for (const auto &[key, newValue] : updateMap)
        {
            auto updateIt = std::find(columns_.begin(), columns_.end(), key);
            if (updateIt != columns_.end())
            {
                size_t updateIndex = std::distance(columns_.begin(), updateIt);
                row[updateIndex] = newValue;
            }
        }
    }
}

void MiniDB::updateWhereFromDisk(const std::string &column,
                                 const std::string &op,
                                 const std::string &value,
                                 const std::map<std::string, std::string> &updateMap)
{
    std::ifstream inFile(getTableFilePath());
    if (!inFile.is_open())
    {
        throw std::runtime_error("Failed to open file for reading.");
    }

    std::string line;

    // read column headers
    std::getline(inFile, line);
    std::vector<std::string> fileColumns;
    std::stringstream headerStream(line);
    std::string header;
    while (std::getline(headerStream, header, ','))
    {
        fileColumns.push_back(header);
    }

    // open temp file
    std::ofstream outFile(getTempFilePath());
    if (!outFile.is_open())
    {
        throw std::runtime_error("Failed to open temporary file for writing.");
    }

    // write headers to temp file
    outFile << line << "\n";

    size_t colIndex = std::distance(fileColumns.begin(), std::find(fileColumns.begin(), fileColumns.end(), column));

    while (std::getline(inFile, line))
    {
        std::vector<std::string> values;
        std::stringstream rowStream(line);
        std::string cell;

        while (std::getline(rowStream, cell, ','))
        {
            values.push_back(cell);
        }

        while (values.size() < fileColumns.size())
        {
            values.push_back("");
        }

        const std::string &cellValue = values[colIndex];
        bool shouldUpdate = false;

        if (NumberValidator::isPureInteger(cellValue) && NumberValidator::isPureInteger(value))
        {
            int rowValue = std::stoi(cellValue);
            int targetValue = std::stoi(value);

            shouldUpdate = MiniDB::compare(rowValue, op, targetValue);
        }
        else if (op == "=" || op == "!=")
        {
            shouldUpdate = MiniDB::compare(cellValue, op, value);
        }
        else
        {
            continue;
        }
        if (shouldUpdate)
        {
            for (const auto &[key, newValue] : updateMap)
            {
                auto updateIt = std::find(fileColumns.begin(), fileColumns.end(), key);
                if (updateIt != fileColumns.end())
                {
                    size_t updateIndex = std::distance(fileColumns.begin(), updateIt);
                    values[updateIndex] = newValue;
                }
            }
        }

        for (size_t i = 0; i < values.size(); ++i)
        {
            outFile << values[i];
            if (i != values.size() - 1)
                outFile << ",";
        }
        outFile << "\n";
    }
    inFile.close();
    outFile.close();
    std::filesystem::rename(getTempFilePath(), getTableFilePath());
}

void MiniDB::deleteWhereFromMemory(const std::string &column,
                                   const std::string &op,
                                   const std::string &value)
{
    if (std::find(columns_.begin(), columns_.end(), column) == columns_.end())
    {
        throw std::invalid_argument("Target column not found: " + column);
    }

    auto colIndex = std::distance(columns_.begin(), std::find(columns_.begin(), columns_.end(), column));

    auto filteredRows = std::remove_if(
        rows_.begin(), rows_.end(),
        [colIndex, op, value, this](const std::vector<std::string> &row)
        {
            if (row.size() != columns_.size())
                return false;

            const std::string &cell = row[colIndex];

            if (NumberValidator::isPureInteger(cell) && NumberValidator::isPureInteger(value))
            {
                int rowVal = std::stoi(cell);
                int cmpVal = std::stoi(value);

                return MiniDB::compare(rowVal, op, cmpVal);
            }
            else if (op == "=" || op == "!=")
            {
                return MiniDB::compare(cell, op, value);
            }
            else
            {
                return false;
            }
        });

    rows_.erase(filteredRows, rows_.end());
}

void MiniDB::deleteWhereFromDisk(const std::string &column,
                                 const std::string &op,
                                 const std::string &value)
{
    std::ifstream inFile(getTableFilePath());
    if (!inFile.is_open())
        throw std::runtime_error("Failed to open file for reading.");

    std::ofstream outFile(getTempFilePath());
    if (!outFile.is_open())
        throw std::runtime_error("Failed to open temporary file for writing.");

    // read header
    std::string line;
    std::getline(inFile, line);
    outFile << line << "\n";

    // tokenize header
    std::vector<std::string> fileColumns;
    std::stringstream headerStream(line);
    std::string header;
    while (std::getline(headerStream, header, ','))
    {
        fileColumns.push_back(header);
    }

    // find the index of the target column
    auto it = std::find(fileColumns.begin(), fileColumns.end(), column);
    if (it == fileColumns.end())
        throw std::invalid_argument("Target column not found: " + column);

    auto colIndex = std::distance(fileColumns.begin(), it);
    while (std::getline(inFile, line))
    {
        std::vector<std::string> rowValues;
        std::stringstream rowStream(line);
        std::string cell;

        // tokenize each row
        while (std::getline(rowStream, cell, ','))
        {
            rowValues.push_back(cell);
        }

        // padding if row is shorter than header
        while (rowValues.size() < fileColumns.size())
        {
            rowValues.push_back("");
        }

        const std::string &cellValue = rowValues[colIndex];
        bool shouldDelete = false;

        if (NumberValidator::isPureInteger(cellValue) && NumberValidator::isPureInteger(value))
        {
            int rowVal = std::stoi(cellValue);
            int cmpVal = std::stoi(value);

            shouldDelete = MiniDB::compare(rowVal, op, cmpVal);
        }
        else if (op == "=" || op == "!=")
        {
            shouldDelete = MiniDB::compare(cellValue, op, value);
        }
        else
        {
            shouldDelete = false;
        }

        if (!shouldDelete)
        {
            for (size_t i = 0; i < rowValues.size(); ++i)
            {
                outFile << rowValues[i];
                if (i != rowValues.size() - 1)
                    outFile << ",";
            }
            outFile << "\n";
        }
    }
    inFile.close();
    outFile.close();
    std::filesystem::rename(getTempFilePath(), getTableFilePath());
}

std::string MiniDB::exportToJson() const
{
    if (columns_.empty())
    {
        throw std::runtime_error("No columns defined.Columns must be defined before exporting to JSON.");
    }

    using json = nlohmann::json;
    json rowsJson = json::array();

    for (const auto &row : rows_)
    {
        json rowObj = json::object();
        for (size_t i = 0; i < columns_.size(); ++i)
        {
            std::string cellValue(i < row.size() ? row[i] : "");
            rowObj[columns_[i]] = cellValue;
        }
        rowsJson.push_back(rowObj);
    }

    return rowsJson.dump(4);
}

std::string MiniDB::exportToJsonFromDisk() const
{
    std::ifstream inFile(getTableFilePath());
    if (!inFile.is_open())
        throw std::runtime_error("Failed to open file for reading.");

    std::string line;
    std::getline(inFile, line);
    std::vector<std::string> fileColumns;
    std::stringstream headerStream(line);
    std::string header;
    while (std::getline(headerStream, header, ','))
    {
        fileColumns.push_back(header);
    }

    // start building JSON array
    nlohmann::json jsonArray = nlohmann::json::array();

    while (getline(inFile, line))
    {
        std::vector<std::string> rowValues;
        std::stringstream rowStream(line);
        std::string cell;

        // tokenize each row
        while (std::getline(rowStream, cell, ','))
        {
            rowValues.push_back(cell);
        }

        // construct JSON object for this row
        nlohmann::json rowObj = nlohmann::json::object();
        for (size_t i = 0; i < fileColumns.size(); ++i)
        {
            rowObj[fileColumns[i]] = rowValues[i];
        }

        // add this row to the JSON array
        jsonArray.push_back(rowObj);
    }

    return jsonArray.dump(4);
}

void MiniDB::importFromJson(const std::string &jsonString)
{

    nlohmann::json parsed;
    try
    {
        parsed = nlohmann::json::parse(jsonString);
    }
    catch (const nlohmann::json::parse_error &e)
    {
        throw std::runtime_error("Invalid JSON format: " + std::string(e.what()));
    }

    if (!parsed.is_array())
        throw std::runtime_error("JSON must be an array.");

    if (parsed.empty())
        throw std::runtime_error("JSON array is empty.");

    if (columns_.empty())
    {
        for (auto it = parsed[0].begin(); it != parsed[0].end(); ++it)
        {
            columns_.push_back(it.key());
        }
    }
    else
    {
        std::set<std::string> expected(columns_.begin(), columns_.end());
        for (const auto &item : parsed)
        {
            std::set<std::string> actual;
            for (auto it = item.begin(); it != item.end(); ++it)
            {
                actual.insert(it.key());
            }
            if (actual != expected)
            {
                throw std::invalid_argument("Column mismatch in JSON data.");
            }
        }
    }

    for (const auto &item : parsed)
    {
        std::vector<std::string> row;
        for (const auto &column : columns_)
        {
            row.push_back(item[column].get<std::string>());
        }
        rows_.push_back(row);
    }
}

void MiniDB::importFromJsonToDisk(const std::string &jsonString, bool append)
{
    nlohmann::json parsed;
    try
    {
        parsed = nlohmann::json::parse(jsonString);
    }
    catch (const nlohmann::json::parse_error &e)
    {
        throw std::runtime_error("Invalid JSON format: " + std::string(e.what()));
    }

    if (!parsed.is_array())
        throw std::runtime_error("JSON must be an array of objects.");

    if (parsed.empty())
        throw std::runtime_error("JSON array is empty.");

    if (!parsed.front().is_object())
        throw std::runtime_error("JSON array elements must be objects.");

    std::vector<std::string> jsonColumns;
    jsonColumns.reserve(parsed.front().size());
    for (auto it = parsed.front().begin(); it != parsed.front().end(); ++it)
    {
        jsonColumns.push_back(it.key());
    }

    std::filesystem::create_directories("data");
    const std::string tableFilePath = getTableFilePath();

    if (!append)
    {
        std::ofstream outFile(getTempFilePath(), std::ios::trunc);

        if (!outFile.is_open())
            throw std::runtime_error("Failed to open temp file for writing.");

        // write headers
        for (size_t i = 0; i < jsonColumns.size(); ++i)
        {
            outFile << jsonColumns[i];
            if (i != jsonColumns.size() - 1)
                outFile << ",";
        }
        outFile << "\n";

        // write rows
        for (const auto &item : parsed)
        {
            for (size_t i = 0; i < jsonColumns.size(); ++i)
            {
                std::string cell;
                if (item.contains(jsonColumns[i]) && !item[jsonColumns[i]].is_null())
                {
                    cell = item[jsonColumns[i]].get<std::string>();
                }
                else
                {
                    cell = "";
                }
                outFile << cell;
                if (i != jsonColumns.size() - 1)
                    outFile << ",";
            }
            outFile << "\n";
        }
        outFile.close();
        std::filesystem::rename(getTempFilePath(), tableFilePath);
    }
    else
    {
        // append mode
        std::vector<std::string> fileColumns;
        std::ifstream inFile(tableFilePath);
        if (inFile.is_open())
        {
            std::string headerLine;
            if (!std::getline(inFile, headerLine))
                throw std::runtime_error("Failed to read header line from existing file.");

            std::stringstream headerStream(headerLine);
            std::string header;
            while (std::getline(headerStream, header, ','))
            {
                fileColumns.push_back(header);
            }
            inFile.close();

            std::set<std::string> expected(fileColumns.begin(), fileColumns.end());
            std::set<std::string> actual(jsonColumns.begin(), jsonColumns.end());
            if (actual != expected)
                throw std::invalid_argument("Column mismatch in JSON data in append mode.");
        }
        else
        {
            // file doesn't exist. Create with header
            std::ofstream outFile(tableFilePath, std::ios::trunc);
            if (!outFile.is_open())
                throw std::runtime_error("Failed to open file for writing.");

            // write headers
            for (size_t i = 0; i < jsonColumns.size(); ++i)
            {
                outFile << jsonColumns[i];
                if (i != jsonColumns.size() - 1)
                    outFile << ",";
            }
            outFile << "\n";
            outFile.close();
            fileColumns = jsonColumns; // Since the file was just created, adopt the JSON columns as the file header
        }

        // append rows
        std::ofstream outFile(tableFilePath, std::ios::app);
        if (!outFile.is_open())
            throw std::runtime_error("Failed to open file for appending.");

        for (const auto &item : parsed)
        {
            for (size_t i = 0; i < fileColumns.size(); ++i)
            {
                std::string cell;
                if (item.contains(fileColumns[i]) && !item[fileColumns[i]].is_null())
                {
                    cell = item[fileColumns[i]].get<std::string>();
                }
                else
                {
                    cell = "";
                }
                outFile << cell;
                if (i != fileColumns.size() - 1)
                    outFile << ",";
            }
            outFile << "\n";
        }
        outFile.close();
    }
}

void MiniDB::clearMemory()
{
    rows_.clear();
}

void MiniDB::clearDisk(bool keepHeader)
{
    const std::string path = getTableFilePath();

    if (!std::filesystem::exists(path))
        return;

    if (!keepHeader)
    {
        std::error_code ec;
        std::filesystem::remove(path, ec);
        if (ec)
            std::cout << "Error: " << ec.message() << std::endl;
    }

    std::ifstream inFile(path);
    if (!inFile.is_open())
        return;

    std::string headerLine;
    if (!std::getline(inFile, headerLine))
    {
        inFile.close();
        std::ofstream outFile(path, std::ios::trunc);
        return;
    }
    inFile.close();

    std::ofstream outFile(path, std::ios::trunc);
    if (!outFile.is_open())
        return;

    outFile << headerLine << "\n";
    outFile.close();
}

bool NumberValidator::isPureInteger(const std::string &str)
{
    if (str.empty())
        return false;

    for (char c : str)
    {
        if (!std::isdigit(c))
            return false;
    }

    return true;
}

bool NumberValidator::isSignedInteger(const std::string &str)
{
    if (str.empty())
        return false;

    if (str[0] != '+' && str[0] != '-' && !std::isdigit(str[0]))
        return false;

    for (size_t i = 1; i < str.length(); ++i)
    {
        if (!std::isdigit(str[i]))
            return false;
    }

    return true;
}

bool NumberValidator::isFloatingPoint(const std::string &str)
{
    if (str.empty())
        return false;

    if (str[0] != '+' && str[0] != '-' && !std::isdigit(str[0]))
        return false;

    bool hasDecimalPoint = false;
    bool digitFound = false;

    for (size_t i = (str[0] == '+' || str[0] == '-') ? 1 : 0; i < str.length(); ++i)
    {
        if (!std::isdigit(str[i]) && str[i] != '.')
            return false;

        char c = str[i];

        if (std::isdigit(c))
        {
            digitFound = true;
        }
        else if (c == '.' && !hasDecimalPoint)
        {
            hasDecimalPoint = true;
        }
        else
        {
            return false;
        }
    }

    return digitFound;
}