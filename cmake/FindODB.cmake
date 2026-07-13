#
# Этот модуль определяет следующие переменные:
#
# ODB_USE_FILE - Путь к файлу UseODB.cmake. Используйте его для включения файла использования ODB.

# Файл использования определяет необходимую функциональность для компиляции и использования
# заголовочных файлов, сгенерированных ODB.

#
# ODB_FOUND - Все необходимые компоненты и основная библиотека найдены
# ODB_INCLUDR_DIRS - Объединенный список всех каталогов включения компонентов
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

# # Так, для библиотеки libodb-mysql.so компонент называется mysql,
# для модуля libodb-qt.so — qt, и так далее.

#

set(ODB_USE_FILE "${CMAKE_CURRENT_LIST_DIR}/UseODB.cmake")

find_package(PkgConfig QUIET)

function(find_odb_api component)
    string(TOUPPER "${component}" component_u)
    set(ODB_${component_u}_FOUND FALSE PARENT_SCOPE)

    pkg_check_modules(PC_ODB_${component} QUIET "libodb-${component}")

    find_path(ODB_${component}_INCLUDE_DIR
        NAMES odb/${component}/version.hxx
        HINTS
            ${ODB_LIBODB_INCLUDE_DIRS}
            ${PC_ODB_${component}_INCLUDE_DIRS})

    find_library(ODB_${component}_LIBRARY
        NAMES odb-${component} libodb-${component}
        HINTS
            ${ODB_LIBRARY_PATH}
            ${PC_ODB_${component}_LIBRARY_DIRS})

    set(ODB_${component_u}_INCLUDE_DIRS ${ODB_${component}_INCLUDE_DIR} CACHE STRING "ODB ${component} include dirs")
    set(ODB_${component_u}_LIBRARIES ${ODB_${component}_LIBRARY} CACHE STRING "ODB ${component} libraries")

    mark_as_advanced(ODB_${component}_INCLUDE_DIR ODB_${component}_LIBRARY)

    if(ODB_${component_u}_INCLUDE_DIRS AND ODB_${component_u}_LIBRARIES)
        set(ODB_${component_u}_FOUND TRUE PARENT_SCOPE)
        set(ODB_${component}_FOUND TRUE PARENT_SCOPE)

        list(APPEND ODB_INCLUDE_DIRS ${ODB_${component_u}_INCLUDE_DIRS})
        list(REMOVE_DUPLICATES ODB_INCLUDE_DIRS)
        set(ODB_INCLUDE_DIRS ${ODB_INCLUDE_DIRS} PARENT_SCOPE)

        list(APPEND ODB_LIBRARIES ${ODB_${component_u}_LIBRARIES})
        list(REMOVE_DUPLICATES ODB_LIBRARIES)
        set(ODB_LIBRARIES ${ODB_LIBRARIES} PARENT_SCOPE)
    endif()
endfunction()

pkg_check_modules(PC_LIBODB QUIET "libodb")

set(ODB_LIBRARY_PATH "" CACHE STRING "Common library search hint for all ODB libs")

find_path(libodb_INCLUDE_DIR
    NAMES odb/version.hxx
    HINTS
        ${PC_LIBODB_INCLUDE_DIRS})

find_library(libodb_LIBRARY
    NAMES odb libodb
    HINTS
        ${ODB_LIBRARY_PATH}
        ${PC_LIBODB_LIBRARY_DIRS})

find_program(odb_BIN
    NAMES odb
    HINTS
        ${libodb_INCLUDE_DIR}/../bin)

set(ODB_LIBODB_INCLUDE_DIRS ${libodb_INCLUDE_DIR} CACHE STRING "ODB libodb include dirs")
set(ODB_LIBODB_LIBRARIES ${libodb_LIBRARY} CACHE STRING "ODB libodb library")
set(ODB_EXECUTABLE ${odb_BIN} CACHE STRING "ODB executable")

mark_as_advanced(libodb_INCLUDE_DIR libodb_LIBRARY odb_BIN)

if(ODB_LIBODB_INCLUDE_DIRS AND ODB_LIBODB_LIBRARIES)
    set(ODB_LIBODB_FOUND TRUE)
endif()

set(ODB_INCLUDE_DIRS ${ODB_LIBODB_INCLUDE_DIRS})
set(ODB_LIBRARIES ${ODB_LIBODB_LIBRARIES})

foreach(component ${ODB_FIND_COMPONENTS})
    find_odb_api(${component})
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(ODB
    FOUND_VAR ODB_FOUND
    REQUIRED_VARS ODB_EXECUTABLE ODB_LIBODB_FOUND
    HANDLE_COMPONENTS)
