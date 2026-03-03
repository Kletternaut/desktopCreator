#include "iconconverter.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QUrl>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QPainter>
#include <QPixmap>
#include <QBuffer>
#include <QTextStream>
#include <QSvgRenderer>
#include <QMessageBox>
#include <QApplication>
#include <QInputDialog>
#include <QLabel>
#include <QScrollArea>
#include <QProcess>
#include <QDebug>

// Standard freedesktop hicolor-Größen
const QList<int> IconConverter::kSizes = {
    16, 22, 24, 32, 48, 64, 96, 128, 256, 512
};

// ── Konstruktor ──────────────────────────────────────────────────────────────

IconConverter::IconConverter(QWidget *parent)
    : QWidget(parent)
{
    setAcceptDrops(true);
    setupUi();
}

// ── UI-Setup ─────────────────────────────────────────────────────────────────

void IconConverter::setupUi()
{
    auto *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(8);

    // ── Drag & Drop Zone ─────────────────────────────────────────────────────
    auto *dropGroup = new QGroupBox(tr("Quellbild"), this);
    auto *dropLayout = new QHBoxLayout(dropGroup);

    m_dropLabel = new QLabel(this);
    m_dropLabel->setFixedSize(200, 120);
    m_dropLabel->setAlignment(Qt::AlignCenter);
    m_dropLabel->setFrameShape(QFrame::StyledPanel);
    m_dropLabel->setAcceptDrops(false);  // Parent übernimmt
    m_dropLabel->setPixmap(QPixmap(":/images/drag-drop-hint.svg").scaled(
        200, 120, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_dropLabel->setToolTip(tr("Bild hier ablegen oder 'Auswählen' klicken"));

    m_srcPreview = new QLabel(this);
    m_srcPreview->setFixedSize(128, 128);
    m_srcPreview->setAlignment(Qt::AlignCenter);
    m_srcPreview->setFrameShape(QFrame::StyledPanel);
    m_srcPreview->setText(tr("Quellbild\nVorschau"));

    m_selectButton = new QPushButton(tr("Bild auswählen..."), this);
    m_selectButton->setMinimumHeight(44);
    connect(m_selectButton, &QPushButton::clicked, this, &IconConverter::onSelectSource);

    dropLayout->addWidget(m_dropLabel);
    dropLayout->addWidget(m_srcPreview);
    dropLayout->addWidget(m_selectButton, 0, Qt::AlignVCenter);
    dropLayout->addStretch();
    mainLayout->addWidget(dropGroup);

    // ── SVG-Option ────────────────────────────────────────────────────────────
    m_svgCheck = new QCheckBox(tr("Skalierbares SVG installieren (~/.local/share/icons/hicolor/scalable/apps/)"), this);
    m_svgCheck->setChecked(true);
    mainLayout->addWidget(m_svgCheck);

    // ── Größen-Auswahl und Vorschauen ─────────────────────────────────────────
    auto *sizesGroup = new QGroupBox(tr("PNG-Größen"), this);
    auto *sizesGrid  = new QGridLayout(sizesGroup);
    sizesGrid->setSpacing(6);

    int col = 0, row = 0;
    constexpr int maxCols = 5;
    for (int sz : kSizes) {
        auto *box = new QWidget(sizesGroup);
        auto *boxLayout = new QVBoxLayout(box);
        boxLayout->setContentsMargins(2, 2, 2, 2);
        boxLayout->setSpacing(2);

        auto *preview = new QLabel(box);
        preview->setFixedSize(sz > 64 ? 64 : sz, sz > 64 ? 64 : sz);
        preview->setAlignment(Qt::AlignCenter);
        preview->setFrameShape(QFrame::StyledPanel);
        preview->setToolTip(QStringLiteral("%1×%1").arg(sz));
        m_previewLabels[sz] = preview;

        auto *check = new QCheckBox(QStringLiteral("%1×%1").arg(sz), box);
        check->setChecked(true);
        m_sizeChecks[sz] = check;

        boxLayout->addWidget(preview, 0, Qt::AlignHCenter);
        boxLayout->addWidget(check,   0, Qt::AlignHCenter);
        sizesGrid->addWidget(box, row, col);

        ++col;
        if (col >= maxCols) { col = 0; ++row; }
    }
    mainLayout->addWidget(sizesGroup);

    // ── Fortschrittsanzeige + Install-Button ─────────────────────────────────
    m_progressBar = new QProgressBar(this);
    m_progressBar->setRange(0, 100);
    m_progressBar->setValue(0);
    m_progressBar->setVisible(false);
    mainLayout->addWidget(m_progressBar);

    m_installButton = new QPushButton(tr("Konvertieren && Alle installieren"), this);
    m_installButton->setMinimumHeight(44);
    m_installButton->setEnabled(false);
    connect(m_installButton, &QPushButton::clicked, this, &IconConverter::onConvertAndInstall);
    mainLayout->addWidget(m_installButton);

    m_refreshCacheButton = new QPushButton(tr("Icon-Cache aktualisieren && Desktop neu laden"), this);
    m_refreshCacheButton->setMinimumHeight(44);
    m_refreshCacheButton->setToolTip(tr(
        "Führt gtk-update-icon-cache aus und lädt den Desktop-Manager neu,\n"
        "damit installierte Icons sofort sichtbar werden."));
    connect(m_refreshCacheButton, &QPushButton::clicked, this, &IconConverter::onRefreshCache);
    mainLayout->addWidget(m_refreshCacheButton);

    mainLayout->addStretch();
}

// ── Drag & Drop ──────────────────────────────────────────────────────────────

void IconConverter::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        const QUrl url = event->mimeData()->urls().first();
        const QString path = url.toLocalFile();
        static const QStringList allowed = { "png","jpg","jpeg","bmp","tiff","tif","svg","xpm" };
        const QString ext = QFileInfo(path).suffix().toLower();
        if (allowed.contains(ext)) {
            event->acceptProposedAction();
            return;
        }
    }
    event->ignore();
}

