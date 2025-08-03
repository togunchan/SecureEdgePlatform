#define CATCH_CONFIG_MAIN
#include <catch2/catch_all.hpp>
#include "cppminidb/MiniDB.hpp"
#include <iostream>
#include <fstream>
#include <nlohmann/json.hpp>

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

    // std::cout << "Row count: " << db.loadFromDisk().size() << std::endl;
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

    // There should be no more lines (no data rows)
    std::string extraLine;
    REQUIRE_FALSE(std::getline(file, extraLine));

    file.close();
}

TEST_CASE("MiniDB throws on empty row insert", "[MiniDB]")
{
    MiniDB db("empty_row_table");
    db.setColumns({"col1", "col2"});

    REQUIRE_THROWS_AS(db.insertRow({}), std::invalid_argument);
}

TEST_CASE("MiniDB loadFromDisk on non-existent file returns empty", "[MiniDB]")
{
    MiniDB db("ghost_table");

    auto result = db.loadFromDisk();

    REQUIRE(result.empty());
}

TEST_CASE("MiniDB loadFromDisk returns empty if only headers exist", "[MiniDB]")
{
    MiniDB db("empty_with_headers");
    db.setColumns({"col1", "col2", "col3"});
    db.save();

    auto result = db.loadFromDisk();

    REQUIRE(result.empty());
}

TEST_CASE("MiniDB handles rows with empty values correctly", "[MiniDB]")
{
    MiniDB db("empty_fields_table");
    db.setColumns({"name", "age", "country"});

    db.insertRow({"Alice", "", "USA"});
    db.insertRow({"", "25", "Canada"});
    db.insertRow({"Charlie", "40", ""});
    db.save();

    auto results = db.loadFromDisk();

    REQUIRE(results.size() == 3);

    REQUIRE(results[0]["name"] == "Alice");
    REQUIRE(results[0]["age"] == "");
    REQUIRE(results[0]["country"] == "USA");

    REQUIRE(results[1]["name"] == "");
    REQUIRE(results[1]["age"] == "25");
    REQUIRE(results[1]["country"] == "Canada");

    REQUIRE(results[2]["name"] == "Charlie");
    REQUIRE(results[2]["age"] == "40");
    REQUIRE(results[2]["country"] == "");
}

TEST_CASE("MiniDB updates rows in memory correctly", "[update][memory]")
{
    MiniDB db("update_memory_table");
    db.setColumns({"Name", "Age"});

    db.insertRow({"Alice", "30"});
    db.insertRow({"Bob", "25"});
    db.insertRow({"Charlie", "30"});

    db.updateWhereFromMemory("Age", "==", "30", {{"Name", "Updated"}});

    auto results = db.selectAll();

    REQUIRE(results[0]["Name"] == "Updated");
    REQUIRE(results[1]["Name"] == "Bob");
    REQUIRE(results[2]["Name"] == "Updated");
}

TEST_CASE("MiniDB updates rows in disk correctly", "[update][disk]")
{
    MiniDB db("update_disk_table");
    db.setColumns({"Name", "Age"});

    db.insertRow({"Alice", "30"});
    db.insertRow({"Bob", "25"});
    db.insertRow({"John", "30"});
    db.insertRow({"Charlie", "30"});
    db.save();

    db.updateWhereFromDisk("Age", "==", "30", {{"Name", "Updated"}});

    auto results = db.loadFromDisk();

    REQUIRE(results[0]["Name"] == "Updated");
    REQUIRE(results[1]["Name"] == "Bob");
    REQUIRE(results[2]["Name"] == "Updated");
}

TEST_CASE("MiniDB selects rows from memory using conditions", "[select][memory]")
{
    MiniDB db("test_memory_select");
    db.setColumns({"Name", "Age"});

    db.insertRow({"Alice", "30"});
    db.insertRow({"Bob", "25"});
    db.insertRow({"Charlie", "30"});

    auto results = db.selectWhereFromMemory("Age", "==", "30");

    REQUIRE(results.size() == 2);
    REQUIRE(results[0]["Name"] == "Alice");
    REQUIRE(results[1]["Name"] == "Charlie");
}

