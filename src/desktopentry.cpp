#include "desktopentry.h"

#include <QFile>
#include <QTextStream>
#include <QFileInfo>
#include <QRegularExpression>
#include <QDebug>

// ── Konstruktor ──────────────────────────────────────────────────────────────

DesktopEntry::DesktopEntry(QObject *parent)
    : QObject(parent)
{
}

// ── Laden / Speichern ────────────────────────────────────────────────────────

bool DesktopEntry::loadFromFile(const QString &path)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "DesktopEntry: Datei konnte nicht geöffnet werden:" << path;
        return false;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    const QString content = in.readAll();
    file.close();

    if (!parseFromString(content)) {
        return false;
    }

    m_filePath = path;
    return true;
}

bool DesktopEntry::saveToFile(const QString &path) const
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "DesktopEntry: Datei konnte nicht geschrieben werden:" << path;
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    out << toDesktopFormat();
    file.close();
    return true;
}

QString DesktopEntry::toDesktopFormat() const
{
    QString result;
    QTextStream out(&result);

    out << "[Desktop Entry]\n";
    out << "Version=" << m_version << "\n";
    out << "Type=" << typeToString(m_type) << "\n";
    out << "Name=" << m_name << "\n";

    // Lokalisierte Name-Varianten
    if (m_localized.contains("Name")) {
        const auto locales = m_localized.value("Name");
        for (auto it = locales.cbegin(); it != locales.cend(); ++it) {
            out << "Name[" << it.key() << "]=" << it.value() << "\n";
        }
    }

    if (!m_genericName.isEmpty()) {
        out << "GenericName=" << m_genericName << "\n";
        if (m_localized.contains("GenericName")) {
            const auto locales = m_localized.value("GenericName");
            for (auto it = locales.cbegin(); it != locales.cend(); ++it) {
                out << "GenericName[" << it.key() << "]=" << it.value() << "\n";
            }
        }
    }

    if (!m_comment.isEmpty()) {
        out << "Comment=" << m_comment << "\n";
        if (m_localized.contains("Comment")) {
            const auto locales = m_localized.value("Comment");
            for (auto it = locales.cbegin(); it != locales.cend(); ++it) {
                out << "Comment[" << it.key() << "]=" << it.value() << "\n";
            }
        }
    }

    if (!m_icon.isEmpty())
        out << "Icon=" << m_icon << "\n";

    if (!m_exec.isEmpty() && m_type != Type::Directory)
        out << "Exec=" << m_exec << "\n";

    if (!m_tryExec.isEmpty())
        out << "TryExec=" << m_tryExec << "\n";

    if (!m_path.isEmpty())
        out << "Path=" << m_path << "\n";

    if (m_terminal)
        out << "Terminal=true\n";

    if (m_noDisplay)
        out << "NoDisplay=true\n";

    if (m_hidden)
        out << "Hidden=true\n";

    if (!m_keywords.isEmpty())
        out << "Keywords=" << joinList(m_keywords) << "\n";

    if (!m_categories.isEmpty())
        out << "Categories=" << joinList(m_categories) << "\n";

    if (!m_mimeTypes.isEmpty())
        out << "MimeType=" << joinList(m_mimeTypes) << "\n";

    if (!m_onlyShowIn.isEmpty())
        out << "OnlyShowIn=" << joinList(m_onlyShowIn) << "\n";

    if (!m_notShowIn.isEmpty())
        out << "NotShowIn=" << joinList(m_notShowIn) << "\n";

    if (m_startupNotify)
        out << "StartupNotify=true\n";

    if (!m_startupWMClass.isEmpty())
        out << "StartupWMClass=" << m_startupWMClass << "\n";

    if (!m_url.isEmpty() && m_type == Type::Link)
        out << "URL=" << m_url << "\n";

    // X-* Felder
    for (auto it = m_customFields.cbegin(); it != m_customFields.cend(); ++it) {
        out << it.key() << "=" << it.value() << "\n";
    }

    // Actions-Liste
    if (!m_actions.isEmpty()) {
        QStringList actionIds;
        for (const auto &a : m_actions) {
            actionIds << a.id;
        }
        out << "Actions=" << actionIds.join(";") << ";\n";

        out << "\n";
        for (const auto &a : m_actions) {
            out << "[Desktop Action " << a.id << "]\n";
            out << "Name=" << a.name << "\n";
            if (!a.exec.isEmpty())
                out << "Exec=" << a.exec << "\n";
            if (!a.icon.isEmpty())
                out << "Icon=" << a.icon << "\n";
            out << "\n";
        }
    }

    return result;
}

