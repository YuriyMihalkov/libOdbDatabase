#pragma once
#include "DatabaseManager.hpp"
#include <odb/database.hxx>
#include <odb/transaction.hxx>

template <typename T>
void Database::save() { 
    odb::database& db = DatabaseManager::instance().get_db();
    odb::transaction transaction(db.begin());
    db.persist(*static_cast<T*>(this)); 
    transaction.commit();
}

template <typename T>
void Database::update() { 
    odb::database& db = DatabaseManager::instance().get_db();
    odb::transaction transaction(db.begin());
    db.update(*static_cast<T*>(this)); 
    transaction.commit();
}

template <typename T>
void Database::remove() { 
    odb::database& db = DatabaseManager::instance().get_db();
    odb::transaction transaction(db.begin());
    db.erase(*static_cast<T*>(this)); 
    transaction.commit();
}

template <typename T>
std::shared_ptr<T> Database::get(unsigned long id) {
    odb::database& db = DatabaseManager::instance().get_db();
    odb::transaction transaction(db.begin());
    try {
        std::shared_ptr<T> result(db.load<T>(id));
        transaction.commit();
        return result;
    } catch (const odb::object_not_persistent&) {
        return nullptr;
    }
}

template <typename T>
std::vector<std::shared_ptr<T>> Database::getAll() {
    odb::database& db = DatabaseManager::instance().get_db();
    odb::transaction transaction(db.begin());
    std::vector<std::shared_ptr<T>> result;
    odb::result<T> r(db.query<T>());
    for (auto& obj : r) {
        result.push_back(std::make_shared<T>(obj)); 
    }
    
    transaction.commit();
    return result;
}