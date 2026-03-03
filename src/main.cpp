#include <QApplication>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QDir>
#include <QFile>
#include <QTimer>
#include <QStandardPaths>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    // HiDPI-Unterstützung – muss vor QApplication gesetzt werden
    QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
    QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    QApplication app(argc, argv);
    app.setApplicationName(".desktopCreator");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Kletternaut");
    app.setWindowIcon(QIcon(":/icons/app-icon.svg"));

    // Qt-eigene Übersetzung laden
    QTranslator qtTranslator;
    const QString qtTransPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    if (qtTranslator.load(QLocale::system(), "qt", "_", qtTransPath))
        app.installTranslator(&qtTranslator);

    // App-Übersetzung laden
    QTranslator appTranslator;
    const QString appTransPath = QStandardPaths::locate(
        QStandardPaths::DataLocation,
        "translations",
        QStandardPaths::LocateDirectory);
    if (appTranslator.load(QLocale::system(), "app", "_", appTransPath))
        app.installTranslator(&appTranslator);

    MainWindow w;
    w.show();

    // Optional: Datei aus Befehlszeile öffnen
    const QStringList args = app.arguments();
    if (args.size() >= 2) {
        const QString filePath = args.at(1);
        if (QFile::exists(filePath)) {
            QTimer::singleShot(0, [&w, filePath]() {
                w.openFile(filePath);
            });
        }
    }

    return app.exec();
}
