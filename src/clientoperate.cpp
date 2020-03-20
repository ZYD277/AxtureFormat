#include "clientoperate.h"
#include "ui_clientoperate.h"

#include <iostream>
#include <functional>

#include <QFileDialog>
#include <QListView>
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QProcess>
#include <QMutexLocker>

#include "util/rutil.h"
#include "util/threadpool.h"
#include "viewdelegate.h"
#include "viewmodel.h"
#include "switchtask.h"

ClientOperate::ClientOperate(QWidget *parent) :
    QWidget(parent),cssBaseFileName("styles.css"),jsBaseFileName("document.js"),jsSinglePageFileName("data.js"),
    ui(new Ui::ClientOperate),m_versionButtGroup(nullptr),m_dirPathButtGroup(nullptr),m_viewDelegate(nullptr),m_model(nullptr),
    m_pool(new ThreadPool(5))
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("Axure转Qt工具-RGCompany"));
    qRegisterMetaType<SwitchProgress>("SwitchProgress");

    initView();
}

ClientOperate::~ClientOperate()
{
    delete ui;
    delete m_model;
    delete m_viewDelegate;
    delete m_pool;
}

void ClientOperate::initView()
{
    m_model = new ViewModel();
    m_viewDelegate = new ViewDelegate();

    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setFocusPolicy(Qt::NoFocus);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableView->setModel(m_model);
    ui->tableView->setItemDelegate(m_viewDelegate);

    //设置指定列宽度
    ui->tableView->horizontalHeader()->setSectionResizeMode(T_Index, QHeaderView::Fixed);
    ui->tableView->horizontalHeader()->setSectionResizeMode(T_Open, QHeaderView::Fixed);
    ui->tableView->horizontalHeader()->setSectionResizeMode(T_Delete, QHeaderView::Fixed);
    ui->tableView->horizontalHeader()->setSectionResizeMode(T_Switch, QHeaderView::Fixed);

    ui->tableView->setColumnWidth(T_Index,50);
    ui->tableView->setColumnWidth(T_Open,Table_Icon_Width);
    ui->tableView->setColumnWidth(T_Delete,Table_Icon_Width);
    ui->tableView->setColumnWidth(T_Switch,Table_Icon_Width);

    connect(m_viewDelegate,SIGNAL(viewFile(QString)),this,SLOT(viewFile(QString)));
    connect(m_viewDelegate,SIGNAL(deleteFile(QString)),this,SLOT(deletFileData(QString)));
    connect(m_viewDelegate,SIGNAL(switchSingleFile(QString)),this,SLOT(switchLineHtmlFile(QString)));

    ui->axureVerision->setView(new QListView());

    m_versionButtGroup = new QButtonGroup(this);
    m_versionButtGroup->addButton(ui->qt4RadioBtn,Qt4);
    m_versionButtGroup->addButton(ui->qt5RadioBtn,Qt5);
    m_versionButtGroup->setExclusive(true);
    ui->qt4RadioBtn->setChecked(true);

    m_dirPathButtGroup = new QButtonGroup(this);
    m_dirPathButtGroup->addButton(ui->sourceeDirRadioBtn,Path_Current);
    m_dirPathButtGroup->addButton(ui->createOutDirRadioBtn,Path_SelfDefine);
    m_dirPathButtGroup->setExclusive(true);

    ui->sourceeDirRadioBtn->setChecked(true);
    ui->createOutDirLedit->setEnabled(false);
    ui->selectDirBtn->setEnabled(false);

    connect(m_versionButtGroup, SIGNAL(buttonClicked(int)),this,SLOT(chooseQtVersion(int)));
    connect(m_dirPathButtGroup, SIGNAL(buttonClicked(int)),this,SLOT(chooseOutputPath(int)));

    connect(ui->openAxPrjBtn,SIGNAL(pressed()),this,SLOT(openAxureProject()));
    connect(ui->clearList,SIGNAL(pressed()),this,SLOT(clearAxureTable()));
    connect(ui->selectDirBtn,SIGNAL(pressed()),this,SLOT(chooseUserFilePath()));
    connect(ui->startChangeBtn,SIGNAL(clicked(bool)),this,SLOT(startSwitchFiles()));
    connect(ui->clearLog,&QPushButton::pressed,[&](){ui->textBrowser->clear();});
}

