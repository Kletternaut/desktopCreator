#include "mainwindow.h"
#include "desktopentry.h"

#include <QApplication>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QStatusBar>
#include <QToolBar>
#include <QProcess>
#include <QSettings>
#include <QDir>
#include <QLabel>
#include <QScrollArea>
#include <QSplitter>
#include <QAction>
#include <QIcon>
#include <QFile>
#include <QFont>
#include <QDebug>

// ── Konstruktor ──────────────────────────────────────────────────────────────

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_entry(new DesktopEntry(this))
{
    setAttribute(Qt::WA_DeleteOnClose, false);

    setupActions();
    setupUi();
    setupToolBar();
    setupStatusBar();
    loadSettings();

    // Initialen Eintrag mit Editor und Vorschau verbinden
    m_editor->setEntry(m_entry);
    m_preview->setEntry(m_entry);

    connect(m_entry, &DesktopEntry::entryChanged, this, &MainWindow::onEntryChanged);

    setMinimumSize(900, 600);
    updateWindowTitle();
    updateStatusBar();
}

// ── UI-Aufbau ─────────────────────────────────────────────────────────────────

void MainWindow::setupUi()
{
    // Haupt-Splitter (drei Spalten)
    m_splitter = new QSplitter(Qt::Horizontal, this);

    // ── Linke Spalte: Zuletzt geöffnete Dateien ──────────────────────────────
    auto *leftWidget  = new QWidget(m_splitter);
    auto *leftLayout  = new QVBoxLayout(leftWidget);
    leftLayout->setContentsMargins(4, 4, 4, 4);
    leftLayout->setSpacing(4);

    auto *recentLabel = new QLabel(tr("Zuletzt geöffnet:"), leftWidget);
    QFont f = recentLabel->font();
    f.setBold(true);
    recentLabel->setFont(f);

    m_recentList = new QListWidget(leftWidget);
    m_recentList->setMinimumWidth(160);
    m_recentList->setMaximumWidth(280);
    connect(m_recentList, &QListWidget::itemDoubleClicked,
            this, &MainWindow::onRecentFileClicked);

    leftLayout->addWidget(recentLabel);
    leftLayout->addWidget(m_recentList, 1);
    m_splitter->addWidget(leftWidget);

    // ── Mittlere Spalte: Editor ───────────────────────────────────────────────
    m_editor = new DesktopEditor(m_splitter);
    m_splitter->addWidget(m_editor);

    // ── Rechte Spalte: Vorschau ───────────────────────────────────────────────
    m_preview = new PreviewWidget(m_splitter);
    m_preview->setMinimumWidth(220);
    m_preview->setMaximumWidth(360);
    m_splitter->addWidget(m_preview);

    m_splitter->setStretchFactor(0, 0);
    m_splitter->setStretchFactor(1, 1);
    m_splitter->setStretchFactor(2, 0);
    m_splitter->setSizes({ 200, 600, 260 });

    setCentralWidget(m_splitter);
}

void MainWindow::setupActions()
{
    auto appIcon = [](const QString& name) -> QIcon {
        const QString path = QStringLiteral(":/icons/") + name + QStringLiteral(".svg");
        if (QFile::exists(path)) return QIcon(path);
        return QIcon::fromTheme(name);
    };

    m_actNew    = new QAction(appIcon("document-new"),    tr("Neu"),            this);
    m_actOpen   = new QAction(appIcon("document-open"),   tr("Öffnen"),         this);
    m_actSave   = new QAction(appIcon("document-save"),   tr("Speichern"),      this);
    m_actSaveAs = new QAction(appIcon("document-save-as"),tr("Speichern unter"),this);
    m_actInstall = new QAction(appIcon("system-software-install"),
                               tr("Für Benutzer installieren"), this);
    m_actInstallSystem = new QAction(appIcon("emblem-system"),
                               tr("Systemweit installieren"), this);
    m_actAbout = new QAction(appIcon("help-about"), tr("Über"), this);

    m_actNew->setShortcut(QKeySequence::New);
    m_actOpen->setShortcut(QKeySequence::Open);
    m_actSave->setShortcut(QKeySequence::Save);
    m_actSaveAs->setShortcut(QKeySequence::SaveAs);

    connect(m_actNew,          &QAction::triggered, this, &MainWindow::onNew);
    connect(m_actOpen,         &QAction::triggered, this, &MainWindow::onOpen);
    connect(m_actSave,         &QAction::triggered, this, &MainWindow::onSave);
    connect(m_actSaveAs,       &QAction::triggered, this, &MainWindow::onSaveAs);
    connect(m_actInstall,      &QAction::triggered, this, &MainWindow::onInstallUser);
    connect(m_actInstallSystem,&QAction::triggered, this, &MainWindow::onInstallSystem);
    connect(m_actAbout,        &QAction::triggered, this, &MainWindow::onAbout);
}

