#pragma once

#include <QDialog>

/**
 * @brief Zeigt Informationen über die Anwendung an.
 *
 * Enthält App-Name, Version, Autor, Lizenz und Repository-Link.
 */
class AboutDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AboutDialog(QWidget *parent = nullptr);
};
