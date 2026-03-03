# .desktopCreator

**Version 1.0.0** – Grafischer `.desktop`-Datei-Editor für den Raspberry Pi

**.desktopCreator** ist eine Qt5-Anwendung zum Erstellen, Bearbeiten und Verwalten von `.desktop`-Dateien nach dem [freedesktop.org Desktop Entry Specification 1.5](https://specifications.freedesktop.org/desktop-entry-spec/latest/)-Standard – mit integrierter Icon-Konvertierungs-Pipeline für alle hicolor-Standardgrößen.

| | |
|---|---|
| **Autor** | [Kletternaut](https://github.com/Kletternaut) |
| **Lizenz** | MIT |
| **Plattform** | Raspberry Pi OS Bookworm (aarch64) |
| **Qt** | Qt 5.15 |

---

## Features

- **Vollständiger Desktop Entry Spec 1.5 Editor**
  - Alle Pflicht- und optionalen Felder
  - Sprachvarianten (Name[de], Comment[en], …)
  - Desktop Actions (Sub-Aktionen mit eigenem Exec/Name/Icon)
  - Benutzerdefinierte X-* Felder

- **Icon-Konverter**
  - Eingabe: PNG, JPG, BMP, TIFF, SVG, XPM
  - Ausgabe: PNG in allen 10 hicolor-Standardgrößen (16×16 bis 512×512)
  - Ausgabe: skalierbares SVG (scalable/apps/)
  - Drag & Drop-Unterstützung
  - Installation mit einem Klick nach `~/.local/share/icons/hicolor/`

- **Live-Vorschau**
  - Simuliertes Desktop-Icon (64px Icon + App-Name)
  - Rohtext der erzeugten `.desktop`-Datei (Echtzeit-Aktualisierung)

- **Datei-Verwaltung**
  - Neu, Öffnen, Speichern, Speichern unter
  - Installation für aktuellen Benutzer (`~/.local/share/applications/`)
  - Systemweite Installation (`/usr/share/applications/`, benötigt sudo)
  - Validierung mit Fehlermeldungen und Warnungen

- **Touch-freundlich** – optimiert für Raspberry Pi Touchscreen (mind. 44px Buttons)
- **Mehrsprachig** – Deutsch und Englisch

---

## Voraussetzungen

### Raspberry Pi OS Bookworm (Debian 12) / ARM64

```bash
sudo apt update
sudo apt install \
    qtbase5-dev \
    qtsvg5-dev \
    libqt5svg5-dev \
    libqt5xml5-dev \
    qttools5-dev \
    qttools5-dev-tools \
    cmake \
    build-essential
```

---

## Build-Anleitung

### Standard-Build

```bash
git clone https://github.com/Kletternaut/desktopCreator.git
cd desktopCreator

mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j$(nproc)
```

### Starten (ohne Installation)

```bash
./desktopCreator
```

### Installation

```bash
sudo make install
```

Installiert:
- `/usr/local/bin/desktopCreator` – Executable
- `/usr/local/share/applications/desktopCreator.desktop` – App-Starter
- `/usr/local/share/icons/hicolor/scalable/apps/desktopCreator.svg` – App-Icon
- `/usr/local/share/desktopCreator/translations/` – Übersetzungen

---

## .deb-Paket bauen

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
make -j4
cpack -G DEB
```

Das erzeugte Paket (`desktopCreator-1.0.0-arm64.deb`) kann mit folgendem Befehl installiert werden:

```bash
sudo dpkg -i desktopCreator-1.0.0-arm64.deb
sudo apt-get install -f   # Abhängigkeiten auflösen
```

---

## Tests ausführen

```bash
cd build
ctest --output-on-failure
```

---

## Projektstruktur

```
.desktopCreator/
├── CMakeLists.txt              Haupt-Build-Datei
├── LICENSE                     MIT-Lizenz
├── README.md                   Diese Datei
├── src/
│   ├── main.cpp                Programmeinstieg
│   ├── mainwindow.h/.cpp       Hauptfenster (3-Spalten-Layout)
│   ├── aboutdialog.h/.cpp      Über-Dialog
│   ├── desktopentry.h/.cpp     Datenmodell (Desktop Entry Spec 1.5)
│   ├── desktopeditor.h/.cpp    Editor-Widget (Tab-Widget)
│   ├── iconconverter.h/.cpp    Icon-Konvertierungs-Pipeline
│   ├── categorymodel.h/.cpp    Kategorien-Listenmodell
│   └── previewwidget.h/.cpp    Live-Vorschau-Widget
├── resources/
│   ├── app.qrc                 Qt-Ressourcen
│   ├── icons/                  Toolbar-Icons (SVG)
│   ├── images/                 UI-Bilder
│   └── translations/           Übersetzungsdateien (.ts)
├── tests/
│   ├── CMakeLists.txt
│   └── test_desktopentry.cpp   Qt Test Unit-Tests (23 Tests)
└── packaging/
    ├── desktopCreator.desktop  App-Starter
    └── debian/                 Debian-Packaging-Dateien
```

---

## Architektur

```
QApplication
└── MainWindow (QMainWindow)
    ├── QToolBar          – Neu/Öffnen/Speichern/Installieren
    ├── QSplitter
    │   ├── QListWidget   – Zuletzt geöffnete Dateien
    │   ├── DesktopEditor – QTabWidget
    │   │   ├── Tab: Allgemein    (Type, Name, Exec, Icon, Sprachvarianten)
    │   │   ├── Tab: Ausführung   (TryExec, Path, Terminal, Actions, MimeType)
    │   │   ├── Tab: Darstellung  (GenericName, Comment, Keywords, Kategorien)
    │   │   ├── Tab: Icon-Konverter (IconConverter)
    │   │   └── Tab: Erweitert    (X-* Felder)
    │   └── PreviewWidget – Icon-Vorschau + Rohtext
    └── QStatusBar        – Dateiname + Validierungsstatus
        
DesktopEntry (QObject)
└── Datenmodell, load/save/validate, Q_PROPERTY für alle Felder
    Signal: entryChanged() → DesktopEditor + PreviewWidget aktualisieren
```

---

## Lizenz

MIT License – Copyright © 2026 [Kletternaut](https://github.com/Kletternaut) – siehe [LICENSE](LICENSE)

---

## Zielplattform

- **Raspberry Pi 5** (Raspberry Pi OS Bookworm, ARM64)
- Qt 5.15.x (Standard in RPi OS Bookworm)
- Desktop-Umgebung: LXDE/Openbox (Standard-RPi-Desktop)
- Compiliert auch auf x86_64 Linux für Entwicklung/Test
