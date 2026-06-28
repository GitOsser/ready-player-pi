cmake_minimum_required(VERSION 3.16)
project(ReadyPlayerPi VERSION 0.1.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_compile_options(-Wall -Wextra)

# ── Dependencies ──────────────────────────────────────────────────────────────
# Install on Raspberry Pi / Ubuntu:
#   sudo apt install nlohmann-json3-dev libasound2-dev \
#                    qt6-base-dev qt6-virtualkeyboard-dev
find_package(nlohmann_json 3.2.0 REQUIRED)
find_package(Threads REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(ALSA REQUIRED IMPORTED_TARGET alsa)
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)
find_package(Qt6 OPTIONAL_COMPONENTS VirtualKeyboard)
if(Qt6VirtualKeyboard_FOUND)
    add_compile_definitions(HAVE_QT_VIRTUAL_KEYBOARD)
endif()

# kissfft (C library for FFT - DSB Req 5)
set(KISSFFT_SRCS
    ThirdParty/kissfft/kiss_fft.c
    ThirdParty/kissfft/kiss_fftr.c
)

# ══════════════════════════════════════════════════════════════════════════════
#  Server (uændret layout: ServerApp/ + Shared/)
# ══════════════════════════════════════════════════════════════════════════════
file(GLOB_RECURSE SERVER_SRCS CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/ServerApp/*.cpp"
    "${CMAKE_SOURCE_DIR}/Shared/*.cpp"
)

add_executable(server ${SERVER_SRCS})
target_include_directories(server PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(server PRIVATE
    nlohmann_json::nlohmann_json
    Threads::Threads
)

# ══════════════════════════════════════════════════════════════════════════════
#  Client (Qt6 lobby-GUI + NotePi DSP-pipeline, ingen hardware-sensor)
# ══════════════════════════════════════════════════════════════════════════════
# CLIENT_SRCS samler hele ClientApp/ rekursivt (inkluderer gui/) + Shared/.
file(GLOB_RECURSE CLIENT_SRCS CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/ClientApp/*.cpp"
    "${CMAKE_SOURCE_DIR}/Shared/*.cpp"
)

add_executable(client ${CLIENT_SRCS} ${KISSFFT_SRCS})
target_include_directories(client PRIVATE
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/ClientApp/NotePi
    ${CMAKE_SOURCE_DIR}/ClientApp/NotePi/control
    ${CMAKE_SOURCE_DIR}/ThirdParty/kissfft
    ${CMAKE_SOURCE_DIR}/ClientApp/gui
)
target_link_libraries(client PRIVATE
    nlohmann_json::nlohmann_json
    Threads::Threads
    PkgConfig::ALSA
    m
    Qt6::Core
    Qt6::Widgets
)
if(Qt6VirtualKeyboard_FOUND)
    target_link_libraries(client PRIVATE Qt6::VirtualKeyboard)
endif()

# ══════════════════════════════════════════════════════════════════════════════
#  Hardware-klient (BMI160 over SPI på Raspberry Pi) — opt-in
# ══════════════════════════════════════════════════════════════════════════════
# Bygges kun når -DBUILD_HARDWARE_CLIENT=ON er sat:
#   cmake -B build -DBUILD_HARDWARE_CLIENT=ON
option(BUILD_HARDWARE_CLIENT "Byg client-hw med BMI160/SPI support" OFF)

if(BUILD_HARDWARE_CLIENT)
    add_executable(client-hw ${CLIENT_SRCS} ${KISSFFT_SRCS})
    target_include_directories(client-hw PRIVATE
        ${CMAKE_SOURCE_DIR}
        ${CMAKE_SOURCE_DIR}/ClientApp/NotePi
        ${CMAKE_SOURCE_DIR}/ClientApp/NotePi/control
        ${CMAKE_SOURCE_DIR}/ThirdParty/kissfft
        ${CMAKE_SOURCE_DIR}/ClientApp/gui
    )
    target_compile_definitions(client-hw PRIVATE USE_HARDWARE_SENSOR)
    target_link_libraries(client-hw PRIVATE
        nlohmann_json::nlohmann_json
        Threads::Threads
        PkgConfig::ALSA
        m
        Qt6::Core
        Qt6::Widgets
    )
    if(Qt6VirtualKeyboard_FOUND)
        target_link_libraries(client-hw PRIVATE Qt6::VirtualKeyboard)
    endif()
endif()
