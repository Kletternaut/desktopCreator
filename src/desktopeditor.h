#pragma once

#include <QWidget>
#include <QTabWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QCheckBox>
#include <QListWidget>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QGroupBox>

#include "categorymodel.h"
#include "iconconverter.h"

class DesktopEntry;

/**
 * @brief Haupt-Editor-Widget für einen Desktop-Eintrag.
 *
 * Enthält ein QTabWidget mit den Reitern:
 *   - Allgemein (Pflichtfelder)
 *   - Ausführung (Ausführungs-Optionen)
 *   - Darstellung (Darstellungs-Optionen)
 *   - Icon-Konverter
 *   - Erweitert (X-* Felder)
 *
 * Alle Felder sind zweiseitig mit einem DesktopEntry-Objekt synchronisiert.
 */
class DesktopEditor : public QWidget
{
    Q_OBJECT

public:
    explicit DesktopEditor(QWidget *parent = nullptr);

    /** @brief Verknüpft den Editor mit einem DesktopEntry-Objekt. */
    void setEntry(DesktopEntry *entry);

    /** @brief Gibt den aktuell verknüpften Eintrag zurück. */
    DesktopEntry *entry() const { return m_entry; }

public slots:
    /** @brief Überträgt alle UI-Felder in das DesktopEntry-Objekt. */
    void applyToEntry();

    /** @brief Befüllt alle UI-Felder aus dem DesktopEntry-Objekt. */
    void loadFromEntry();

private slots:
    void onBrowseExec();
    void onBrowseIcon();
    void onBrowseTryExec();
    void onBrowsePath();
    void onIconConverterInstalled(const QString &iconName);
    void onAddAction();
    void onRemoveAction();
    void onAddCustomField();
    void onRemoveCustomField();
    void onAddLanguageVariant();

private:
    void setupUi();
    QWidget *createGeneralTab();
    QWidget *createExecutionTab();
    QWidget *createDisplayTab();
    QWidget *createIconConverterTab();
    QWidget *createAdvancedTab();

    // Hilfsmethoden für wiederkehrende UI-Muster
    QWidget *makeFieldRow(const QString &labelText, QWidget *field);
    QLineEdit *makeLineEdit(const QString &placeholder = {});

    DesktopEntry   *m_entry = nullptr;
    QTabWidget     *m_tabs  = nullptr;

    // ── Allgemein ─────────────────────────────────────────────────────────────
    QComboBox  *m_typeCombo    = nullptr;
    QLineEdit  *m_nameEdit     = nullptr;
    QLineEdit  *m_execEdit     = nullptr;
    QLineEdit  *m_iconEdit     = nullptr;
    QLabel     *m_iconPreview  = nullptr;

    // Sprachvarianten (Name[de], Name[en], …)
    QTableWidget *m_nameLocTable = nullptr;

    // ── Ausführung ────────────────────────────────────────────────────────────
    QLineEdit  *m_tryExecEdit      = nullptr;
    QLineEdit  *m_pathEdit         = nullptr;
    QCheckBox  *m_terminalCheck    = nullptr;
    QCheckBox  *m_startupCheck     = nullptr;
    QLineEdit  *m_wmClassEdit      = nullptr;
    QLineEdit  *m_mimeEdit         = nullptr;
    QLineEdit  *m_urlEdit          = nullptr;

    // Actions
    QTableWidget *m_actionsTable  = nullptr;

    // ── Darstellung ───────────────────────────────────────────────────────────
    QLineEdit  *m_genericNameEdit  = nullptr;
    QLineEdit  *m_commentEdit      = nullptr;
    QLineEdit  *m_keywordsEdit     = nullptr;
    QCheckBox  *m_noDisplayCheck   = nullptr;
    QCheckBox  *m_hiddenCheck      = nullptr;

    QListWidget *m_onlyShowInList  = nullptr;
    QListWidget *m_notShowInList   = nullptr;

    // Kategorien
    CategoryModel *m_categoryModel = nullptr;
    QListView     *m_categoryView  = nullptr;

    // ── Icon Converter ────────────────────────────────────────────────────────
    IconConverter *m_iconConverter = nullptr;

    // ── Erweitert (X-*) ──────────────────────────────────────────────────────
    QTableWidget *m_customTable    = nullptr;

    // Bekannte OnlyShowIn / NotShowIn-Umgebungen
    static const QStringList kDesktopEnvironments;
};