void MainWindow::setupToolBar()
{
    QToolBar *tb = addToolBar(tr("Werkzeuge"));
    tb->setObjectName("mainToolBar");
    tb->setMovable(false);
    tb->setIconSize(QSize(32, 32));
    tb->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

    // Mindestgröße 44px für Touch-Bedienung (Raspberry Pi Touchscreen)
    tb->setMinimumHeight(56);

    tb->addAction(m_actNew);
    tb->addAction(m_actOpen);
    tb->addSeparator();
    tb->addAction(m_actSave);
    tb->addAction(m_actSaveAs);
    tb->addSeparator();
    tb->addAction(m_actInstall);
    tb->addAction(m_actInstallSystem);
    tb->addSeparator();
    tb->addAction(m_actAbout);
}

void MainWindow::setupStatusBar()
{
    statusBar()->showMessage(tr("Bereit"));
}

// ── Dateioperationen ──────────────────────────────────────────────────────────

void MainWindow::onNew()
{
    if (!maybeSave())
        return;
    m_entry->reset();
    m_editor->loadFromEntry();
    setCurrentFile(QString());
    m_modified = false;
    updateWindowTitle();
    updateStatusBar();
}

void MainWindow::onOpen()
{
    if (!maybeSave())
        return;

    const QString path = QFileDialog::getOpenFileName(
        this,
        tr("Desktop-Eintrag öffnen"),
        QDir::homePath() + "/.local/share/applications",
        tr("Desktop-Eintrag-Dateien (*.desktop);;Alle Dateien (*)")
    );
    if (!path.isEmpty())
        openFile(path);
}

void MainWindow::openFile(const QString &path)
{
    if (!m_entry->loadFromFile(path)) {
        QMessageBox::critical(this, tr("Fehler"),
            tr("Datei konnte nicht geöffnet werden:\n%1").arg(path));
        return;
    }
    m_editor->loadFromEntry();
    setCurrentFile(path);
    addRecentFile(path);
    m_modified = false;
    updateWindowTitle();
    updateStatusBar();
}

void MainWindow::onSave()
{
    if (m_currentFile.isEmpty()) {
        onSaveAs();
        return;
    }
    saveFile(m_currentFile);
}

void MainWindow::onSaveAs()
{
    const QString path = QFileDialog::getSaveFileName(
        this,
        tr("Desktop-Eintrag speichern unter"),
        m_currentFile.isEmpty()
            ? QDir::homePath() + "/" + m_entry->name().toLower().replace(' ', '-') + ".desktop"
            : m_currentFile,
        tr("Desktop-Eintrag-Dateien (*.desktop)")
    );
    if (!path.isEmpty())
        saveFile(path);
}

bool MainWindow::saveFile(const QString &path)
{
    // Zuerst UI → Entry synchronisieren
    m_editor->applyToEntry();

    // Validierung
    const auto result = m_entry->validate();
    if (!result.errors.isEmpty()) {
        QMessageBox::critical(this, tr("Validierungsfehler"),
            tr("Folgende Fehler verhindern das Speichern:\n\n• ") +
            result.errors.join("\n• "));
        return false;
    }
    if (!result.warnings.isEmpty()) {
        const int btn = QMessageBox::warning(this, tr("Warnungen"),
            tr("Folgende Warnungen wurden gefunden:\n\n• ") +
            result.warnings.join("\n• ") +
            tr("\n\nTrotzdem speichern?"),
            QMessageBox::Save | QMessageBox::Cancel);
        if (btn != QMessageBox::Save)
            return false;
    }

    if (!m_entry->saveToFile(path)) {
        QMessageBox::critical(this, tr("Fehler"),
            tr("Datei konnte nicht geschrieben werden:\n%1").arg(path));
        return false;
    }

    setCurrentFile(path);
    addRecentFile(path);
    m_modified = false;
    updateWindowTitle();
    statusBar()->showMessage(tr("Datei gespeichert: %1").arg(path), 4000);
    return true;
}

void MainWindow::onInstallUser()
{
    m_editor->applyToEntry();
    const auto result = m_entry->validate();
    if (!result.errors.isEmpty()) {
        QMessageBox::critical(this, tr("Validierungsfehler"),
            result.errors.join("\n"));
        return;
    }

    const QString name = m_entry->name().isEmpty()
        ? "unnamed" : m_entry->name().toLower().replace(' ', '-');
    const QString dir  = QDir::homePath() + "/.local/share/applications";
    QDir().mkpath(dir);
    const QString dest = dir + "/" + name + ".desktop";

    if (!m_entry->saveToFile(dest)) {
        QMessageBox::critical(this, tr("Fehler"),
            tr("Installation fehlgeschlagen:\n%1").arg(dest));
        return;
    }
    setCurrentFile(dest);
    addRecentFile(dest);
    m_modified = false;
    updateWindowTitle();
    QMessageBox::information(this, tr("Installiert"),
        tr("Desktop-Eintrag installiert nach:\n%1").arg(dest));
}

