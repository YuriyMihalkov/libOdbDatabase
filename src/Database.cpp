#include "Database.hpp"
#include "DatabaseManager.hpp"
#include <odb/database.hxx>
#include <odb/transaction.hxx>
#include <algorithm>
#include <iostream>

#include "Database-odb.hxx" 

void Database::dropAllTable() {
    try {
        std::vector<std::string> tables = Database::getTablesBySchema("");
        for (const auto& tableName : tables) {
            Database::dropTable(tableName);
        }

    } catch (const std::exception& error) {
        std::cerr << "Ошибка при очистке таблиц: " << error.what() << std::endl;
    }
}

std::vector<std::string> Database::getTablesBySchema(const std::string& schemaName) {
    std::vector<std::string> tables;
    try {
        odb::database& database = DatabaseManager::instance().getDatabase();
        odb::transaction transaction(database.begin());
        std::string lowerSchema = schemaName;
        std::transform(lowerSchema.begin(), lowerSchema.end(), lowerSchema.begin(), 
                       [](unsigned char c){ return std::tolower(c); });

        std::string sqlWhere = "WHERE table_schema = '" + lowerSchema + "' AND table_type = 'BASE TABLE'";
        odb::result<SchemaTableView> result(database.query<SchemaTableView>(odb::query<SchemaTableView>(sqlWhere)));

        for (const SchemaTableView& row : result) {
            tables.push_back(row.table_name);
        }
        
        transaction.commit();
    } catch (const odb::exception& error) {
        std::cerr << "Ошибка при получении списка таблиц: " << error.what() << std::endl;
    }
    return tables;   
}

void Database::dropTable(const std::string& tableName) {
    if (tableName.empty()) {
        return;
    }
    try {
        odb::database& database = DatabaseManager::instance().getDatabase();
        odb::connection_ptr connection = database.connection();
        odb::transaction transaction(connection->begin());
        std::string sql = "DROP TABLE IF EXISTS \"" + tableName + "\" CASCADE;";
        connection->execute(sql);
        transaction.commit();
        std::cout << "[Database] Таблица \"" << tableName << "\" успешно удалена." << std::endl;
    } catch (const odb::exception& error) {
        std::cerr << "Ошибка при удалении таблицы \"" << tableName << "\": " << error.what() << std::endl;
    }
}