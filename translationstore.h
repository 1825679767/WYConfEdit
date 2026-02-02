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

    QStringList availableVersions() const;
    QString currentVersion() const;
    bool setCurrentVersion(const QString &version);

    bool contains(const QString &key) const;
    TranslationItem item(const QString &key) const;
    void upsert(const TranslationItem &item);
    QVector<TranslationItem> allItems() const;

private:
    bool loadFromYaml(const QString &path, QString *error);
    bool loadFromSqlite(const QString &path, QString *error);
    bool saveToYaml(const QString &path, QString *error) const;
    bool saveToSqlite(const QString &path, QString *error) const;

    QHash<QString, QHash<QString, TranslationItem>> m_versions;
    QStringList m_versionOrder;
    QString m_currentVersion;
};
