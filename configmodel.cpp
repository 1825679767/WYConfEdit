#include "configmodel.h"

#include <QStringList>

ConfigModel::ConfigModel(QObject *parent)
    : QAbstractTableModel(parent)
{
}

void ConfigModel::setEntries(QVector<ConfigEntry> *entries)
{
    beginResetModel();
    m_entries = entries;
    endResetModel();
}

int ConfigModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() || !m_entries)
        return 0;
    return m_entries->size();
}

int ConfigModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;
    return 3;
}

QVariant ConfigModel::data(const QModelIndex &index, int role) const
{
    if (!m_entries || !index.isValid())
        return QVariant();

    const ConfigEntry &entry = (*m_entries)[index.row()];

    if (role == Qt::DisplayRole)
    {
        switch (index.column())
        {
        case 0: return entry.key;
        case 1: return entry.nameZh;
        case 2: return entry.value;
        default: break;
        }
    }

    if (role == Qt::ToolTipRole)
    {
        // Only show tooltip on value column (column 2)
        if (index.column() == 2 && !entry.descriptionZh.isEmpty())
            return entry.descriptionZh;
        return QVariant();
    }

    if (role == Qt::TextAlignmentRole)
    {
        switch (index.column())
        {
        case 0: return QVariant(Qt::AlignLeft | Qt::AlignVCenter);
        case 1: return QVariant(Qt::AlignCenter);
        case 2: return QVariant(Qt::AlignCenter);
        default: break;
        }
    }

    return QVariant();
}

QVariant ConfigModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal)
    {
        if (role == Qt::DisplayRole)
        {
            switch (section)
            {
            case 0: return QString("键名");
            case 1: return QString("名称");
            case 2: return QString("数值");
            default: break;
            }
        }
        if (role == Qt::TextAlignmentRole)
        {
            return QVariant(Qt::AlignCenter);
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags ConfigModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled;
}

const ConfigEntry &ConfigModel::entryAt(int row) const
{
    static ConfigEntry empty;
    if (!m_entries || row < 0 || row >= m_entries->size())
        return empty;
    return (*m_entries)[row];
}

void ConfigModel::notifyRowChanged(int row)
{
    if (!m_entries)
        return;
    QModelIndex left = index(row, 0);
    QModelIndex right = index(row, columnCount() - 1);
    emit dataChanged(left, right, {Qt::DisplayRole});
}

ConfigFilterProxy::ConfigFilterProxy(QObject *parent)
    : QSortFilterProxyModel(parent)
{
}

void ConfigFilterProxy::setSearchText(const QString &text)
{
    m_searchText = text.trimmed();
    invalidateFilter();
}

void ConfigFilterProxy::setSectionFilter(const QString &section)
{
    m_sectionFilter = section.trimmed();
    invalidateFilter();
}

bool ConfigFilterProxy::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent)

    const ConfigModel *model = qobject_cast<const ConfigModel *>(sourceModel());
    if (!model)
        return true;

    const ConfigEntry &entry = model->entryAt(sourceRow);

    if (!m_sectionFilter.isEmpty())
    {
        if (m_sectionFilter == QStringLiteral("未分类"))
            return entry.section.isEmpty();
        if (entry.section != m_sectionFilter)
            return false;
    }

    if (m_searchText.isEmpty())
        return true;

    const QString haystack = QString("%1 %2 %3").arg(entry.key, entry.nameZh, entry.descriptionZh).toLower();
    return haystack.contains(m_searchText.toLower());
}
