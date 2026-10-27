cmake_minimum_required(VERSION 3.15)
project(OdbDatabase LANGUAGES CXX)

# Санитайзеры
option(ENABLE_SANITIZERS "Enable Address and Undefined Behavior Sanitizers" OFF)

if(ENABLE_SANITIZERS)
    message(STATUS "Building with Address and Undefined Behavior Sanitizers")
    
    # 2. Добавляем флаги компиляции и линковки
    add_compile_options(-fsanitize=address,undefined -fno-omit-frame-pointer -g)
    add_link_options(-fsanitize=address,undefined)
endif()

# =====================================================================
# 0. Инициализация и обработка опций
# =====================================================================

if(NOT DEFINED ODB_DATABASE_TYPE)
    set(ODB_DATABASE_TYPE "pgsql" CACHE STRING "Type of database" FORCE)
endif()

# Выбираем тип базы данных
if(ODB_DATABASE_TYPE STREQUAL "pgsql")
    set(ODB_TARGET_NAME ODB::pgsql)    
elseif(ODB_DATABASE_TYPE STREQUAL "sqlite")
    set(ODB_TARGET_NAME ODB::sqlite)
else()
    message(FATAL_ERROR "Unknown database type: '${ODB_DATABASE_TYPE}'. Supported types are: pgsql, sqlite")
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
file(GLOB CMAKE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake/*.cmake")
foreach(FILE_PATH ${CMAKE_FILES})
    include(${FILE_PATH})
endforeach()

find_package(ODB REQUIRED COMPONENTS ${ODB_DATABASE_TYPE})

# Создаем интерфейсный таргет-обертку, если его нет
if(NOT TARGET ODB::odb)
    add_library(ODB::odb INTERFACE IMPORTED)
    set_target_properties(ODB::odb PROPERTIES
        INTERFACE_INCLUDE_DIRECTORIES "${ODB_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "${ODB_LIBRARIES}"
    )
endif()

if(NOT TARGET ${ODB_TARGET_NAME})
    add_library(${ODB_TARGET_NAME} INTERFACE IMPORTED)
    set_target_properties(${ODB_TARGET_NAME} PROPERTIES
        INTERFACE_LINK_LIBRARIES "${ODB_PGSQL_LIBRARIES};ODB::odb"
    )
endif()

if(NOT DEFINED ODB_DATABASE_DISABLE_TEST)
    option(ODB_DATABASE_DISABLE_TEST "Disable tests" FALSE)
endif()

find_unique_entities_database_dir(ENTIRES_PATHS)

# Список всех компилируемых файлов
set(ALL_ODB_SOURCES 
    "${CMAKE_CURRENT_BINARY_DIR}/Database-odb.cxx"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/Database.cpp"
)

# =====================================================================
# 1. Генерация для базового файла Database.hpp
# =====================================================================
add_custom_command(
    OUTPUT "${CMAKE_CURRENT_BINARY_DIR}/Database-odb.cxx"
           "${CMAKE_CURRENT_BINARY_DIR}/Database-odb.hxx"
           "${CMAKE_CURRENT_BINARY_DIR}/Database-odb.ixx"
    COMMAND ${ODB_EXECUTABLE}
            --std c++20 --database ${ODB_DATABASE_TYPE} --generate-query --generate-schema --schema-format embedded
            --output-dir "${CMAKE_CURRENT_BINARY_DIR}"
            -I "${ODB_INCLUDE_DIRS}"
            -I "${CMAKE_CURRENT_SOURCE_DIR}/include"
            -I "${CMAKE_CURRENT_SOURCE_DIR}"
            "${CMAKE_CURRENT_SOURCE_DIR}/include/Database.hpp"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/Database.hpp"
    VERBATIM
)

# =====================================================================
# 2. Динамическая генерация для сущностей из entities_database
# =====================================================================
file(GLOB HANDLER_ENTIRES_PATHS
     CONFIGURE_DEPENDS
     "${ENTIRES_PATHS}/*.hpp"
)

foreach(header IN LISTS HANDLER_ENTIRES_PATHS)
    get_filename_component(name_we "${header}" NAME_WLE)
    list(APPEND ALL_ODB_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/${name_we}-odb.cxx")
    
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/${name_we}-odb.cxx"
                "${CMAKE_CURRENT_BINARY_DIR}/${name_we}-odb.hxx"
                "${CMAKE_CURRENT_BINARY_DIR}/${name_we}-odb.ixx"
        COMMAND ${ODB_EXECUTABLE}
                --std c++20 -d ${ODB_DATABASE_TYPE} --generate-query --generate-schema --schema-format embedded
                --output-dir "${CMAKE_CURRENT_BINARY_DIR}"
                -I "${ODB_INCLUDE_DIRS}"
                -I "${CMAKE_CURRENT_SOURCE_DIR}/include" 
                -I "${CMAKE_CURRENT_SOURCE_DIR}" 
                -I "${ENTIRES_PATHS}"
                "${ENTIRES_PATHS}/${name_we}.hpp"
        DEPENDS "${ENTIRES_PATHS}/${name_we}.hpp" "${CMAKE_CURRENT_BINARY_DIR}/Database-odb.hxx"
        VERBATIM
    )
endforeach()

# =====================================================================
# 3. Таргеты, автогенерация карты инклюдов и их настройка
# =====================================================================
add_library(${PROJECT_NAME} STATIC ${ALL_ODB_SOURCES})

# Передаем тип базы данных в качестве константы
if(ODB_DATABASE_TYPE STREQUAL "pgsql")
    target_compile_definitions(${PROJECT_NAME} PUBLIC ODB_DATABASE_TYPE_PGSQL)
elseif(ODB_DATABASE_TYPE STREQUAL "sqlite")
    target_compile_definitions(${PROJECT_NAME} PUBLIC ODB_DATABASE_TYPE_SQLITE)
endif()

if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE ODB_COMPILER)
endif()

target_include_directories(${PROJECT_NAME} 
    PUBLIC 
        "${CMAKE_CURRENT_SOURCE_DIR}/include"
        "${CMAKE_CURRENT_SOURCE_DIR}/config"
        "${CMAKE_CURRENT_SOURCE_DIR}/src"
        "${CMAKE_CURRENT_BINARY_DIR}"
        "${ENTIRES_PATHS}"
	${ODB_INCLUDE_DIRS}
)

target_link_libraries(${PROJECT_NAME} PUBLIC ODB::odb ${ODB_TARGET_NAME})

# =====================================================================
# 4. Тесты
# =====================================================================
if(NOT ODB_DATABASE_DISABLE_TEST)
    find_package(GTest CONFIG REQUIRED)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()
