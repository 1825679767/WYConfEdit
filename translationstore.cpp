#include "translationstore.h"

#include <algorithm>
#include <QFile>
#include <QTextStream>

static QString stripQuotes(const QString &value)
{
    if (value.size() >= 2)
    {
        if ((value.startsWith('"') && value.endsWith('"')) ||
            (value.startsWith('\'') && value.endsWith('\'')))
        {
            return value.mid(1, value.size() - 2);
        }
    }
    return value;
}

static QString formatYamlValue(const QString &value)
{
    bool needsQuotes = value.contains(':') || value.contains('#') || value.startsWith(' ') || value.endsWith(' ');
    if (!needsQuotes)
        return value;

    QString escaped = value;
    escaped.replace("\"", "\\\"");
    return QString("\"") + escaped + QString("\"");
}

static bool parseKeyValue(const QString &line, QString *keyOut, QString *valueOut)
{
    int colon = line.indexOf(':');
    if (colon < 0)
        return false;

    QString key = line.left(colon).trimmed();
    QString value = line.mid(colon + 1).trimmed();
    if (key.isEmpty())
        return false;

    if (keyOut)
        *keyOut = key;
    if (valueOut)
        *valueOut = stripQuotes(value);
    return true;
}

bool TranslationStore::load(const QString &path, QString *error)
{
    m_versions.clear();
    m_versionOrder.clear();
    m_currentVersion.clear();

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (error)
            *error = QString("Failed to open translation: %1").arg(path);
        return false;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    TranslationItem current;
    bool inItem = false;
    bool inMultilineDesc = false;
    int multilineIndent = 0;
    QStringList multilineLines;
    bool inVersions = false;
    bool inItems = false;
    bool inLegacyList = false;
    QString currentVersion;

    while (!in.atEnd())
    {
        QString line = in.readLine();
        QString trimmed = line.trimmed();
        int leadingSpaces = 0;
        while (leadingSpaces < line.size() && line[leadingSpaces] == ' ')
            leadingSpaces++;

        // Handle multiline description continuation
        if (inMultilineDesc)
        {
            // Check if this line is still part of the multiline block
            int indent = leadingSpaces;

            // If line is empty or has sufficient indentation, it's part of the block
            if (trimmed.isEmpty() || indent >= multilineIndent)
            {
                multilineLines.append(trimmed);
                continue;
            }
            else
            {
                // End of multiline block
                current.descriptionZh = multilineLines.join("\n");
                inMultilineDesc = false;
                multilineLines.clear();
            }
        }

        if (trimmed.isEmpty() || trimmed.startsWith('#'))
            continue;

        if (trimmed == "versions:")
        {
            inVersions = true;
            inItems = false;
            inLegacyList = false;
            continue;
        }

        if (inVersions)
        {
            // Version keys are only valid at indent level 2: "  'version':"
            if (leadingSpaces == 2 && trimmed.endsWith(':') && trimmed != "items:")
            {
                if (inItem && !current.key.isEmpty() && !currentVersion.isEmpty())
                {
                    m_versions[currentVersion].insert(current.key, current);
                    current = TranslationItem();
                    inItem = false;
                }
                // Validate that the next meaningful line is "items:" at indent 4
                qint64 lastPos = in.pos();
                QString nextLine;
                while (!in.atEnd())
                {
                    nextLine = in.readLine();
                    QString nextTrimmed = nextLine.trimmed();
                    if (nextTrimmed.isEmpty() || nextTrimmed.startsWith('#'))
                        continue;
                    int nextLeading = 0;
                    while (nextLeading < nextLine.size() && nextLine[nextLeading] == ' ')
                        nextLeading++;
                    bool validItems = (nextLeading == 4 && nextTrimmed == "items:");
                    in.seek(lastPos);
                    if (!validItems)
                        nextLine.clear();
                    break;
                }
                if (!nextLine.isEmpty())
                {
                    currentVersion = stripQuotes(trimmed.left(trimmed.size() - 1).trimmed());
                    if (!currentVersion.isEmpty() && !m_versions.contains(currentVersion))
                    {
                        m_versions.insert(currentVersion, QHash<QString, TranslationItem>());
                        m_versionOrder.append(currentVersion);
                    }
                    inItems = false;
                    continue;
                }
            }
            // "items:" is only valid under a version at indent level 4
            if (leadingSpaces == 4 && trimmed == "items:")
            {
                inItems = true;
                continue;
            }
        }

        if (trimmed.startsWith('-'))
        {
            if (!inVersions)
            {
                if (!inLegacyList)
                {
                    currentVersion = "default";
                    m_versions.insert(currentVersion, QHash<QString, TranslationItem>());
                    m_versionOrder.append(currentVersion);
                    inLegacyList = true;
                }
                inItems = true;
            }
            else if (!inItems)
            {
                continue;
            }

            if (inItem && !current.key.isEmpty())
                m_versions[currentVersion].insert(current.key, current);

            current = TranslationItem();
            inItem = true;

            QString rest = trimmed.mid(1).trimmed();
            if (!rest.isEmpty())
            {
                QString key;
                QString value;
                if (parseKeyValue(rest, &key, &value))
                {
                    if (key == "key")
                        current.key = value;
                }
            }
            continue;
        }

        if (!inItem)
            continue;

        QString key;
        QString value;
        if (!parseKeyValue(trimmed, &key, &value))
            continue;

        if (key == "key")
            current.key = value;
        else if (key == "section")
            current.section = value;
        else if (key == "name_zh")
            current.nameZh = value;
        else if (key == "description_zh")
        {
            if (value == "|" || value == ">")
            {
                // Start multiline block
                inMultilineDesc = true;
                multilineIndent = leadingSpaces + 2; // YAML block indent is parent indent + 2
                multilineLines.clear();
            }
            else
            {
                current.descriptionZh = value;
            }
        }
    }

    // Handle end of file with multiline description
    if (inMultilineDesc && !multilineLines.isEmpty())
    {
        current.descriptionZh = multilineLines.join("\n");
    }

    if (inItem && !current.key.isEmpty())
        m_versions[currentVersion].insert(current.key, current);

    if (!m_versionOrder.isEmpty())
        m_currentVersion = m_versionOrder.first();

    return true;
}

