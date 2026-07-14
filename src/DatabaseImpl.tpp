#pragma once
#include "Database.hpp"
#include "DatabaseManager.hpp"
#include <odb/database.hxx>
#include <odb/transaction.hxx>

template <typename T>
bool Database::save() { 
    try {
        odb::database& database = DatabaseManager::instance().getDatabase();
        odb::transaction transaction(database.begin());
        database.persist(*static_cast<T*>(this)); 
        transaction.commit();
        return true;
    } catch (const odb::exception& error) {
        std::cerr << error.what() << std::endl;
        return false;
    }
}

template <typename T>
bool Database::update() { 
    try {
        odb::database& database = DatabaseManager::instance().getDatabase();
        odb::transaction transaction(database.begin());
        database.update(*static_cast<T*>(this)); 
        transaction.commit();
        return true;
    } catch (const odb::exception& error) {
        std::cerr << error.what() << std::endl;
        return false;
    }
}

template <typename T>
bool Database::remove() { 
    try {
        odb::database& database = DatabaseManager::instance().getDatabase();
        odb::transaction transaction(database.begin());
        database.erase(*static_cast<T*>(this)); 
        transaction.commit();
        return true;
    } catch (const odb::exception& error) {
        std::cerr << error.what() << std::endl;
        return false;
    }
}

template <typename T>
bool Database::clear() {
    try {
        odb::database& database = DatabaseManager::instance().getDatabase();
        odb::transaction transaction(database.begin());
        database.erase_query<T>();
        transaction.commit();
        return true;
    } catch (const odb::exception& error) {
        std::cerr << error.what() << std::endl;
        return false;
    }
}

template <typename T>
std::shared_ptr<T> Database::get(unsigned long id) {
    try {
        odb::database& database = DatabaseManager::instance().getDatabase();
        odb::transaction transaction(database.begin());
        std::shared_ptr<T> result(database.load<T>(id));
        transaction.commit();
        return result;
    } catch (const odb::object_not_persistent& error) {
        std::cerr << error.what() << std::endl;
        return nullptr; // Объект с таким ID не существует
    } catch (const odb::exception& error) {
        std::cerr << error.what() << std::endl;
        return nullptr; // Другие ошибки БД (например, проблемы с соединением)
    }
}

template <typename T>
std::vector<std::shared_ptr<T>> Database::getAll() {
    std::vector<std::shared_ptr<T>> result;
    try {
        odb::database& database = DatabaseManager::instance().getDatabase();
        odb::transaction transaction(database.begin());
        odb::result<T> r(database.query<T>());
        for (auto& obj : r) {
            result.push_back(std::make_shared<T>(obj)); 
        }
        transaction.commit();
    } catch (const odb::exception& error) {
        std::cerr << error.what() << std::endl;
        result.clear(); // В случае ошибки возвращаем пустой вектор
    }
    return result;
}

template <typename T>  
void Database::dropAllTable() {
    try {
        odb::database& database = DatabaseManager::instance().getDatabase();
        odb::transaction transaction(database.begin());
        odb::schema_catalog::drop_schema(database);
        transaction.commit();
    } catch (const odb::exception& error) {
        std::cerr << error.what() << std::endl;
        std::cerr << "Ошибка при удалении схемы: " << error.what() << std::endl;
    }
}

