#include "editentrydialog.h"
#include "confparser.h"

#include <QApplication>
#include <QFormLayout>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScreen>
#include <QVBoxLayout>

EditEntryDialog::EditEntryDialog(QWidget *parent)
    : QDialog(parent)
{
    setObjectName("EditEntryDialog");
    setWindowTitle("编辑配置项");
    setModal(true);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);

    // Calculate dialog size based on screen resolution
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    // Target: 35% of screen width, 50% of screen height
    // with min/max constraints
    int dialogWidth = qBound(450, static_cast<int>(screenWidth * 0.35), 600);
    int dialogHeight = qBound(400, static_cast<int>(screenHeight * 0.5), 550);

    setFixedSize(dialogWidth, dialogHeight);

    buildUi();
}

void EditEntryDialog::buildUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 24, 24, 24);
    mainLayout->setSpacing(16);

    // Form
    QFormLayout *formLayout = new QFormLayout();
    formLayout->setSpacing(12);
    formLayout->setLabelAlignment(Qt::AlignRight);

    m_keyEdit = new QLineEdit(this);
    m_keyEdit->setObjectName("KeyEdit");
    m_keyEdit->setReadOnly(true);
    m_keyEdit->setPlaceholderText("配置键名");
    formLayout->addRow("键名:", m_keyEdit);

    m_valueEdit = new QLineEdit(this);
    m_valueEdit->setPlaceholderText("配置值");
    formLayout->addRow("数值:", m_valueEdit);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setPlaceholderText("显示名称");
    formLayout->addRow("名称:", m_nameEdit);

    m_sectionEdit = new QLineEdit(this);
    m_sectionEdit->setPlaceholderText("分类名称");
    formLayout->addRow("分类:", m_sectionEdit);

    m_descEdit = new QPlainTextEdit(this);
    m_descEdit->setPlaceholderText("配置项的描述说明...");
    m_descEdit->setMinimumHeight(100);
    formLayout->addRow("描述:", m_descEdit);

    mainLayout->addLayout(formLayout);
    mainLayout->addStretch();

    // Button row
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);
    buttonLayout->addStretch();

    m_cancelButton = new QPushButton("取消", this);
    m_cancelButton->setObjectName("GhostButton");
    m_cancelButton->setCursor(Qt::PointingHandCursor);
    buttonLayout->addWidget(m_cancelButton);

    m_applyButton = new QPushButton("应用", this);
    m_applyButton->setObjectName("PrimaryButton");
    m_applyButton->setCursor(Qt::PointingHandCursor);
    buttonLayout->addWidget(m_applyButton);

    mainLayout->addLayout(buttonLayout);

    // Connections
    connect(m_cancelButton, &QPushButton::clicked, this, &QDialog::reject);
    connect(m_applyButton, &QPushButton::clicked, this, &QDialog::accept);
}

void EditEntryDialog::setEntry(const ConfigEntry &entry)
{
    m_keyEdit->setText(entry.key);
    m_valueEdit->setText(entry.value);
    m_nameEdit->setText(entry.nameZh);
    m_sectionEdit->setText(entry.section);
    m_descEdit->setPlainText(entry.descriptionZh);
}

QString EditEntryDialog::name() const
{
    return m_nameEdit->text().trimmed();
}

QString EditEntryDialog::description() const
{
    return m_descEdit->toPlainText().trimmed();
}

QString EditEntryDialog::section() const
{
    return m_sectionEdit->text().trimmed();
}

QString EditEntryDialog::value() const
{
    return m_valueEdit->text().trimmed();
}
