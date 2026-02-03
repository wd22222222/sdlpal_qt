///* -*- mode: c; tab-width: 4; c-basic-offset: 4; c-file-style: "linux" -*- */
//
// Copyright (c) 2021-2026, Wu Dong.
// 
// All rights reserved.
//
// This file is part of SDLPAL.
//
// SDLPAL is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License, version 3
// as published by the Free Software Foundation.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//

#include "fontfinder.h"
#include <qpushbutton.h>
#include <qheaderview.h>
#include <qscreen.h>
#include <qdatetime.h>
#include <qcheckbox.h>
#include <qboxlayout.h>
#include <qfiledialog.h>
#include <qfileinfo.h>
#include <qfontinfo.h>
#include <qfontdatabase.h>
#include <qmap.h>
#include <qvariant.h>
#include <qtextedit.h>
#include <qmessagebox.h>
#include <qlistwidget.h>
#include <qtreewidget.h>
#include <QDesktopServices.h>
#include <qsystemdetection.h>
#include <qtextstream.h>
#include <qfont.h>
#include <qguiapplication.h>
#include <qdialog.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qwidget.h>
#include <qchar.h>
#include <qcontainerfwd.h>
#include <qdir.h>
#include <qfile.h>
#if QT_VERSION >= QT_VERSION_CHECK( 6,0, 0)
#include <qforeach.h>
#endif
#include <qiodevice.h>
#include <qlist.h>
#include <qnamespace.h>
#include <qrect.h>
#include <qstring.h>

#ifdef Q_OS_WIN
#include <windows.h>
#endif
#include <qurl.h>

// 字体名称ID定义
#define NAME_ID_COPYRIGHT         0
#define NAME_ID_FONT_FAMILY       1
#define NAME_ID_FONT_SUBFAMILY    2
#define NAME_ID_UNIQUE_ID         3
#define NAME_ID_FULL_NAME         4
#define NAME_ID_VERSION           5
#define NAME_ID_POSTSCRIPT_NAME   6
#define NAME_ID_TRADEMARK         7
#define NAME_ID_MANUFACTURER      8
#define NAME_ID_DESIGNER          9
#define NAME_ID_DESCRIPTION       10
#define NAME_ID_VENDOR_URL        11
#define NAME_ID_DESIGNER_URL      12
#define NAME_ID_LICENSE           13
#define NAME_ID_LICENSE_URL       14
#define NAME_ID_PREFERRE_FAMILY   16
#define NAME_ID_PREFERRE_SUBFAMILY 17
#define NAME_ID_COMPAT_FULL_NAME  18
#define NAME_ID_SAMPLE_TEXT       19


FontFinder::FontFinder(const QString & fontPath, QWidget* parent) : QDialog(parent)
{
    this->selectedFontFile = fontPath;
    setupUI();
    setWindowTitle("字体文件查找工具 - 带中文解释");
    // 确保 main() 中已设置 AA_EnableHighDpiScaling（Qt5）
    QScreen* screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect logicalScreen = screen->geometry();
        resize(logicalScreen.width() * 0.75, logicalScreen.height() * 0.75);
    }
    else {
        resize(800, 600); // fallback
    }
    if(selectedFontFile.size())
        showFontProperties(selectedFontFile);
}

// 字体样式中文翻译
QString FontFinder::translateFontStyle(const QString& style)
{
    static QMap<QString, QString> styleMap = {
        {"Regular", "常规"},
        {"Normal", "正常"},
        {"Bold", "粗体"},
        {"Italic", "斜体"},
        {"Bold Italic", "粗斜体"},
        {"Light", "细体"},
        {"Medium", "中等"},
        {"Semibold", "半粗体"},
        {"Black", "黑体"},
        {"Heavy", "重体"},
        {"Thin", "极细"},
        {"Ultra Light", "超细"},
        {"Extra Light", "特细"},
        {"Demi Bold", "半粗"},
        {"Extra Bold", "特粗"},
        {"Ultra Bold", "超粗"},
        {"Oblique", "倾斜"},
        {"Condensed", "紧缩"},
        {"Extended", "扩展"}
    };

    return styleMap.value(style, style);
}

