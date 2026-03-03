#include "aboutdialog.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QApplication>
#include <QIcon>
#include <QPixmap>
#include <QDesktopServices>
#include <QUrl>

AboutDialog::AboutDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(tr("Über .desktopCreator"));
    setFixedWidth(440);
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);

    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(24, 20, 24, 20);

    // ── Icon + Titel ──────────────────────────────────────────────────────────
    auto *headerLayout = new QHBoxLayout;
    auto *iconLabel    = new QLabel(this);
    QIcon appIcon = QIcon(":/icons/app-icon.svg");
    iconLabel->setPixmap(appIcon.pixmap(64, 64));
    iconLabel->setFixedSize(64, 64);
    headerLayout->addWidget(iconLabel);
    headerLayout->addSpacing(16);

    auto *titleLayout = new QVBoxLayout;
    auto *nameLabel   = new QLabel(QStringLiteral("<span style='font-size:22pt; font-weight:bold;'>.desktopCreator</span>"), this);
    auto *verLabel    = new QLabel(tr("Version %1").arg(QApplication::applicationVersion()), this);
    titleLayout->addWidget(nameLabel);
    titleLayout->addWidget(verLabel);
    titleLayout->addStretch();
    headerLayout->addLayout(titleLayout);
    headerLayout->addStretch();
    mainLayout->addLayout(headerLayout);

    // ── Beschreibung ──────────────────────────────────────────────────────────
    auto *descLabel = new QLabel(
        tr("Grafischer Editor für <b>.desktop</b>-Dateien nach dem<br>"
           "freedesktop.org Desktop Entry Specification 1.5-Standard.<br>"
           "Inklusive Icon-Konvertierungs-Pipeline für alle hicolor-Größen."),
        this);
    descLabel->setWordWrap(true);
    mainLayout->addWidget(descLabel);

    // ── Trennlinie ────────────────────────────────────────────────────────────
    auto *line = new QFrame(this);
    line->setFrameShape(QFrame::HLine);
    line->setFrameShadow(QFrame::Sunken);
    mainLayout->addWidget(line);

    // ── Autor / Lizenz ────────────────────────────────────────────────────────
    auto *infoLabel = new QLabel(
        tr("<b>Autor:</b> Kletternaut<br>"
           "<b>Lizenz:</b> MIT<br>"
           "<b>GitHub:</b> <a href='https://github.com/Kletternaut/desktopCreator'>"
           "github.com/Kletternaut/desktopCreator</a>"),
        this);
    infoLabel->setOpenExternalLinks(true);
    infoLabel->setWordWrap(true);
    mainLayout->addWidget(infoLabel);

    // ── Lizenztext ────────────────────────────────────────────────────────────
    auto *licLabel = new QLabel(
        QStringLiteral(
            "<small>MIT License – Copyright © 2026 Kletternaut<br><br>"
            "Permission is hereby granted, free of charge, to any person obtaining "
            "a copy of this software and associated documentation files (the "
            "\"Software\"), to deal in the Software without restriction. "
            "The above copyright notice and this permission notice shall be included "
            "in all copies or substantial portions of the Software.</small>"),
        this);
    licLabel->setWordWrap(true);
    licLabel->setStyleSheet("color: #777;");
    mainLayout->addWidget(licLabel);

    // ── Qt-Info ───────────────────────────────────────────────────────────────
    auto *qtLabel = new QLabel(
        tr("<small>Erstellt mit Qt %1 – läuft auf Raspberry Pi OS (aarch64)</small>")
            .arg(QString::fromLatin1(qVersion())),
        this);
    qtLabel->setStyleSheet("color: #999;");
    mainLayout->addWidget(qtLabel);

    // ── Schließen-Schaltfläche ────────────────────────────────────────────────
    auto *btnLayout = new QHBoxLayout;
    btnLayout->addStretch();
    auto *closeBtn = new QPushButton(tr("Schließen"), this);
    closeBtn->setDefault(true);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    btnLayout->addWidget(closeBtn);
    mainLayout->addLayout(btnLayout);
}