bool DesktopEntry::parseFromString(const QString &content)
{
    reset();

    const QStringList lines = content.split('\n');
    QString currentGroup;
    QString currentActionId;

    static const QRegularExpression reLine(R"(^([A-Za-z0-9\-_]+(?:\[[^\]]+\])?)=(.*)$)");
    static const QRegularExpression reGroup(R"(^\[(.+)\]$)");

    for (const QString &rawLine : lines) {
        const QString line = rawLine.trimmed();
        if (line.isEmpty() || line.startsWith('#'))
            continue;

        // Gruppe / Sektion
        QRegularExpressionMatch mg = reGroup.match(line);
        if (mg.hasMatch()) {
            currentGroup = mg.captured(1);
            if (currentGroup.startsWith("Desktop Action ")) {
                currentActionId = currentGroup.mid(15).trimmed();
                Action a;
                a.id = currentActionId;
                m_actions.append(a);
            } else {
                currentActionId.clear();
            }
            continue;
        }

        // Schlüssel=Wert
        QRegularExpressionMatch ml = reLine.match(line);
        if (!ml.hasMatch())
            continue;

        const QString rawKey = ml.captured(1);
        const QString value  = ml.captured(2);

        // Lokalisierung auslesen, z.B. Name[de]
        QString key = rawKey;
        QString locale;
        static const QRegularExpression reLocale(R"(^(.+)\[([^\]]+)\]$)");
        QRegularExpressionMatch mloc = reLocale.match(rawKey);
        if (mloc.hasMatch()) {
            key    = mloc.captured(1);
            locale = mloc.captured(2);
        }

        // Action-Sektion?
        if (!currentActionId.isEmpty()) {
            if (!m_actions.isEmpty()) {
                auto &a = m_actions.last();
                if      (key == "Name") a.name = value;
                else if (key == "Exec") a.exec = value;
                else if (key == "Icon") a.icon = value;
            }
            continue;
        }

        // Desktop Entry-Sektion
        if (currentGroup != "Desktop Entry")
            continue;

        if (!locale.isEmpty()) {
            m_localized[key][locale] = value;
            continue;
        }

        if      (key == "Version")         m_version         = value;
        else if (key == "Type")            m_type            = stringToType(value);
        else if (key == "Name")            m_name            = value;
        else if (key == "GenericName")     m_genericName     = value;
        else if (key == "Comment")         m_comment         = value;
        else if (key == "Icon")            m_icon            = value;
        else if (key == "Exec")            m_exec            = value;
        else if (key == "TryExec")         m_tryExec         = value;
        else if (key == "Path")            m_path            = value;
        else if (key == "Terminal")        m_terminal        = (value.toLower() == "true");
        else if (key == "NoDisplay")       m_noDisplay       = (value.toLower() == "true");
        else if (key == "Hidden")          m_hidden          = (value.toLower() == "true");
        else if (key == "StartupNotify")   m_startupNotify   = (value.toLower() == "true");
        else if (key == "StartupWMClass")  m_startupWMClass  = value;
        else if (key == "URL")             m_url             = value;
        else if (key == "Keywords")        m_keywords        = splitList(value);
        else if (key == "Categories")      m_categories      = splitList(value);
        else if (key == "MimeType")        m_mimeTypes       = splitList(value);
        else if (key == "OnlyShowIn")      m_onlyShowIn      = splitList(value);
        else if (key == "NotShowIn")       m_notShowIn       = splitList(value);
        else if (key == "Actions") { /* wird über Sektionen aufgelöst */ }
        else if (key.startsWith("X-"))     m_customFields[key] = value;
    }

    return true;
}