static QMap<QString, QString> familyMap = {
    { "Deng.ttf","等线"},
    { "DengXian","等线"},
    { "Dengb.ttf","等线粗"},
    { "Dengl.ttf","等线细"},
    { "DengXian Light","等线细"},
    { "FZSTK.TTF","方正舒体"},
    { "FZShuTi","方正舒体"},
    { "FZYTK.TTF","方正姚休"},
    { "FZYaoTi","方正姚休"},
    { "malgun.ttf","朝鲜语"},
    { "simfang.ttf","仿宋"},
    { "simhei.ttf","黑体"},
    { "simkai.ttf","楷体"},
    { "SIMLI.TTF","隶书"},
    { "LiSu","隶书"},
    { "simsun.ttc","中文"},
    { "simsunb.ttf","中文"},
    { "SimSun-ExtB","中文"},
    { "SIMYOU.TTF","幼圆"},
    { "STCAIYUN.TTF","华文彩云"},
    { "STFANGSO.TTF","华文仿宋"},
    { "STHUPO.TTF","华文琥珀"},
    { "STHupo","华文琥珀"},
    { "STKAITI.TTF","华文楷体"},
    { "STLITI.TTF","华文隶书"},
    { "STLiti","华文隶书"},
    { "STSONG.TTF","华文宋体"},
    { "STXIHEI.TTF","华文细黑"},
    { "STXihei","华文细黑"},
    { "STXINGKA.TTF","华文行楷"},
    { "STXingkai","华文行楷"},
    { "STXINWEI.TTF","华文新魏"},
    { "STXinwei","华文新魏"},
    { "STZHONGS.TTF","华文中宋"},
    { "STZhongsong","华文中宋"},
    { "Arial", "雅黑体"},
    { "SimSun", "宋体"},
    { "msyh.ttc", "微软雅黑"},
    { "msjhbd.ttc", "微软雅黑"},
    { "msyhl.ttc", "微软雅黑细"},
    { "Microsoft YaHei", "微软雅黑"},
    { "Microsoft YaHei Light", "微软雅黑细"},
    { "SimHei", "黑体"},
    { "simhei", "黑体"},
    { "KaiTi", "楷体"},
    { "FangSong", "仿宋"},
    { "simfang", "仿宋"},
    { "FZYaoTi", "方正姚体"},
    { "FZSTK", "方正舒体"},
    { "NSimSun", "新宋体"},
    { "YouYuan", "幼圆"},
    { "LiSu", "隶书"},
    { "STCaiyun", "华文彩云"},
    { "STSong", "华文宋体"},
    { "STKaiti", "华文楷体"},
    { "STHeiti", "华文黑体"},
    { "STFangsong", "华文仿宋"},
    { "STHUPO", "华文琥珀"},
    { "PingFang SC", "苹方"},
    { "Hiragino Sans GB", "冬青黑体"},
    { "Source Han Sans CN", "思源黑体"},
    { "Source Han Serif CN", "思源宋体"},
    { "Adobe Heiti Std", "Adobe黑体"},
    { "Adobe Song Std", "Adobe宋体"},
    { "msjh.ttc", "繁体中文" },
    { "Microsoft JhengHei", "繁体中文" },
    { "msjhl.ttc", "繁体中文" },
    { "Microsoft JhengHei Light", "繁体中文细" },
    { "mingliub.ttc", "繁体中文" },
    { "MingLiU-ExtB", "繁体中文" },
    { "mingliub.ttc", "繁体中文" },
};

// 字体家族中文翻译（常见字体）
QString FontFinder::translateFontFamily(const QString& family)
{
    return familyMap.value(family,family);
}


// 获取特定名称表的字体信息（简化版）
QString FontFinder::getFontNameFromTable(const QString& filePath, int nameId)
{
    // 这里是一个简化的实现，实际需要完整的TTF/OTF解析
    // 我们可以使用QFontDatabase已经提供的信息

    int fontId = QFontDatabase::addApplicationFont(filePath);
    if (fontId == -1) {
        return "未知";
    }

    QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
    QFontDatabase::removeApplicationFont(fontId);

    if (fontFamilies.isEmpty()) {
        return "未知";
    }

    QString family = fontFamilies.first();

    switch (nameId) {
    case NAME_ID_FONT_FAMILY:
        return translateFontFamily(family);
    case NAME_ID_FULL_NAME:
        return family + " (完整名称)";
    case NAME_ID_VERSION:
        return "1.0"; // 简化版本号
    default:
        return "未提供";
    }
}

