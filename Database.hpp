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

    template <typename T> void save();
    template <typename T> void update();
    template <typename T> void remove();
    
    template <typename T> static std::shared_ptr<T> get(unsigned long id);
    template <typename T> static std::vector<std::shared_ptr<T>> getAll();
};

#ifndef ODB_COMPILER
    #include "DatabaseImpl.tpp"
#endif