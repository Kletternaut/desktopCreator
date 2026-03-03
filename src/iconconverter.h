#pragma once

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QCheckBox>
#include <QGridLayout>
#include <QGroupBox>
#include <QImage>
#include <QMap>
#include <QFrame>

/**
 * @brief Widget für die Icon-Konvertierungs-Pipeline.
 *
 * Nimmt eine Quellgrafik (PNG, JPG, BMP, TIFF, SVG, XPM) entgegen
 * und erzeugt Icons in allen hicolor-Standard-Größen sowie optional
 * ein skalierbares SVG.
 *
 * Unterstützt Drag & Drop für die Quellgrafik.
 */
class IconConverter : public QWidget
{
    Q_OBJECT

public:
    explicit IconConverter(QWidget *parent = nullptr);

    /** @brief Setzt den Quellbild-Pfad und aktualisiert die Vorschau. */
    void setSourcePath(const QString &path);

    /** @brief Gibt den Quellbild-Pfad zurück. */
    QString sourcePath() const { return m_sourcePath; }

    /** @brief Gibt den zuletzt installierten Icon-Namen zurück (ohne Pfad/Extension). */
    QString installedIconName() const { return m_installedIconName; }

signals:
    /** @brief Wird after der Installation ausgelöst, enthält den Icon-Namen. */
    void iconInstalled(const QString &iconName);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;

private slots:
    void onSelectSource();
    void onConvertAndInstall();

private:
    void setupUi();
    void updateSourcePreview();
    void updateSizePreviews();

    /**
     * @brief Rendert die Quellgrafik in eine QImage der gewünschten Größe.
     * @param size  Kante des quadratischen Zielbilds in Pixeln.
     * @return Skaliertes QImage oder leeres QImage bei Fehler.
     */
    QImage renderToSize(int size) const;

    /**
     * @brief Installiert PNGs in alle ausgewählten hicolor-Verzeichnisse.
     * @param baseName  Icon-Basisname (z.B. "meine-app")
     * @return true bei Erfolg aller Installationen.
     */
    bool installPngs(const QString &baseName);

    /**
     * @brief Versucht, ein SVG-Icon zu erzeugen und zu installieren.
     * Falls das Quellbild bereits SVG ist, wird es direkt kopiert.
     * Andernfalls wird ein PNG-in-SVG-Wrapper erzeugt.
     * @param baseName  Icon-Basisname.
     * @return true bei Erfolg.
     */
    bool installSvg(const QString &baseName);

    // Standard hicolor-Größen
    static const QList<int> kSizes;

    QString m_sourcePath;
    QString m_installedIconName;

    // UI-Elemente
    QLabel        *m_dropLabel       = nullptr;
    QLabel        *m_srcPreview      = nullptr;
    QProgressBar  *m_progressBar     = nullptr;
    QPushButton   *m_selectButton    = nullptr;
    QPushButton   *m_installButton   = nullptr;

    // Größen-Vorschau-Labels (size -> label)
    QMap<int, QLabel *> m_previewLabels;

    // Checkboxen für Größenauswahl
    QMap<int, QCheckBox *> m_sizeChecks;

    QCheckBox     *m_svgCheck        = nullptr;
};
