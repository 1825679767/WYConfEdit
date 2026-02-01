#include "mainwindow.h"
#include "editentrydialog.h"

#include <QApplication>
#include <QCloseEvent>
#include <QComboBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QFrame>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QIcon>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPushButton>
#include <QScreen>
#include <QSet>
#include <QSettings>
#include <QTableView>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("WY配置编辑器");
    setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    buildUi();
    applyGlobalStyles();
    loadDefaultFiles();
}

void MainWindow::buildUi()
{
    QWidget *central = new QWidget(this);
    central->setObjectName("CentralWidget");

    QVBoxLayout *rootLayout = new QVBoxLayout(central);
    rootLayout->setContentsMargins(20, 20, 20, 20);
    rootLayout->setSpacing(0);

    // Main container with rounded corners
    QWidget *container = new QWidget(this);
    container->setObjectName("MainContainer");

    QVBoxLayout *containerLayout = new QVBoxLayout(container);
    containerLayout->setContentsMargins(0, 0, 0, 0);
    containerLayout->setSpacing(0);

    // ========================================
    // TOP BAR - Window controls
    // ========================================
    m_topBar = new QWidget(this);
    m_topBar->setObjectName("TopBar");
    m_topBar->setFixedHeight(60);

    QHBoxLayout *topBarLayout = new QHBoxLayout(m_topBar);
    topBarLayout->setContentsMargins(20, 0, 12, 0);
    topBarLayout->setSpacing(8);

    // Logo icon
    QLabel *logoLabel = new QLabel(this);
    QIcon logoIcon(":/logo.ico");
    logoLabel->setPixmap(logoIcon.pixmap(28, 28));
    logoLabel->setFixedSize(28, 28);
    topBarLayout->addWidget(logoLabel);

    QLabel *appTitle = new QLabel("WY配置编辑器", this);
    appTitle->setObjectName("AppTitle");
    topBarLayout->addWidget(appTitle);

    // File path label
    m_filePathLabel = new QLabel(this);
    m_filePathLabel->setObjectName("FilePathLabel");
    topBarLayout->addWidget(m_filePathLabel);

    topBarLayout->addStretch();

    QLabel *versionLabel = new QLabel("版本", this);
    versionLabel->setObjectName("VersionLabel");
    topBarLayout->addWidget(versionLabel);

    m_versionCombo = new QComboBox(this);
    m_versionCombo->setObjectName("VersionCombo");
    m_versionCombo->setCursor(Qt::PointingHandCursor);
    topBarLayout->addWidget(m_versionCombo);

    topBarLayout->addSpacing(12);

    QLabel *adLabel = new QLabel("WY技术交流群:738942437", this);
    adLabel->setObjectName("AdLabel");
    topBarLayout->addWidget(adLabel);

    topBarLayout->addStretch();

    QPushButton *minButton = new QPushButton("—", this);
    minButton->setObjectName("WindowButton");
    minButton->setFixedSize(36, 36);
    minButton->setCursor(Qt::PointingHandCursor);
    topBarLayout->addWidget(minButton);

    QPushButton *closeButton = new QPushButton("×", this);
    closeButton->setObjectName("CloseButton");
    closeButton->setFixedSize(36, 36);
    closeButton->setCursor(Qt::PointingHandCursor);
    topBarLayout->addWidget(closeButton);

    containerLayout->addWidget(m_topBar);

    // ========================================
    // CONTENT AREA
    // ========================================
    QWidget *contentArea = new QWidget(this);
    contentArea->setObjectName("ContentArea");

    QHBoxLayout *mainLayout = new QHBoxLayout(contentArea);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // LEFT PANEL - Navigation
    QWidget *leftPanel = new QWidget(this);
    leftPanel->setObjectName("LeftPanel");
    leftPanel->setFixedWidth(200);

    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(12, 6, 12, 12);
    leftLayout->setSpacing(0);

    m_sectionList = new QListWidget(this);
    m_sectionList->setObjectName("SectionList");
    leftLayout->addWidget(m_sectionList);

    mainLayout->addWidget(leftPanel);

    // RIGHT PANEL - Content Area
    QWidget *rightPanel = new QWidget(this);
    rightPanel->setObjectName("RightPanel");

    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(16, 12, 16, 16);
    rightLayout->setSpacing(12);

    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    toolbarLayout->setSpacing(8);

    m_searchEdit = new QLineEdit(this);
    m_searchEdit->setObjectName("SearchBox");
    m_searchEdit->setPlaceholderText("搜索配置项...");
    toolbarLayout->addWidget(m_searchEdit, 1);

    QPushButton *openButton = new QPushButton("打开配置", this);
    openButton->setObjectName("GhostButton");
    openButton->setCursor(Qt::PointingHandCursor);
    toolbarLayout->addWidget(openButton);

    QPushButton *saveButton = new QPushButton("保存", this);
    saveButton->setObjectName("PrimaryButton");
    saveButton->setCursor(Qt::PointingHandCursor);
    toolbarLayout->addWidget(saveButton);

    rightLayout->addLayout(toolbarLayout);

    m_table = new QTableView(this);
    m_table->setObjectName("ConfigTable");
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setAlternatingRowColors(true);
    m_table->horizontalHeader()->setVisible(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->verticalHeader()->setDefaultSectionSize(36);
    m_table->setShowGrid(true);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    m_table->horizontalHeader()->setDefaultAlignment(Qt::AlignCenter);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setFocusPolicy(Qt::NoFocus);
    rightLayout->addWidget(m_table);

    mainLayout->addWidget(rightPanel, 1);

    containerLayout->addWidget(contentArea, 1);

    rootLayout->addWidget(container);

    setCentralWidget(central);

    // Calculate window size based on screen resolution
    QScreen *screen = QApplication::primaryScreen();
    QRect screenGeometry = screen->availableGeometry();
    int screenWidth = screenGeometry.width();
    int screenHeight = screenGeometry.height();

    // Target: 60% of screen width, 75% of screen height
    // with min/max constraints
    int windowWidth = qBound(1000, static_cast<int>(screenWidth * 0.6), 1600);
    int windowHeight = qBound(700, static_cast<int>(screenHeight * 0.75), 1000);

    setFixedSize(windowWidth, windowHeight);

    m_model = new ConfigModel(this);
    m_proxy = new ConfigFilterProxy(this);
    m_proxy->setSourceModel(m_model);
    m_table->setModel(m_proxy);

    // Calculate column widths based on window width
    int tableWidth = windowWidth - 250; // Subtract left panel and margins
    m_table->horizontalHeader()->resizeSection(0, static_cast<int>(tableWidth * 0.35));
    m_table->horizontalHeader()->resizeSection(1, static_cast<int>(tableWidth * 0.25));
    m_table->horizontalHeader()->resizeSection(2, static_cast<int>(tableWidth * 0.20));

    connect(m_searchEdit, &QLineEdit::textChanged,
            this, &MainWindow::onSearchChanged);
    connect(m_sectionList, &QListWidget::currentItemChanged,
            this, &MainWindow::onSectionChanged);
    connect(m_versionCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onVersionChanged);
    connect(m_table, &QTableView::doubleClicked,
            this, &MainWindow::onTableDoubleClicked);
    connect(openButton, &QPushButton::clicked,
            this, &MainWindow::onOpenConfig);
    connect(saveButton, &QPushButton::clicked,
            this, &MainWindow::onSaveAll);
    connect(minButton, &QPushButton::clicked,
            this, &MainWindow::showMinimized);
    connect(closeButton, &QPushButton::clicked,
            this, &MainWindow::close);
}

void MainWindow::applyGlobalStyles()
{
    QString styleSheet = R"(
        QWidget#CentralWidget {
            background: transparent;
        }
        QWidget#MainContainer {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #a8edea, stop:0.4 #fff0f5, stop:0.7 #f5e6f0, stop:1 #fff5f8);
            border-radius: 20px;
        }
        QWidget#TopBar {
            background: transparent;
            border-top-left-radius: 20px;
            border-top-right-radius: 20px;
        }
        QLabel#AppTitle {
            color: rgba(80, 60, 80, 0.9);
            font-size: 18px;
            font-weight: 600;
        }
        QLabel#FilePathLabel {
            color: rgba(80, 60, 80, 0.5);
            font-size: 13px;
            margin-left: 12px;
        }
        QLabel#VersionLabel {
            color: rgba(80, 60, 80, 0.7);
            font-size: 13px;
            margin-right: 6px;
        }
        QComboBox#VersionCombo {
            background-color: rgba(255, 255, 255, 0.5);
            border: 1px solid rgba(255, 255, 255, 0.7);
            border-radius: 10px;
            padding: 6px 10px;
            color: rgba(80, 60, 80, 0.9);
            font-size: 13px;
            min-width: 120px;
        }
        QComboBox#VersionCombo:hover {
            background-color: rgba(255, 255, 255, 0.6);
        }
        QComboBox#VersionCombo::drop-down {
            border: none;
            width: 18px;
        }
        QComboBox#VersionCombo QAbstractItemView {
            background-color: rgba(255, 255, 255, 0.95);
            border: 1px solid rgba(210, 153, 194, 0.4);
            selection-background-color: rgba(210, 153, 194, 0.25);
            selection-color: rgba(80, 60, 80, 0.95);
        }
        QPushButton#WindowButton {
            background-color: rgba(80, 60, 80, 0.08);
            border: none;
            border-radius: 18px;
            color: rgba(80, 60, 80, 0.6);
            font-size: 16px;
            font-weight: 400;
        }
        QPushButton#WindowButton:hover {
            background-color: rgba(80, 60, 80, 0.15);
        }
        QPushButton#CloseButton {
            background-color: rgba(80, 60, 80, 0.08);
            border: none;
            border-radius: 18px;
            color: rgba(80, 60, 80, 0.6);
            font-size: 20px;
            font-weight: 300;
        }
        QPushButton#CloseButton:hover {
            background-color: #ef5350;
            color: #ffffff;
        }
        QWidget#ContentArea {
            background: transparent;
        }
        QWidget#LeftPanel {
            background-color: rgba(255, 255, 255, 0.4);
            border: 1px solid rgba(255, 255, 255, 0.6);
            border-radius: 16px;
            margin: 0px 8px 12px 12px;
        }
        QListWidget#SectionList {
            background-color: transparent;
            border: none;
            outline: none;
            color: rgba(80, 60, 80, 0.9);
            font-size: 14px;
        }
        QListWidget#SectionList::item {
            padding: 10px 16px;
            border-radius: 10px;
            margin: 2px 0px;
        }
        QListWidget#SectionList::item:hover {
            background-color: rgba(255, 255, 255, 0.45);
        }
        QListWidget#SectionList::item:selected {
            background-color: rgba(255, 255, 255, 0.65);
            color: rgba(80, 60, 80, 0.95);
        }
        QWidget#RightPanel {
            background: transparent;
        }
        QLineEdit#SearchBox {
            background-color: rgba(255, 255, 255, 0.5);
            border: 1px solid rgba(255, 255, 255, 0.7);
            border-radius: 10px;
            padding: 10px 16px;
            color: rgba(80, 60, 80, 0.9);
            font-size: 14px;
            min-height: 20px;
        }
        QLineEdit#SearchBox:hover {
            background-color: rgba(255, 255, 255, 0.6);
        }
        QLineEdit#SearchBox:focus {
            background-color: rgba(255, 255, 255, 0.7);
            border-color: rgba(210, 153, 194, 0.6);
        }
        QLineEdit#SearchBox::placeholder {
            color: rgba(80, 60, 80, 0.4);
        }
        QPushButton {
            background-color: rgba(255, 255, 255, 0.4);
            border: 1px solid rgba(255, 255, 255, 0.6);
            border-radius: 10px;
            padding: 10px 24px;
            color: rgba(80, 60, 80, 0.85);
            font-size: 14px;
            min-height: 20px;
        }
        QPushButton:hover {
            background-color: rgba(255, 255, 255, 0.55);
        }
        QPushButton:pressed {
            background-color: rgba(255, 255, 255, 0.65);
        }
        QPushButton#PrimaryButton {
            background-color: rgba(210, 153, 194, 0.9);
            border: none;
            color: #ffffff;
            font-weight: 600;
        }
        QPushButton#PrimaryButton:hover {
            background-color: rgba(210, 153, 194, 1.0);
        }
        QPushButton#PrimaryButton:pressed {
            background-color: rgba(210, 153, 194, 0.8);
        }
        QPushButton#GhostButton {
            background-color: transparent;
            border: 1px solid rgba(210, 153, 194, 0.5);
            color: rgba(180, 120, 160, 0.95);
        }
        QPushButton#GhostButton:hover {
            background-color: rgba(255, 255, 255, 0.35);
            border-color: rgba(210, 153, 194, 0.7);
        }
        QTableView {
            background-color: rgba(255, 255, 255, 0.4);
            alternate-background-color: rgba(255, 255, 255, 0.3);
            border: 1px solid rgba(255, 255, 255, 0.6);
            border-radius: 16px;
            color: rgba(80, 60, 80, 0.9);
            font-size: 14px;
            selection-background-color: rgba(210, 153, 194, 0.25);
            selection-color: rgba(80, 60, 80, 0.95);
            gridline-color: rgba(210, 153, 194, 0.2);
            outline: none;
        }
        QTableView::item {
            padding: 10px;
            border: none;
            border-right: 1px solid rgba(210, 153, 194, 0.2);
            outline: none;
        }
        QTableView::item:focus {
            outline: none;
            border: none;
            border-right: 1px solid rgba(210, 153, 194, 0.2);
        }
        QTableView::item:hover {
            background-color: rgba(255, 255, 255, 0.35);
        }
        QTableView::item:selected {
            background-color: rgba(210, 153, 194, 0.25);
            color: rgba(80, 60, 80, 0.95);
            border-right: 1px solid rgba(210, 153, 194, 0.25);
        }
        QHeaderView {
            background-color: transparent;
            border: none;
        }
        QHeaderView::section {
            background-color: rgba(255, 255, 255, 0.3);
            color: rgba(80, 60, 80, 0.7);
            font-size: 13px;
            font-weight: 600;
            padding: 14px 10px;
            border: none;
            border-bottom: 1px solid rgba(210, 153, 194, 0.25);
            border-right: 1px solid rgba(210, 153, 194, 0.2);
        }
        QHeaderView::section:first {
            border-top-left-radius: 16px;
        }
        QHeaderView::section:last {
            border-right: none;
            border-top-right-radius: 16px;
        }
        QScrollBar:vertical {
            background-color: transparent;
            width: 10px;
            border: none;
            margin: 4px;
        }
        QScrollBar::handle:vertical {
            background-color: rgba(210, 153, 194, 0.35);
            border-radius: 5px;
            min-height: 30px;
        }
        QScrollBar::handle:vertical:hover {
            background-color: rgba(210, 153, 194, 0.5);
        }
        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical {
            height: 0px;
        }
        QScrollBar:horizontal {
            background-color: transparent;
            height: 10px;
            border: none;
            margin: 4px;
        }
        QScrollBar::handle:horizontal {
            background-color: rgba(210, 153, 194, 0.35);
            border-radius: 5px;
            min-width: 30px;
        }
        QScrollBar::handle:horizontal:hover {
            background-color: rgba(210, 153, 194, 0.5);
        }
        QScrollBar::add-line:horizontal,
        QScrollBar::sub-line:horizontal {
            width: 0px;
        }
        QDialog#EditEntryDialog {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #d4f5f3, stop:0.4 #fff8fa, stop:0.7 #faf5f8, stop:1 #fffafb);
            border: 1px solid rgba(255, 255, 255, 0.8);
            border-radius: 20px;
        }
        QDialog#EditEntryDialog QLabel#DialogTitle {
            color: rgba(80, 60, 80, 0.9);
            font-size: 20px;
            font-weight: 700;
        }
        QDialog#EditEntryDialog QLabel {
            color: rgba(80, 60, 80, 0.7);
            font-size: 14px;
        }
        QDialog#EditEntryDialog QLineEdit {
            background-color: rgba(255, 255, 255, 0.5);
            border: 1px solid rgba(255, 255, 255, 0.7);
            border-radius: 10px;
            padding: 10px 14px;
            color: rgba(80, 60, 80, 0.9);
            font-size: 14px;
        }
        QDialog#EditEntryDialog QLineEdit:hover {
            background-color: rgba(255, 255, 255, 0.6);
        }
        QDialog#EditEntryDialog QLineEdit:focus {
            background-color: rgba(255, 255, 255, 0.7);
            border-color: rgba(210, 153, 194, 0.6);
        }
        QDialog#EditEntryDialog QLineEdit:read-only {
            background-color: rgba(255, 255, 255, 0.25);
            color: rgba(80, 60, 80, 0.5);
        }
        QDialog#EditEntryDialog QLineEdit#KeyEdit {
            color: rgba(60, 40, 60, 0.9);
            font-weight: bold;
        }
        QDialog#EditEntryDialog QPlainTextEdit {
            background-color: rgba(255, 255, 255, 0.5);
            border: 1px solid rgba(255, 255, 255, 0.7);
            border-radius: 10px;
            padding: 10px 14px;
            color: rgba(80, 60, 80, 0.9);
            font-size: 14px;
        }
        QDialog#EditEntryDialog QPlainTextEdit:hover {
            background-color: rgba(255, 255, 255, 0.6);
        }
        QDialog#EditEntryDialog QPlainTextEdit:focus {
            background-color: rgba(255, 255, 255, 0.7);
            border-color: rgba(210, 153, 194, 0.6);
        }
        QDialog#EditEntryDialog QFrame#Separator {
            background-color: rgba(210, 153, 194, 0.3);
        }
        QMessageBox {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #a8edea, stop:0.5 #fff0f5, stop:1 #fff5f8);
        }
        QMessageBox QLabel {
            color: rgba(80, 60, 80, 0.9);
            font-size: 14px;
        }
        QMessageBox QPushButton {
            min-width: 80px;
        }
        QToolTip {
            background-color: rgba(255, 255, 255, 0.95);
            border: 1px solid rgba(210, 153, 194, 0.4);
            border-radius: 8px;
            padding: 8px 12px;
            color: rgba(80, 60, 80, 0.9);
            font-size: 13px;
        }
    )";

    qApp->setStyleSheet(styleSheet);
}