// ── Validierung ──────────────────────────────────────────────────────────────

DesktopEntry::ValidationResult DesktopEntry::validate() const
{
    ValidationResult result;
    result.valid = true;

    // Pflichtfeld: Name
    if (m_name.trimmed().isEmpty()) {
        result.errors << tr("Pflichtfeld 'Name' fehlt.");
        result.valid = false;
    }

    // Pflichtfeld: Type
    // (immer gesetzt durch Enum-Default)

    // Type-spezifische Pflichtfelder
    if (m_type == Type::Application) {
        if (m_exec.trimmed().isEmpty()) {
            result.errors << tr("Pflichtfeld 'Exec' fehlt (Type=Application).");
            result.valid = false;
        }
    } else if (m_type == Type::Link) {
        if (m_url.trimmed().isEmpty()) {
            result.errors << tr("Pflichtfeld 'URL' fehlt (Type=Link).");
            result.valid = false;
        }
    }

    // Warnungen
    if (m_icon.trimmed().isEmpty()) {
        result.warnings << tr("Kein Icon angegeben.");
    }

    if (m_type == Type::Application && !m_exec.isEmpty()) {
        const QString execBin = m_exec.split(' ').first();
        QFileInfo execInfo(execBin);
        if (execBin.startsWith('/') && !execInfo.exists()) {
            result.warnings << tr("Exec-Pfad '%1' existiert nicht.").arg(execBin);
        }
    }

    if (!m_icon.isEmpty() && m_icon.startsWith('/')) {
        QFileInfo iconInfo(m_icon);
        if (!iconInfo.exists()) {
            result.warnings << tr("Icon-Pfad '%1' existiert nicht.").arg(m_icon);
        }
    }

    for (const auto &a : m_actions) {
        if (a.name.isEmpty()) {
            result.warnings << tr("Action '%1' hat keinen Namen.").arg(a.id);
        }
    }

    return result;
}

// ── Setter ───────────────────────────────────────────────────────────────────

#define EMIT_IF_CHANGED(member, value) \
    if (member != (value)) { member = (value); emit entryChanged(); }

void DesktopEntry::setType   (Type    v) { EMIT_IF_CHANGED(m_type,    v) }
void DesktopEntry::setName   (const QString &v) { EMIT_IF_CHANGED(m_name,    v) }
void DesktopEntry::setExec   (const QString &v) { EMIT_IF_CHANGED(m_exec,    v) }
void DesktopEntry::setIcon   (const QString &v) { EMIT_IF_CHANGED(m_icon,    v) }
void DesktopEntry::setVersion(const QString &v) { EMIT_IF_CHANGED(m_version, v) }

void DesktopEntry::setGenericName(const QString &v) { EMIT_IF_CHANGED(m_genericName, v) }
void DesktopEntry::setComment    (const QString &v) { EMIT_IF_CHANGED(m_comment,     v) }
void DesktopEntry::setNoDisplay  (bool v) { EMIT_IF_CHANGED(m_noDisplay, v) }
void DesktopEntry::setHidden     (bool v) { EMIT_IF_CHANGED(m_hidden,    v) }
void DesktopEntry::setKeywords   (const QStringList &v) { EMIT_IF_CHANGED(m_keywords,   v) }
void DesktopEntry::setOnlyShowIn (const QStringList &v) { EMIT_IF_CHANGED(m_onlyShowIn, v) }
void DesktopEntry::setNotShowIn  (const QStringList &v) { EMIT_IF_CHANGED(m_notShowIn,  v) }

