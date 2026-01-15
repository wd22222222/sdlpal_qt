#ifndef FONTFINDER_H
#define FONTFINDER_H

//#include <QMainWindow>
#include <qdialog.h>
#include <QLineEdit.h>
#include <QListWidget.h>
#include <QLabel.h>
#include <QTextEdit.h>
#include <QTreeWidget.h>

#ifdef Q_OS_WIN
#include <windows.h>
#endif

class FontFinder : public QDialog
{
    Q_OBJECT

public:
    FontFinder(const QString & fontpath, QWidget* parent = nullptr);
    QString  selectedFontFile;
private slots:
    void browseFontDirectory();
    void selectFontFile();
    void searchFonts();
    void openFontDirectory();
    void onFontItemDoubleClicked(QListWidgetItem* item);
    void onFontItemClicked(QListWidgetItem* item);
    void showFontProperties(const QString& filePath);

private:
    void setupUI();
    void setDefaultFontDirectory();
    int isChineseFamily(const QString& family);
    int isChineseFont(const QString& filename);
    QMap<QString, QVariant> getFontProperties(const QString& filePath);

    // 字体元数据获取函数
    QMap<QString, QString> getFontMetadata(const QString& filePath);
    QString getFontNameFromTable(const QString& filePath, int nameId);
    QString translateFontStyle(const QString& style);
    QString translateFontFamily(const QString& family);

    QLineEdit* directoryPath;
    QLineEdit* searchInput;
    BOOL onlyChineseLibrary{1};
    QListWidget* fontList;
    QTextEdit* selectedFilesDisplay;
    QTreeWidget* fontPropertiesTree;
    QLabel* statusLabel;
    QPushButton* m_ok;
};

#endif // FONTFINDER_H