void MainWindow::loadDefaultFiles()
{
    QString base = QDir::currentPath();
    m_translationPath = QDir(base).filePath("translation.yaml");

    // Load translation file
    loadTranslation(m_translationPath);

    // Try to load last opened config file
    QString lastFile = loadLastOpenedFile();
    if (!lastFile.isEmpty() && QFileInfo::exists(lastFile))
    {
        loadConfig(lastFile);
        mergeTranslations();
        refreshSectionFilter();
        updateFilePathLabel();
    }
}

void MainWindow::loadConfig(const QString &path)
{
    QString error;
    if (!m_parser.load(path, &error))
    {
        QMessageBox::warning(this, "加载配置", error);
        return;
    }
    m_confPath = path;
    m_configDirty = false;
    m_model->setEntries(&m_parser.entries());
    saveLastOpenedFile();
    updateFilePathLabel();
}

void MainWindow::loadTranslation(const QString &path)
{
    QString error;
    if (!m_translations.load(path, &error))
    {
        QMessageBox::warning(this, "加载翻译", error);
        return;
    }
    m_translationPath = path;
    m_translationDirty = false;

    if (m_versionCombo)
    {
        QStringList versions = m_translations.availableVersions();
        QSettings settings("WY", "ConfEdit");
        QString preferred = settings.value("translationVersion").toString();
        QString selected = versions.contains(preferred) ? preferred : (versions.isEmpty() ? QString() : versions.first());

        m_versionCombo->blockSignals(true);
        m_versionCombo->clear();
        for (const QString &ver : versions)
            m_versionCombo->addItem(ver, ver);
        if (!selected.isEmpty())
        {
            m_translations.setCurrentVersion(selected);
            int index = m_versionCombo->findData(selected);
            if (index >= 0)
                m_versionCombo->setCurrentIndex(index);
        }
        m_versionCombo->blockSignals(false);
    }
}