void DesktopEntry::setTryExec       (const QString &v) { EMIT_IF_CHANGED(m_tryExec,        v) }
void DesktopEntry::setPath          (const QString &v) { EMIT_IF_CHANGED(m_path,            v) }
void DesktopEntry::setTerminal      (bool v) { EMIT_IF_CHANGED(m_terminal,      v) }
void DesktopEntry::setMimeTypes     (const QStringList &v) { EMIT_IF_CHANGED(m_mimeTypes,   v) }
void DesktopEntry::setCategories    (const QStringList &v) { EMIT_IF_CHANGED(m_categories,  v) }
void DesktopEntry::setStartupNotify (bool v) { EMIT_IF_CHANGED(m_startupNotify,  v) }
void DesktopEntry::setStartupWMClass(const QString &v) { EMIT_IF_CHANGED(m_startupWMClass, v) }
void DesktopEntry::setUrl           (const QString &v) { EMIT_IF_CHANGED(m_url,             v) }
void DesktopEntry::setActions       (const QList<Action> &v) { EMIT_IF_CHANGED(m_actions,   v) }

#undef EMIT_IF_CHANGED

// ── Sprachvarianten ──────────────────────────────────────────────────────────

QString DesktopEntry::getLocalized(const QString &key, const QString &locale) const
{
    return m_localized.value(key).value(locale, QString());
}

void DesktopEntry::setLocalized(const QString &key, const QString &locale, const QString &value)
{
    if (m_localized[key].value(locale) != value) {
        m_localized[key][locale] = value;
        emit entryChanged();
    }
}

QStringList DesktopEntry::localesForKey(const QString &key) const
{
    return m_localized.value(key).keys();
}

// ── Custom X-* Felder ────────────────────────────────────────────────────────

QString DesktopEntry::customField(const QString &key) const
{
    return m_customFields.value(key);
}

void DesktopEntry::setCustomField(const QString &key, const QString &value)
{
    if (m_customFields.value(key) != value) {
        m_customFields[key] = value;
        emit entryChanged();
    }
}

void DesktopEntry::removeCustomField(const QString &key)
{
    if (m_customFields.remove(key) > 0) {
        emit entryChanged();
    }
}

// ── Reset ────────────────────────────────────────────────────────────────────

void DesktopEntry::reset()
{
    m_filePath.clear();
    m_type           = Type::Application;
    m_name.clear();
    m_exec.clear();
    m_icon.clear();
    m_version        = "1.5";
    m_genericName.clear();
    m_comment.clear();
    m_noDisplay      = false;
    m_hidden         = false;
    m_keywords.clear();
    m_onlyShowIn.clear();
    m_notShowIn.clear();
    m_tryExec.clear();
    m_path.clear();
    m_terminal       = false;
    m_mimeTypes.clear();
    m_categories.clear();
    m_startupNotify  = false;
    m_startupWMClass.clear();
    m_url.clear();
    m_localized.clear();
    m_actions.clear();
    m_customFields.clear();
}

// ── Hilfsmethoden ────────────────────────────────────────────────────────────

QString DesktopEntry::typeToString(Type t)
{
    switch (t) {
        case Type::Application: return QStringLiteral("Application");
        case Type::Link:        return QStringLiteral("Link");
        case Type::Directory:   return QStringLiteral("Directory");
    }
    return QStringLiteral("Application");
}

DesktopEntry::Type DesktopEntry::stringToType(const QString &s)
{
    if (s == "Link")      return Type::Link;
    if (s == "Directory") return Type::Directory;
    return Type::Application;
}

QString DesktopEntry::joinList(const QStringList &list)
{
    // Freedesktop nutzt Semikolon als Trennzeichen, mit abschließendem Semikolon
    QString result = list.join(';');
    if (!result.isEmpty() && !result.endsWith(';'))
        result += ';';
    return result;
}

QStringList DesktopEntry::splitList(const QString &s)
{
    QStringList result;
    const QStringList parts = s.split(';');
    for (const QString &part : parts) {
        const QString trimmed = part.trimmed();
        if (!trimmed.isEmpty())
            result << trimmed;
    }
    return result;
}
