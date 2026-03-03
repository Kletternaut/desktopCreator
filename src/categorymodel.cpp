#include "categorymodel.h"

CategoryModel::CategoryModel(QObject *parent)
    : QAbstractListModel(parent)
{
    initCategories();
}

void CategoryModel::initCategories()
{
    m_items = {
        { "AudioVideo",   tr("Audio & Video")         },
        { "Audio",        tr("Audio")                 },
        { "Video",        tr("Video")                 },
        { "Development",  tr("Entwicklung")           },
        { "Education",    tr("Bildung")               },
        { "Game",         tr("Spiele")                },
        { "Graphics",     tr("Grafik")                },
        { "Network",      tr("Internet & Netzwerk")   },
        { "Office",       tr("Büro")                  },
        { "Science",      tr("Wissenschaft")          },
        { "Settings",     tr("Einstellungen")         },
        { "System",       tr("System")                },
        { "Utility",      tr("Zubehör / Hilfsprogramme") },
    };
}

int CategoryModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return m_items.size();
}

QVariant CategoryModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() >= m_items.size())
        return {};

    const auto &item = m_items.at(index.row());

    switch (role) {
    case Qt::DisplayRole:
        return QStringLiteral("%1  (%2)").arg(item.name, item.description);
    case Qt::CheckStateRole:
        return item.checked ? Qt::Checked : Qt::Unchecked;
    case Qt::UserRole:
        return item.name;   // Roher Kategoriename zur Weiterverarbeitung
    default:
        return {};
    }
}

bool CategoryModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || index.row() >= m_items.size())
        return false;

    if (role == Qt::CheckStateRole) {
        m_items[index.row()].checked = (value.toInt() == Qt::Checked);
        emit dataChanged(index, index, { Qt::CheckStateRole });
        return true;
    }
    return false;
}

Qt::ItemFlags CategoryModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable | Qt::ItemIsSelectable;
}

QStringList CategoryModel::checkedCategories() const
{
    QStringList result;
    for (const auto &item : m_items) {
        if (item.checked)
            result << item.name;
    }
    return result;
}

void CategoryModel::setCheckedCategories(const QStringList &categories)
{
    for (auto &item : m_items) {
        item.checked = categories.contains(item.name);
    }
    if (!m_items.isEmpty()) {
        emit dataChanged(index(0), index(m_items.size() - 1), { Qt::CheckStateRole });
    }
}

QStringList CategoryModel::allCategories() const
{
    QStringList result;
    for (const auto &item : m_items)
        result << item.name;
    return result;
}
