cmake_minimum_required(VERSION 3.28)
project(ObjectTestProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
file(GLOB CMAKE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake/*.cmake")
foreach(FILE_PATH ${CMAKE_FILES})
    include(${FILE_PATH})
endforeach()

find_package(ODB REQUIRED COMPONENTS pgsql)

# Инициализируем список исходников сгенерированной библиотеки
set(ALL_ODB_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/Database-odb.cxx")

# =====================================================================
# 1. Генерация для базового файла Database.hpp
# =====================================================================
set(DATABASE_GENERATED
    "${CMAKE_CURRENT_BINARY_DIR}/Database-odb.cxx"
    "${CMAKE_CURRENT_BINARY_DIR}/Database-odb.hxx"
    "${CMAKE_CURRENT_BINARY_DIR}/Database-odb.ixx"
)

add_custom_command(
    OUTPUT ${DATABASE_GENERATED}
    COMMAND ${ODB_EXECUTABLE}
            --std c++20   
            --database pgsql
            --generate-query
            --generate-schema
            --schema-format embedded
            --schema-name app
            --output-dir "${CMAKE_CURRENT_BINARY_DIR}"
            -I "${CMAKE_CURRENT_SOURCE_DIR}/include"  # Исправлено: путь к папке с Database.hpp
            -I "${CMAKE_CURRENT_SOURCE_DIR}"
            "${CMAKE_CURRENT_SOURCE_DIR}/include/Database.hpp"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/Database.hpp"
    COMMENT "Генерация ODB файлов для Database.hpp"
    VERBATIM
)

# =====================================================================
# 2. Динамическая генерация для сущностей из entities_database
# =====================================================================
find_unique_entities_database_dir(ENTIRES_PATH)

file(GLOB ENTIRES_PATHS
     CONFIGURE_DEPENDS
     "${ENTIRES_PATH}/*.hpp"
)

list(LENGTH ENTIRES_PATHS files_count)
if(files_count EQUAL 0)
    message(FATAL_ERROR "Ошибка сборки: В директории '${ENTIRES_PATH}' не найдено ни одного файла *.hpp!")
endif()

foreach(header IN LISTS ENTIRES_PATHS)
    get_filename_component(name_we "${header}" NAME_WLE)
    
    set(generated_cxx "${CMAKE_CURRENT_BINARY_DIR}/${name_we}-odb.cxx")
    set(generated_hxx "${CMAKE_CURRENT_BINARY_DIR}/${name_we}-odb.hxx")
    set(generated_ixx "${CMAKE_CURRENT_BINARY_DIR}/${name_we}-odb.ixx")
    
    add_custom_command(
        OUTPUT "${generated_cxx}" "${generated_hxx}" "${generated_ixx}"
        COMMAND ${ODB_EXECUTABLE}
                --std c++20
                -d pgsql 
                --generate-query 
                --output-dir "${CMAKE_CURRENT_BINARY_DIR}"
                -I "${CMAKE_CURRENT_SOURCE_DIR}/include" # Чтобы ODB видел Database.hpp
                -I "${CMAKE_CURRENT_SOURCE_DIR}"
                -I "${ENTIRES_PATH}"
                "${header}"
        DEPENDS "${header}" "${CMAKE_CURRENT_SOURCE_DIR}/include/Database.hpp" ${DATABASE_GENERATED}
        COMMENT "Generating ODB files for ${name_we}"
        VERBATIM
    )
    
    list(APPEND ALL_ODB_SOURCES "${generated_cxx}")
endforeach()

# =====================================================================
# 3. Таргеты, автогенерация карты инклюдов и их настройка
# =====================================================================

# Путь к файлу, который автоматически объединит все ODB-заголовки
set(AUTO_INCLUDES_FILE "${CMAKE_CURRENT_BINARY_DIR}/AllOdbTraits.hxx")

# Генерируем содержимое файла на лету
set(AUTO_INCLUDES_CONTENT "//#pragma once\n// Автоматически сгенерированный файл. Не редактировать!\n")
string(APPEND AUTO_INCLUDES_CONTENT "#include <Database-odb.hxx>\n")

foreach(header IN LISTS ENTIRES_PATHS)
    get_filename_component(name_we "${header}" NAME_WLE)
    string(APPEND AUTO_INCLUDES_CONTENT "#include <${name_we}-odb.hxx>\n")
endforeach()

# Записываем файл на диск (обновляется только если состав папки изменился)
file(WRITE "${AUTO_INCLUDES_FILE}" "${AUTO_INCLUDES_CONTENT}")

# Создаем статическую библиотеку из сгенерированных ODB файлов
add_library(OdbGeneratedObjects STATIC ${ALL_ODB_SOURCES})

target_include_directories(OdbGeneratedObjects PUBLIC 
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${ENTIRES_PATH}"
)

target_link_libraries(OdbGeneratedObjects PUBLIC
    ${ODB_LIBRARIES}
    ${ODB_PGSQL_LIBRARIES}
)

# Основной исполняемый файл
add_executable(ObjectTestProject main.cpp)

# Насильно внедряем сгенерированную карту инклюдов во все файлы исполняемого проекта.
# Это избавит от необходимости писать #include в main.cpp вручную!
target_compile_options(ObjectTestProject PRIVATE
    "SHELL:-include ${AUTO_INCLUDES_FILE}"
)

target_include_directories(ObjectTestProject PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${ENTIRES_PATH}"             
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_BINARY_DIR}"
)

# Линкуем сгенерированную библиотеку.
# target_link_libraries гарантирует, что сначала запустятся add_custom_command,
# файлы сгенерируются, и только потом начнется компиляция main.cpp
target_link_libraries(ObjectTestProject PRIVATE 
    OdbGeneratedObjects
)