#
# Этот модуль определяет следующие переменные:
#
# ODB_USE_FILE - Путь к файлу UseODB.cmake. Используйте его для включения файла использования ODB.
#
# Файл использования определяет необходимую функциональность для компиляции и использования
# заголовочных файлов, сгенерированных ODB.
#
# ODB_FOUND - Все необходимые компоненты и основная библиотека найдены
# ODB_INCLUDE_DIRS - Объединенный список всех каталогов включения компонентов
# ODB_LIBRARIES - Объединенный список всех библиотек компонентов
#
# ODB_LIBODB_FOUND - Основная библиотека Libodb найдена
# ODB_LIBODB_INCLUDE_DIRS - Каталоги включения для основной библиотеки Libodb
# ODB_LIBODB_LIBRARIES - Библиотеки для основной библиотеки Libodb
#
# Для каждого запрошенного компонента определены следующие переменные:
#
# ODB_<component>_FOUND - Компонент найден
# ODB_<component>_INCLUDE_DIRS - Каталоги включения компонентов
# ODB_<component>_LIBRARIES - Библиотеки компонентов
#
# <component> - это исходное или написанное заглавными буквами имя компонента
#
# Имена компонентов напрямую связаны с именами модулей ODB.
#

set(ODB_USE_FILE "${CMAKE_CURRENT_LIST_DIR}/UseODB.cmake")

find_package(PkgConfig QUIET)

# ---------------------------------------------------------------------------
# Вспомогательная функция: вывести корень ODB-инсталляции из пути к odb.exe
# ---------------------------------------------------------------------------
function(_odb_derive_root_from_exe odb_exe out_var)
    if(odb_exe AND EXISTS "${odb_exe}")
        get_filename_component(_bin_dir "${odb_exe}" DIRECTORY)
        get_filename_component(_root "${_bin_dir}" DIRECTORY)
        set(${out_var} "${_root}" PARENT_SCOPE)
    else()
        set(${out_var} "" PARENT_SCOPE)
    endif()
endfunction()

# ---------------------------------------------------------------------------
# Поиск исполняемого файла ODB (code generator)
# ---------------------------------------------------------------------------
find_program(odb_BIN
    NAMES odb odb.exe
    HINTS
        ${ODB_BIN_PATH}
        ${ODB_LIBRARY_PATH}/../bin
        ${libodb_INCLUDE_DIR}/../bin)

# ---------------------------------------------------------------------------
# Поиск include-директории libodb (odb/version.hxx)
#
# Ищем в нескольких местах:
#   1. Явно заданный libodb_INCLUDE_DIR (из -D...)
#   2. ODB_INCLUDE_PATH (свой hint)
#   3. pkg-config
#   4. Рядом с найденным odb.exe (../include от корня ODB)
#   5. Стандартные пути CMake
# ---------------------------------------------------------------------------
set(_odb_root_from_exe "")
_odb_derive_root_from_exe("${odb_BIN}" _odb_root_from_exe)

pkg_check_modules(PC_LIBODB QUIET "libodb")

find_path(libodb_INCLUDE_DIR
    NAMES odb/version.hxx
    HINTS
        ${ODB_INCLUDE_PATH}
        ${PC_LIBODB_INCLUDE_DIRS}
        "${_odb_root_from_exe}/include")

# ---------------------------------------------------------------------------
# Поиск линкуемой библиотеки libodb.
#
# ВНИМАНИЕ: на многих Windows/MinGW-инсталляциях ODB присутствует ТОЛЬКО
# как code generator (odb.exe). Тогда libodb.a / libodb.dll.a отсутствует,
# и это НЕ ошибка — сборка всё равно возможна, если проект не линкуется
# с libodb напрямую.
# ---------------------------------------------------------------------------
set(ODB_LIBRARY_PATH "" CACHE STRING "Common library search hint for all ODB libs")

find_library(libodb_LIBRARY
    NAMES odb libodb
    HINTS
        ${ODB_LIBRARY_PATH}
        ${PC_LIBODB_LIBRARY_DIRS}
        "${_odb_root_from_exe}/lib")

set(ODB_LIBODB_INCLUDE_DIRS ${libodb_INCLUDE_DIR} CACHE STRING "ODB libodb include dirs")
set(ODB_LIBODB_LIBRARIES    ${libodb_LIBRARY}    CACHE STRING "ODB libodb library")
set(ODB_EXECUTABLE          ${odb_BIN}           CACHE STRING "ODB executable")

