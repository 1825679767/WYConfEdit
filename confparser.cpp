#include "confparser.h"

#include <QFile>
#include <QTextStream>

static QString trimCommentPrefix(const QString &line)
{
    int i = 0;
    while (i < line.size() && line[i].isSpace())
        ++i;
    if (i < line.size() && line[i] == '#')
        ++i;
    while (i < line.size() && line[i].isSpace())
        ++i;
    return line.mid(i);
}

bool ConfParser::load(const QString &path, QString *error)
{
    m_lines.clear();
    m_entries.clear();

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        if (error)
            *error = QString("Failed to open config: %1").arg(path);
        return false;
    }

    QTextStream in(&file);
    in.setCodec("UTF-8");
    QString currentSection;

    int lineIndex = 0;
    while (!in.atEnd())
    {
        QString line = in.readLine();
        ConfLine cl;
        cl.raw = line;

        QString trimmed = line.trimmed();
        if (trimmed.isEmpty())
        {
            cl.type = ConfLine::Blank;
        }
        else if (trimmed.startsWith('#') || trimmed.startsWith(';'))
        {
            cl.type = ConfLine::Comment;
            QString sectionText;
            if (isSectionHeader(line, &sectionText))
                currentSection = sectionText;
        }
        else if (parseKeyValueLine(line, &cl))
        {
            cl.type = ConfLine::KeyValue;
            cl.section = currentSection;

            ConfigEntry entry;
            entry.key = cl.key;
            entry.section = currentSection;
            entry.value = cl.value;
            entry.lineIndex = lineIndex;
            m_entries.push_back(entry);
        }
        else
        {
            cl.type = ConfLine::Other;
        }

        m_lines.push_back(cl);
        ++lineIndex;
    }

    return true;
}

void ConfParser::setEntryValue(int entryIndex, const QString &value)
{
    if (entryIndex < 0 || entryIndex >= m_entries.size())
        return;

    ConfigEntry &entry = m_entries[entryIndex];
    entry.value = value;

    if (entry.lineIndex < 0 || entry.lineIndex >= m_lines.size())
        return;

    ConfLine &line = m_lines[entry.lineIndex];
    if (line.type != ConfLine::KeyValue)
        return;

    line.hasNewValue = true;
    line.newValue = value;
}

bool ConfParser::save(const QString &path, QString *error)
{
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        if (error)
            *error = QString("Failed to write config: %1").arg(path);
        return false;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");
    for (const ConfLine &line : m_lines)
    {
        if (line.type == ConfLine::KeyValue && line.hasNewValue && line.newValue != line.value)
        {
            out << line.prefix << line.newValue << line.valueTrailingSpace << line.suffix << "\n";
        }
        else
        {
            out << line.raw << "\n";
        }
    }
    return true;
}

bool ConfParser::isSectionHeader(const QString &line, QString *sectionOut) const
{
    QString text = trimCommentPrefix(line).trimmed();
    if (text.isEmpty())
        return false;

    if (text.contains(':'))
        return false;

    bool hasLetter = false;
    bool allUpper = true;
    for (QChar ch : text)
    {
        if (ch.isLetter())
        {
            hasLetter = true;
            if (ch.toUpper() != ch)
                allUpper = false;
        }
    }

    if (!hasLetter || !allUpper)
        return false;

    if (text.size() < 3 || text.size() > 80)
        return false;

    if (sectionOut)
        *sectionOut = text;
    return true;
}

bool ConfParser::parseKeyValueLine(const QString &line, ConfLine *out) const
{
    int eq = line.indexOf('=');
    if (eq < 0)
        return false;

    QString left = line.left(eq).trimmed();
    if (left.isEmpty())
        return false;

    int valueStart = eq + 1;
    while (valueStart < line.size() && line[valueStart].isSpace())
        ++valueStart;

    bool inQuotes = false;
    int commentPos = -1;
    for (int i = valueStart; i < line.size(); ++i)
    {
        QChar ch = line[i];
        if (ch == '"' && (i == 0 || line[i - 1] != '\\'))
            inQuotes = !inQuotes;
        if (ch == '#' && !inQuotes)
        {
            commentPos = i;
            break;
        }
    }

    int valueEnd = (commentPos == -1) ? line.size() : commentPos;
    QString valueRaw = line.mid(valueStart, valueEnd - valueStart);

    int trimEnd = valueRaw.size();
    while (trimEnd > 0 && valueRaw[trimEnd - 1].isSpace())
        --trimEnd;
    QString valueTrailingSpace = valueRaw.mid(trimEnd);

    if (out)
    {
        out->key = left;
        out->value = valueRaw.trimmed();
        out->prefix = line.left(valueStart);
        out->suffix = line.mid(valueEnd);
        out->valueTrailingSpace = valueTrailingSpace;
    }

    return true;
}
