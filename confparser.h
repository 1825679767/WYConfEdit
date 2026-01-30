#pragma once

#include <QString>
#include <QVector>
#include <QHash>

struct ConfLine
{
    enum Type
    {
        Blank,
        Comment,
        KeyValue,
        Other
    };

    Type type = Other;
    QString raw;
    QString key;
    QString value;
    QString prefix;
    QString suffix;
    QString valueTrailingSpace;
    QString section;
    bool hasNewValue = false;
    QString newValue;
};

struct ConfigEntry
{
    QString key;
    QString section;
    QString nameZh;
    QString descriptionZh;
    QString value;
    int lineIndex = -1;
};

class ConfParser
{
public:
    bool load(const QString &path, QString *error);
    bool save(const QString &path, QString *error);

    const QVector<ConfLine> &lines() const { return m_lines; }
    QVector<ConfLine> &lines() { return m_lines; }

    const QVector<ConfigEntry> &entries() const { return m_entries; }
    QVector<ConfigEntry> &entries() { return m_entries; }

    void setEntryValue(int entryIndex, const QString &value);

private:
    bool isSectionHeader(const QString &line, QString *sectionOut) const;
    bool parseKeyValueLine(const QString &line, ConfLine *out) const;

    QVector<ConfLine> m_lines;
    QVector<ConfigEntry> m_entries;
};
