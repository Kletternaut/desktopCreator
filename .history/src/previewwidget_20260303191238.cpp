#include "previewwidget.h"
#include "desktopentry.h"

#include <QGroupBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QIcon>
#include <QPixmap>
#include <QSvgRenderer>
#include <QPainter>
#include <QFont>
#include <QFrame>
#include <QDir>
#include <QFileInfo>

// Sucht ein Icon zuerst direkt auf der Platte in den hicolor-Verzeichnissen,
// um Qt's internen (veralteten) Theme-Cache zu umgehen.
static QPixmap resolveIconPixmap(const QString &iconName, int size)
{
    if (iconName.isEmpty())
        return {};

    // Direktpfad: SVG
    if (iconName.toLower().endsWith(".svg")) {
        QSvgRenderer r(iconName);
        if (r.isValid()) {
            QPixmap pix(size, size);
            pix.fill(Qt::transparent);
            QPainter p(&pix);
            r.render(&p);
            return pix;
        }
    }

    // Direktpfad: Rasterbild
    if (iconName.contains(QLatin1Char('/')) || iconName.contains(QLatin1Char('.'))) {
        QPixmap pix(iconName);
        if (!pix.isNull()) return pix;
    }

    // Manuelle hicolor-Suche (umgeht Qt-Cache)
    const QStringList bases = {
        QDir::homePath() + QStringLiteral("/.local/share/icons/hicolor"),
        QStringLiteral("/usr/local/share/icons/hicolor"),
        QStringLiteral("/usr/share/icons/hicolor")
    };
    const QString sizePart = QStringLiteral("%1x%1/apps/").arg(size);
    for (const QString &base : bases) {
        // Bevorzuge PNG in der passenden Größe
        const QString pngPath = base + QLatin1Char('/') + sizePart + iconName + QStringLiteral(".png");
        if (QFileInfo::exists(pngPath)) {
            QPixmap pix(pngPath);
            if (!pix.isNull()) return pix;
        }
        // Scalable SVG
        const QString svgPath = base + QStringLiteral("/scalable/apps/") + iconName + QStringLiteral(".svg");
        if (QFileInfo::exists(svgPath)) {
            QSvgRenderer r(svgPath);
            if (r.isValid()) {
                QPixmap pix(size, size);
                pix.fill(Qt::transparent);
                QPainter p(&pix);
                r.render(&p);
                return pix;
            }
        }
    }

    // Letzter Fallback: Qt Theme-Cache
    const QIcon ti = QIcon::fromTheme(iconName);
    if (!ti.isNull())
        return ti.pixmap(size, size);

    return {};
}

PreviewWidget::PreviewWidget(QWidget *parent)
    : QWidget(parent)
{
    setupUi();
}

void PreviewWidget::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(4, 4, 4, 4);
    mainLayout->setSpacing(8);

    // ── Simuliertes Desktop-Icon ─────────────────────────────────────────────
    auto *iconGroup = new QGroupBox(tr("Vorschau"), this);
    auto *iconLayout = new QVBoxLayout(iconGroup);
    iconLayout->setAlignment(Qt::AlignHCenter);

    m_iconLabel = new QLabel(iconGroup);
    m_iconLabel->setFixedSize(kIconSize, kIconSize);
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_iconLabel->setFrameShape(QFrame::StyledPanel);
    m_iconLabel->setToolTip(tr("App-Icon (64×64 Pixel)"));

    m_nameLabel = new QLabel(iconGroup);
    m_nameLabel->setAlignment(Qt::AlignHCenter | Qt::AlignTop);
    m_nameLabel->setWordWrap(true);
    m_nameLabel->setMaximumWidth(120);
    QFont f = m_nameLabel->font();
    f.setPointSize(9);
    m_nameLabel->setFont(f);

    iconLayout->addWidget(m_iconLabel, 0, Qt::AlignHCenter);
    iconLayout->addWidget(m_nameLabel, 0, Qt::AlignHCenter);
    mainLayout->addWidget(iconGroup);

    // ── Rohtext-Vorschau ─────────────────────────────────────────────────────
    auto *rawGroup = new QGroupBox(tr("Raw .desktop content"), this);
    auto *rawLayout = new QVBoxLayout(rawGroup);

    m_rawText = new QPlainTextEdit(rawGroup);
    m_rawText->setReadOnly(true);
    m_rawText->setMinimumHeight(200);
    QFont mono("Monospace");
    mono.setPointSize(8);
    mono.setStyleHint(QFont::Monospace);
    m_rawText->setFont(mono);
    m_rawText->setLineWrapMode(QPlainTextEdit::NoWrap);

    rawLayout->addWidget(m_rawText);
    mainLayout->addWidget(rawGroup, 1);
}

void PreviewWidget::setEntry(DesktopEntry *entry)
{
    if (m_entry) {
        disconnect(m_entry, &DesktopEntry::entryChanged, this, &PreviewWidget::refresh);
    }
    m_entry = entry;
    if (m_entry) {
        connect(m_entry, &DesktopEntry::entryChanged, this, &PreviewWidget::refresh);
    }
    refresh();
}

void PreviewWidget::refresh()
{
    updateIconPreview();
    updateRawText();
}

void PreviewWidget::updateIconPreview()
{
    if (!m_entry) {
        m_iconLabel->clear();
        m_nameLabel->clear();
        return;
    }

    // Name-Label
    const QString displayName = m_entry->name().isEmpty()
        ? tr("(kein Name)")
        : m_entry->name();
    m_nameLabel->setText(displayName);

    // Icon
    const QString iconPath = m_entry->icon();
    QPixmap pix;

    if (!iconPath.isEmpty())
        pix = resolveIconPixmap(iconPath, kIconSize);

    if (pix.isNull()) {
        // Fallback: generisches App-Icon
        pix = QIcon::fromTheme(QStringLiteral("application-x-executable"),
              QIcon(QStringLiteral(":/icons/app-icon.svg"))).pixmap(kIconSize, kIconSize);
    }

    if (!pix.isNull()) {
        m_iconLabel->setPixmap(pix.scaled(
            kIconSize, kIconSize,
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation));
    } else {
        m_iconLabel->setText("?");
    }
}

void PreviewWidget::updateRawText()
{
    if (!m_entry) {
        m_rawText->clear();
        return;
    }
    m_rawText->setPlainText(m_entry->toDesktopFormat());
}
