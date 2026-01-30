#pragma once

#include <QString>
#include <QVector>
#include <QHash>

struct TranslationItem
{
    QString key;
    QString section;
    QString nameZh;
    QString descriptionZh;
};

class TranslationStore
{
public:
    bool load(const QString &path, QString *error);
    bool save(const QString &path, QString *error) const;

    bool contains(const QString &key) const;
    TranslationItem item(const QString &key) const;
    void upsert(const TranslationItem &item);
    QVector<TranslationItem> allItems() const;

private:
    QHash<QString, TranslationItem> m_items;
};