void IconConverter::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        const QString path = event->mimeData()->urls().first().toLocalFile();
        if (!path.isEmpty()) {
            setSourcePath(path);
            event->acceptProposedAction();
        }
    }
}

// ── Slots ────────────────────────────────────────────────────────────────────

void IconConverter::onSelectSource()
{
    const QString path = QFileDialog::getOpenFileName(
        this,
        tr("Quellbild auswählen"),
        QDir::homePath(),
        tr("Bilder (*.png *.jpg *.jpeg *.bmp *.tiff *.svg *.xpm)")
    );
    if (!path.isEmpty()) {
        setSourcePath(path);
    }
}

void IconConverter::onConvertAndInstall()
{
    if (m_sourcePath.isEmpty())
        return;

    bool ok = false;
    const QString defaultName = QFileInfo(m_sourcePath).completeBaseName()
        .toLower().replace(' ', '-');
    const QString baseName = QInputDialog::getText(
        this,
        tr("Icon-Name"),
        tr("Basisname des Icons (z.B. meine-app):"),
        QLineEdit::Normal,
        defaultName,
        &ok
    );

    if (!ok || baseName.trimmed().isEmpty())
        return;

    const QString name = baseName.trimmed().toLower().replace(' ', '-');

    m_progressBar->setVisible(true);
    m_progressBar->setValue(0);
    m_installButton->setEnabled(false);
    QApplication::processEvents();

    int total = 0;
    for (int sz : kSizes) {
        if (m_sizeChecks.value(sz) && m_sizeChecks[sz]->isChecked())
            ++total;
    }
    if (m_svgCheck->isChecked())
        ++total;

    int done = 0;
    bool allOk = true;

    // PNG-Größen installieren
    if (!installPngs(name)) allOk = false;
    for (int sz : kSizes) {
        if (m_sizeChecks.value(sz) && m_sizeChecks[sz]->isChecked()) {
            ++done;
            m_progressBar->setValue(static_cast<int>(100.0 * done / total));
            QApplication::processEvents();
        }
    }

    // SVG installieren
    if (m_svgCheck->isChecked()) {
        if (!installSvg(name)) allOk = false;
        ++done;
        m_progressBar->setValue(100);
        QApplication::processEvents();
    }

    m_progressBar->setValue(100);
    m_installButton->setEnabled(true);

    if (allOk) {
        m_installedIconName = name;
        emit iconInstalled(name);
        // Cache automatisch nach erfolgreicher Installation aktualisieren
        onRefreshCache();
        QMessageBox::information(this, tr("Fertig"),
            tr("Icon '%1' erfolgreich installiert und Cache aktualisiert.\n"
               "Pfad: ~/.local/share/icons/hicolor/").arg(name));
    } else {
        QMessageBox::warning(this, tr("Teilweise Fehler"),
            tr("Einige Icons konnten nicht installiert werden.\n"
               "Bitte Verzeichnisberechtigungen prüfen."));
    }
}

