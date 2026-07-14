#pragma once
#include "DatabaseManager.hpp"
#include <cerrno>
#include <odb/database.hxx>
#include <odb/transaction.hxx>

template <typename T>
bool Database::save() { 
    try {
        odb::database& db = DatabaseManager::instance().get_db();
        odb::transaction transaction(db.begin());
        db.persist(*static_cast<T*>(this)); 
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
        odb::database& db = DatabaseManager::instance().get_db();
        odb::transaction transaction(db.begin());
        db.update(*static_cast<T*>(this)); 
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
        odb::database& db = DatabaseManager::instance().get_db();
        odb::transaction transaction(db.begin());
        db.erase(*static_cast<T*>(this)); 
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
        odb::database& db = DatabaseManager::instance().get_db();
        odb::transaction transaction(db.begin());
        db.erase_query<T>();
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
        odb::database& db = DatabaseManager::instance().get_db();
        odb::transaction transaction(db.begin());
        std::shared_ptr<T> result(db.load<T>(id));
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
        odb::database& db = DatabaseManager::instance().get_db();
        odb::transaction transaction(db.begin());
        odb::result<T> r(db.query<T>());
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

