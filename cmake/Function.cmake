# Получает список всех файлов в указанной директории и её поддиректориях, 
# возвращая их в переменную родительской области видимости.
# Использование: get_entity_files(<TARGET_DIR> <RESULT_PATCHES>)
# Аргументы:
#   TARGET_DIR: Путь к целевой директории, в которой нужно искать файлы
#   RESULT_PATCHES: Имя переменной, в которую будет возвращён список найденных файлов
function(get_entity_files TARGET_DIR RESULT_PATCHES)
    # Рекурсивный поиск всех файлов в папке
    file(GLOB_RECURSE found_files "${TARGET_DIR}/*")
    
    # Возвращаем результат в переменную родительской области видимости
    set(${RESULT_PATCHES} "${found_files}" PARENT_SCOPE)
endfunction()

# Функция ищет уникальную директорию с именем "entities_database" в проекте.
# Если не найдено не одной директории или более одной директории, сборка прерывается с ошибкой.
# Использование: find_unique_entities_database_dir(<RESULT_VAR>)
# Аргументы:
#   RESULT_VAR: Имя переменной, в которую будет возвращён путь к найденной директории
function(find_unique_entities_database_dir RESULT_VAR)
    # Используем GLOB_RECURSE для бесконечной вложенности.
    # Шаблон **/entities_database заставит CMake искать эту папку везде внутри CMAKE_SOURCE_DIR.
    file(GLOB_RECURSE found_dirs 
         LIST_DIRECTORIES true 
         CONFIGURE_DEPENDS
         "${CMAKE_SOURCE_DIR}/**/entities_database"
    )

    set(filtered_dirs "")
    
    # Заранее приводим путь сборки к абсолютному виду для корректного сравнения
    get_filename_component(abs_binary_dir "${CMAKE_BINARY_DIR}" ABSOLUTE)

    foreach(path IN LISTS found_dirs)
        get_filename_component(abs_path "${path}" ABSOLUTE)
        
        # Получаем имя конкретной папки
        get_filename_component(dir_name "${abs_path}" NAME)

        # Безопасная проверка нахождения внутри папки сборки
        string(FIND "${abs_path}" "${abs_binary_dir}" is_inside_build)

        # Условия:
        # 1. Директория
        # 2. Имя совпадает с "entities_database"
        # 3. НЕ build
        # 4. Путь НЕ содержит скрытых папок
        if(IS_DIRECTORY "${abs_path}" 
           AND "${dir_name}" STREQUAL "entities_database"
           AND NOT is_inside_build EQUAL 0
           AND NOT abs_path MATCHES "[/\\\\]\\.[^/\\\\]+")
            
            # Исключаем дубликаты, которые GLOB_RECURSE может вернуть для одной и той же папки
            list(FIND filtered_dirs "${abs_path}" _already_added)
            if(_already_added EQUAL -1)
                list(APPEND filtered_dirs "${abs_path}")
            endif()
        endif()
    endforeach()

    # Считаем количество найденных папок
    list(LENGTH filtered_dirs dirs_count)

    # Проверка 1: Папка не найдена
    if(dirs_count EQUAL 0)
        message(FATAL_ERROR "Ошибка сборки: Директория 'entities_database' не найдена в проекте!")
    endif()

    # Проверка 2: Найдено больше одной папки
    if(dirs_count GREATER 1)
        message(STATUS "Найденные папки entities_database:")
        foreach(path IN LISTS filtered_dirs)
            message(STATUS "  - ${path}")
        endforeach()
        message(FATAL_ERROR "Ошибка сборки: Найдено более одной директории 'entities_database' в проекте: 
            (${dirs_count}). Должна быть только одна такая директория!")
    endif()

    # Если всё хорошо, возвращаем единственный путь в родительскую область видимости
    list(GET filtered_dirs 0 unique_dir)
    set(${RESULT_VAR} "${unique_dir}" PARENT_SCOPE)
endfunction()

# Функция ищет все файлы с расширением .hpp в указанной директории.
# Если файлов не найдено, сборка прерывается с ошибкой.
# Использование: get_hpp_files_in_dir(<DIR_PATH> <RESULT_VAR>)
# Аргументы:
#   DIR_PATH: Путь к директории, в которой нужно искать файлы
#   RESULT_VAR: Имя переменной, в которую будет возвращён список найденных файлов
function(get_hpp_files_in_dir dir_path RESULT_VAR)
    # Ищем только файлы с расширением .hpp
    # CONFIGURE_DEPENDS автоматически перезапустит CMake при добавлении/удалении файлов
    file(GLOB found_items
         CONFIGURE_DEPENDS
         "${dir_path}/*.hpp"
    )

    set(filtered_files "")
    foreach(item IN LISTS found_items)
        # Исключаем случайные директории, если они имеют имя с точкой (например, folder.hpp)
        if(NOT IS_DIRECTORY "${item}")
            list(APPEND filtered_files "${item}")
        endif()
    endforeach()

    # Считаем количество найденных файлов
    list(LENGTH filtered_files files_count)

    # Если файлов не найдено — прерываем сборку
    if(files_count EQUAL 0)
        message(FATAL_ERROR "Ошибка сборки: В директории '${dir_path}' не найдено ни одного файла *.hpp!")
    endif()

    # Возвращаем список файлов в родительскую область видимости
    set(${RESULT_VAR} "${filtered_files}" PARENT_SCOPE)
endfunction()

# Функция для добавления тестового исполняемого файла и регистрации его в CTest.
# Использование: add_test_exe(<NAME> SOURCES <source_files> LIBS <libraries> DEFS <definitions> LABELS <labels>)
# Аргументы:
#   NAME: Имя тестового исполняемого файла
#   SOURCES: Список исходных файлов для компиляции теста
#   LIBS: Список библиотек для линковки с тестом
#   DEFS: Список определений препроцессора для теста
#   LABELS: Список меток для теста (опционально)
function(add_test_exe NAME)
    cmake_parse_arguments(T "" "" "SOURCES;LIBS;DEFS;LABELS" ${ARGN})

    set(ABS_SOURCES)
    foreach(src IN LISTS T_SOURCES)
        if(IS_ABSOLUTE "${src}")
            list(APPEND ABS_SOURCES "${src}")
        else()
            list(APPEND ABS_SOURCES "${CMAKE_SOURCE_DIR}/${src}")
        endif()
    endforeach()

    add_executable(${NAME} ${ABS_SOURCES})

    target_link_libraries(${NAME} PUBLIC ${COMMON_LIBS} ${T_LIBS})

    target_include_directories(${NAME} PRIVATE ${COMMON_DIRECTORIES})

    if(T_DEFS)
        target_compile_definitions(${NAME} PRIVATE ${T_DEFS})
    endif()

    add_test(NAME ${NAME} COMMAND ${NAME}
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/tests)
    if (T_LABELS)
        set_tests_properties(${NAME} PROPERTIES LABELS "${T_LABELS}")
    endif()
endfunction()
