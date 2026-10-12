#pragma once
#include <iostream>
#include <odb/database.hxx>
#if defined(ODB_DATABASE_TYPE_PGSQL)
#  include <odb/pgsql/database.hxx>
#elif defined(ODB_DATABASE_TYPE_SQLITE)
#  include <odb/sqlite/database.hxx>
#else
#  error "КРИТИЧЕСКАЯ ОШИБКА: CMake не передал ни ODB_DATABASE_TYPE_PGSQL, ни ODB_DATABASE_TYPE_SQLITE!"
#endif
#include <odb/schema-catalog.hxx> 
#include "config.h"

class DatabaseManager {
public:
    // Запрещаем копирование синглтона
    DatabaseManager(const DatabaseManager&) = delete;

    // Запрещаем присваивание синглтона
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    // Метод получения единственного экземпляра синглтона
    static DatabaseManager& instance() {
        static DatabaseManager instance;
        return instance;
    }

    // Метод получения ссылки на объект БД для выполнения запросов
    odb::database& getDatabase() {
        return *dataBase;
    }

private:
    std::unique_ptr<odb::database> dataBase; ///< Умный указатель на объект базы данных

    /// Конструктор синглтона, который инициализирует соединение с базой данных
    DatabaseManager() {
        try {
#if defined(ODB_DATABASE_TYPE_PGSQL)
            dataBase = std::make_unique<odb::pgsql::database>(
                            DATABASE_USER, DATABASE_PASSWD, DATABASE_NAME, DATABASE_ADDRESS, DATABASE_PORT
                        );
#elif defined(ODB_DATABASE_TYPE_SQLITE)
            dataBase = std::make_unique<odb::sqlite::database>(
                            DATABASE_NAME, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE
                        );
#else
#  error "КРИТИЧЕСКАЯ ОШИБКА: CMake не передал ни ODB_DATABASE_TYPE_PGSQL, ни ODB_DATABASE_TYPE_SQLITE!"
#endif

            odb::transaction transaction(dataBase->begin());
            
            // Разворачиваем дефолтную схему ("")
            if (odb::schema_catalog::exists(*dataBase, "")) {
                odb::schema_catalog::drop_schema(*dataBase, ""); 
            }
            
            odb::schema_catalog::create_schema(*dataBase, "");
            
            transaction.commit();
            std::cout << "[Database] Соединение установлено, таблицы развернуты в public." << std::endl;
        } catch (const odb::exception& e) {
            std::cerr << "[Database CRITICAL ERROR]: " << e.what() << std::endl;
            std::exit(EXIT_FAILURE); 
        }
    }

    /// Деструктор синглтона
    ~DatabaseManager() = default;
};