// 更新getFontProperties函数，添加中文解释
QMap<QString, QVariant> FontFinder::getFontProperties(const QString& filePath)
{
    QMap<QString, QVariant> properties;

    // 文件属性
    QFileInfo fileInfo(filePath);
    properties["文件名称"] = fileInfo.fileName();
    properties["完整路径"] = fileInfo.absoluteFilePath();
    properties["文件大小"] = QString("%1 KB").arg(fileInfo.size() / 1024.0, 0, 'f', 2);

    
#ifdef Q_OS_WIN
    //防止时间显示乱码
    auto qTimeDateToQstring = [](const QDateTime& ft)->QString {
        if (!ft.isValid())
            return QString();
        return QString("%1-%2-%3 %4:%5:%6")
            .arg(ft.date().year(), 4, 10, QLatin1Char('0'))
            .arg(ft.date().month(), 2, 10, QLatin1Char('0'))
            .arg(ft.date().day(), 2, 10, QLatin1Char('0'))
            .arg(ft.time().hour(), 2, 10, QLatin1Char('0'))
            .arg(ft.time().minute(), 2, 10, QLatin1Char('0'))
            .arg(ft.time().second(), 2, 10, QLatin1Char('0')); 
        };

    properties["创建时间"] = qTimeDateToQstring(fileInfo.birthTime());
    properties["修改时间"] = qTimeDateToQstring(fileInfo.lastModified());
    //properties["访问时间"] = qTimeToQstring(fileInfo.lastRead());
#else
    properties["创建时间"] = fileInfo.birthTime().toString("yyyy-MM-dd HH:mm:ss");
    properties["修改时间"] = fileInfo.lastModified().toString("yyyy-MM-dd HH:mm:ss");
    //properties["访问时间"] = fileInfo.lastRead().toString("yyyy-MM-dd HH:mm:ss");
#endif

    QString extension = fileInfo.suffix().toUpper();
    QMap<QString, QString> formatDesc = {
        {"TTF", "TrueType字体"},
        {"OTF", "OpenType字体"},
        {"TTC", "TrueType字体集合"},
        {"FON", "Windows位图字体"},
        {"FNT", "位图字体文件"},
        {"WOFF", "Web开放字体格式"},
        {"WOFF2", "Web开放字体格式2.0"}
    };
    properties["文件类型"] = formatDesc.value(extension, extension + "字体文件");

    // 字体属性
    int fontId = QFontDatabase::addApplicationFont(filePath);
    if (fontId != -1) {
        QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
        if (!fontFamilies.isEmpty()) {
            QString family = fontFamilies.first();

            // 中文解释
            properties["字体家族"] = QString("%1\n(%2)\n成员%3")
                .arg(translateFontFamily(family))
                .arg(family).arg(fontFamilies.size());

            properties["字体家族(英文)"] = family;

            QFontDatabase fontDb;

            // 获取样式并添加中文解释
            QStringList styles = fontDb.styles(family);
            QStringList stylesWithTranslation;
            for (const QString& style : styles) {
                stylesWithTranslation << QString("%1 (%2)").arg(translateFontStyle(style)).arg(style);
            }
            properties["可用样式"] = stylesWithTranslation.join(", ");

            // 大小信息
            QList<int> sizes = fontDb.pointSizes(family);
            QStringList sizeStrs;
            for (int size : sizes) {
                sizeStrs.append(QString::number(size));
            }
            properties["支持字号"] = sizeStrs.join(", ") + " pt";

            if (!styles.isEmpty()) {
                QString firstStyle = styles.first();
                properties["粗体支持"] = fontDb.bold(family, firstStyle) ? "支持" : "不支持";
                properties["斜体支持"] = fontDb.italic(family, firstStyle) ? "支持" : "不支持";

                // 样式中文描述
                properties["当前样式"] = translateFontStyle(firstStyle);
            }

            // 更多字体信息
            QFont font(family, 12);
            QFontInfo fontInfo(font);
            properties["像素大小"] = QString("%1 像素").arg(fontInfo.pixelSize());
            properties["点大小"] = QString("%1 点").arg(fontInfo.pointSize());
            properties["固定间距"] = fontInfo.fixedPitch() ? "是（等宽字体）" : "否（比例字体）";
            properties["字体权重"] = QString::number(fontInfo.weight());

            // 字体元数据信息
            properties["版权信息"] = getFontNameFromTable(filePath, NAME_ID_COPYRIGHT);
            properties["版本信息"] = getFontNameFromTable(filePath, NAME_ID_VERSION);
            properties["制造商"] = getFontNameFromTable(filePath, NAME_ID_MANUFACTURER);
            properties["设计师"] = getFontNameFromTable(filePath, NAME_ID_DESIGNER);

            // 字体类型判断，包含 不分大小写
            bool hasChinese = isChineseFamily(family);
            properties["语言支持"] = hasChinese ? "中文支持良好" : "主要支持西方语言";
            properties["推荐用途"] = hasChinese ? "中文显示" : "西文显示";
        }

        QFontDatabase::removeApplicationFont(fontId);
    }

    // 文件权限信息
    properties["可读性"] = fileInfo.isReadable() ? "可读" : "不可读";
    properties["可写性"] = fileInfo.isWritable() ? "可写" : "不可写";
    properties["隐藏属性"] = fileInfo.isHidden() ? "隐藏文件" : "可见文件";
    properties["文件类型"] = fileInfo.isFile() ? "普通文件" : "非文件";

    return properties;
}

