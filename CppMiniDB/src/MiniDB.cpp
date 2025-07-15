#include "../include/cppminidb/MiniDB.hpp"

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