/**
 * @brief 获取qt版本
 */
void ClientOperate::chooseQtVersion(int)
{
    QtVersion version = static_cast<QtVersion>(m_versionButtGroup->checkedId());
    switch(version)
    {
        case Qt4:
            break;
        case Qt5:
            break;
        default:
            break;
    }
}

/**
 * @brief 获取文件存储路径
 */
void ClientOperate::chooseOutputPath(int)
{
    ExportPath path = static_cast<ExportPath>(m_dirPathButtGroup->checkedId());
    switch(path)
    {
        case Path_Current:
            ui->createOutDirLedit->setEnabled(false);
            ui->selectDirBtn->setEnabled(false);
            break;
        case Path_SelfDefine:
            ui->createOutDirLedit->setEnabled(true);
            ui->selectDirBtn->setEnabled(true);
            break;
        default:
            break;
    }
}

/**
 * @brief 打开axure工程
 * @details 选择axure工程目录，检测目录下可转换的html，若能提取到则将文件名添加到表格中；若之前已经添加则忽略
 */
void ClientOperate::openAxureProject()
{
    QDir parent = QDir(m_lastAxureProjectPath);
    parent.cdUp();

    QString projectPath = QFileDialog::getExistingDirectory(this,QStringLiteral("选择Axure工程目录"),
                                                            m_lastAxureProjectPath.isEmpty() ? QDir::homePath() : parent.path());
    Check_Return(projectPath.isEmpty(),);
    m_lastAxureProjectPath = projectPath;

    QDir pDir(projectPath);
    if(pDir.exists())
    {
        appendLog(LogNormal,QString(QStringLiteral("开始解析工程[%1]")).arg(pDir.path()));

        QString basePath = projectPath + QDir::separator() + "data";
        if(!checkJsCssExisted(basePath,false)){
            showWarnings(QStringLiteral("通用样式表或js文件不存在!"));
            return;
        }

        appendLog(LogNormal,QStringLiteral("检测通用样式."));
        QPair<QString,QString> jsCssPair = getJsCssFile(basePath,false);

        QStringList nameFilters;
        nameFilters<<"*.html";

        QFileInfoList htmlFiles = pDir.entryInfoList(nameFilters,QDir::Files | QDir::NoDotAndDotDot);
        std::for_each(htmlFiles.begin(),htmlFiles.end(),[&](const QFileInfo & fileInfo){
            if(!isRepeatedFile(fileInfo.filePath())){
                AxurePage page;

                page.baseJsFilePath = jsCssPair.first;
                page.baseCssFilePath = jsCssPair.second;


                page.htmlFilePath = fileInfo.filePath();
                page.htmlFileName = fileInfo.fileName();
                QString pageJsCssPath = fileInfo.absoluteDir().path() + QDir::separator() + "files" + QDir::separator() + fileInfo.baseName();

                if(checkJsCssExisted(pageJsCssPath)){
                    jsCssPair = getJsCssFile(pageJsCssPath);
                    page.jsFilePath = jsCssPair.first;
                    page.cssFilePath = jsCssPair.second;

                    appendLog(LogNormal,QString(QStringLiteral("检测到页面[%1].")).arg(fileInfo.fileName()));

                    page.processData.error = false;
                    page.processData.textDescription = QStringLiteral("0%");

                    m_pageList.append(page);
                }
            }
        });

        updateTableModel();
    }
}

/*!
 * @brief 检查指定目录下js和css文件是否存在
 * @param[in] path 待检测的目录
 * @param[in] isSinglePage 是否为单个html页面，单个HTML页面和通用的js文件名不一致
 * @return true:都存在；false:至少有一个不存在
 */