// 更新showFontProperties函数，改进显示
void FontFinder::showFontProperties(const QString& filePath)
{
    selectedFontFile = filePath;
    m_ok->setDisabled(false);
    fontPropertiesTree->clear();

    QMap<QString, QVariant> properties = getFontProperties(filePath);

    // 重新组织分类
    QStringList categories = {
        "文件基本信息",
        "字体详细信息",
        "样式与支持",
        "版权与元数据",
        "系统信息"
    };

    QMap<QString, QStringList> categoryMapping = {
        {"文件基本信息", {"文件名称", "完整路径", "文件大小", "文件类型", "创建时间", "修改时间", "访问时间"}},
        {"字体详细信息", {"字体家族", "字体家族(英文)", "支持字号", "像素大小", "点大小", "固定间距", "字体权重"}},
        {"样式与支持", {"可用样式", "当前样式", "粗体支持", "斜体支持", "语言支持", "推荐用途"}},
        {"版权与元数据", {"版权信息", "版本信息", "制造商", "设计师"}},
        {"系统信息", {"可读性", "可写性", "隐藏属性"}}
    };

    foreach(const QString & category, categories) {
        QTreeWidgetItem* categoryItem = new QTreeWidgetItem(fontPropertiesTree);
        categoryItem->setText(0, category);
        categoryItem->setExpanded(true);

        QFont categoryFont = categoryItem->font(0);
        categoryFont.setBold(true);
        categoryFont.setPointSize(10);
        categoryItem->setFont(0, categoryFont);

        foreach(const QString & key, categoryMapping[category]) {
            if (properties.contains(key)) {
                QTreeWidgetItem* propertyItem = new QTreeWidgetItem(categoryItem);
                propertyItem->setText(0, key);

                // 对重要信息进行突出显示
                QString value = properties[key].toString();
                if (key.contains("字体家族") || key.contains("推荐用途")) {
                    QFont font = propertyItem->font(1);
                    font.setBold(true);
                    propertyItem->setFont(1, font);
                }

                propertyItem->setText(1, value);
            }
        }
    }

    fontPropertiesTree->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    fontPropertiesTree->header()->setSectionResizeMode(1, QHeaderView::Stretch);

    // 在文本框中显示总结信息
    selectedFilesDisplay->clear();
    selectedFilesDisplay->append(QString("字体文件: %1").arg(filePath));

    if (properties.contains("字体家族")) {
        selectedFilesDisplay->append(QString("中文名称: %1").arg(properties["字体家族"].toString()));
    }
    if (properties.contains("推荐用途")) {
        selectedFilesDisplay->append(QString("推荐用途: %1").arg(properties["推荐用途"].toString()));
    }
    if (properties.contains("语言支持")) {
        selectedFilesDisplay->append(QString("语言支持: %1").arg(properties["语言支持"].toString()));
    }
}

