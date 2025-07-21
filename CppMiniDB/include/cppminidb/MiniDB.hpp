#pragma once
/**
 * ╔══════════════════════════════════════════════════════════════════════╗
 * ║                          MiniDB Overview                          ║
 * ╚══════════════════════════════════════════════════════════════════════╝
 *
 * MiniDB is a minimal in-memory table representation built around
 * rows and columns, similar to a spreadsheet or a simple SQL table.
 *
 * ┌────────────┐     ┌───────────────────────────────┐
 * │  columns_  │ --> │ {"Name", "Age", "Country"}    │
 * └────────────┘     └───────────────────────────────┘
 *
 * ┌────────────┐     ┌────────────────────────────────────────────────────┐
 * │   rows_    │ --> │ { {"Alice", "30", "USA"}, {"Bob", "25", "Canada"} }│
 * └────────────┘     └────────────────────────────────────────────────────┘
 *
 * Each row is a vector of strings aligned to the columns vector.
 *
 * selectAll()
 *    Converts each row to a map:
 *    { "Name" → "Alice", "Age" → "30", "Country" → "USA" }
 *
 * getTableFilePath()
 *    Builds the full path for saving the table to disk:
 *    Example: "data/users.tbl"
 *
 * Summary:
 * - columns_ defines the schema.
 * - rows_ stores raw data.
 * - selectAll() gives structured access to that data.
 * - save() writes everything to disk for persistence.
 *
 * This class helps simulate lightweight tabular operations
 * in C++ applications without relying on external database systems.
 *
 * Perfect for: embedded use cases, CSV-like data structures,
 * learning database principles, or local caching systems.
 *
 */

#include <vector>
#include <string>
#include <map>

/**
 * @class MiniDB
 * @brief A lightweight in-memory table abstraction with basic persistence features.
 *
 * This class simulates a minimal relational table structure where data is stored
 * in rows and columns. It allows inserting, selecting, and saving tabular data,
 * making it suitable for simple embedded data handling in C++ applications.
 */
class MiniDB
{
public:
    /**
     * @brief Constructor to initialize the MiniDB instance with a table name.
     * @param tableName Name of the table, also used for file storage.
     *
     * Creates a new database object tied to a specific table identifier.
     */
    MiniDB(const std::string &tableName);

    /**
     * @brief Defines the column headers for the table.
     * @param columnNames Vector of column names as strings.
     *
     * This method should be called before inserting rows. Each row added later must
     * match the number and order of these column names.
     */
    void setColumns(const std::vector<std::string> &columnNames);

    /**
     * @brief Inserts a row of data into the table.
     * @param values Vector of values, each corresponding to a column.
     *
     * Stores one complete row in the internal table structure. The size of this vector
     * should match the number of columns defined in setColumns().
     */
    void insertRow(const std::vector<std::string> &values);

    /**
     * @brief Returns all stored rows as a vector of key-value mappings.
     * @return A vector of maps, where each map represents a row with column-value pairs.
     *
     * Useful for displaying or exporting data. It adds semantic clarity by
     * using column names as keys rather than relying on index positions.
     */
    std::vector<std::map<std::string, std::string>> loadFromDisk() const;

    /**
     * @brief Retrieves all rows currently stored in memory.
     * @return A vector of maps, where each map represents a row with column-value pairs.
     *
     * Useful for in-memory queries, display, or exporting the current working dataset.
     * Offers semantic clarity by using column names as keys instead of relying on column indices.
     */
    std::vector<std::map<std::string, std::string>> selectAll() const;

    /**
     * @brief Persists the table data to disk.
     *
     * Saves the current in-memory content to a `.tbl` file located in a predefined
     * folder (e.g., `data/`). Useful for durability between program runs.
     */
    void save() const;

    /**
     * @brief Clears all row data from memory and resets the persisted file.
     *
     * This method removes all in-memory rows and truncates the associated `.tbl` file,
     * effectively resetting the table while preserving its column schema.
     *
     * Behavior:
     * - `rows_` is cleared, removing all stored data.
     * - The `.tbl` file is overwritten using `std::ios::trunc`.
     * - Column headers are re-written to maintain schema consistency.
     *
     * Use cases:
     * - Resetting the table between test runs.
     * - Clearing temporary or outdated data.
     * - Reinitializing the table without redefining columns.
     *
     * @note Column definitions (`columns_`) remain unchanged.
     * @throws std::runtime_error if the file cannot be opened for writing.
     */
    void clear();

    /**
     * @brief Serializes the table data into a JSON-formatted string.
     * @return A string containing the table rows in JSON array format.
     *
     * Converts the in-memory table rows into a human-readable JSON representation.
     * Each row is serialized as a JSON object, with column names as keys and cell
     * values as string values.
     *
     * Example output:
     * @code
     * [
     *   {
     *     "Name": "Alice",
     *     "Age": "30",
     *     "Country": "USA"
     *   },
     *   {
     *     "Name": "Bob",
     *     "Age": "25",
     *     "Country": "Canada"
     *   }
     * ]
     * @endcode
     *
     * Use cases:
     * - Exporting data for web APIs or external tools.
     * - Debugging or logging table contents.
     * - Interfacing with systems that consume JSON.
     *
     * @note Assumes that `columns_` and `rows_` are properly aligned.
     * @throws None. This method does not perform file I/O or error handling.
     */
    std::string exportToJson() const;

private:
    /**
     * @brief Stores the name of the table.
     *
     * Used both for identification and as the file name for persistence.
     */
    std::string tableName_;

    /**
     * @brief List of column names used to define the table schema.
     */
    std::vector<std::string> columns_;

    /**
     * @brief Two-dimensional container for all table rows.
     *
     * Each row is represented as a vector of strings aligned with the column order.
     */
    std::vector<std::vector<std::string>> rows_;

    /**
     * @brief Constructs and returns the full file path for saving the table.
     * @return String containing the relative or absolute path to the table file.
     *
     * Typically combines the table name with a folder prefix like `data/` and a `.tbl` suffix.
     */
    std::string getTableFilePath() const;
};