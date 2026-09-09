#pragma once
#include <mutex>
#include <odb/core.hxx>
#include <memory>
#include <optional>
#include <vector>

#pragma db object polymorphic
class Database {
public:
    virtual ~Database() = default;

    #pragma db id auto

    unsigned long id = 0; ///< Уникальный идентификатор объекта в базе данных

    /// Сохраняет объект в базе данных
    /// @return true, если объект сохранен, false, если объект не сохранен
    bool save();

    /// Актуализирует значения объекта, загружая его из базы данных
    /// @return true, если объект актуализирован, false, если объект не актуализирован
    bool actual();

    /// Обновляет объект в базе данных
    /// @return true, если объект обновлен, false, если объект не обновлен 
    bool update();

    /// Удаляет объект из базы данных
    /// @return true, если объект обновлен, false, если объект не обновлен 
    bool remove();

    /// Ищет объект типа T по идентификатору поля и значению поля
    /// @param fieldName имя поля
    /// @param value значение поля
    /// @return объект типа T или пустой объект, если объект не найден
    template <typename T, typename ValueType> 
    static std::optional<std::vector<T>> find(const std::string& fieldName, const ValueType& value);
    
    /// Очищает все объекты типа T из базы данных
    template <typename T> static bool clear();

    /// Удаляет объект из базы данных
    /// @param id идентификатор объекта в базе данных
    /// @return true, если объект удален, false, если объект не найден
    template <typename T> static bool remove(const unsigned long id);

    /// Получает объект типа T по его идентификатору
    /// @param id идентификатор объекта в базе данных
    /// @return объект типа T или пустой объект, если объект не найден
    template <typename T> static std::shared_ptr<T> get(const unsigned long id);

    /// Получает все объекты типа T из базы данных
    /// @return вектор объектов
    template <typename T> static std::vector<std::shared_ptr<T>> getAll();

    /// Удаляет все таблицы из базы данных
    static void dropAllTable();

    /// Получает список таблиц в базе данных по имени схемы
    /// @param schemaName имя схемы
    /// @return список таблиц
    static std::vector<std::string> getTablesBySchema(const std::string& schemaName);

    /// Удаляет таблицу из базы данных по имени
    /// @param tableName имя таблицы
    static void dropTable(const std::string& tableName);

private:
    static std::mutex dbMutex; ///< Мьютекс для синхронизации доступа к операциям БД.

    /// Сохраняет объект в базе данных
    /// @return true, если объект сохранен, false, если объект не сохранен
    template <typename T> bool save_impl();

    /// Актуализирует значения объекта, загружая его из базы данных
    /// @return true, если объект актуализирован, false, если объект не актуализирован
    template <typename T> bool actual_impl();

    /// Обновляет объект в базе данных
    /// @return true, если объект обновлен, false, если объект не обновлен 
    template <typename T> bool update_impl();

    /// Удаляет объект из базы данных
    /// @return true, если объект обновлен, false, если объект не обновлен 
    template <typename T> bool remove_impl();
 
};

#pragma db view query("SELECT table_name FROM information_schema.tables")
struct SchemaTableView {
    std::string table_name; 
};

#ifndef ODB_COMPILER
    #include "../src/DatabaseImpl.tpp"
#endif

