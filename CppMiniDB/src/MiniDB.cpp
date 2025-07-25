#include "../include/cppminidb/MiniDB.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <filesystem>

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

std::string MiniDB::exportToJson() const
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

    throw std::invalid_argument("Unsupported comparison operator: " + op);
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

    if (!NumberValidator::isPureInteger(value))
        throw std::invalid_argument("Value must be an integer.");

    auto it = std::find(columns_.begin(), columns_.end(), column);
    size_t colIndex = std::distance(columns_.begin(), it);

    for (const auto &row : rows_)
    {
        if (columns_.size() != row.size())
            continue;

        const std::string &candidate = row[colIndex];
        if (!NumberValidator::isPureInteger(candidate))
            continue;

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
    return result;
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