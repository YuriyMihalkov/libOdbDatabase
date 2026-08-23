#include <gtest/gtest.h>
#include <memory>
#include <odb/connection.hxx>
#include <odb/transaction.hxx>
#include <odb/pgsql/database.hxx>

#include "Database.hpp"
#include "User.hpp"
#include "User-odb.hxx" 

class DatabaseTest : public ::testing::Test {
    void SetUp() override {
        Database::dropAllTable();
    }
};

// 1. Тест успешного создания объекта 
TEST_F(DatabaseTest, CreateDatabase) {
    // Создаем указатель на тип
    User user("Дмитрий", "dima@mail.com");

    // Сохраняем указатель на тип в базе данных, проверяем возвращаемый результат
    EXPECT_TRUE(user.save()); 
}

// 2. Тест успешной загрузки объекта
TEST_F(DatabaseTest, loadDataBase) {
    // Получаем данные о пользователе из базы данных
    std::vector<std::shared_ptr<User>>users = Database::getAll<User>();

    EXPECT_TRUE(users.size() > 0);
    EXPECT_EQ(users.begin()->get()->email, "dima@mail.com");
    EXPECT_EQ(users.begin()->get()->name, "Дмитрий");
}

// 3. Тест успешного изменения объекта
TEST_F(DatabaseTest, UpdateDataBase) {
    // Получаем данные о пользователе из базы данных
    std::shared_ptr<User> user = *Database::getAll<User>().begin();

    // Изменяем значение
    user->email = "dima123@mail.com";

    // Обновляем данные в базе
    user->update();

    // Сбрасываем значение указателя
    user.reset();

    // Загружаем новое значение из базы данных
    user = *Database::getAll<User>().begin();
    EXPECT_EQ(user->email, "dima123@mail.com");
}

// 4. Тест попытки загрузки несуществующего объекта
TEST_F(DatabaseTest, LoadNotExistDatabase) {
    std::shared_ptr<User> user = Database::get<User>(99999); 

    EXPECT_EQ(user, nullptr);
}

// 5. Тест актуализации измененного объекта 
TEST_F(DatabaseTest, ActualDataBase) {
    std::shared_ptr<User> user = *Database::getAll<User>().begin();

    user->email = "dima321@mail.com";
    EXPECT_EQ(user->email, "dima321@mail.com");

    user->actual();
    EXPECT_EQ(user->email, "dima123@mail.com");
}

// 6. Тест удаления записи
TEST_F(DatabaseTest, RemoveDataBase) {
    std::shared_ptr<User> user = std::make_shared<User>("Юрий", "yuriy@mail.com");

    // Сохраняем еще одну запись
    EXPECT_TRUE(user->save()); 

    // Проверяем, что записей теперь две
    EXPECT_EQ(Database::getAll<User>().size(), 2);

    for (std::shared_ptr<User> user: Database::getAll<User>()) {
        if(user->name == "Дмитрий") {
            user->remove();
        }
    }

    // Проверяем что запись осталась только одна
    EXPECT_EQ(Database::getAll<User>().size(), 1);

    // Проверяем что оставшееся база данных - ожидаемая
    EXPECT_EQ(Database::getAll<User>().begin()->get()->name, "Юрий");
}

/// 7. Тест поиска данных в таблице
TEST_F(DatabaseTest, FindInDatabase) {
    // Подготовка тестовых данных: добавляем двух пользователей с одинаковым именем
    auto user1 = std::make_shared<User>();
    user1->name = "Alice";
    user1->age = 25;
    user1->save();

    auto user2 = std::make_shared<User>();
    user2->name = "Alice";
    user2->age =30;
    user2->save();

    auto user3 = std::make_shared<User>();
    user3->name = "Bob";
    user3->age = 40;
    user3->save();

    // --- Сценарий 1: Поиск по полю, где есть несколько совпадений ---
    {
        auto result = Database::find<User>("name", std::string("Alice"));
        
        ASSERT_TRUE(result.has_value());             // Проверяем, что вернулся не nullopt
        EXPECT_EQ(result->size(), 2);                // Должно найтись ровно 2 записи
        
        // Проверяем, что данные внутри вектора корректны
        EXPECT_EQ((*result)[0].name, "Alice");
        EXPECT_EQ((*result)[1].name, "Alice");
    }

    // --- Сценарий 2: Поиск по уникальному значению (одно совпадение) ---
    {
        auto result = Database::find<User>("age", 40);
        
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result->size(), 1);                // Должна найтись 1 запись
        EXPECT_EQ((*result)[0].name, "Bob");
    }

    // --- Сценарий 3: Поиск несуществующих данных ---
    {
        auto result = Database::find<User>("name", std::string("Charlie"));
        
        EXPECT_FALSE(result.has_value());            // Должен вернуться std::nullopt
    }

    Database::clear<User>();
}

// 8. Тест очистки всех данных таблицы
TEST_F(DatabaseTest, ClearTableDataBase) {
    User user = User("Дмитрий", "dima@mail.com");

    // Сохраняем еще одну запись
    EXPECT_TRUE(user.save()); 

    User user1 = User("Петр", "Petr@mail.com");

    // Сохраняем еще одну запись
    EXPECT_TRUE(user1.save()); 

    User user2 = User("Петр", "Petr@mail.com");

    // Сохраняем еще одну запись
    EXPECT_TRUE(user2.save()); 

    // Проверяем, что записей теперь две
    EXPECT_EQ(Database::getAll<User>().size(), 3);

    // Очищаем таблицу
    EXPECT_TRUE(Database::clear<User>());

    // Проверяем что не осталось не одной записи в таблице
    EXPECT_EQ(Database::getAll<User>().size(), 0);
}