void FontFinder::setupUI()
{

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    // 目录选择部分
    QHBoxLayout* directoryLayout = new QHBoxLayout();
    directoryPath = new QLineEdit();
    QPushButton* browseButton = new QPushButton("浏览其他目录");
    QPushButton* openDirButton = new QPushButton("浏览系统字体");
    QPushButton* selectFileButton = new QPushButton("选择自定义字体");

    directoryLayout->addWidget(new QLabel("字体目录:"));
    directoryLayout->addWidget(directoryPath, 2);
    directoryLayout->addWidget(browseButton);
    directoryLayout->addWidget(openDirButton);
    directoryLayout->addWidget(selectFileButton);

    // 搜索部分
    QHBoxLayout* searchLayout = new QHBoxLayout();
    searchInput = new QLineEdit();
    searchInput->setPlaceholderText("输入字体名称搜索...");
    QPushButton* searchButton = new QPushButton("搜索");
    QCheckBox* onlyChinesFont{};
    searchLayout->addWidget(new QLabel("只搜索中文字库:"));
    onlyChinesFont = new QCheckBox();
    onlyChinesFont->setChecked(TRUE);
    searchLayout->addWidget(onlyChinesFont);
    searchLayout->addWidget(searchInput,Qt::AlignRight);
    searchLayout->addWidget(searchButton);

    // 主要内容区域
    QHBoxLayout* contentLayout = new QHBoxLayout();

    // 左侧：字体列表
    QVBoxLayout* leftLayout = new QVBoxLayout();
    leftLayout->addWidget(new QLabel("字体文件列表:"));
    fontList = new QListWidget();
    leftLayout->addWidget(fontList);

    // 右侧：属性显示区域
    QVBoxLayout* rightLayout = new QVBoxLayout();
    rightLayout->addWidget(new QLabel("字体属性:"));
    fontPropertiesTree = new QTreeWidget();
    fontPropertiesTree->setHeaderLabels(QStringList() << "属性" << "值");
    fontPropertiesTree->setColumnCount(2);
    fontPropertiesTree->setAlternatingRowColors(true);
    rightLayout->addWidget(fontPropertiesTree);

    // 底部：选中文件显示
    selectedFilesDisplay = new QTextEdit();
    selectedFilesDisplay->setReadOnly(true);
    selectedFilesDisplay->setMaximumHeight(100);
    selectedFilesDisplay->setPlaceholderText("选中的文件将显示在这里...");
    rightLayout->addWidget(new QLabel("选中的文件:"));
    rightLayout->addWidget(selectedFilesDisplay);
    //添加确定取消按钮
    QHBoxLayout* rightLayoutBotten = new QHBoxLayout();
    rightLayoutBotten->addWidget(new QLabel(""),3);
    m_ok = new QPushButton("确定");
    m_ok->setDisabled(true);
    QPushButton* m_cancel = new QPushButton("取消");

    rightLayoutBotten->addWidget(m_ok,1);
    rightLayoutBotten->addWidget(m_cancel,1);
    rightLayout->addLayout(rightLayoutBotten);

    contentLayout->addLayout(leftLayout, 1);
    contentLayout->addLayout(rightLayout, 3);

    statusLabel = new QLabel("准备就绪");
    mainLayout->addLayout(directoryLayout);
    mainLayout->addLayout(searchLayout);
    mainLayout->addLayout(contentLayout);
    mainLayout->addWidget(statusLabel);

    // 连接信号和槽
    connect(browseButton, &QPushButton::clicked, this, &FontFinder::browseFontDirectory);
    connect(openDirButton, &QPushButton::clicked, this, &FontFinder::openFontDirectory);
    connect(selectFileButton, &QPushButton::clicked, this, &FontFinder::selectFontFile);
    connect(searchButton, &QPushButton::clicked, this, &FontFinder::searchFonts);
    connect(onlyChinesFont, &QCheckBox::stateChanged, this, [&](int state) {
        if (state == Qt::Checked)
            onlyChineseLibrary = TRUE;
        else
            onlyChineseLibrary = FALSE;//all
        searchFonts();
    });
    connect(m_ok, &QPushButton::clicked, this, [this]() {
        done(1);
        });
    connect(m_cancel, &QPushButton::clicked, this, [this]() {
        selectedFontFile.clear();
        done(0); 
        });

    connect(searchInput, &QLineEdit::returnPressed, this, &FontFinder::searchFonts);
    connect(fontList, &QListWidget::itemDoubleClicked, this, &FontFinder::onFontItemDoubleClicked);
    connect(fontList, &QListWidget::itemClicked, this, &FontFinder::onFontItemClicked);

    // 设置默认字体目录
    setDefaultFontDirectory();
}

