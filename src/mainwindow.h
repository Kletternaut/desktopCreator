#pragma once

#include <QMainWindow>
#include <QListWidget>
#include <QLabel>
#include <QStatusBar>
#include <QSplitter>
#include <QAction>
#include <QToolBar>

#include "desktopentry.h"
#include "desktopeditor.h"
#include "previewwidget.h"
#include "aboutdialog.h"

/**
 * @brief Hauptfenster der RPi Desktop Creator Applikation.
 *
 * Layout (drei Spalten nebeneinander via QSplitter):
 *  - Links:  Toolbar + Liste zuletzt geöffneter Dateien
 *  - Mitte:  DesktopEditor (Tab-Widget)
 *  - Rechts: PreviewWidget (simuliertes Icon + Rohtext)
 *
 * Unterer Rand: QStatusBar mit Dateiname und Validierungsstatus.
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override = default;

protected:
    void closeEvent(QCloseEvent *event) override;

public slots:
    /** @brief Öffnet eine .desktop-Datei direkt (z.B. aus Kommandozeile). */
    void openFile(const QString &path);

private slots:
    void onNew();
    void onOpen();
    void onSave();
    void onSaveAs();
    void onInstallUser();
    void onInstallSystem();
    void onAbout();
    void onRecentFileClicked(QListWidgetItem *item);
    void onEntryChanged();

private:
    void setupUi();
    void setupActions();
    void setupToolBar();
    void setupStatusBar();
    void setupRecentFiles();

    void loadSettings();
    void saveSettings();


    bool saveFile(const QString &path);
    bool maybeSave();
    void setCurrentFile(const QString &path);
    void addRecentFile(const QString &path);
    void updateWindowTitle();
    void updateStatusBar();

    // ── Menü-/Toolbar-Aktionen ────────────────────────────────────────────────
    QAction *m_actNew     = nullptr;
    QAction *m_actOpen    = nullptr;
    QAction *m_actSave    = nullptr;
    QAction *m_actSaveAs  = nullptr;
    QAction *m_actInstall = nullptr;
    QAction *m_actInstallSystem = nullptr;
    QAction *m_actAbout = nullptr;

    // ── Haupt-Widgets ─────────────────────────────────────────────────────────
    QSplitter    *m_splitter       = nullptr;
    QListWidget  *m_recentList     = nullptr;
    DesktopEditor *m_editor        = nullptr;
    PreviewWidget *m_preview       = nullptr;

    // ── Daten ─────────────────────────────────────────────────────────────────
    DesktopEntry *m_entry          = nullptr;
    QString       m_currentFile;
    bool          m_modified       = false;

    static constexpr int kMaxRecent = 10;
    QStringList m_recentFiles;
};
