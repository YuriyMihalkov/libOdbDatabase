#pragma once
#include <odb/core.hxx>

#pragma db view query("SELECT table_name FROM information_schema.tables")
struct SchemaTableView {
    std::string table_name; 
};