bool ClientOperate::checkJsCssExisted(QString path,bool isSinglePage)
{
    QDir basePath(path);
    if(!basePath.exists())
        return false;

    auto existedFile = [&](QString fileName)->bool{
        QFileInfo fpath(basePath.absolutePath() + QDir::separator()+ fileName);
        return fpath.exists();
    };

    return existedFile(cssBaseFileName) && (isSinglePage?existedFile(jsSinglePageFileName):existedFile(jsBaseFileName));
}

/*!
 * @brief 获取指定目录下js和css文件
 * @param[in] path 待获取的目录
 * @param[in] isSinglePage 是否为单个html页面，单个HTML页面和通用的js文件名不一致
 * @return first:js全路径，second:css全路径
 */
QPair<QString, QString> ClientOperate::getJsCssFile(QString path,bool isSinglePage)
{
    return qMakePair((isSinglePage?path + QDir::separator()+ jsSinglePageFileName:path + QDir::separator()+ jsBaseFileName),
                     path + QDir::separator()+ cssBaseFileName);
}

void ClientOperate::showWarnings(QString content)
{
    QMessageBox::warning(this,QStringLiteral("警告"),content,QMessageBox::Yes);
}

void ClientOperate::generateTask(AxurePage &page)
{
    QString outName = "/" + QFileInfo(page.htmlFilePath).path().split("/").last() + "_qt";
    QString outDir = ui->createOutDirRadioBtn->isChecked() ?ui->createOutDirLedit->text() + outName:QFileInfo(page.htmlFilePath).path() + "_qt";
    SwitchTask * task = new SwitchTask;
    connect(task,SIGNAL(updateProgress(SwitchProgress)),this,SLOT(updateProgress(SwitchProgress)),Qt::QueuedConnection);
    m_pool->enqueue([&](SwitchTask * switchTask,AxurePage & pp,QString outPath){
        switchTask->initTask(pp,outPath);
        delete switchTask;
    },task,page,outDir);
}

/**
 * @brief 是否已经将文件加入队列
 * @param[in] filePath 待判断文件全路径
 * @return true:文件已经存在
 */
bool ClientOperate::isRepeatedFile(QString filePath)
{
    auto iter = std::find_if(m_pageList.begin(),m_pageList.end(),[&filePath](const AxurePage & page){return page.htmlFilePath == filePath;});

    if(iter != m_pageList.end()){
        appendLog(LogWarn,QString(QStringLiteral("该文件已经存在[%1]")).arg((*iter).htmlFileName));
        return true;
    }
    return false;
}

/**
 * @brief 更新列表信息
 */
void ClientOperate::updateTableModel()
{
    m_model->setModelData(m_pageList);
    m_model->refreshModel();
}

void ClientOperate::appendLog(ClientOperate::LogLevel level, QString record)
{
    QString textColor;
    switch(level){
        case LogNormal:textColor = "#000000";break;
        case LogWarn:textColor = "#FFC000";break;
        case LogError:textColor = "#FF0000";break;
        default:break;
    }

    ui->textBrowser->append(QString("<font color=\"%1\">" + RUtil::getTimeStamp()+":"+record + "</font>").arg(textColor));
}

/**
 * @brief 转换单个html文件
 */
void ClientOperate::switchLineHtmlFile(QString pageId)
{
    Check_Return(!checkBeforeSwitch(),);

    auto iter =std::find_if(m_pageList.begin(),m_pageList.end(),[&pageId](const AxurePage & page){return page.id == pageId;});
    if(iter != m_pageList.end()){
        appendLog(LogError,QStringLiteral("开始转换..."));
        generateTask(*iter);
    }
}

/**
 * @brief 开始转换所有文件
 */
