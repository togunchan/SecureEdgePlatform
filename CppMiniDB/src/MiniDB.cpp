#include "../include/cppminidb/MiniDB.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>
#include <nlohmann/json.hpp>
#include <set>

MiniDB::MiniDB(const std::string &tableName) : tableName_(tableName) {}

void MiniDB::setColumns(const std::vector<std::string> &names)
{
    if (names.empty())
        throw std::runtime_error("Column names cannot be empty.");
    columns_ = names;
    columnTypes_.assign(names.size(), ColumnType::String);
}

void MiniDB::setColumns(const std::vector<std::string> &names, const std::vector<ColumnType> &types)
{
    if (names.empty())
        throw std::runtime_error("Column names cannot be empty.");
    if (types.empty())
        throw std::runtime_error("Column types cannot be empty.");
    if (names.size() != types.size())
        throw std::runtime_error("Column names and types must have the same size");

    columns_ = names;
    columnTypes_ = types;
}

MiniDB::ColumnType MiniDB::columnTypeOf(const std::string &columnName) const
{
    auto it = std::find(columns_.begin(), columns_.end(), columnName);
    if (it != columns_.end())
    {
        size_t index = std::distance(columns_.begin(), it);
        return columnTypes_.at(index);
    }
    else
    {
        throw std::invalid_argument("Column not found: " + columnName);
    }
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
    std::lock_guard<std::mutex> lock(mtx_);

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
    std::lock_guard<std::mutex> lock(mtx_);
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

bool MiniDB::compareNumeric(int a, const std::string &op, int b) const
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

bool MiniDB::compareString(std::string a, const std::string &op, std::string b) const
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

    const size_t colIndex = std::distance(columns_.begin(), it);

    auto ct = MiniDB::columnTypeOf(column);
    if (!isOpAllowedForType(op, ct))
        throw std::invalid_argument("Operator not allowed for this column type:" + op);

    // here we preparse RHS(right hand side) once if numeric
    int rhsI = 0;          // for int colmuns
    double rhsF = 0.0;     // for float columns
    bool rhsParsed = true; // assume true for string

    if (MiniDB::ColumnType::Int == ct)
        rhsParsed = tryParseInt(value, rhsI);
    else if (MiniDB::ColumnType::Float == ct)
        rhsParsed = tryParseFloat(value, rhsF);

    for (const auto &row : rows_)
    {
        if (columns_.size() != row.size())
            continue;

        const std::string &cell = row[colIndex];
        bool match = false;

        switch (ct)
        {
        case ColumnType::String:
            if (op == "==" || op == "!=")
                match = MiniDB::compareString(cell, op, value);
            break;
        case ColumnType::Int:
        {
            if (!rhsParsed)
                break;
            int cellInt = 0; // left hand side
            if (!tryParseInt(cell, cellInt))
                break;
            match = MiniDB::compareNumeric(cellInt, op, rhsI);
            break;
        }

        case ColumnType::Float:
        {
            if (!rhsParsed)
                break;
            double cellFloat = 0.0; // left hand side
            if (!tryParseFloat(cell, cellFloat))
                break;
            match = MiniDB::compareNumeric(cellFloat, op, rhsF);
            break;
        }

        default:
            break;
        }

        if (match)
        {
            std::map<std::string, std::string> rowMap;
            for (size_t i = 0; i < columns_.size(); ++i)
                rowMap[columns_[i]] = row[i];

            result.push_back(rowMap);
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

                if (MiniDB::compareNumeric(rowValue, op, targetValue))
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
                if (MiniDB::compareString(values[colIndex], op, value))
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

            if (!MiniDB::compareNumeric(rowValue, op, targetValue))
            {
                continue;
            }
        }
        else if (op == "=" || op == "!=")
        {
            if (!MiniDB::compareString(cell, op, value))
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

            shouldUpdate = MiniDB::compareNumeric(rowValue, op, targetValue);
        }
        else if (op == "=" || op == "!=")
        {
            shouldUpdate = MiniDB::compareString(cellValue, op, value);
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

                return MiniDB::compareNumeric(rowVal, op, cmpVal);
            }
            else if (op == "=" || op == "!=")
            {
                return MiniDB::compareString(cell, op, value);
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

            shouldDelete = MiniDB::compareNumeric(rowVal, op, cmpVal);
        }
        else if (op == "=" || op == "!=")
        {
            shouldDelete = MiniDB::compareString(cellValue, op, value);
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
    logs_.clear();
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

bool MiniDB::hasColumn(const std::string &name) const
{
    return std::find(columns_.begin(), columns_.end(), name) != columns_.end();
}

std::size_t MiniDB::columnCount() const noexcept
{
    return columns_.size();
}

std::size_t MiniDB::rowCount() const noexcept
{
    return rows_.size();
}

bool MiniDB::isOpAllowedForType(const std::string &op, ColumnType t)
{
    if (op == "==" || op == "!=")
        return true;

    const bool ordering = (op == ">" || op == ">=" || op == "<" || op == "<=");
    if (!ordering)
        return false;

    return (t == MiniDB::ColumnType::Int || t == MiniDB::ColumnType::Float);
}

bool MiniDB::tryParseInt(const std::string &str, int &out)
{
    if (!NumberValidator::isSignedInteger(str))
        return false;
    try
    {
        out = std::stoi(str);
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
    return false;
}

bool MiniDB::tryParseFloat(const std::string &str, double &out)
{
    if (!NumberValidator::isFloatingPoint(str))
        return false;
    try
    {
        out = std::stof(str);
        return true;
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << '\n';
        return false;
    }
    return false;
}

void MiniDB::appendLog(const std::string &sensorId,
                       uint64_t timestampMs,
                       double value,
                       const std::vector<std::string> &faults)
{
    std::lock_guard<std::mutex> lock(mtx_);
    std::string faultFlags;
    for (size_t i = 0; i < faults.size(); ++i)
    {
        faultFlags += faults[i];
        if (i < faults.size() - 1)
            faultFlags += ",";
    }

    std::vector<std::string> row;
    row.push_back(std::to_string(timestampMs));
    row.push_back(sensorId);
    row.push_back(std::to_string(value));
    row.push_back(faultFlags.empty() ? "-" : faultFlags);

    insertRow(row);
    logs_.push_back(LogEntry{timestampMs, sensorId, value, faults});
}

const std::vector<LogEntry> &MiniDB::getLogs() const
{
    return logs_;
}

void MiniDB::loadLogsIntoMemory()
{
    logs_.clear();

    auto loaded = loadFromDisk();
    for (const auto &row : loaded)
    {
        auto ts = std::stoull(row.at("timestamp_ms"));
        auto sensorId = row.at("sensor_id");
        auto value = std::stod(row.at("value"));

        std::vector<std::string> faults;
        auto faultStr = row.at("fault_flags");
        if (faultStr != "-" && !faultStr.empty())
        {
            std::stringstream faultStream(faultStr);
            std::string fault;
            while (std::getline(faultStream, fault, ','))
            {
                faults.push_back(fault);
            }
        }
        logs_.push_back(LogEntry{ts, sensorId, value, faults});
    }
}

std::vector<LogEntry> MiniDB::getLogsSnapshot() const
{
    std::lock_guard<std::mutex> lock(mtx_);
    return logs_;
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