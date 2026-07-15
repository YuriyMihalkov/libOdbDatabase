#pragma once
#include <odb/core.hxx>
#include <memory>
#include <vector>

#pragma db object polymorphic
class Database {
public:
    virtual ~Database() = default;

    #pragma db id auto
    unsigned long id = 0;

    template <typename T> bool save();
    template <typename T> bool update();
    template <typename T> bool remove();
    template <typename T> bool clear();

    template <typename T> static std::shared_ptr<T> get(unsigned long id);
    template <typename T> static std::vector<std::shared_ptr<T>> getAll();

    static void dropAllTable();
    static std::vector<std::string> getTablesBySchema(const std::string& schema_name);
    static void dropTable(const std::string& tableName);
};

#pragma db view query("SELECT table_name FROM information_schema.tables")
struct schema_table_view {
    std::string table_name; 
};

#ifndef ODB_COMPILER
    #include "../src/DatabaseImpl.tpp"
#endif