TEST_CASE("MiniDB selects rows from disk using conditions", "[select][disk]")
{
    MiniDB db("test_disk_select");
    db.setColumns({"Name", "Age"});

    db.insertRow({"Alice", "30"});
    db.insertRow({"Bob", "25"});
    db.insertRow({"Charlie", "30"});
    db.save();

    auto results = db.selectWhereFromDisk("Age", "==", "30");

    REQUIRE(results.size() == 2);
    REQUIRE(results[0]["Name"] == "Alice");
    REQUIRE(results[1]["Name"] == "Charlie");
}

TEST_CASE("MiniDB deletes rows from memory correctly", "[delete][memory]")
{
    MiniDB db("delete_memory_table");
    db.setColumns({"Name", "Age"});
    db.insertRow({"Alice", "30"});
    db.insertRow({"Bob", "25"});
    db.insertRow({"Charlie", "30"});
    db.insertRow({"David", "40"});

    db.deleteWhereFromMemory("Age", "==", "30");

    auto results = db.selectAll();

    REQUIRE(results.size() == 2);
    REQUIRE(results[0]["Name"] == "Bob");
    REQUIRE(results[1]["Name"] == "David");
}

TEST_CASE("MiniDB does not delete when no condition matches", "[delete][memory]")
{
    MiniDB db("no_match_delete_test");
    db.setColumns({"Name", "Age"});

    db.insertRow({"Alice", "30"});
    db.insertRow({"Bob", "25"});

    db.deleteWhereFromMemory("Age", "==", "100");

    auto results = db.selectAll();
    REQUIRE(results.size() == 2);
}

TEST_CASE("MiniDB deletes rows from disk correctly", "[delete][disk]")
{
    MiniDB db("delete_disk_table");
    db.setColumns({"Name", "Age"});

    db.insertRow({"Alice", "30"});
    db.insertRow({"Bob", "25"});
    db.insertRow({"Charlie", "30"});
    db.insertRow({"David", "40"});
    db.save();

    db.deleteWhereFromDisk("Age", "==", "30");

    auto results = db.loadFromDisk();

    REQUIRE(results.size() == 2);
    REQUIRE(results[0]["Name"] == "Bob");
    REQUIRE(results[1]["Name"] == "David");
}

TEST_CASE("MiniDB exports JSON from memory correctly", "[export][json][memory]")
{
    MiniDB db("json_memory_table");
    db.setColumns({"Name", "Age"});
    db.insertRow({"Alice", "30"});
    db.insertRow({"Bob", "25"});

    std::string jsonOutput = db.exportToJson();

    nlohmann::json parsed = nlohmann::json::parse(jsonOutput);

    REQUIRE(parsed.is_array());
    REQUIRE(parsed.size() == 2);
    REQUIRE(parsed[0]["Name"] == "Alice");
    REQUIRE(parsed[1]["Age"] == "25");
}

TEST_CASE("MiniDB exports JSON from disk correctly", "[export][json][disk]")
{
    MiniDB db("json_disk_table");
    db.setColumns({"Name", "Age"});
    db.insertRow({"Charlie", "22"});
    db.insertRow({"Diana", "28"});
    db.save();

    std::string jsonOutput = db.exportToJsonFromDisk();

    nlohmann::json parsed = nlohmann::json::parse(jsonOutput);

    REQUIRE(parsed.is_array());
    REQUIRE(parsed.size() == 2);
    REQUIRE(parsed[0]["Name"] == "Charlie");
    REQUIRE(parsed[1]["Age"] == "28");
}

TEST_CASE("MiniDB imports valid JSON correctly", "[import][memory]")
{
    MiniDB db("json_import_test");

    std::string jsonStr = R"([
        { "Name": "Alice", "Age": "30" },
        { "Name": "Bob", "Age": "25" },
        { "Name": "Charlie", "Age": "28" }
    ])";

    db.importFromJson(jsonStr);
    auto results = db.selectAll();

    REQUIRE(results.size() == 3);
    REQUIRE(results[0]["Name"] == "Alice");
    REQUIRE(results[0]["Age"] == "30");
    REQUIRE(results[1]["Name"] == "Bob");
    REQUIRE(results[2]["Name"] == "Charlie");
}