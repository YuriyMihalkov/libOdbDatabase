# Функция для получения всех файлов в указанной директории
function(get_entity_files target_dir result_var)
    # Рекурсивный поиск всех файлов в папке
    file(GLOB_RECURSE found_files "${target_dir}/*")
    
    # Возвращаем результат в переменную родительской области видимости
    set(${result_var} "${found_files}" PARENT_SCOPE)
endfunction()

# Функция поиска директории проекта
function(find_unique_entities_database_dir result_var)
    # Ищем строго папки с нужным именем на разной глубине, 
    # избегая тотального сканирования всего диска и папки build
    file(GLOB found_dirs 
         LIST_DIRECTORIES true 
         CONFIGURE_DEPENDS
         "${CMAKE_SOURCE_DIR}/entities_database"        # В самом корне
         "${CMAKE_SOURCE_DIR}/*/entities_database"      # На 1 уровень глубже (например, src/entities_database)
         "${CMAKE_SOURCE_DIR}/*/*/entities_database"    # На 2 уровня глубже
    )

    set(filtered_dirs "")
    foreach(path IN LISTS found_dirs)
        get_filename_component(abs_binary_dir "${CMAKE_BINARY_DIR}" ABSOLUTE)
        get_filename_component(abs_path "${path}" ABSOLUTE)
        
        # Получаем имя конкретной папки
        get_filename_component(dir_name "${abs_path}" NAME)

        # Безопасная проверка нахождения внутри папки сборки
        string(FIND "${abs_path}" "${abs_binary_dir}" is_inside_build)

        # Проверяем условия
        if(IS_DIRECTORY "${abs_path}" 
           AND "${dir_name}" STREQUAL "entities_database"
           AND NOT is_inside_build EQUAL 0
           AND NOT abs_path MATCHES "[/\\\\]\\.[^/\\\\]+")
            list(APPEND filtered_dirs "${abs_path}")
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
        message(FATAL_ERROR "Ошибка сборки: Найдено более одной директории 'entities_database' (${dirs_count}). Должна быть только одна!")
    endif()

    # Если всё хорошо, возвращаем единственный путь в родительскую область видимости
    list(GET filtered_dirs 0 unique_dir)
    set(${result_var} "${unique_dir}" PARENT_SCOPE)
endfunction()

function(get_hpp_files_in_dir dir_path result_var)
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
    set(${result_var} "${filtered_files}" PARENT_SCOPE)
endfunction()

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
