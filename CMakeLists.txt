cmake_minimum_required(VERSION 3.15)
project(OdbDatabase LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake/")
file(GLOB CMAKE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/cmake/*.cmake")
foreach(FILE_PATH ${CMAKE_FILES})
    include(${FILE_PATH})
endforeach()

find_package(ODB REQUIRED COMPONENTS pgsql)
find_unique_entities_database_dir(ENTIRES_PATH)

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
            --std c++20 --database pgsql --generate-query --generate-schema --schema-format embedded
            --output-dir "${CMAKE_CURRENT_BINARY_DIR}"
            -I "${CMAKE_CURRENT_SOURCE_DIR}/include" -I "${CMAKE_CURRENT_SOURCE_DIR}"
            "${CMAKE_CURRENT_SOURCE_DIR}/include/Database.hpp"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/include/Database.hpp"
    VERBATIM
)

# =====================================================================
# 2. Динамическая генерация для сущностей из entities_database
# =====================================================================
file(GLOB ENTIRES_PATHS
     CONFIGURE_DEPENDS
     "${ENTIRES_PATH}/*.hpp"
)

foreach(header IN LISTS ENTIRES_PATHS)
    get_filename_component(name_we "${header}" NAME_WLE)
    list(APPEND ALL_ODB_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/${name_we}-odb.cxx")
    
    add_custom_command(
        OUTPUT  "${CMAKE_CURRENT_BINARY_DIR}/${name_we}-odb.cxx"
                "${CMAKE_CURRENT_BINARY_DIR}/${name_we}-odb.hxx"
                "${CMAKE_CURRENT_BINARY_DIR}/${name_we}-odb.ixx"
        COMMAND ${ODB_EXECUTABLE}
                --std c++20 -d pgsql --generate-query --generate-schema --schema-format embedded
                --output-dir "${CMAKE_CURRENT_BINARY_DIR}"
                -I "${CMAKE_CURRENT_SOURCE_DIR}/include" 
                -I "${CMAKE_CURRENT_SOURCE_DIR}" 
                -I "${ENTIRES_PATH}"
                "${ENTIRES_PATH}/${name_we}.hpp"
        DEPENDS "${ENTIRES_PATH}/${name_we}.hpp" "${CMAKE_CURRENT_BINARY_DIR}/Database-odb.hxx"
        VERBATIM
    )
endforeach()

# =====================================================================
# 3. Таргеты, автогенерация карты инклюдов и их настройка
# =====================================================================
add_library(${PROJECT_NAME} STATIC ${ALL_ODB_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC 
    "${CMAKE_CURRENT_BINARY_DIR}"
    "${CMAKE_CURRENT_SOURCE_DIR}/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${ENTIRES_PATH}"
)

target_link_libraries(${PROJECT_NAME} PUBLIC ${ODB_LIBRARIES} ${ODB_PGSQL_LIBRARIES})

# =====================================================================
# 4. Тесты
# =====================================================================
option(ODB_DATABASE_DISABLE_TEST "Disable tests" FALSE)
if(NOT ODB_DATABASE_DISABLE_TEST)
    find_package(GTest CONFIG REQUIRED)
    include(CTest)
    enable_testing()
    add_subdirectory(tests)
endif()