void MainWindow::mergeTranslations()
{
    QVector<ConfigEntry> &entries = m_parser.entries();
    for (ConfigEntry &entry : entries)
    {
        entry.section.clear();
        if (m_translations.contains(entry.key))
        {
            TranslationItem item = m_translations.item(entry.key);
            entry.section = item.section;
            entry.nameZh = item.nameZh;
            entry.descriptionZh = item.descriptionZh;
        }
    }

    if (m_model)
        m_model->setEntries(&m_parser.entries());
}

void MainWindow::refreshSectionFilter()
{
    QSet<QString> sections;
    bool hasUncategorized = false;
    for (const ConfigEntry &entry : m_parser.entries())
    {
        if (entry.section.isEmpty())
        {
            hasUncategorized = true;
        }
        else
        {
            sections.insert(entry.section);
        }
    }

    QString current;
    if (m_sectionList->currentItem())
        current = m_sectionList->currentItem()->text();

    m_sectionList->blockSignals(true);
    m_sectionList->clear();
    m_sectionList->addItem("全部");
    if (hasUncategorized)
        m_sectionList->addItem("未分类");

    QStringList list = sections.values();
    list.sort();
    for (const QString &sec : list)
        m_sectionList->addItem(sec);

    QList<QListWidgetItem*> matches = m_sectionList->findItems(current, Qt::MatchExactly);
    if (!matches.isEmpty())
        m_sectionList->setCurrentItem(matches.first());
    else
        m_sectionList->setCurrentRow(0);
    m_sectionList->blockSignals(false);
}

