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
    /// Column type descriptor for typed comparisons
    enum class ColumnType
    {
        String,
        Int,
        Float
    };
    /**
     * @brief Constructor to initialize the MiniDB instance with a table name.
     * @param tableName Name of the table, also used for file storage.
     *
     * Creates a new database object tied to a specific table identifier.
     */
    MiniDB(const std::string &tableName);

    /**
     * @brief Sets schema with names only; defaults all column types to String.
     *
     * @param names Column names in order.
     * @throws std::invalid_argument if names is empty.
     */
    void setColumns(const std::vector<std::string> &names);

    /**
     * @brief Sets schema with explicit types per column.
     *
     * @param names Column names in order.
     * @param types Column types corresponding to each name.
     * @throws std::invalid_argument if sizes mismatch or names is empty.
     */
    void setColumns(const std::vector<std::string> &names,
                    const std::vector<ColumnType> &types);

    /**
     * @brief Returns the declared type for a column name.
     *
     * @param name Column name to query.
     * @return ColumnType The declared type.
     * @throws std::invalid_argument if column name is not found.
     */
    ColumnType columnTypeOf(const std::string &name) const;
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
    std::string exportToJsonLegacy() const;

    /**
     * @brief Executes a filter query on in-memory rows.
     *
     * Iterates over all stored rows in memory, applies the filter based on the target column,
     * comparison operator, and value, then returns the matching rows as maps from column name to cell value.
     *
     * @param column  Name of the column to filter on (e.g., "age")
     * @param op      Comparison operator as string (e.g., ">", "<", "==")
     * @param value   Target value for comparison (e.g., "20")
     * @return        A list of rows (as maps) that satisfy the condition
     */
    std::vector<std::map<std::string, std::string>> selectWhereFromMemory(
        const std::string &column,
        const std::string &op,
        const std::string &value) const;

    /**
     * @brief Executes a filter query on rows loaded from disk.
     *
     * Reads rows from disk (not memory), evaluates each one against the filter criteria,
     * and returns those that match the condition. Useful when data is stored externally.
     *
     * @param column  Column name to apply the filter on
     * @param op      Comparison operator (e.g., "==", ">", "<")
     * @param value   Comparison value used in the filter
     * @return        A vector of matching rows represented as key-value maps
     */
    std::vector<std::map<std::string, std::string>> selectWhereFromDisk(
        const std::string &column,
        const std::string &op,
        const std::string &value) const;

    /**
     * @brief Compares two integers using a given operator string.
     *
     * Evaluates the comparison between `a` and `b` based on the specified operator.
     * Supports common relational operators like ">", "<", "==", "!=".
     *
     * @param a    Left-hand side operand (row value)
     * @param op   Comparison operator as string
     * @param b    Right-hand side operand (target value)
     * @return     True if the comparison is valid, false otherwise
     */
    bool compare(int a, const std::string &op, int b) const;
    bool compare(std::string a, const std::string &op, std::string b) const;

    /**
     * @brief Filters in-memory rows based on a conditional expression and updates matching entries.
     *
     * This function operates on MiniDB's in-memory data. It filters rows by applying the given
     * comparison operator (`op`) to the specified `column` and reference `value`. For rows that
     * match the condition, it applies updates using the key-value pairs in `updateMap`.
     *
     * @param column The name of the column to apply the condition on.
     * @param op The comparison operator to use. Examples: "==", "!=", "<", ">", "<=", ">=".
     * @param value The reference value to compare against.
     * @param updateMap A map specifying which columns to update and their new values.
     *
     * @note This operation only affects in-memory data and does not persist changes to disk.
     */
    void updateWhereFromMemory(const std::string &column,
                               const std::string &op,
                               const std::string &value,
                               const std::map<std::string, std::string> &updateMap);

    /**
     * @brief Filters on-disk rows based on a conditional expression and applies updates to matching entries.
     *
     * This function operates on MiniDB's persistent storage layer. It reads rows from disk,
     * evaluates each row by applying the comparison operator (`op`) to the specified `column`
     * and reference `value`, and performs updates on matching rows according to the key-value
     * pairs provided in `updateMap`.
     *
     * @param column The name of the column to apply the condition on.
     * @param op The comparison operator to use. Examples include "==", "!=", "<", ">", "<=", ">=".
     * @param value The reference value to compare against.
     * @param updateMap A map specifying which columns to update and their corresponding new values.
     *
     * @note This operation directly modifies the persistent data on disk. It is essential to ensure
     *       data integrity and consistency during this process.
     */
    void updateWhereFromDisk(const std::string &column,
                             const std::string &op,
                             const std::string &value,
                             const std::map<std::string, std::string> &updateMap);

    /**
     * @brief Filters in-memory rows based on a conditional expression and deletes matching entries.
     *
     * This function operates on MiniDB's in-memory data. It scans each row and applies the given
     * comparison operator (`op`) to the specified `column` and reference `value`. If a row satisfies
     * the condition, it is removed from memory.
     *
     * @param column The name of the column to apply the condition on.
     * @param op The comparison operator to use. Examples: "==", "!=", "<", ">", "<=", ">=".
     * @param value The reference value to compare against.
     *
     * @note This operation only affects in-memory data and does not persist changes to disk.
     *       Deleted rows are permanently removed from the current session's memory state.
     */
    void deleteWhereFromMemory(const std::string &column,
                               const std::string &op,
                               const std::string &value);

    /**
     * @brief Filters on-disk rows based on a conditional expression and deletes matching entries.
     *
     * This function operates on MiniDB's persistent storage layer. It reads rows from disk,
     * evaluates each row by applying the comparison operator (`op`) to the specified `column`
     * and reference `value`, and removes rows that satisfy the condition.
     *
     * @param column The name of the column to apply the condition on.
     * @param op The comparison operator to use. Examples include "==", "!=", "<", ">", "<=", ">=".
     * @param value The reference value to compare against.
     *
     * @note This operation directly modifies the persistent data on disk. Deleted rows are
     *       permanently removed from storage, so it is critical to ensure correctness and
     *       consistency before performing this action.
     */
    void deleteWhereFromDisk(const std::string &column,
                             const std::string &op,
                             const std::string &value);

    /**
     * @brief Serializes in-memory table data into a JSON-formatted string.
     *
     * This function converts the current in-memory rows of the MiniDB instance into a
     * JSON representation. Each row is serialized as a JSON object, where keys correspond
     * to column names and values to the respective cell contents. The result is a JSON array
     * of row objects.
     *
     * @return A JSON-formatted string representing the in-memory table contents.
     *
     * @note This function only reflects the current in-memory state and does not include
     *       any data stored on disk. Column order is preserved, and all values are treated
     *       as strings in the output.
     *
     * @example
     * Given columns: ["id", "name"]
     * And rows: [["1", "Alice"], ["2", "Bob"]]
     * The output will be:
     * [
     *   { "id": "1", "name": "Alice" },
     *   { "id": "2", "name": "Bob" }
     * ]
     */
    std::string exportToJson() const;
    std::string exportToJsonFromDisk() const;

    /**
     * @brief Imports table data from a JSON-formatted string into memory.
     *
     * This function parses the input JSON string and populates the in-memory table
     * with the resulting rows. The input must be a JSON array of objects, where
     * each object represents a row, and each key corresponds to a column name.
     *
     * If no columns are previously defined in the MiniDB instance, the columns will be
     * inferred from the first JSON object. If columns already exist, the input JSON
     * must match the existing schema; otherwise, an exception is thrown.
     *
     * @param jsonString A JSON-formatted string representing an array of row objects.
     *
     * @throws std::invalid_argument If the JSON is invalid, not an array, or mismatched schema.
     *
     * @example
     * Input:
     * [
     *   { "id": "1", "name": "Alice" },
     *   { "id": "2", "name": "Bob" }
     * ]
     *
     * After calling this function, MiniDB will contain two rows:
     * ["1", "Alice"], ["2", "Bob"]
     */
    void importFromJson(const std::string &jsonString);

    /**
     * @brief Imports JSON array directly to the table file on disk.
     *
     * Parses a JSON array of objects and writes rows to the `.tbl` file without
     * populating in-memory rows_. When append=false, the file is rewritten with
     * a header line (derived from the first JSON object's keys) followed by all rows.
     * When append=true and the file exists, the header must match the incoming JSON keys;
     * otherwise, an exception is thrown. If the file does not exist, it will be created
     * and initialized with the inferred header.
     *
     * Missing keys are written as empty strings, extra keys are ignored. Column order is
     * determined by the inferred/loaded header and preserved in the file.
     *
     * @param jsonString JSON-formatted string (array of objects) to import.
     * @param append If true, rows are appended to existing table (header must match).
     *               If false (default), the file is recreated from the JSON input.
     *
     * @throws std::runtime_error on I/O errors, malformed JSON or empty array.
     * @throws std::invalid_argument on header/schema mismatch in append mode.
     *
     * @example
     * // Given:
     * // [
     * //   {"Name":"Alice","Age":"30"},
     * //   {"Name":"Bob","Age":"25"}
     * // ]
     * // Resulting file (overwrite mode):
     * // Name,Age
     * // Alice,30
     * // Bob,25
     */
    void importFromJsonToDisk(const std::string &jsonString, bool append = false);

    /**
     * @brief Clears all in-memory rows while preserving the table schema.
     *
     * Removes every row stored in memory (rows_) without modifying the column
     * definitions or the on-disk table file. Useful when you want to reset the
     * in-memory cache and start inserting fresh rows against the same schema.
     *
     * @note This function does not affect the .tbl file on disk.
     */
    void clearMemory();

    /**
     * @brief Clears the on-disk table file, optionally preserving the header row.
     *
     * When keepHeader is true, the .tbl file is rewritten to contain only the
     * first (header) line. When keepHeader is false, the file is removed from disk.
     * If the file does not exist, this function is a no-op.
     *
     * @param keepHeader If true, preserves the header line; otherwise removes the file.
     *
     * @note This function does not modify in-memory rows_ or columns_.
     */
    void clearDisk(bool keepHeader = true);

    /**
     * @brief Checks if the given column name exists in the current schema.
     */
    bool hasColumn(const std::string &name) const;

    /**
     * @brief Returns the number of defined columns in the schema.
     */
    std::size_t columnCount() const noexcept;

    /**
     * @brief Returns the number of in-memory rows currently stored.
     *
     * @note This reports the size of rows_ (memory), not on-disk row count.
     */
    std::size_t rowCount() const noexcept;

    // Validates whether the operator is allowed for the given column type
    static bool isOpAllowedForType(const std::string &op, ColumnType t);

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
    std::string getTempFilePath() const;

    std::vector<ColumnType> columnTypes_;
};