void FontFinder::browseFontDirectory()
{
    QString directory = QFileDialog::getExistingDirectory(this, "选择字体目录",
        QDir::homePath(),
        QFileDialog::ShowDirsOnly);
    if (!directory.isEmpty()) {
        directoryPath->setText(directory);
        searchFonts();
    }
}

void FontFinder::selectFontFile()
{
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "选择非系统字体文件",
        directoryPath->text(),
        "字体文件 (*.ttf ,*.ttc)"
    );

    if (!fileName.isEmpty()) {
        selectedFilesDisplay->clear();
        selectedFilesDisplay->append("选中的字体文件: " + fileName);
        showFontProperties(fileName);
        selectedFilesDisplay->append(fileName);
        //selectedFontFile = fileName;
    }
}
void FontFinder::searchFonts()
{
    QString directory = directoryPath->text();
    QString searchText = searchInput->text();

    if (directory.isEmpty()) {
        QMessageBox::warning(this, "警告", "请先选择字体目录");
        return;
    }

    fontList->clear();
    fontPropertiesTree->clear();

    QDir dir(directory);
    QStringList filters;
    filters << "*.ttf"<<"*.ttc";
    QStringList fontFiles = dir.entryList(filters, QDir::Files, QDir::SortFlag::Name);
    fontList->clear();
    int foundCount = 0;
    
    QFile f("d:/a/a.txt");
    f.open(QIODevice::WriteOnly | QIODevice::Text);
    QTextStream out(&f);

    foreach(QString fontFile, fontFiles) {
        QString filePath = dir.filePath(fontFile);
        int fontId = QFontDatabase::addApplicationFont(filePath);
        QString family;
        if (fontId != -1) {
            QStringList fontFamilies = QFontDatabase::applicationFontFamilies(fontId);
            if (!fontFamilies.isEmpty()) {
                family = fontFamilies.first();
            }
        }

        QFileInfo fileInfo(filePath);
        if (fileInfo.suffix().toLower() == "ttc" && fileInfo.size() > 1024 * 1024 * 3) {
            out << fontFile << "\t" << family << "\n";
        }
        //
        if (searchText.isEmpty() ||
            fontFile.contains(fontFile, Qt::CaseInsensitive)) {
            if (onlyChineseLibrary && 
                !isChineseFont(fontFile) && 
                !isChineseFamily(family))//只显示中文
            {
                continue;
                //如果不是中文跳过
            }
            QListWidgetItem* item = new QListWidgetItem(fontFile);
            item->setData(Qt::UserRole, dir.filePath(fontFile)); // 存储完整路径
            item->setToolTip(dir.filePath(fontFile)); // 显示完整路径提示
            fontList->addItem(item);
            foundCount++;
        }
    }
    f.close();
    statusLabel->setText(QString("找到 %1 个字体文件").arg(foundCount));
}

void FontFinder::openFontDirectory()
{
    setDefaultFontDirectory();
    QString directory = directoryPath->text();

    QDesktopServices::openUrl(QUrl::fromLocalFile(directory));
}

void FontFinder::onFontItemDoubleClicked(QListWidgetItem* item)
{
    QString filePath = item->data(Qt::UserRole).toString();
    selectedFilesDisplay->clear();
    selectedFilesDisplay->append("选中的字体文件: " + filePath);
    showFontProperties(filePath);
}

void FontFinder::onFontItemClicked(QListWidgetItem* item)
{
    QString filePath = item->data(Qt::UserRole).toString();
    showFontProperties(filePath);
}

void FontFinder::setDefaultFontDirectory()
{
#ifdef Q_OS_WIN
    directoryPath->setText("C:/Windows/Fonts");
#elif defined(Q_OS_LINUX)
    directoryPath->setText("/usr/share/fonts");
#elif defined(Q_OS_MAC)
    directoryPath->setText("/System/Library/Fonts");
#else
    directoryPath->setText(QDir::homePath());
#endif
}

//检测字库是否支持中文，是1 不是0
int FontFinder::isChineseFamily(const QString& family)
{
    return familyMap.value(family).size() > 0;

}

int FontFinder::isChineseFont(const QString& filename)
{
    return familyMap.value(filename).size() > 0;
}




