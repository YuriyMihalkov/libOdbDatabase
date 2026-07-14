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

    template <typename T> static void dropAllTable();
};

#ifndef ODB_COMPILER
    #include "../src/DatabaseImpl.tpp"
#endif