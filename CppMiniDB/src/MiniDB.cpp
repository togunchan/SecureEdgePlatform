#include "../include/cppminidb/MiniDB.hpp"
#include <fstream>
#include <sstream>

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
        throw std::runtime_error("Number of values must match the number of columns.");
    }

    rows_.push_back(values);
}

std::string MiniDB::getTableFilePath() const
{
    return "data/" + tableName_ + ".tbl";
}

void MiniDB::save() const
{
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

std::vector<std::map<std::string, std::string>> MiniDB::selectAll() const
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