mark_as_advanced(libodb_INCLUDE_DIR libodb_LIBRARY odb_BIN)

# ---------------------------------------------------------------------------
# Определяем ODB_LIBODB_FOUND.
#
# Для code-generator-only режима достаточно ODB_EXECUTABLE + include.
# Линкуемая библиотека не обязательна.
# ---------------------------------------------------------------------------
if(ODB_LIBODB_INCLUDE_DIRS AND ODB_EXECUTABLE)
    set(ODB_LIBODB_FOUND TRUE)
    if(ODB_LIBODB_LIBRARIES)
        message(STATUS "ODB: found libodb include + library")
    else()
        message(STATUS "ODB: found include + executable (code-generator-only mode, no libodb library)")
    endif()
endif()

set(ODB_INCLUDE_DIRS ${ODB_LIBODB_INCLUDE_DIRS})
set(ODB_LIBRARIES    ${ODB_LIBODB_LIBRARIES})

# ---------------------------------------------------------------------------
# Поиск компонентов (pgsql, mysql, sqlite, qt и т.п.)
# ---------------------------------------------------------------------------
function(find_odb_api component)
    string(TOUPPER "${component}" component_u)
    set(ODB_${component_u}_FOUND FALSE PARENT_SCOPE)

    # Копируем текущие значения из родительской области в локальные переменные,
    # чтобы накапливать include-директории компонентов без потери базовых.
    set(_odb_inc_dirs ${ODB_INCLUDE_DIRS})
    set(_odb_libs     ${ODB_LIBRARIES})

    pkg_check_modules(PC_ODB_${component} QUIET "libodb-${component}")

    find_path(ODB_${component}_INCLUDE_DIR
        NAMES odb/${component}/version.hxx
        HINTS
            ${ODB_INCLUDE_PATH}
            ${_odb_inc_dirs}
            ${PC_ODB_${component}_INCLUDE_DIRS}
            "${_odb_root_from_exe}/include")

    find_library(ODB_${component}_LIBRARY
        NAMES odb-${component} libodb-${component}
        HINTS
            ${ODB_LIBRARY_PATH}
            ${PC_ODB_${component}_LIBRARY_DIRS}
            "${_odb_root_from_exe}/lib")

    set(ODB_${component_u}_INCLUDE_DIRS ${ODB_${component}_INCLUDE_DIR} CACHE STRING "ODB ${component} include dirs")
    set(ODB_${component_u}_LIBRARIES    ${ODB_${component}_LIBRARY}    CACHE STRING "ODB ${component} libraries")

    mark_as_advanced(ODB_${component}_INCLUDE_DIR ODB_${component}_LIBRARY)

    if(ODB_${component_u}_INCLUDE_DIRS)
        set(ODB_${component_u}_FOUND TRUE PARENT_SCOPE)
        set(ODB_${component}_FOUND   TRUE PARENT_SCOPE)

        list(APPEND _odb_inc_dirs ${ODB_${component_u}_INCLUDE_DIRS})
        list(REMOVE_DUPLICATES _odb_inc_dirs)
        set(ODB_INCLUDE_DIRS ${_odb_inc_dirs} PARENT_SCOPE)

        if(ODB_${component_u}_LIBRARIES)
            list(APPEND _odb_libs ${ODB_${component_u}_LIBRARIES})
            list(REMOVE_DUPLICATES _odb_libs)
            set(ODB_LIBRARIES ${_odb_libs} PARENT_SCOPE)
        endif()
    endif()
endfunction()

message(STATUS "=== ODB_INCLUDE_DIRS = '${ODB_INCLUDE_DIRS}'")
message(STATUS "=== ODB_LIBODB_INCLUDE_DIRS = '${ODB_LIBODB_INCLUDE_DIRS}'")
message(STATUS "=== ODB_PGSQL_INCLUDE_DIRS = '${ODB_PGSQL_INCLUDE_DIRS}'")

foreach(component ${ODB_FIND_COMPONENTS})
    find_odb_api(${component})
endforeach()

include(FindPackageHandleStandardArgs)

# Требуем odb.exe и include libodb. Линкуемые библиотеки — опционально.
find_package_handle_standard_args(ODB
    FOUND_VAR ODB_FOUND
    REQUIRED_VARS ODB_EXECUTABLE ODB_LIBODB_INCLUDE_DIRS
    HANDLE_COMPONENTS)
