FROM ubuntu:26.04

ENV DEBIAN_FRONTEND=noninteractive

# Настройка русской локализации
RUN apt-get update && apt-get install -y locales && \
    locale-gen ru_RU.UTF-8 && \
    update-locale LANG=ru_RU.UTF-8
ENV LANG=ru_RU.UTF-8
ENV LC_ALL=ru_RU.UTF-8

# Установка компиляторов, утилит и КЛИЕНТСКИХ библиотек PostgreSQL (для ODB)
# Установка компиляторов, утилит и КЛИЕНТСКИХ библиотек PostgreSQL (для ODB)
RUN apt-get install -y --no-install-recommends \
    build-essential \
    gcc-15 \
    g++-15 \
    clang-18 \
    cmake \
    ninja-build \
    gdb \
    valgrind \
    git \
    curl \
    wget \
    ca-certificates \
    sudo \
    libpq-dev \
    bash-completion \
    sqlite3 \
    libsqlite3-dev \
    googletest \
    libgtest-dev \
    gcc-15-plugin-dev \
    libcutl-dev \
    qt6-base-dev \
    qt6-httpserver-dev \
    qt6-tools-dev \
    qt6-tools-dev-tools \
    qt6-websockets-dev \
    && rm -rf /var/lib/apt/lists/*

# === СКАЧИВАНИЕ И УСТАНОВКА ODB 2.5.0 С ОФИЦИАЛЬНОГО САЙТА ===
WORKDIR /tmp/odb_install
RUN wget -q \
    https://www.codesynthesis.com/download/odb/2.6.0/ubuntu/ubuntu26.04/x86_64/odb_2.6.0-0~ubuntu26.04_amd64.deb \
    https://www.codesynthesis.com/download/odb/2.6.0/ubuntu/ubuntu26.04/x86_64/libodb_2.6.0-0~ubuntu26.04_amd64.deb \
    https://www.codesynthesis.com/download/odb/2.6.0/ubuntu/ubuntu26.04/x86_64/libodb-dev_2.6.0-0~ubuntu26.04_amd64.deb \
    https://www.codesynthesis.com/download/odb/2.6.0/ubuntu/ubuntu26.04/x86_64/libodb-qt_2.6.0-0~ubuntu26.04_amd64.deb \
    https://www.codesynthesis.com/download/odb/2.6.0/ubuntu/ubuntu26.04/x86_64/libodb-qt-dev_2.6.0-0~ubuntu26.04_amd64.deb \
    https://www.codesynthesis.com/download/odb/2.6.0/ubuntu/ubuntu26.04/x86_64/libodb-pgsql_2.6.0-0~ubuntu26.04_amd64.deb \
    https://www.codesynthesis.com/download/odb/2.6.0/ubuntu/ubuntu26.04/x86_64/libodb-pgsql-dev_2.6.0-0~ubuntu26.04_amd64.deb \
    https://www.codesynthesis.com/download/odb/2.6.0/ubuntu/ubuntu26.04/x86_64/libodb-sqlite_2.6.0-0~ubuntu26.04_amd64.deb \
    https://www.codesynthesis.com/download/odb/2.6.0/ubuntu/ubuntu26.04/x86_64/libodb-sqlite-dev_2.6.0-0~ubuntu26.04_amd64.deb \
    https://www.codesynthesis.com/download/odb/2.6.0/ubuntu/ubuntu26.04/x86_64/libodb-boost_2.6.0-0~ubuntu26.04_amd64.deb \
    https://www.codesynthesis.com/download/odb/2.6.0/ubuntu/ubuntu26.04/x86_64/libodb-boost-dev_2.6.0-0~ubuntu26.04_amd64.deb \
    && dpkg -i *.deb \
    && rm -rf /tmp/odb_install

# Сборка и установка GTest через временную директорию
RUN cp -r /usr/src/googletest /tmp/gtest_build && \
    cd /tmp/gtest_build && \
    mkdir build && \
    cd build && \
    cmake -DCMAKE_BUILD_TYPE=Release .. && \
    make -j$(nproc) && \
    make install && \
    rm -rf /tmp/gtest_build

# Дополнительно: Установка рантайма ODB (библиотек libodb и libodb-pgsql), если они есть в репозиториях,
# либо подготовка зависимостsudo apt-get install -y qttools5-dev qttools5-dev-toolsей для сборки ODB из исходников/build2.
# Примечание: Если вы ставите сам компилятор 'odb' бинарником, пакет libpq-dev необходим для линковки.

# Настраиваем компиляторы как независимые master-альтернативы
RUN update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-15 100 && \
    update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-15 100 && \
    update-alternatives --install /usr/bin/cc cc /usr/bin/gcc-15 100 && \
    update-alternatives --install /usr/bin/c++ c++ /usr/bin/g++-15 100

RUN useradd -m -s /bin/bash -G sudo developer && \
    echo "developer ALL=(ALL) NOPASSWD:ALL" >> /etc/sudoers

WORKDIR /workspace
USER developer

CMD ["sleep", "infinity"]

# Команда пересоздания
# docker build --no-cache -t ubuntu2204_forprojectcontrol-cpp-dev .