void MainWindow::onSearchChanged(const QString &text)
{
    m_proxy->setSearchText(text);
}

void MainWindow::onSectionChanged()
{
    QString text;
    if (m_sectionList->currentItem())
        text = m_sectionList->currentItem()->text();

    if (text == "全部")
        m_proxy->setSectionFilter(QString());
    else
        m_proxy->setSectionFilter(text);
}

void MainWindow::onTableDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid())
        return;

    QModelIndex sourceIndex = m_proxy->mapToSource(index);
    openEditDialog(sourceIndex.row());
}

void MainWindow::openEditDialog(int sourceRow)
{
    if (sourceRow < 0 || sourceRow >= m_parser.entries().size())
        return;

    const ConfigEntry &entry = m_model->entryAt(sourceRow);

    EditEntryDialog dialog(this);
    dialog.setEntry(entry);

    if (dialog.exec() == QDialog::Accepted)
    {
        QVector<ConfigEntry> &entries = m_parser.entries();
        ConfigEntry &editEntry = entries[sourceRow];

        bool translationChanged = false;
        bool valueChanged = false;

        QString newName = dialog.name();
        QString newDesc = dialog.description();
        QString newSection = dialog.section();
        QString newValue = dialog.value();

        if (editEntry.nameZh != newName)
        {
            editEntry.nameZh = newName;
            translationChanged = true;
        }
        if (editEntry.descriptionZh != newDesc)
        {
            editEntry.descriptionZh = newDesc;
            translationChanged = true;
        }
        if (editEntry.section != newSection)
        {
            editEntry.section = newSection;
            translationChanged = true;
        }
        if (editEntry.value != newValue)
        {
            m_parser.setEntryValue(sourceRow, newValue);
            valueChanged = true;
        }

        if (translationChanged)
        {
            TranslationItem item;
            item.key = editEntry.key;
            item.section = editEntry.section;
            item.nameZh = editEntry.nameZh;
            item.descriptionZh = editEntry.descriptionZh;
            m_translations.upsert(item);
            m_translationDirty = true;
            refreshSectionFilter();
        }

        if (valueChanged)
        {
            m_configDirty = true;
        }

        m_model->notifyRowChanged(sourceRow);
    }
}

