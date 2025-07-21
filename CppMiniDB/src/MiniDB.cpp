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
        for (size_t i = 0; i < row.size(); ++i)
        {
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
        std::runtime_error("Failed to open file for reading:" + getTableFilePath());
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

        if (values.size() != fileColumns.size())
        {
            continue;
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
