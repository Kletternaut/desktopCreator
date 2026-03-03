#include "desktopeditor.h"
#include "desktopentry.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QScrollArea>
#include <QPushButton>
#include <QFileDialog>
#include <QHeaderView>
#include <QListView>
#include <QFrame>
#include <QSvgRenderer>
#include <QPainter>
#include <QDialog>
#include <QDialogButtonBox>
#include <QInputDialog>

// Bekannte Desktop-Umgebungen für OnlyShowIn / NotShowIn
const QStringList DesktopEditor::kDesktopEnvironments = {
    "GNOME", "KDE", "XFCE", "LXDE", "LXQt", "MATE", "Cinnamon",
    "Unity", "Openbox", "Enlightenment", "Pantheon", "Budgie",
    "DDE", "ROX", "Old"
};

// ── Konstruktor ──────────────────────────────────────────────────────────────

DesktopEditor::DesktopEditor(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

// ── UI-Aufbau ────────────────────────────────────────────────────────────────

void DesktopEditor::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);

    m_tabs = new QTabWidget(this);
    m_tabs->addTab(createGeneralTab(),       tr("Allgemein"));
    m_tabs->addTab(createExecutionTab(),     tr("Ausführung"));
    m_tabs->addTab(createDisplayTab(),       tr("Darstellung"));
    m_tabs->addTab(createIconConverterTab(), tr("Icon-Konverter"));
    m_tabs->addTab(createAdvancedTab(),      tr("Erweitert"));

    mainLayout->addWidget(m_tabs);
}

// ── Tab: Allgemein ────────────────────────────────────────────────────────────

