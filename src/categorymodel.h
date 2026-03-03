#pragma once

#include <QAbstractListModel>
#include <QStringList>

/**
 * @brief Listenmodell für freedesktop.org Desktop Entry Kategorien.
 *
 * Stellt alle Standard-Hauptkategorien bereit und ermöglicht
 * Multi-Select via Checkboxen.
 */
class CategoryModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit CategoryModel(QObject *parent = nullptr);

    // QAbstractListModel interface
    int      rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool     setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    /** @brief Gibt alle aktuell ausgewählten Kategorien zurück. */
    QStringList checkedCategories() const;

    /** @brief Setzt die ausgewählten Kategorien. */
    void setCheckedCategories(const QStringList &categories);

    /** @brief Gibt alle verfügbaren Kategorien zurück. */
    QStringList allCategories() const;

private:
    struct CategoryItem {
        QString  name;
        QString  description;
        bool     checked = false;
    };

    QList<CategoryItem> m_items;

    void initCategories();
};
