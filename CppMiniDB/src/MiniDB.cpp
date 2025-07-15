#include "../include/cppminidb/MiniDB.hpp"

MiniDB::MiniDB(const std::string &tableName) : tableName_(tableName) {}

void MiniDB::setColumns(const std::vector<std::string> &columnNames)
{
    columns_ = columnNames;
}
