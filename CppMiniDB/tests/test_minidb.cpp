#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "cppminidb/MiniDB.hpp"
#include <iostream>
#include <fstream>

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

TEST_CASE("MiniDB throws on mismatched row insert", "[MiniDB]")
{
    MiniDB db("fail_table");
    db.setColumns({"id", "name"});

    REQUIRE_THROWS_AS(db.insertRow({"only_one_value"}), std::invalid_argument);
}

TEST_CASE("MiniDB selectAll returns structured data", "[MiniDB]")
{
    MiniDB db("select_table");
    db.setColumns({"city", "temperature"});

    db.insertRow({"Istanbul", "29"});
    db.insertRow({"Ankara", "25"});
    db.save();

    auto results = db.loadFromDisk();

    std::cout << "Row count: " << db.loadFromDisk().size() << std::endl;
    REQUIRE(results.size() == 2);

    REQUIRE(results[0]["city"] == "Istanbul");
    REQUIRE(results[0]["temperature"] == "29");

    REQUIRE(results[1]["city"] == "Ankara");
    REQUIRE(results[1]["temperature"] == "25");
}

TEST_CASE("MiniDB save creates file with only headers when no rows exist", "[MiniDB]")
{
    const std::string tableName = "empty_table";
    MiniDB db(tableName);
    db.setColumns({"sensor_id", "value", "timestamp"});

    db.save();

    // Open file and read lines
    std::ifstream file("./data/" + tableName + ".tbl");
    REQUIRE(file.is_open());

    std::string headerLine;
    std::getline(file, headerLine);

    // Header must match column names
    REQUIRE(headerLine == "sensor_id,value,timestamp");
    std::cout << headerLine;

    // There should be no more lines (no data rows)
    std::string extraLine;
    REQUIRE_FALSE(std::getline(file, extraLine));

    file.close();
}