/**
 * @brief Utility class for validating numeric content in strings.
 *
 * NumberValidator provides static methods to determine whether a given
 * string represents a specific numeric format, such as unsigned integers,
 * signed integers, or floating-point numbers. These checks are designed
 * for lightweight validation prior to parsing or user input handling.
 *
 * All methods focus on ASCII-compatible formats and do not support locale-based
 * or scientific number representations.
 */
class NumberValidator
{
public:
    /**
     * @brief Checks if the input string contains only digit characters (0–9).
     *
     * Returns true if all characters in the string are ASCII digits with no
     * leading signs, decimal points, or other symbols.
     *
     * Examples:
     * - "12345" → true
     * - "+123" → false
     * - "12.3" → false
     * - "abc" → false
     *
     * @param str Input string to evaluate.
     * @return true if all characters are digits, false otherwise.
     */
    static bool isPureInteger(const std::string &str);

    /**
     * @brief Validates whether the string is a properly signed integer.
     *
     * Accepts optional '+' or '-' signs at the beginning, followed by
     * digit characters only. No decimal points or letters are permitted.
     *
     * Examples:
     * - "-42" → true
     * - "+15" → true
     * - "007" → true
     * - "15.0" → false
     * - "abc" → false
     *
     * @param str Input string to check.
     * @return true if the string is a valid signed integer, false otherwise.
     */
    static bool isSignedInteger(const std::string &str);

    /**
     * @brief Determines whether the input string represents a valid floating-point number.
     *
     * Accepts standard decimal formats with optional signs and a single decimal point.
     * Scientific notation, locale-specific separators, and malformed formats are rejected.
     *
     * Examples:
     * - "3.14" → true
     * - "-0.001" → true
     * - "+2.0" → true
     * - "42" → true
     * - "3.14.15" → false
     * - "1e10" → false
     *
     * @param str Input string to validate.
     * @return true if the string is a valid floating-point number, false otherwise.
     */
    static bool isFloatingPoint(const std::string &str);
};