// ── Quellbild setzen ─────────────────────────────────────────────────────────

void IconConverter::setSourcePath(const QString &path)
{
    m_sourcePath = path;
    updateSourcePreview();
    updateSizePreviews();
    m_installButton->setEnabled(!path.isEmpty());
}

void IconConverter::updateSourcePreview()
{
    if (m_sourcePath.isEmpty()) {
        m_srcPreview->clear();
        m_srcPreview->setText(tr("Quellbild\nVorschau"));
        return;
    }

    QImage img = renderToSize(128);
    if (!img.isNull()) {
        m_srcPreview->setPixmap(QPixmap::fromImage(img));
    } else {
        m_srcPreview->setText(tr("Kein Vorschau"));
    }
}

void IconConverter::updateSizePreviews()
{
    for (auto it = m_previewLabels.cbegin(); it != m_previewLabels.cend(); ++it) {
        const int sz = it.key();
        QLabel *lbl  = it.value();
        if (m_sourcePath.isEmpty()) {
            lbl->clear();
            continue;
        }
        QImage img = renderToSize(sz);
        if (!img.isNull()) {
            const int displaySz = qMin(sz, 64);
            lbl->setPixmap(QPixmap::fromImage(img).scaled(
                displaySz, displaySz,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation));
        } else {
            lbl->setText("?");
        }
    }
}

// ── Rendering ────────────────────────────────────────────────────────────────

QImage IconConverter::renderToSize(int size) const
{
    if (m_sourcePath.isEmpty())
        return {};

    const QString ext = QFileInfo(m_sourcePath).suffix().toLower();

    if (ext == "svg") {
        QSvgRenderer renderer(m_sourcePath);
        if (!renderer.isValid())
            return {};
        QImage img(size, size, QImage::Format_ARGB32);
        img.fill(Qt::transparent);
        QPainter painter(&img);
        renderer.render(&painter);
        return img;
    }

    // Rasterformate
    QImage img(m_sourcePath);
    if (img.isNull())
        return {};
    return img.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
}

// ── Installation ─────────────────────────────────────────────────────────────

bool IconConverter::installPngs(const QString &baseName)
{
    bool allOk = true;
    const QString hicolorBase = QDir::homePath()
        + "/.local/share/icons/hicolor";

    for (int sz : kSizes) {
        if (!m_sizeChecks.value(sz) || !m_sizeChecks[sz]->isChecked())
            continue;

        const QString dirPath = QStringLiteral("%1/%2x%2/apps")
            .arg(hicolorBase).arg(sz);
        QDir dir;
        if (!dir.mkpath(dirPath)) {
            qWarning() << "IconConverter: Konnte Verzeichnis nicht erstellen:" << dirPath;
            allOk = false;
            continue;
        }

        QImage img = renderToSize(sz);
        if (img.isNull()) {
            allOk = false;
            continue;
        }

        const QString destPath = QStringLiteral("%1/%2.png").arg(dirPath, baseName);
        if (!img.save(destPath, "PNG")) {
            qWarning() << "IconConverter: Konnte PNG nicht speichern:" << destPath;
            allOk = false;
        }
    }
    return allOk;
}