void ClientOperate::startSwitchFiles()
{
    Check_Return(!checkBeforeSwitch(),);

    appendLog(LogError,QString(QStringLiteral("准备转换[%1]个页面...")).arg(m_pageList.size()));
    std::for_each(m_pageList.begin(),m_pageList.end(),[&](AxurePage & page){
        generateTask(page);
    });
}

/*!
 * @brief 转换前验证相关参数是否具备
 */
bool ClientOperate::checkBeforeSwitch()
{
    if(m_pageList.size() == 0){
        showWarnings(QStringLiteral("转换列表为空，请选择axure工程。"));
        return false;
    }

    if(ui->createOutDirRadioBtn->isChecked()){
        QFileInfo fileInfo(ui->createOutDirLedit->text());
        if(!fileInfo.exists()){
            showWarnings(QStringLiteral("自定义导出目录不存在，请重新选择!"));
            return false;
        }
    }

    return true;
}

/**
 * @brief 控制控件使能情况
 * @param state控件使能状态
 */
void ClientOperate::controlWidget(bool state)
{
    ui->openAxPrjBtn->setEnabled(state);
    ui->clearList->setEnabled(state);
    ui->axureVerision->setEnabled(state);
    ui->qt4RadioBtn->setEnabled(state);
    ui->qt5RadioBtn->setEnabled(state);
    ui->sourceeDirRadioBtn->setEnabled(state);
    ui->createOutDirRadioBtn->setEnabled(state);
    ui->createOutDirLedit->setEnabled(state);
    ui->selectDirBtn->setEnabled(state);
    ui->tableView->setEnabled(state);
    ui->startChangeBtn->setEnabled(state);
}

/**
 * @brief 清空用户表格
 */
void ClientOperate::clearAxureTable()
{
    int result = QMessageBox::question(this,QStringLiteral("提示"),QStringLiteral("是否清空当前列表?"),QMessageBox::Yes | QMessageBox::No,QMessageBox::No);
    if(result == QMessageBox::Yes){
        m_pageList.clear();
        updateTableModel();
    }
}

/**
 * @brief 获取自定义存放目录路径
 */
void ClientOperate::chooseUserFilePath()
{
    QString dirPath = QFileDialog::getExistingDirectory(this,QStringLiteral("选择导出目录"),QDir::currentPath());
    if (dirPath.isEmpty())
        return;

    ui->createOutDirLedit->setText(dirPath);
}

/*!
 * @brief 调用本地浏览器查看文件
 * @param[in] htmlFilePath 待查看的html文件
 */
void ClientOperate::viewFile(QString htmlFilePath)
{
    QStringList arguments;
    arguments << "/c" << htmlFilePath;
    QProcess g_process;
    g_process.start("cmd.exe",arguments);
    g_process.waitForStarted();
    g_process.waitForFinished();
}

/**
 * @brief 删除一行数据
 * @param filePath 删除行文件名
 */
void ClientOperate::deletFileData(QString pageId)
{
    int result = QMessageBox::question(this,QStringLiteral("提示"),QStringLiteral("是否删除此记录?"),QMessageBox::Yes | QMessageBox::No,QMessageBox::No);

    if(result == QMessageBox::Yes){
        auto iter =std::find_if(m_pageList.begin(),m_pageList.end(),[&pageId](const AxurePage & page){
            return page.id == pageId;
        });

        if(iter != m_pageList.end()){
            m_pageList.erase(iter);
            updateTableModel();
        }
    }
}

/**
 * @brief 更新进度条
 */
void ClientOperate::updateProgress(SwitchProgress proj)
{
    QMutexLocker locker(&m_progressMutex);
    for(int i = 0;i < m_pageList.size();i++)
    {
        AxurePage & page = m_pageList[i];
        if(proj.pageId == page.id){
            page.processData = proj.progress;
            break;
        }
    }

    if(proj.progress.error){
        appendLog(LogError,"["+proj.pageName + "] "+ proj.progress.textDescription);
    }else{
        appendLog(LogNormal,"["+proj.pageName + "] "+ proj.progress.textDescription);
    }

    updateTableModel();
}
