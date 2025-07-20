#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "cppminidb/MiniDB.hpp"

TEST_CASE("MiniDB basic insert and export", "[MiniDB]")
{
    MiniDB db("test_table");
    db.setColumns({"name", "age"});

    db.insertRow({"Alice", "30"});
    db.insertRow({"Bob", "25"});

    std::string json = db.exportToJson();

    REQUIRE(json.find("Alice") != std::string::npos);
    REQUIRE(json.find("30") != std::string::npos);
    REQUIRE(json.find("Bob") != std::string::npos);
    REQUIRE(json.find("25") != std::string::npos);
}