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

# Инициализируем общий список исходников. 
# Заранее вносим туда файл Database-odb.cxx, чтобы таргет НИКОГДА не был пустым!
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
            -I "${CMAKE_CURRENT_SOURCE_DIR}"
            "${CMAKE_CURRENT_SOURCE_DIR}/Database.hpp"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/Database.hpp"
    COMMENT "Генерация ODB файлов для Database.hpp"
    VERBATIM
)


# =====================================================================
# 2. Динамическая генерация для сущностей из entities_database
# =====================================================================
find_unique_entities_database_dir(ENTIRES_PATH)

# Собираем hpp файлы прямо здесь, чтобы избежать багов с областями видимости функций
file(GLOB ENTIRES_PATHS
     CONFIGURE_DEPENDS
     "${ENTIRES_PATH}/*.hpp"
)

# Проверяем, что файлы вообще есть
list(LENGTH ENTIRES_PATHS files_count)
if(files_count EQUAL 0)
    message(FATAL_ERROR "Ошибка сборки: В директории '${ENTIRES_PATH}' не найдено ни одного файла *.hpp!")
endif()

foreach(header IN LISTS ENTIRES_PATHS)
    get_filename_component(name_we "${header}" NAME_WE)
    
    # Пути к файлам, которые создаст ODB для этой сущности
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
                -I "${CMAKE_CURRENT_SOURCE_DIR}"
                -I "${ENTIRES_PATH}"
                "${header}"
        DEPENDS "${header}" "${CMAKE_CURRENT_SOURCE_DIR}/Database.hpp"
        COMMENT "Generating ODB files for ${name_we}"
        VERBATIM
    )
    
    # Добавляем сгенерированный файл в общий список
    list(APPEND ALL_ODB_SOURCES "${generated_cxx}")
endforeach()


# =====================================================================
# 3. Таргеты и их настройка
# =====================================================================

# Создаем библиотеку (теперь в ALL_ODB_SOURCES гарантированно есть как минимум Database-odb.cxx)
add_library(OdbGeneratedObjects STATIC ${ALL_ODB_SOURCES})

target_include_directories(OdbGeneratedObjects PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${ENTIRES_PATH}"            
    "${CMAKE_CURRENT_BINARY_DIR}"
    ${ODB_INCLUDE_DIRS}          
)

# Основной исполняемый файл
add_executable(ObjectTestProject main.cpp)

# Автоматически внедряем заголовки базы данных при компиляции исполняемого файла
target_compile_options(ObjectTestProject PRIVATE
    "SHELL:-include ${CMAKE_CURRENT_BINARY_DIR}/Database-odb.hxx"
    "SHELL:-include ${CMAKE_CURRENT_BINARY_DIR}/User-odb.hxx"
)

target_include_directories(ObjectTestProject PRIVATE 
    "${CMAKE_CURRENT_SOURCE_DIR}"
    "${ENTIRES_PATH}"             
    "${CMAKE_CURRENT_BINARY_DIR}" 
    ${ODB_INCLUDE_DIRS}
)

# Линкуем все зависимости
target_link_libraries(ObjectTestProject PRIVATE 
    OdbGeneratedObjects
    ${ODB_LIBRARIES}
    ${ODB_PGSQL_LIBRARIES}
)