bool TranslationStore::save(const QString &path, QString *error) const
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        if (error)
            *error = QString("Failed to write translation: %1").arg(path);
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");

    out << "versions:\n";

    QStringList versions = m_versionOrder;
    if (versions.isEmpty())
        versions = m_versions.keys();

    for (const QString &version : versions)
    {
        if (!m_versions.contains(version))
            continue;
        out << "  " << formatYamlValue(version) << ":\n";
        out << "    items:\n";

        QVector<TranslationItem> items;
        items.reserve(m_versions[version].size());
        for (const TranslationItem &item : m_versions[version])
            items.push_back(item);
        std::sort(items.begin(), items.end(), [](const TranslationItem &a, const TranslationItem &b) {
            return a.key < b.key;
        });

        for (const TranslationItem &item : items)
        {
            out << "    - key: " << formatYamlValue(item.key) << "\n";
            if (!item.section.isEmpty())
                out << "      section: " << formatYamlValue(item.section) << "\n";
            if (!item.nameZh.isEmpty())
                out << "      name_zh: " << formatYamlValue(item.nameZh) << "\n";
            if (!item.descriptionZh.isEmpty())
            {
                if (item.descriptionZh.contains('\n'))
                {
                    out << "      description_zh: |\n";
                    QStringList lines = item.descriptionZh.split('\n');
                    for (const QString &line : lines)
                    {
                        out << "        " << line << "\n";
                    }
                }
                else
                {
                    out << "      description_zh: " << formatYamlValue(item.descriptionZh) << "\n";
                }
            }
        }
    }

    return true;
}

QStringList TranslationStore::availableVersions() const
{
    if (!m_versionOrder.isEmpty())
        return m_versionOrder;
    return m_versions.keys();
}

QString TranslationStore::currentVersion() const
{
    return m_currentVersion;
}

bool TranslationStore::setCurrentVersion(const QString &version)
{
    if (!m_versions.contains(version))
        return false;
    m_currentVersion = version;
    return true;
}

bool TranslationStore::contains(const QString &key) const
{
    return m_versions.contains(m_currentVersion) && m_versions[m_currentVersion].contains(key);
}

TranslationItem TranslationStore::item(const QString &key) const
{
    if (!m_versions.contains(m_currentVersion))
        return TranslationItem();
    return m_versions[m_currentVersion].value(key);
}

void TranslationStore::upsert(const TranslationItem &item)
{
    if (item.key.isEmpty())
        return;
    if (m_currentVersion.isEmpty())
    {
        m_currentVersion = "default";
        if (!m_versions.contains(m_currentVersion))
            m_versions.insert(m_currentVersion, QHash<QString, TranslationItem>());
        if (!m_versionOrder.contains(m_currentVersion))
            m_versionOrder.append(m_currentVersion);
    }
    m_versions[m_currentVersion].insert(item.key, item);
}

QVector<TranslationItem> TranslationStore::allItems() const
{
    QVector<TranslationItem> items;
    if (!m_versions.contains(m_currentVersion))
        return items;

    items.reserve(m_versions[m_currentVersion].size());
    for (const TranslationItem &item : m_versions[m_currentVersion])
        items.push_back(item);

    std::sort(items.begin(), items.end(), [](const TranslationItem &a, const TranslationItem &b) {
        return a.key < b.key;
    });
    return items;
}