QWidget *DesktopEditor::createGeneralTab()
{
    auto *scroll  = new QScrollArea;
    auto *content = new QWidget;
    auto *layout  = new QFormLayout(content);
    layout->setSpacing(8);
    layout->setContentsMargins(12, 12, 12, 12);

    // Typ
    m_typeCombo = new QComboBox(content);
    m_typeCombo->addItem(tr("Anwendung"),  static_cast<int>(DesktopEntry::Type::Application));
    m_typeCombo->addItem(tr("Link"),       static_cast<int>(DesktopEntry::Type::Link));
    m_typeCombo->addItem(tr("Verzeichnis"),static_cast<int>(DesktopEntry::Type::Directory));
    layout->addRow(tr("Typ:"), m_typeCombo);

    // Name
    m_nameEdit = makeLineEdit(tr("z.B. Meine Anwendung"));
    layout->addRow(tr("Name:"), m_nameEdit);

    // Exec + Browse-Button
    auto *execRow = new QWidget(content);
    auto *execLayout = new QHBoxLayout(execRow);
    execLayout->setContentsMargins(0, 0, 0, 0);
    m_execEdit = makeLineEdit(tr("Programmaufruf, z.B. /usr/bin/myapp %F"));
    auto *execBrowse = new QPushButton(tr("…"), execRow);
    execBrowse->setFixedSize(44, 44);
    execBrowse->setToolTip(tr("Programm auswählen"));
    connect(execBrowse, &QPushButton::clicked, this, &DesktopEditor::onBrowseExec);
    execLayout->addWidget(m_execEdit);
    execLayout->addWidget(execBrowse);
    layout->addRow(tr("Exec:"), execRow);

    // Icon + Browse-Button + Vorschau
    auto *iconRow = new QWidget(content);
    auto *iconLayout = new QHBoxLayout(iconRow);
    iconLayout->setContentsMargins(0, 0, 0, 0);
    m_iconEdit = makeLineEdit(tr("Icon-Name oder Pfad"));
    auto *iconBrowse = new QPushButton(tr("…"), iconRow);
    iconBrowse->setFixedSize(44, 44);
    iconBrowse->setToolTip(tr("Icon auswählen"));
    connect(iconBrowse, &QPushButton::clicked, this, &DesktopEditor::onBrowseIcon);
    m_iconPreview = new QLabel(iconRow);
    m_iconPreview->setFixedSize(64, 64);
    m_iconPreview->setFrameShape(QFrame::StyledPanel);
    m_iconPreview->setAlignment(Qt::AlignCenter);
    connect(m_iconEdit, &QLineEdit::textChanged, this, [this](const QString &path) {
        QPixmap pix;
        if (!path.isEmpty()) {
            if (path.toLower().endsWith(".svg")) {
                QSvgRenderer r(path);
                if (r.isValid()) {
                    pix = QPixmap(64, 64);
                    pix.fill(Qt::transparent);
                    QPainter p(&pix);
                    r.render(&p);
                }
            } else {
                pix = QPixmap(path);
                if (pix.isNull())
                    pix = QIcon::fromTheme(path).pixmap(64, 64);
            }
        }
        if (!pix.isNull())
            m_iconPreview->setPixmap(pix.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        else
            m_iconPreview->clear();
    });
    iconLayout->addWidget(m_iconEdit);
    iconLayout->addWidget(iconBrowse);
    iconLayout->addWidget(m_iconPreview);
    layout->addRow(tr("Icon:"), iconRow);

    // Sprachvarianten für Name
    auto *locGroup = new QGroupBox(tr("Name – Sprachvarianten"), content);
    auto *locLayout = new QVBoxLayout(locGroup);
    m_nameLocTable = new QTableWidget(0, 2, locGroup);
    m_nameLocTable->setHorizontalHeaderLabels({ tr("Sprache"), tr("Name") });
    m_nameLocTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_nameLocTable->setMinimumHeight(100);
    auto *addLocBtn = new QPushButton(tr("Sprachvariante hinzufügen"), locGroup);
    addLocBtn->setMinimumHeight(44);
    connect(addLocBtn, &QPushButton::clicked, this, &DesktopEditor::onAddLanguageVariant);
    locLayout->addWidget(m_nameLocTable);
    locLayout->addWidget(addLocBtn);
    layout->addRow(locGroup);

    scroll->setWidget(content);
    scroll->setWidgetResizable(true);
    return scroll;
}

// ── Tab: Ausführung ───────────────────────────────────────────────────────────

QWidget *DesktopEditor::createExecutionTab()
{
    auto *scroll  = new QScrollArea;
    auto *content = new QWidget;
    auto *layout  = new QFormLayout(content);
    layout->setSpacing(8);
    layout->setContentsMargins(12, 12, 12, 12);

    // TryExec
    auto *tryRow = new QWidget;
    auto *tryLayout = new QHBoxLayout(tryRow);
    tryLayout->setContentsMargins(0, 0, 0, 0);
    m_tryExecEdit = makeLineEdit(tr("Alternativer Test-Programmpfad"));
    auto *tryBrowse = new QPushButton(tr("…"), tryRow);
    tryBrowse->setFixedSize(44, 44);
    connect(tryBrowse, &QPushButton::clicked, this, &DesktopEditor::onBrowseTryExec);
    tryLayout->addWidget(m_tryExecEdit);
    tryLayout->addWidget(tryBrowse);
    layout->addRow(tr("TryExec:"), tryRow);

    // Path (Arbeitsverzeichnis)
    auto *pathRow = new QWidget;
    auto *pathLayout = new QHBoxLayout(pathRow);
    pathLayout->setContentsMargins(0, 0, 0, 0);
    m_pathEdit = makeLineEdit(tr("Arbeitsverzeichnis"));
    auto *pathBrowse = new QPushButton(tr("…"), pathRow);
    pathBrowse->setFixedSize(44, 44);
    connect(pathBrowse, &QPushButton::clicked, this, &DesktopEditor::onBrowsePath);
    pathLayout->addWidget(m_pathEdit);
    pathLayout->addWidget(pathBrowse);
    layout->addRow(tr("Path:"), pathRow);

    // Terminal / StartupNotify / WMClass / URL
    m_terminalCheck = new QCheckBox(tr("Im Terminal ausführen"), content);
    layout->addRow(tr("Terminal:"), m_terminalCheck);

    m_startupCheck = new QCheckBox(tr("Startbenachrichtigung aktivieren"), content);
    layout->addRow(tr("StartupNotify:"), m_startupCheck);

    m_wmClassEdit = makeLineEdit(tr("z.B. myapp.MyApp"));
    layout->addRow(tr("StartupWMClass:"), m_wmClassEdit);

    m_urlEdit = makeLineEdit(tr("Nur bei Type=Link, z.B. https://example.com"));
    layout->addRow(tr("URL:"), m_urlEdit);

    m_mimeEdit = makeLineEdit(tr("z.B. text/plain;image/png;"));
    layout->addRow(tr("MimeType:"), m_mimeEdit);

    // Actions
    auto *actGroup = new QGroupBox(tr("Desktop Actions"), content);
    auto *actLayout = new QVBoxLayout(actGroup);
    m_actionsTable = new QTableWidget(0, 3, actGroup);
    m_actionsTable->setHorizontalHeaderLabels({ tr("ID"), tr("Name"), tr("Exec") });
    m_actionsTable->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_actionsTable->setMinimumHeight(120);
    auto *actBtnRow = new QWidget(actGroup);
    auto *actBtnLayout = new QHBoxLayout(actBtnRow);
    actBtnLayout->setContentsMargins(0, 0, 0, 0);
    auto *addActBtn = new QPushButton(tr("Hinzufügen"), actGroup);
    addActBtn->setMinimumHeight(44);
    auto *removeActBtn = new QPushButton(tr("Entfernen"), actGroup);
    removeActBtn->setMinimumHeight(44);
    connect(addActBtn,    &QPushButton::clicked, this, &DesktopEditor::onAddAction);
    connect(removeActBtn, &QPushButton::clicked, this, &DesktopEditor::onRemoveAction);
    actBtnLayout->addWidget(addActBtn);
    actBtnLayout->addWidget(removeActBtn);
    actBtnLayout->addStretch();
    actLayout->addWidget(m_actionsTable);
    actLayout->addWidget(actBtnRow);
    layout->addRow(actGroup);

    scroll->setWidget(content);
    scroll->setWidgetResizable(true);
    return scroll;
}

// ── Tab: Darstellung ──────────────────────────────────────────────────────────

QWidget *DesktopEditor::createDisplayTab()
{
    auto *scroll  = new QScrollArea;
    auto *content = new QWidget;
    auto *layout  = new QFormLayout(content);
    layout->setSpacing(8);
    layout->setContentsMargins(12, 12, 12, 12);

    m_genericNameEdit = makeLineEdit(tr("Generischer Name, z.B. Webbrowser"));
    layout->addRow(tr("GenericName:"), m_genericNameEdit);

    m_commentEdit = makeLineEdit(tr("Kurze Beschreibung"));
    layout->addRow(tr("Comment:"), m_commentEdit);

    m_keywordsEdit = makeLineEdit(tr("Schlüsselwörter, semikolon-getrennt"));
    layout->addRow(tr("Keywords:"), m_keywordsEdit);

    m_noDisplayCheck = new QCheckBox(tr("Nicht im Menü anzeigen"), content);
    layout->addRow(tr("NoDisplay:"), m_noDisplayCheck);

    m_hiddenCheck = new QCheckBox(tr("Versteckt (gelöscht markiert)"), content);
    layout->addRow(tr("Hidden:"), m_hiddenCheck);

    // OnlyShowIn
    auto *onlyGroup = new QGroupBox(tr("OnlyShowIn – Nur in diesen Umgebungen anzeigen"), content);
    auto *onlyLayout = new QVBoxLayout(onlyGroup);
    m_onlyShowInList = new QListWidget(onlyGroup);
    m_onlyShowInList->setSelectionMode(QAbstractItemView::MultiSelection);
    for (const QString &de : kDesktopEnvironments)
        m_onlyShowInList->addItem(de);
    m_onlyShowInList->setMaximumHeight(120);
    onlyLayout->addWidget(m_onlyShowInList);
    layout->addRow(onlyGroup);

    // NotShowIn
    auto *notGroup = new QGroupBox(tr("NotShowIn – Nicht in diesen Umgebungen anzeigen"), content);
    auto *notLayout = new QVBoxLayout(notGroup);
    m_notShowInList = new QListWidget(notGroup);
    m_notShowInList->setSelectionMode(QAbstractItemView::MultiSelection);
    for (const QString &de : kDesktopEnvironments)
        m_notShowInList->addItem(de);
    m_notShowInList->setMaximumHeight(120);
    notLayout->addWidget(m_notShowInList);
    layout->addRow(notGroup);

    // Kategorien
    auto *catGroup = new QGroupBox(tr("Kategorien"), content);
    auto *catLayout = new QVBoxLayout(catGroup);
    m_categoryModel = new CategoryModel(this);
    m_categoryView  = new QListView(catGroup);
    m_categoryView->setModel(m_categoryModel);
    m_categoryView->setMaximumHeight(200);
    catLayout->addWidget(m_categoryView);
    layout->addRow(catGroup);

    scroll->setWidget(content);
    scroll->setWidgetResizable(true);
    return scroll;
}

// ── Tab: Icon-Konverter ───────────────────────────────────────────────────────

QWidget *DesktopEditor::createIconConverterTab()
{
    auto *scroll = new QScrollArea;
    m_iconConverter = new IconConverter(this);
    connect(m_iconConverter, &IconConverter::iconInstalled,
            this, &DesktopEditor::onIconConverterInstalled);
    // Aktuellen Icon-Namen aus dem Eintrag an den Converter weitergeben
    connect(m_iconEdit, &QLineEdit::textChanged,
            m_iconConverter, &IconConverter::setPreferredName);
    scroll->setWidget(m_iconConverter);
    scroll->setWidgetResizable(true);
    return scroll;
}

// ── Tab: Erweitert ────────────────────────────────────────────────────────────

QWidget *DesktopEditor::createAdvancedTab()
{
    auto *content = new QWidget;
    auto *layout  = new QVBoxLayout(content);
    layout->setContentsMargins(12, 12, 12, 12);
    layout->setSpacing(8);

    auto *label = new QLabel(tr("Benutzerdefinierte X-* Felder (z.B. X-GNOME-Autostart)"), content);
    layout->addWidget(label);

    m_customTable = new QTableWidget(0, 2, content);
    m_customTable->setHorizontalHeaderLabels({ tr("Schlüssel"), tr("Wert") });
    m_customTable->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    m_customTable->setMinimumHeight(200);
    layout->addWidget(m_customTable, 1);

    auto *btnRow = new QWidget(content);
    auto *btnLayout = new QHBoxLayout(btnRow);
    btnLayout->setContentsMargins(0, 0, 0, 0);
    auto *addBtn = new QPushButton(tr("Feld hinzufügen"), btnRow);
    addBtn->setMinimumHeight(44);
    auto *removeBtn = new QPushButton(tr("Feld entfernen"), btnRow);
    removeBtn->setMinimumHeight(44);
    connect(addBtn,    &QPushButton::clicked, this, &DesktopEditor::onAddCustomField);
    connect(removeBtn, &QPushButton::clicked, this, &DesktopEditor::onRemoveCustomField);
    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(removeBtn);
    btnLayout->addStretch();
    layout->addWidget(btnRow);

    return content;
}

// ── Entry-Synchronisation ─────────────────────────────────────────────────────

void DesktopEditor::setEntry(DesktopEntry *entry)
{
    m_entry = entry;
    loadFromEntry();
}

void DesktopEditor::loadFromEntry()
{
    if (!m_entry)
        return;

    // Allgemein
    const int typeIdx = m_typeCombo->findData(static_cast<int>(m_entry->type()));
    m_typeCombo->setCurrentIndex(typeIdx >= 0 ? typeIdx : 0);
    m_nameEdit->setText(m_entry->name());
    m_execEdit->setText(m_entry->exec());
    m_iconEdit->setText(m_entry->icon());

    // Name-Sprachvarianten
    m_nameLocTable->setRowCount(0);
    const QStringList locales = m_entry->localesForKey("Name");
    for (const QString &lo : locales) {
        const int row = m_nameLocTable->rowCount();
        m_nameLocTable->insertRow(row);
        m_nameLocTable->setItem(row, 0, new QTableWidgetItem(lo));
        m_nameLocTable->setItem(row, 1, new QTableWidgetItem(m_entry->getLocalized("Name", lo)));
    }

    // Ausführung
    m_tryExecEdit->setText(m_entry->tryExec());
    m_pathEdit->setText(m_entry->path());
    m_terminalCheck->setChecked(m_entry->terminal());
    m_startupCheck->setChecked(m_entry->startupNotify());
    m_wmClassEdit->setText(m_entry->startupWMClass());
    m_urlEdit->setText(m_entry->url());
    m_mimeEdit->setText(m_entry->mimeTypes().join(';'));

    // Actions
    m_actionsTable->setRowCount(0);
    for (const auto &a : m_entry->actions()) {
        const int row = m_actionsTable->rowCount();
        m_actionsTable->insertRow(row);
        m_actionsTable->setItem(row, 0, new QTableWidgetItem(a.id));
        m_actionsTable->setItem(row, 1, new QTableWidgetItem(a.name));
        m_actionsTable->setItem(row, 2, new QTableWidgetItem(a.exec));
    }

    // Darstellung
    m_genericNameEdit->setText(m_entry->genericName());
    m_commentEdit->setText(m_entry->comment());
    m_keywordsEdit->setText(m_entry->keywords().join(';'));
    m_noDisplayCheck->setChecked(m_entry->noDisplay());
    m_hiddenCheck->setChecked(m_entry->hidden());

    // OnlyShowIn / NotShowIn – Selektion
    const QStringList onlyList = m_entry->onlyShowIn();
    const QStringList notList  = m_entry->notShowIn();
    for (int i = 0; i < m_onlyShowInList->count(); ++i) {
        auto *item = m_onlyShowInList->item(i);
        item->setSelected(onlyList.contains(item->text()));
    }
    for (int i = 0; i < m_notShowInList->count(); ++i) {
        auto *item = m_notShowInList->item(i);
        item->setSelected(notList.contains(item->text()));
    }

    // Kategorien
    m_categoryModel->setCheckedCategories(m_entry->categories());

    // Erweitert
    m_customTable->setRowCount(0);
    const auto customFields = m_entry->customFields();
    for (auto it = customFields.cbegin(); it != customFields.cend(); ++it) {
        const int row = m_customTable->rowCount();
        m_customTable->insertRow(row);
        m_customTable->setItem(row, 0, new QTableWidgetItem(it.key()));
        m_customTable->setItem(row, 1, new QTableWidgetItem(it.value()));
    }
}

void DesktopEditor::applyToEntry()
{
    if (!m_entry)
        return;

    // Allgemein
    const int typeData = m_typeCombo->currentData().toInt();
    m_entry->setType(static_cast<DesktopEntry::Type>(typeData));
    m_entry->setName(m_nameEdit->text());
    m_entry->setExec(m_execEdit->text());
    m_entry->setIcon(m_iconEdit->text());

    // Name-Sprachvarianten
    for (int row = 0; row < m_nameLocTable->rowCount(); ++row) {
        const QString lo  = m_nameLocTable->item(row, 0) ? m_nameLocTable->item(row, 0)->text() : QString();
        const QString val = m_nameLocTable->item(row, 1) ? m_nameLocTable->item(row, 1)->text() : QString();
        if (!lo.isEmpty())
            m_entry->setLocalized("Name", lo, val);
    }

    // Ausführung
    m_entry->setTryExec(m_tryExecEdit->text());
    m_entry->setPath(m_pathEdit->text());
    m_entry->setTerminal(m_terminalCheck->isChecked());
    m_entry->setStartupNotify(m_startupCheck->isChecked());
    m_entry->setStartupWMClass(m_wmClassEdit->text());
    m_entry->setUrl(m_urlEdit->text());
    {
        QStringList mimes;
        const QString raw = m_mimeEdit->text();
        for (const QString &s : raw.split(';')) {
            const QString t = s.trimmed();
            if (!t.isEmpty()) mimes << t;
        }
        m_entry->setMimeTypes(mimes);
    }

    // Actions
    QList<DesktopEntry::Action> actions;
    for (int row = 0; row < m_actionsTable->rowCount(); ++row) {
        DesktopEntry::Action a;
        a.id   = m_actionsTable->item(row, 0) ? m_actionsTable->item(row, 0)->text() : QString();
        a.name = m_actionsTable->item(row, 1) ? m_actionsTable->item(row, 1)->text() : QString();
        a.exec = m_actionsTable->item(row, 2) ? m_actionsTable->item(row, 2)->text() : QString();
        if (!a.id.isEmpty())
            actions << a;
    }
    m_entry->setActions(actions);

    // Darstellung
    m_entry->setGenericName(m_genericNameEdit->text());
    m_entry->setComment(m_commentEdit->text());
    {
        QStringList kw;
        for (const QString &s : m_keywordsEdit->text().split(';')) {
            const QString t = s.trimmed();
            if (!t.isEmpty()) kw << t;
        }
        m_entry->setKeywords(kw);
    }
    m_entry->setNoDisplay(m_noDisplayCheck->isChecked());
    m_entry->setHidden(m_hiddenCheck->isChecked());

    QStringList onlyIn, notIn;
    for (auto *item : m_onlyShowInList->selectedItems()) onlyIn << item->text();
    for (auto *item : m_notShowInList->selectedItems())  notIn  << item->text();
    m_entry->setOnlyShowIn(onlyIn);
    m_entry->setNotShowIn(notIn);

    m_entry->setCategories(m_categoryModel->checkedCategories());

    // Erweitert
    for (int row = 0; row < m_customTable->rowCount(); ++row) {
        const QString key = m_customTable->item(row, 0) ? m_customTable->item(row, 0)->text() : QString();
        const QString val = m_customTable->item(row, 1) ? m_customTable->item(row, 1)->text() : QString();
        if (!key.isEmpty())
            m_entry->setCustomField(key, val);
    }
}

// ── Slots ─────────────────────────────────────────────────────────────────────

void DesktopEditor::onBrowseExec()
{
    const QString path = QFileDialog::getOpenFileName(this, tr("Programm auswählen"),
        m_pathEdit->text().isEmpty() ? "/usr/bin" : m_pathEdit->text());
    if (!path.isEmpty())
        m_execEdit->setText(path);
}

void DesktopEditor::onBrowseIcon()
{
    const QString path = QFileDialog::getOpenFileName(this,
        tr("Icon auswählen"),
        QDir::homePath() + "/.local/share/icons",
        tr("Bilder (*.png *.svg *.xpm *.jpg *.ico);;Alle Dateien (*)"));
    if (!path.isEmpty())
        m_iconEdit->setText(path);
}

void DesktopEditor::onBrowseTryExec()
{
    const QString path = QFileDialog::getOpenFileName(this, tr("Programm auswählen"), "/usr/bin");
    if (!path.isEmpty())
        m_tryExecEdit->setText(path);
}

void DesktopEditor::onBrowsePath()
{
    const QString path = QFileDialog::getExistingDirectory(this, tr("Arbeitsverzeichnis auswählen"),
        m_pathEdit->text().isEmpty() ? QDir::homePath() : m_pathEdit->text());
    if (!path.isEmpty())
        m_pathEdit->setText(path);
}

void DesktopEditor::onIconConverterInstalled(const QString &iconName)
{
    // Icon-Name nur setzen wenn das Feld noch leer ist;
    // einen bereits gesetzten Namen (z.B. aus geöffneter .desktop-Datei)
    // nicht überschreiben.
    if (m_iconEdit->text().trimmed().isEmpty())
        m_iconEdit->setText(iconName);
}

void DesktopEditor::onAddAction()
{
    const int row = m_actionsTable->rowCount();
    m_actionsTable->insertRow(row);
    m_actionsTable->setItem(row, 0, new QTableWidgetItem(QStringLiteral("new-action-%1").arg(row)));
    m_actionsTable->setItem(row, 1, new QTableWidgetItem(tr("Neue Aktion")));
    m_actionsTable->setItem(row, 2, new QTableWidgetItem(QString()));
    m_actionsTable->scrollToBottom();
    m_actionsTable->editItem(m_actionsTable->item(row, 0));
}

void DesktopEditor::onRemoveAction()
{
    const int row = m_actionsTable->currentRow();
    if (row >= 0)
        m_actionsTable->removeRow(row);
}

void DesktopEditor::onAddCustomField()
{
    const int row = m_customTable->rowCount();
    m_customTable->insertRow(row);
    m_customTable->setItem(row, 0, new QTableWidgetItem("X-"));
    m_customTable->setItem(row, 1, new QTableWidgetItem(QString()));
    m_customTable->scrollToBottom();
    m_customTable->editItem(m_customTable->item(row, 0));
}

void DesktopEditor::onRemoveCustomField()
{
    const int row = m_customTable->currentRow();
    if (row >= 0)
        m_customTable->removeRow(row);
}

void DesktopEditor::onAddLanguageVariant()
{
    bool ok = false;
    const QString locale = QInputDialog::getText(this,
        tr("Sprachvariante"),
        tr("Sprachkürzel (z.B. de, en, fr):"),
        QLineEdit::Normal, QString(), &ok);
    if (!ok || locale.trimmed().isEmpty())
        return;

    // Prüfen ob bereits vorhanden
    for (int i = 0; i < m_nameLocTable->rowCount(); ++i) {
        if (m_nameLocTable->item(i, 0) &&
            m_nameLocTable->item(i, 0)->text() == locale.trimmed())
            return;
    }

    const int row = m_nameLocTable->rowCount();
    m_nameLocTable->insertRow(row);
    m_nameLocTable->setItem(row, 0, new QTableWidgetItem(locale.trimmed()));
    m_nameLocTable->setItem(row, 1, new QTableWidgetItem(QString()));
    m_nameLocTable->editItem(m_nameLocTable->item(row, 1));
}

// ── Hilfsmethoden ─────────────────────────────────────────────────────────────

QLineEdit *DesktopEditor::makeLineEdit(const QString &placeholder)
{
    auto *edit = new QLineEdit(this);
    edit->setMinimumHeight(36);
    if (!placeholder.isEmpty())
        edit->setPlaceholderText(placeholder);
    return edit;
}
