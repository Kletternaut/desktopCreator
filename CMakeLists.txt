cmake_minimum_required(VERSION 3.16)

project(desktopCreator
    VERSION 1.0.0
    DESCRIPTION ".desktopCreator – grafischer .desktop-Datei-Editor"
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Qt5 automoc / resources / ui
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# Raspberry Pi 5 Optimierungen (armv8-a, Cortex-A76)
if(CMAKE_SYSTEM_PROCESSOR MATCHES "aarch64|arm")
    add_compile_options(-march=armv8-a+crc -mtune=cortex-a76)
endif()

# Warnungen
add_compile_options(-Wall -Wextra -Wpedantic)

# Qt5-Module
find_package(Qt5 REQUIRED COMPONENTS
    Core
    Widgets
    Gui
    Svg
    Xml
    LinguistTools
)

# HiDPI
add_compile_definitions(QT_DEPRECATED_WARNINGS)

# Quelldateien
set(SOURCES
    src/main.cpp
    src/mainwindow.cpp
    src/aboutdialog.cpp
    src/desktopentry.cpp
    src/desktopeditor.cpp
    src/iconconverter.cpp
    src/categorymodel.cpp
    src/previewwidget.cpp
)

set(HEADERS
    src/mainwindow.h
    src/aboutdialog.h
    src/desktopentry.h
    src/desktopeditor.h
    src/iconconverter.h
    src/categorymodel.h
    src/previewwidget.h
)

# Übersetzungen
set(TS_FILES
    resources/translations/app_de.ts
    resources/translations/app_en.ts
)

qt5_add_translation(QM_FILES ${TS_FILES})

# Executable
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    resources/app.qrc
    ${QM_FILES}
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::Svg
    Qt5::Xml
)

target_include_directories(${PROJECT_NAME} PRIVATE src)

# Install-Ziele
include(GNUInstallDirs)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

install(FILES packaging/desktopCreator.desktop
    DESTINATION ${CMAKE_INSTALL_DATADIR}/applications
)

install(FILES resources/icons/app-icon.svg
    DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps
    RENAME desktopCreator.svg
)

install(FILES ${QM_FILES}
    DESTINATION ${CMAKE_INSTALL_DATADIR}/desktopCreator/translations
)

# CPack – .deb Paket
set(CPACK_GENERATOR "DEB")
set(CPACK_PACKAGE_NAME "desktopCreator")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VENDOR "Kletternaut")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "${PROJECT_DESCRIPTION}")
set(CPACK_PACKAGE_CONTACT "maintainer@example.com")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "arm64")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libqt5widgets5 (>= 5.15), libqt5svg5 (>= 5.15), libqt5xml5 (>= 5.15)")
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_PRIORITY "optional")

include(CPack)

# Tests
enable_testing()
add_subdirectory(tests)
