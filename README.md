# C++ ODB Database Wrapper (PostgreSQL)

Легковесная и потокобезопасная объектно-ориентированная обертка (Wrapper) над ORM-библиотекой **ODB** для C++. Проект предоставляет удобный шаблонный CRUD-интерфейс для работы с полиморфными сущностями и ориентирован на работу с СУБД **PostgreSQL**.

## 🚀 Особенности

* **Полиморфная модель данных**: Базовый класс `Database` размечен прагмой `#pragma db object polymorphic`, что позволяет прозрачно наследовать от него любые таблицы и сущности БД.
* **Шаблонный CRUD**: Универсальные методы `save()`, `actual()`, `update()` и `remove()` для управления записями без написания сырого SQL.
* **Потокобезопасность**: Все критические операции синхронизированы с помощью внутреннего `std::mutex`.
* **Инспекция схем PostgreSQL**: Встроенная поддержка получения списка таблиц напрямую из системного каталога `information_schema.tables` и функции безопасной очистки данных (`Drop`).

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
# Установка ODB (если нет в репозиториях, требуется сборка из исходников или ppa)
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

## 🔨 Сборка проекта

Проект собирается стандартным для CMake способом. На этапе генерации скрипт `cmake/Function.cmake` автоматически вызывает компилятор `odb` с флагом `--database pgsql` для генерации файлов связи (`.hxx`, `.cxx`, `.ixx`).

```bash
# 1. Клонируйте репозиторий
git clone <url_вашего_репозитория>
cd <имя_папки_проекта>

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

## 📝 Особенности реализации для PostgreSQL

* **Инспекция Системного Каталога**: Метод `getTablesBySchema` использует встроенное ODB-представление `SchemaTableView`, которое обращается к таблице метаданных `information_schema.tables`. Это позволяет динамически отслеживать структуру базы данных PostgreSQL без выполнения «сырых» текстовых запросов вручную.
* **Разделение компиляции**: Использование макроса `#ifndef ODB_COMPILER`