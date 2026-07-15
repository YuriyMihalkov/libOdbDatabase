#include <gtest/gtest.h>
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
    std::shared_ptr<User> user = std::make_shared<User>("Дмитрий", "dima@mail.com");

    // Сохраняем указатель на тип в базе данных, проверяем возвращаемый результат
    EXPECT_TRUE(user->save<User>()); 
}

TEST_F(DatabaseTest, loadDataBase) {
    // Получаем данные о пользователе из базы данных
    std::vector<std::shared_ptr<User>>users = Database::getAll<User>();

    EXPECT_TRUE(users.size() > 0);
    EXPECT_TRUE(users.begin()->get()->email == "dima@mail.com");
    EXPECT_TRUE(users.begin()->get()->name == "Дмитрий");
}



// // 2. Тест получения несуществующего ID (get)
// TEST_F(DatabaseTest, GetNonExistentIdReturnsNullptr) {
//     odb::transaction t(db->begin());
    
//     // ODB при поиске несуществующего id возвращает исключение или nullptr в зависимости от метода.
//     // Если ваш Database::get использует db->find(), он вернет nullptr.
//     std::shared_ptr<User> retrieved = Database::get<User>(99999); 
//     t.commit();

//     EXPECT_EQ(retrieved, nullptr);
// }

// // 3. Тест получения всех объектов (getAll)
// TEST_F(DatabaseTest, GetAllObjectsReturnsAllStoredEntries) {
//     // Подготовка данных: сохраняем двух пользователей
//     {
//         odb::transaction t(db->begin());
//         auto user1 = std::make_shared<User>("Bob");
//         auto user2 = std::make_shared<User>("Charlie");
//         db->persist(user1);
//         db->persist(user2);
//         t.commit();
//     }

//     // Действие: вызываем ваш тестируемый метод getAll
//     odb::transaction t(db->begin());
//     std::vector<std::shared_ptr<User>> all_users = Database::getAll<User>();
//     t.commit();

//     // Проверка
//     ASSERT_EQ(all_users.size(), 2);
//     EXPECT_EQ(all_users[0]->name, "Bob");
//     EXPECT_EQ(all_users[1]->name, "Charlie");
// }

// // 4. Тест получения списка из пустой таблицы (getAll)
// TEST_F(DatabaseTest, GetAllObjectsWhenEmptyReturnsEmptyVector) {
//     odb::transaction t(db->begin());
//     std::vector<std::shared_ptr<User>> all_users = Database::getAll<User>();
//     t.commit();

//     EXPECT_TRUE(all_users.empty());
//     EXPECT_EQ(all_users.size(), 0);
// }