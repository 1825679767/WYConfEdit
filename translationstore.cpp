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
    m_items.clear();

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

    while (!in.atEnd())
    {
        QString line = in.readLine();
        QString trimmed = line.trimmed();

        // Handle multiline description continuation
        if (inMultilineDesc)
        {
            // Check if this line is still part of the multiline block
            int indent = 0;
            for (int i = 0; i < line.length(); ++i)
            {
                if (line[i] == ' ')
                    indent++;
                else
                    break;
            }

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

        if (trimmed.startsWith('-'))
        {
            if (inItem && !current.key.isEmpty())
                m_items.insert(current.key, current);

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
                multilineIndent = 4; // Assume 4 spaces indentation
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
        m_items.insert(current.key, current);

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
    QVector<TranslationItem> items = allItems();

    for (const TranslationItem &item : items)
    {
        out << "- key: " << formatYamlValue(item.key) << "\n";
        if (!item.section.isEmpty())
            out << "  section: " << formatYamlValue(item.section) << "\n";
        if (!item.nameZh.isEmpty())
            out << "  name_zh: " << formatYamlValue(item.nameZh) << "\n";
        if (!item.descriptionZh.isEmpty())
        {
            // Check if description contains newlines
            if (item.descriptionZh.contains('\n'))
            {
                out << "  description_zh: |\n";
                QStringList lines = item.descriptionZh.split('\n');
                for (const QString &line : lines)
                {
                    out << "    " << line << "\n";
                }
            }
            else
            {
                out << "  description_zh: " << formatYamlValue(item.descriptionZh) << "\n";
            }
        }
    }

    return true;
}

bool TranslationStore::contains(const QString &key) const
{
    return m_items.contains(key);
}

TranslationItem TranslationStore::item(const QString &key) const
{
    return m_items.value(key);
}

void TranslationStore::upsert(const TranslationItem &item)
{
    if (item.key.isEmpty())
        return;
    m_items.insert(item.key, item);
}

QVector<TranslationItem> TranslationStore::allItems() const
{
    QVector<TranslationItem> items;
    items.reserve(m_items.size());
    for (const TranslationItem &item : m_items)
        items.push_back(item);

    std::sort(items.begin(), items.end(), [](const TranslationItem &a, const TranslationItem &b) {
        return a.key < b.key;
    });
    return items;
}
