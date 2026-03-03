#pragma once

#include <QWidget>
#include <QLabel>
#include <QPlainTextEdit>
#include <QVBoxLayout>

class DesktopEntry;

/**
 * @brief Zeigt eine Live-Vorschau des Desktop-Eintrags.
 *
 * Besteht aus:
 * - einem simulierten Desktop-Icon (64px Icon + App-Name)
 * - dem Rohtext der erzeugten .desktop-Datei
 *
 * Aktualisiert sich automatisch über das Signal DesktopEntry::entryChanged().
 */
class PreviewWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PreviewWidget(QWidget *parent = nullptr);

    /** @brief Verknüpft das Widget mit einem DesktopEntry-Objekt. */
    void setEntry(DesktopEntry *entry);

public slots:
    /** @brief Aktualisiert alle Vorschau-Elemente aus dem aktuellen Entry. */
    void refresh();

private:
    void setupUi();
    void updateIconPreview();
    void updateRawText();

    DesktopEntry *m_entry = nullptr;

    // Simuliertes Desktop-Icon
    QLabel       *m_iconLabel    = nullptr;
    QLabel       *m_nameLabel    = nullptr;

    // Rohtext-Vorschau
    QPlainTextEdit *m_rawText    = nullptr;

    static constexpr int kIconSize = 64;
};
