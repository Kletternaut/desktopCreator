#pragma once

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QVariant>

/**
 * @brief Repräsentiert einen einzelnen Desktop-Eintrag (freedesktop.org Desktop Entry Spec 1.5)
 *
 * Diese Klasse verwaltet alle Felder einer .desktop-Datei und bietet
 * Methoden zum Laden, Speichern, Serialisieren und Validieren.
 */
class DesktopEntry : public QObject
{
    Q_OBJECT

public:
    /// Typ des Desktop-Eintrags
    enum class Type {
        Application,
        Link,
        Directory
    };
    Q_ENUM(Type)

    /// Ergebnis der Validierung
    struct ValidationResult {
        bool valid = false;
        QStringList errors;
        QStringList warnings;
    };

    /// Repräsentiert eine Desktop-Action (Sub-Action)
    struct Action {
        QString id;          ///< Eindeutige ID, z.B. "new-window"
        QString name;        ///< Anzeigename
        QString exec;        ///< Programmaufruf
        QString icon;        ///< Icon-Pfad/-Name

        bool operator==(const Action &other) const {
            return id == other.id && name == other.name
                && exec == other.exec && icon == other.icon;
        }
        bool operator!=(const Action &other) const { return !(*this == other); }
    };

    explicit DesktopEntry(QObject *parent = nullptr);
    ~DesktopEntry() override = default;

    // ── Laden / Speichern ───────────────────────────────────────────────────

    /**
     * @brief Lädt einen Desktop-Eintrag aus einer .desktop-Datei.
     * @param path Absoluter oder relativer Dateipfad.
     * @return true bei Erfolg, false bei Fehler (isValid() gibt dann false zurück).
     */
    bool loadFromFile(const QString &path);

    /**
     * @brief Speichert den Eintrag als .desktop-Datei.
     * @param path Zieldateipfad.
     * @return true bei Erfolg.
     */
    bool saveToFile(const QString &path) const;

    /**
     * @brief Serialisiert den Eintrag als .desktop-Format-String.
     * @return Vollständiger Inhalt der .desktop-Datei.
     */
    QString toDesktopFormat() const;

    /**
     * @brief Parst einen .desktop-Format-String und befüllt alle Felder.
     * @param content Dateiinhalt als String.
     * @return true bei Erfolg.
     */
    bool parseFromString(const QString &content);

    // ── Validierung ─────────────────────────────────────────────────────────

    /**
     * @brief Validiert den Eintrag gegen die Desktop Entry Spec 1.5.
     * @return ValidationResult mit Fehlern und Warnungen.
     */
    ValidationResult validate() const;

    // ── Allgemeine Pflichtfelder ─────────────────────────────────────────────

    Q_PROPERTY(Type    type    READ type    WRITE setType    NOTIFY entryChanged)
    Q_PROPERTY(QString name    READ name    WRITE setName    NOTIFY entryChanged)
    Q_PROPERTY(QString exec    READ exec    WRITE setExec    NOTIFY entryChanged)
    Q_PROPERTY(QString icon    READ icon    WRITE setIcon    NOTIFY entryChanged)
    Q_PROPERTY(QString version READ version WRITE setVersion NOTIFY entryChanged)

    Type    type()    const { return m_type; }
    QString name()    const { return m_name; }
    QString exec()    const { return m_exec; }
    QString icon()    const { return m_icon; }
    QString version() const { return m_version; }

    void setType   (Type    v);
    void setName   (const QString &v);
    void setExec   (const QString &v);
    void setIcon   (const QString &v);
    void setVersion(const QString &v);

    // ── Optionale Darstellungsfelder ─────────────────────────────────────────

    Q_PROPERTY(QString genericName   READ genericName   WRITE setGenericName   NOTIFY entryChanged)
    Q_PROPERTY(QString comment       READ comment       WRITE setComment       NOTIFY entryChanged)
    Q_PROPERTY(bool    noDisplay     READ noDisplay     WRITE setNoDisplay     NOTIFY entryChanged)
    Q_PROPERTY(bool    hidden        READ hidden        WRITE setHidden        NOTIFY entryChanged)
    Q_PROPERTY(QStringList keywords  READ keywords      WRITE setKeywords      NOTIFY entryChanged)
    Q_PROPERTY(QStringList onlyShowIn READ onlyShowIn   WRITE setOnlyShowIn   NOTIFY entryChanged)
    Q_PROPERTY(QStringList notShowIn READ notShowIn     WRITE setNotShowIn    NOTIFY entryChanged)

    QString    genericName() const { return m_genericName; }
    QString    comment()     const { return m_comment; }
    bool       noDisplay()   const { return m_noDisplay; }
    bool       hidden()      const { return m_hidden; }
    QStringList keywords()   const { return m_keywords; }
    QStringList onlyShowIn() const { return m_onlyShowIn; }
    QStringList notShowIn()  const { return m_notShowIn; }

    void setGenericName(const QString &v);
    void setComment    (const QString &v);
    void setNoDisplay  (bool v);
    void setHidden     (bool v);
    void setKeywords   (const QStringList &v);
    void setOnlyShowIn (const QStringList &v);
    void setNotShowIn  (const QStringList &v);

