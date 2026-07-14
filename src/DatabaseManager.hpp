#pragma once
#include <iostream>
#include <odb/database.hxx>
#include <odb/pgsql/database.hxx> 
#include <odb/schema-catalog.hxx> 

class DatabaseManager {
public:
    // Запрещаем копирование синглтона
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    // Глобальная точка доступа к менеджеру
    static DatabaseManager& instance() {
        static DatabaseManager instance;
        return instance;
    }

    // Метод получения ссылки на объект БД для выполнения запросов
    odb::database& getDatabase() {
        return *dataBase_;
    }

private:
    std::unique_ptr<odb::database> dataBase_;

    // Приватный конструктор: вся магия автоматической инициализации происходит здесь
    DatabaseManager() {
        try {
            // 1. Создаем подключение (параметры можно вынести в конфиг или переменные окружения)
            dataBase_ = std::make_unique<odb::pgsql::database>(
                "postgres", "my_secret_password", "my_database", "127.0.0.1", 5432
            );

            // 2. Автоматически проверяем и создаем/обновляем схему таблиц
            odb::transaction transaction(dataBase_->begin());
            
            if (odb::schema_catalog::exists(*dataBase_, "app")) {
                odb::schema_catalog::drop_schema(*dataBase_, "app"); // Оставляем для тестов, в продакшене эту строку убирают
            }
            
            odb::schema_catalog::create_schema(*dataBase_, "app");
            transaction.commit();
            
            std::cout << "[Database] Соединение установлено, схема 'app' развернута автоматически." << std::endl;
        } catch (const odb::exception& e) {
            std::cerr << "[Database CRITICAL ERROR]: " << e.what() << std::endl;
            // Если БД не поднялась, продолжать работу приложения обычно нет смысла
            std::exit(EXIT_FAILURE); 
        }
    }

    ~DatabaseManager() = default;
};