void MainWindow::onOpenConfig()
{
    QString path = QFileDialog::getOpenFileName(this, "打开配置文件", m_confPath, "配置文件 (*.conf);;所有文件 (*)");
    if (path.isEmpty())
        return;

    loadConfig(path);
    mergeTranslations();
    refreshSectionFilter();
}

void MainWindow::onSaveAll()
{
    if (m_confPath.isEmpty())
    {
        QMessageBox::warning(this, "保存", "请先打开一个配置文件。");
        return;
    }

    QString error;
    if (!m_parser.save(m_confPath, &error))
    {
        QMessageBox::warning(this, "保存配置", error);
        return;
    }
    m_configDirty = false;

    if (m_translationDirty)
    {
        if (!m_translations.save(m_translationPath, &error))
        {
            QMessageBox::warning(this, "保存翻译", error);
            return;
        }
        m_translationDirty = false;
    }

    QMessageBox::information(this, "保存", "所有更改已保存成功。");
}

void MainWindow::onVersionChanged(int index)
{
    if (!m_versionCombo)
        return;
    QString version = m_versionCombo->itemData(index).toString();
    if (version.isEmpty())
        version = m_versionCombo->currentText();
    if (version.isEmpty())
        return;
    if (!m_translations.setCurrentVersion(version))
        return;

    QSettings settings("WY", "ConfEdit");
    settings.setValue("translationVersion", version);

    mergeTranslations();
    refreshSectionFilter();
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_topBar)
    {
        QPoint pos = event->pos();
        QRect topBarRect = m_topBar->geometry();
        topBarRect.translate(centralWidget()->geometry().topLeft());

        if (topBarRect.contains(pos))
        {
            m_dragging = true;
            m_dragPos = event->globalPos() - frameGeometry().topLeft();
            event->accept();
            return;
        }
    }
    QMainWindow::mousePressEvent(event);
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton))
    {
        move(event->globalPos() - m_dragPos);
        event->accept();
        return;
    }
    QMainWindow::mouseMoveEvent(event);
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        m_dragging = false;
    }
    QMainWindow::mouseReleaseEvent(event);
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (hasUnsavedChanges())
    {
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("未保存的更改");
        msgBox.setText("有未保存的更改，是否保存后退出？");
        msgBox.setIcon(QMessageBox::Question);

        QPushButton *saveBtn = msgBox.addButton("保存", QMessageBox::AcceptRole);
        QPushButton *discardBtn = msgBox.addButton("不保存", QMessageBox::DestructiveRole);
        msgBox.addButton("取消", QMessageBox::RejectRole);
        msgBox.setDefaultButton(saveBtn);

        msgBox.exec();

        if (msgBox.clickedButton() == saveBtn)
        {
            onSaveAll();
            event->accept();
        }
        else if (msgBox.clickedButton() == discardBtn)
        {
            event->accept();
        }
        else
        {
            event->ignore();
        }
    }
    else
    {
        event->accept();
    }
}

void MainWindow::updateFilePathLabel()
{
    if (m_confPath.isEmpty())
    {
        m_filePathLabel->setText("");
    }
    else
    {
        QFileInfo fileInfo(m_confPath);
        m_filePathLabel->setText(QString("- %1").arg(fileInfo.fileName()));
        m_filePathLabel->setToolTip(m_confPath);
    }
}

void MainWindow::saveLastOpenedFile()
{
    QSettings settings("WY", "ConfEdit");
    settings.setValue("lastOpenedFile", m_confPath);
}

QString MainWindow::loadLastOpenedFile()
{
    QSettings settings("WY", "ConfEdit");
    return settings.value("lastOpenedFile").toString();
}

bool MainWindow::hasUnsavedChanges() const
{
    return m_configDirty || m_translationDirty;
}