void MainWindow::onInstallSystem()
{
    m_editor->applyToEntry();
    const auto result = m_entry->validate();
    if (!result.errors.isEmpty()) {
        QMessageBox::critical(this, tr("Validierungsfehler"),
            result.errors.join("\n"));
        return;
    }

    const QString name = m_entry->name().isEmpty()
        ? "unnamed" : m_entry->name().toLower().replace(' ', '-');
    const QString dest = QStringLiteral("/usr/share/applications/%1.desktop").arg(name);

    // Temporäre Datei erstellen und dann per sudo kopieren
    const QString tmpPath = QDir::tempPath() + "/" + name + ".desktop";
    if (!m_entry->saveToFile(tmpPath)) {
        QMessageBox::critical(this, tr("Fehler"), tr("Temporäre Datei konnte nicht erstellt werden."));
        return;
    }

    QProcess proc;
    proc.start("pkexec", { "cp", tmpPath, dest });
    proc.waitForFinished(30000);
    QFile::remove(tmpPath);

    if (proc.exitCode() != 0) {
        QMessageBox::critical(this, tr("Fehler"),
            tr("Systemweite Installation fehlgeschlagen:\n%1").arg(
                QString::fromLocal8Bit(proc.readAllStandardError())));
        return;
    }
    QMessageBox::information(this, tr("Installiert"),
        tr("Desktop-Eintrag systemweit installiert:\n%1").arg(dest));
}

void MainWindow::onRecentFileClicked(QListWidgetItem *item)
{
    if (!item)
        return;
    const QString path = item->data(Qt::UserRole).toString();
    if (!path.isEmpty() && maybeSave())
        openFile(path);
}

void MainWindow::onEntryChanged()
{
    m_modified = true;
    updateWindowTitle();
    updateStatusBar();
}

void MainWindow::onAbout()
{
    AboutDialog dlg(this);
    dlg.exec();
}

// ── Hilfsmethoden ─────────────────────────────────────────────────────────────

bool MainWindow::maybeSave()
{
    if (!m_modified)
        return true;

    const int ret = QMessageBox::warning(
        this,
        tr("Ungespeicherte Änderungen"),
        tr("Der aktuelle Desktop-Eintrag wurde geändert.\nMöchten Sie die Änderungen speichern?"),
        QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel
    );

    if (ret == QMessageBox::Save)
        return onSave(), !m_modified;
    if (ret == QMessageBox::Cancel)
        return false;
    return true; // Discard
}

void MainWindow::setCurrentFile(const QString &path)
{
    m_currentFile = path;
    updateWindowTitle();
}

void MainWindow::addRecentFile(const QString &path)
{
    m_recentFiles.removeAll(path);
    m_recentFiles.prepend(path);
    while (m_recentFiles.size() > kMaxRecent)
        m_recentFiles.removeLast();

    m_recentList->clear();
    for (const QString &f : qAsConst(m_recentFiles)) {
        auto *item = new QListWidgetItem(QFileInfo(f).fileName(), m_recentList);
        item->setData(Qt::UserRole, f);
        item->setToolTip(f);
    }
    saveSettings();
}

void MainWindow::updateWindowTitle()
{
    QString title = tr(".desktopCreator");
    if (!m_currentFile.isEmpty())
        title += " – " + QFileInfo(m_currentFile).fileName();
    else if (!m_entry->name().isEmpty())
        title += " – " + m_entry->name();
    if (m_modified)
        title += " *";
    setWindowTitle(title);
}

void MainWindow::updateStatusBar()
{
    if (!m_entry)
        return;

    // Beim frischen, noch unberührten neuen Eintrag keine Fehler anzeigen
    if (!m_modified && m_currentFile.isEmpty()) {
        statusBar()->showMessage(tr("Neu"));
        return;
    }

    const auto result = m_entry->validate();
    QString msg;
    if (!m_currentFile.isEmpty())
        msg += m_currentFile + "  ";

    if (result.valid && result.warnings.isEmpty())
        msg += tr("✓ Gültig");
    else if (!result.valid)
        msg += tr("⚠ %1 Fehler").arg(result.errors.size());
    else
        msg += tr("△ %1 Warnung(en)").arg(result.warnings.size());

    statusBar()->showMessage(msg);
}

// ── Einstellungen ─────────────────────────────────────────────────────────────

void MainWindow::loadSettings()
{
    QSettings s("Kletternaut", "desktopCreator");
    m_recentFiles = s.value("recentFiles").toStringList();
    restoreGeometry(s.value("geometry").toByteArray());
    restoreState(s.value("windowState").toByteArray());

    // Zuletzt-geöffnete-Liste aufbauen
    m_recentList->clear();
    for (const QString &f : qAsConst(m_recentFiles)) {
        auto *item = new QListWidgetItem(QFileInfo(f).fileName(), m_recentList);
        item->setData(Qt::UserRole, f);
        item->setToolTip(f);
    }
}

void MainWindow::saveSettings()
{
    QSettings s("Kletternaut", "desktopCreator");
    s.setValue("recentFiles",  m_recentFiles);
    s.setValue("geometry",     saveGeometry());
    s.setValue("windowState",  saveState());
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (!maybeSave()) {
        event->ignore();
        return;
    }
    saveSettings();
    event->accept();
}
