#pragma once

#include <QAbstractTableModel>
#include <QSortFilterProxyModel>

#include "confparser.h"

class ConfigModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ConfigModel(QObject *parent = nullptr);

    void setEntries(QVector<ConfigEntry> *entries);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;

    const ConfigEntry &entryAt(int row) const;
    void notifyRowChanged(int row);

private:
    QVector<ConfigEntry> *m_entries = nullptr;
};

class ConfigFilterProxy : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    explicit ConfigFilterProxy(QObject *parent = nullptr);

    void setSearchText(const QString &text);
    void setSectionFilter(const QString &section);

protected:
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;

private:
    QString m_searchText;
    QString m_sectionFilter;
};
