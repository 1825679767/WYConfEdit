#pragma once

#include <QMainWindow>
#include <QVector>
#include <QPoint>

#include "confparser.h"
#include "translationstore.h"
#include "configmodel.h"

class QLineEdit;
class QListWidget;
class QTableView;
class QPushButton;
class EditEntryDialog;
class QLabel;
class QComboBox;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private slots:
    void onSearchChanged(const QString &text);
    void onSectionChanged();
    void onTableDoubleClicked(const QModelIndex &index);
    void onOpenConfig();
    void onSaveAll();
    void onVersionChanged(int index);

private:
    void buildUi();
    void applyGlobalStyles();
    void loadDefaultFiles();
    void loadConfig(const QString &path);
    void loadTranslation(const QString &path);
    void mergeTranslations();
    void refreshSectionFilter();
    void openEditDialog(int sourceRow);
    void updateFilePathLabel();
    void saveLastOpenedFile();
    QString loadLastOpenedFile();
    bool hasUnsavedChanges() const;

    ConfParser m_parser;
    TranslationStore m_translations;

    QString m_confPath;
    QString m_translationPath;
    bool m_translationDirty = false;
    bool m_configDirty = false;

    ConfigModel *m_model = nullptr;
    ConfigFilterProxy *m_proxy = nullptr;

    QLineEdit *m_searchEdit = nullptr;
    QListWidget *m_sectionList = nullptr;
    QTableView *m_table = nullptr;
    QLabel *m_filePathLabel = nullptr;
    QComboBox *m_versionCombo = nullptr;

    // Window dragging
    QWidget *m_topBar = nullptr;
    bool m_dragging = false;
    QPoint m_dragPos;
};
