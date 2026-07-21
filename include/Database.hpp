#pragma once
#include <mutex>
#include <odb/core.hxx>
#include <memory>
#include <vector>

#pragma db object polymorphic
class Database {
public:
    virtual ~Database() = default;

    #pragma db id auto
    unsigned long id = 0;

    /// Сохраняет объект в базе данных  
    template <typename T> bool save();

    /// Актуализирует значения объекта в базе данных 
    template <typename T> bool actual();

    /// Обновляет объект в базе данных
    template <typename T> bool update();

    /// Удаляет объект из базы данных
    template <typename T> bool remove();
    
    /// Очищает все объекты типа T из базы данных
    template <typename T> static bool clear();

    /// Получает объект типа T по его идентификатору
    template <typename T> static std::shared_ptr<T> get(const unsigned long id);

    /// Получает все объекты типа T из базы данных
    template <typename T> static std::vector<std::shared_ptr<T>> getAll();

    /// Удаляет все таблицы из базы данных
    static void dropAllTable();

    /// Получает список таблиц в базе данных по имени схемы
    static std::vector<std::string> getTablesBySchema(const std::string& schemaName);

    /// Удаляет таблицу из базы данных по имени
    static void dropTable(const std::string& tableName);

private:
    static std::mutex dbMutex; ///< Мьютекс для синхронизации доступа к операциям БД.
};

#pragma db view query("SELECT table_name FROM information_schema.tables")
struct SchemaTableView {
    std::string table_name; 
};

#ifndef ODB_COMPILER
    #include "../src/DatabaseImpl.tpp"
#endif