void IconConverter::onRefreshCache()
{
    // 1. gtk-update-icon-cache für das User-Hicolor-Theme
    const QString hicolorPath = QDir::homePath() + "/.local/share/icons/hicolor";
    QProcess cacheProcess;
    cacheProcess.start("gtk-update-icon-cache",
                       { "-f", "-t", hicolorPath });
    const bool cacheOk = cacheProcess.waitForFinished(5000)
                         && cacheProcess.exitCode() == 0;

    // 2. Desktop-Manager-Reload nur auf explizite Nachfrage
    const int btn = QMessageBox::question(this, tr("Desktop neu laden?"),
        tr("Icon-Cache wurde aktualisiert.\n\n"
           "Soll der Desktop-Manager jetzt neu geladen werden,\n"
           "damit Icons sofort sichtbar werden?\n\n"
           "Hinweis: Desktop-Neustart kann Positionen / Hintergrund kurz flackern lassen."),
        QMessageBox::Yes | QMessageBox::No,
        QMessageBox::No);

    if (btn == QMessageBox::Yes) {
        const QString de = qgetenv("XDG_CURRENT_DESKTOP").toLower();

        if (de.contains("lxde") || de.contains("lxqt") ||
            QProcess::execute("pgrep", {"-x", "pcmanfm"}) == 0) {
            // Sanfter Reload via SIGHUP statt Desktop-Off/On
            QProcess::startDetached("bash",
                { "-c", "killall -HUP pcmanfm 2>/dev/null || pcmanfm --desktop &" });
        } else if (de.contains("xfce")) {
            QProcess::startDetached("xfdesktop", { "--reload" });
        }
        // GNOME/KDE: keine Aktion nötig (inotify erkennt Änderungen automatisch)
    }

    if (cacheOk) {
        QMessageBox::information(this, tr("Fertig"),
            tr("Icon-Cache erfolgreich aktualisiert."));
    } else {
        QMessageBox::warning(this, tr("Cache-Warnung"),
            tr("gtk-update-icon-cache schlug fehl oder ist nicht installiert.\n"
               "Icons werden spätestens beim nächsten Login sichtbar."));
    }
}

bool IconConverter::installSvg(const QString &baseName)
{
    const QString dirPath = QDir::homePath()
        + "/.local/share/icons/hicolor/scalable/apps";
    QDir dir;
    if (!dir.mkpath(dirPath)) {
        qWarning() << "IconConverter: Konnte scalable-Verzeichnis nicht erstellen:" << dirPath;
        return false;
    }

    const QString destPath = QStringLiteral("%1/%2.svg").arg(dirPath, baseName);
    const QString ext = QFileInfo(m_sourcePath).suffix().toLower();

    if (ext == "svg") {
        // Direkt kopieren
        if (QFile::exists(destPath))
            QFile::remove(destPath);
        return QFile::copy(m_sourcePath, destPath);
    }

    // PNG-in-SVG-Wrapper: 256px PNG als Base64 in SVG einbetten
    QImage img = renderToSize(256);
    if (img.isNull())
        return false;

    QByteArray pngData;
    QBuffer buf(&pngData);
    buf.open(QIODevice::WriteOnly);
    img.save(&buf, "PNG");
    buf.close();

    const QString base64 = QString::fromLatin1(pngData.toBase64());
    const QString svg = QStringLiteral(
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
        "<svg xmlns=\"http://www.w3.org/2000/svg\"\n"
        "     xmlns:xlink=\"http://www.w3.org/1999/xlink\"\n"
        "     viewBox=\"0 0 256 256\" width=\"256\" height=\"256\">\n"
        "  <image xlink:href=\"data:image/png;base64,%1\"\n"
        "         width=\"256\" height=\"256\"/>\n"
        "</svg>\n")
        .arg(base64);

    QFile svgFile(destPath);
    if (!svgFile.open(QIODevice::WriteOnly | QIODevice::Text))
        return false;
    QTextStream s(&svgFile);
    s.setCodec("UTF-8");
    s << svg;
    svgFile.close();
    return true;
}
