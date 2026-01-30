#pragma once

#include <QDialog>

class QLineEdit;
class QPlainTextEdit;
class QPushButton;

struct ConfigEntry;

class EditEntryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit EditEntryDialog(QWidget *parent = nullptr);

    void setEntry(const ConfigEntry &entry);

    QString name() const;
    QString description() const;
    QString section() const;
    QString value() const;

private:
    void buildUi();

    QLineEdit *m_keyEdit = nullptr;
    QLineEdit *m_valueEdit = nullptr;
    QLineEdit *m_nameEdit = nullptr;
    QLineEdit *m_sectionEdit = nullptr;
    QPlainTextEdit *m_descEdit = nullptr;
    QPushButton *m_cancelButton = nullptr;
    QPushButton *m_applyButton = nullptr;
};