    // ── Optionale Ausführungsfelder ──────────────────────────────────────────

    Q_PROPERTY(QString tryExec        READ tryExec        WRITE setTryExec        NOTIFY entryChanged)
    Q_PROPERTY(QString path           READ path           WRITE setPath           NOTIFY entryChanged)
    Q_PROPERTY(bool    terminal       READ terminal       WRITE setTerminal       NOTIFY entryChanged)
    Q_PROPERTY(QStringList mimeTypes  READ mimeTypes      WRITE setMimeTypes      NOTIFY entryChanged)
    Q_PROPERTY(QStringList categories READ categories     WRITE setCategories     NOTIFY entryChanged)
    Q_PROPERTY(bool    startupNotify  READ startupNotify  WRITE setStartupNotify  NOTIFY entryChanged)
    Q_PROPERTY(QString startupWMClass READ startupWMClass WRITE setStartupWMClass NOTIFY entryChanged)
    Q_PROPERTY(QString url            READ url            WRITE setUrl            NOTIFY entryChanged)

    QString    tryExec()        const { return m_tryExec; }
    QString    path()           const { return m_path; }
    bool       terminal()       const { return m_terminal; }
    QStringList mimeTypes()     const { return m_mimeTypes; }
    QStringList categories()    const { return m_categories; }
    bool       startupNotify()  const { return m_startupNotify; }
    QString    startupWMClass() const { return m_startupWMClass; }
    QString    url()            const { return m_url; }

    void setTryExec       (const QString &v);
    void setPath          (const QString &v);
    void setTerminal      (bool v);
    void setMimeTypes     (const QStringList &v);
    void setCategories    (const QStringList &v);
    void setStartupNotify (bool v);
    void setStartupWMClass(const QString &v);
    void setUrl           (const QString &v);

    // ── Sprachvarianten ──────────────────────────────────────────────────────

    /** @brief Gibt den Wert eines lokalisierten Feldes zurück, z.B. getName("de"). */
    QString getLocalized(const QString &key, const QString &locale) const;
    /** @brief Setzt den Wert eines lokalisierten Feldes. */
    void    setLocalized(const QString &key, const QString &locale, const QString &value);
    /** @brief Gibt alle verfügbaren Locales für einen Key zurück. */
    QStringList localesForKey(const QString &key) const;

    // ── Actions ──────────────────────────────────────────────────────────────

    Q_PROPERTY(QList<Action> actions READ actions WRITE setActions NOTIFY entryChanged)
    QList<Action> actions() const { return m_actions; }
    void setActions(const QList<Action> &v);

    // ── Custom X- Felder ─────────────────────────────────────────────────────

    /** @brief Gibt den Wert eines X-* Feldes zurück. */
    QString    customField(const QString &key) const;
    /** @brief Setzt ein X-* Feld. */
    void       setCustomField(const QString &key, const QString &value);
    /** @brief Entfernt ein X-* Feld. */
    void       removeCustomField(const QString &key);
    /** @brief Gibt alle X-* Felder zurück. */
    QMap<QString, QString> customFields() const { return m_customFields; }

    // ── Hilfsmethoden ────────────────────────────────────────────────────────

    /** @brief Gibt true zurück wenn der Eintrag noch nicht aus einer Datei geladen wurde. */
    bool isNew()     const { return m_filePath.isEmpty(); }
    /** @brief Gibt den Pfad der zuletzt geladenen/gespeicherten Datei zurück. */
    QString filePath() const { return m_filePath; }

    /** @brief Setzt alle Felder auf ihre Standardwerte zurück. */
    void reset();

    /** @brief Gibt den Typen als String zurück ("Application", "Link", "Directory"). */
    static QString typeToString(Type t);
    /** @brief Gibt den Typen aus einem String zurück. */
    static Type    stringToType(const QString &s);

signals:
    /** @brief Wird bei jeder Feldänderung ausgelöst. */
    void entryChanged();

private:
    // Hilfsmethode: Schreibt eine Zeile mit ; oder ,-getrennten Listenfeldern
    static QString joinList(const QStringList &list);
    // Hilfsmethode: Parst eine ; oder ,-getrennte Listzeile
    static QStringList splitList(const QString &s);

    QString m_filePath;

    // Pflichtfelder
    Type    m_type        = Type::Application;
    QString m_name;
    QString m_exec;
    QString m_icon;
    QString m_version     = "1.5";

    // Optionale Darstellungsfelder
    QString     m_genericName;
    QString     m_comment;
    bool        m_noDisplay   = false;
    bool        m_hidden      = false;
    QStringList m_keywords;
    QStringList m_onlyShowIn;
    QStringList m_notShowIn;

    // Optionale Ausführungsfelder
    QString     m_tryExec;
    QString     m_path;
    bool        m_terminal      = false;
    QStringList m_mimeTypes;
    QStringList m_categories;
    bool        m_startupNotify = false;
    QString     m_startupWMClass;
    QString     m_url;

    // Sprachvarianten: key -> { locale -> value }
    QMap<QString, QMap<QString, QString>> m_localized;

    // Actions
    QList<Action> m_actions;

    // Custom X-* Felder
    QMap<QString, QString> m_customFields;
};
