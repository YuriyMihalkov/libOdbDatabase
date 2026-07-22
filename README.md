# C++ OdbDatabase (PostgreSQL)

Легковесная объектно-ориентированная обертка над ORM-библиотекой **ODB** для C++. Проект предоставляет удобный шаблонный CRUD-интерфейс для работы с сущностями и ориентирован на работу с СУБД **PostgreSQL**, при необходимости могут быть легко поддержаны остальные платформы, поддерживаемые библиотекой **ODB**.

## Цель создания

Максимально упростить работу с реляционной базой данных, без необходимости написания больших объемов кода.

## 🚀 Особенности
* **Полиморфная модель данных**: Базовый класс `Database` размечен прагмой `#pragma db object polymorphic`, что позволяет прозрачно наследовать от него любые таблицы и сущности БД.
* **Шаблонный CRUD**: Универсальные методы `save()`, `actual()`, `update()` и `remove()` для управления записями без написания сырого SQL.
* **Потокобезопасность**: Все критические операции синхронизированы с помощью внутреннего `std::mutex`.
* **Инспекция схем PostgreSQL**: Встроенная поддержка получения списка таблиц напрямую из системного каталога `information_schema.tables` и функции безопасной очистки данных (`Drop`).

## ⚙️ Особенности использования

* Все классы объектов, которые планируется разместить в SQL-базе данных должны находиться в папке **"entities_database"** для автоматического добавления их в компилятор ODB ORM. Данная папка может находиться в любом месте проекта. 
* Классы объектов должны наследоваться от класса **"Database"**
* В классах должна быть указана специальная прагма, согласно [мануалу](https://www.codesynthesis.com/products/odb/doc/manual.xhtml#14) ODB ODB. Примеры такого класса имеются примерах использования.

---

## 📁 Структура проекта

```text
├── cmake/
│   ├── FindODB.cmake          # Скрипт поиска установленной библиотеки ODB
│   └── Function.cmake         # Скрипт автоматизации вызова компилятора odb
├── CMakeLists.txt              # Главный конфигурационный файл сборки
├── include/
│   └── Database.hpp            # Интерфейс базового класса БД и представлений
├── src/
│   ├── Database.cpp            # Реализация статических методов и метаданных PostgreSQL
│   ├── DatabaseImpl.tpp        # Реализация шаблонных CRUD-методов (Template Impl)
│   └── DatabaseManager.hpp     # Управление пулом соединений libodb-pgsql
└── tests/
    ├── CMakeLists.txt
    ├── entities_database/
    │   └── User.hpp            # Пример сущности-наследника класса Database
    └── tests_main.cpp          # Модульные тесты
```

---

## 🛠️ Требования к окружению

Перед сборкой проекта убедитесь, что в вашей системе установлены следующие компоненты:

1. **Компилятор C++** с поддержкой стандарта C++17 или выше (GCC, Clang).
2. **Система сборки CMake** версии 3.15 или выше.
3. **Клиентские библиотеки PostgreSQL**: `libpq-dev` (для Linux) или официальный PostgreSQL SDK.
4. **Компилятор ODB**: утилита командной строки `odb`.
5. **Рантайм-библиотеки ODB для PostgreSQL**: `libodb` и `libodb-pgsql`.

### Установка dependencies (на примере Ubuntu/Debian):
```bash
sudo apt-get update
sudo apt-get install build-essential cmake libpq-dev
sudo apt install postgresql-server-dev-all postgresql
sudo apt-get install odb libodb-dev libodb-pgsql-dev
```

---

## 💻 Пример использования

### 1. Создание сущности (Entity)
Чтобы отобразить C++ класс в таблицу PostgreSQL, унаследуйте его от `Database`:

```cpp
// tests/entities_database/User.hpp
#pragma once
#include "Database.hpp"
#include <string>

#pragma db object table("users")
class User : public Database {
public:
    User() = default;
    User(std::string name, std::string email) 
        : name(std::move(name)), email(std::move(email)) {}

    #pragma db type("VARCHAR(100)")
    std::string name;

    #pragma db type("VARCHAR(255)") unique
    std::string email;
};
```

### 2. CRUD операции и работа с базой
```cpp
#include "Database.hpp"
#include "entities_database/User.hpp"
#include <iostream>

int main() {
    // 1. Создание и сохранение объекта в PostgreSQL
    auto user = std::make_shared<User>("Иван", "ivan@example.com");
    if (user->save<User>()) {
        std::cout << "Пользователь сохранен в PG с ID: " << user->id << std::endl;
    }

    // 2. Получение объекта по ID
    unsigned long userId = user->id;
    std::shared_ptr<User> loadedUser = Database::get<User>(userId);
    
    if (loadedUser) {
        std::cout << "Загружен из PG: " << loadedUser->name << std::endl;
    }

    // 3. Обновление данных
    user->name = "Иван Измененный";
    user->update<User>();

    // 4. Удаление объекта
    user->remove<User>();

    // 5. Работа со схемой PostgreSQL
    auto tables = Database::getTablesBySchema("public");
    std::cout << "Список таблиц в схеме public:\n";
    for (const auto& table : tables) {
        std::cout << " - " << table << "\n";
    }

    return 0;
}
```

---

## 🔨 Сборка и запуск

1. Создание роли для базы данных

Для использования сервера вам понадобится пользователь с правами на создание базы данных:

```shell
sudo -u postgres createuser -d $USER
```

2. Создание базы данных, с которой будет работать сервер

По умолчанию используется имя `kpiok`, но вы можете дать любое имя и указать его при запуске сервера:

```shell
createdb my_database
```

3. Проект собирается стандартным для CMake способом. На этапе генерации скрипт `cmake/Function.cmake` автоматически вызывает компилятор `odb` с флагом `--database pgsql` для генерации файлов связи (`.hxx`, `.cxx`, `.ixx`).

```shell
# 1. Клонируйте репозиторий
git clone git@github.com:YuriyMihalkov/libOdbDatabase.git
cd libOdbDatabase

# 2. Создайте директорию для сборки
mkdir build && cd build

# 3. Сконфигурируйте проект
cmake -DCMAKE_BUILD_TYPE=Release ..

# 4. Скомпилируйте проект и тесты
cmake --build .
```

---

## 🧪 Запуск тестов

Перед запуском тестов убедитесь, что у вас поднят тестовый инстанс PostgreSQL, а параметры подключения в `DatabaseManager.hpp` или переменные окружения настроены корректно.

```bash
cd build
ctest --output-on-failure
```

---
