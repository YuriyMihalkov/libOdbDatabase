#pragma once
#include <iostream>
#include <odb/database.hxx>
#include <odb/pgsql/database.hxx> 
#include <odb/schema-catalog.hxx> 
#include "../config/config.h"

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
        return *dataBase_;
    }

private:
    std::unique_ptr<odb::database> dataBase_; ///< Умный указатель на объект базы данных

    /// Конструктор синглтона, который инициализирует соединение с базой данных
    DatabaseManager() {
        try {
            dataBase_ = std::make_unique<odb::pgsql::database>(
                DATABASE_USER, DATABASE_PASSWD, DATABASE_NAME, DATABASE_ADDRESS, DATABASE_PORT
            );

            odb::transaction transaction(dataBase_->begin());
            
            // Разворачиваем дефолтную схему ("")
            if (odb::schema_catalog::exists(*dataBase_, "")) {
                odb::schema_catalog::drop_schema(*dataBase_, ""); 
            }
            odb::schema_catalog::create_schema(*dataBase_, "");
            
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
