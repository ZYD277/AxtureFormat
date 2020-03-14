#include "clientoperate.h"
#include "ui_clientoperate.h"

#include <QFileDialog>
#include <QListView>
#include <QDebug>
#include <QDir>
#include <QMessageBox>
#include <QProcess>

#include "util/rutil.h"
#include "viewdelegate.h"
#include "viewmodel.h"

ClientOperate::ClientOperate(QWidget *parent) :
    QWidget(parent),cssBaseFileName("styles.css"),jsBaseFileName("document.js"),jsSinglePageFileName("data.js"),
    ui(new Ui::ClientOperate),m_versionButtGroup(nullptr),m_dirPathButtGroup(nullptr),m_viewDelegate(nullptr),m_model(nullptr)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("Axure转Qt工具-RGCompany"));

    sonThread = nullptr;
    m_signalFile = nullptr;

    if(!sonThread)
    {
        sonThread = new ThreadSwitch();
    }

    initView();
}

ClientOperate::~ClientOperate()
{
    delete ui;
    delete m_model;
    delete m_viewDelegate;

    delete sonThread;
    delete m_signalFile;
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

    connect(m_versionButtGroup, SIGNAL(buttonClicked(int)),this,SLOT(slot_QtVersionGroupClicked(int)));
    connect(m_dirPathButtGroup, SIGNAL(buttonClicked(int)),this,SLOT(slot_DirPathGroupClicked(int)));

    connect(ui->openAxPrjBtn,SIGNAL(pressed()),this,SLOT(openAxureProject()));
    connect(ui->clearList,SIGNAL(pressed()),this,SLOT(clearAxureTable()));
    connect(ui->selectDirBtn,SIGNAL(pressed()),this,SLOT(chooseUserFilePath()));
    connect(ui->startChangeBtn,SIGNAL(clicked(bool)),this,SLOT(startSwitchFiles()));
}

/**
 * @brief 获取qt版本
 */
void ClientOperate::slot_QtVersionGroupClicked(int)
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
void ClientOperate::slot_DirPathGroupClicked(int)
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
    QString projectPath = QFileDialog::getExistingDirectory(this,QStringLiteral("选择Axure工程目录"),
                                                            m_lastAxureProjectPath.isEmpty() ? QDir::homePath() : m_lastAxureProjectPath);
    Check_Return(projectPath.isEmpty(),);
    m_lastAxureProjectPath = projectPath;

    QDir pDir(projectPath);
    if(pDir.exists())
    {
        appendRecord(QString(QStringLiteral("开始解析[%1]")).arg(pDir.path()));

        AxtureProject project;
        project.projectName = pDir.dirName();
        project.projectPath = projectPath;

        QString basePath = projectPath + QDir::separator() + "data";
        if(!checkJsCssExisted(basePath,false)){
            showWarnings(QStringLiteral("通用样式表或js文件不存在!"));
            return;
        }

        appendRecord(QStringLiteral("检测通用样式."));
        QPair<QString,QString> jsCssPair = getJsCssFile(basePath,false);
        project.baseJsFilePath = jsCssPair.first;
        project.baseCssFilePath = jsCssPair.second;

        QStringList nameFilters;
        nameFilters<<"*.html";

        QFileInfoList htmlFiles = pDir.entryInfoList(nameFilters,QDir::Files | QDir::NoDotAndDotDot);
        std::for_each(htmlFiles.begin(),htmlFiles.end(),[&](const QFileInfo & fileInfo){
            if(!isRepeatedFile(fileInfo.filePath())){
                AxturePage page;
                page.htmlFilePath = fileInfo.filePath();
                page.htmlFileName = fileInfo.fileName();
                QString pageJsCssPath = fileInfo.absoluteDir().path() + QDir::separator() + "files" + QDir::separator() + fileInfo.baseName();

                if(checkJsCssExisted(pageJsCssPath)){
                    jsCssPair = getJsCssFile(pageJsCssPath);
                    page.jsFilePath = jsCssPair.first;
                    page.cssFilePath = jsCssPair.second;

                    appendRecord(QString(QStringLiteral("检测到页面[%1].")).arg(fileInfo.fileName()));

                    //初始化进度条数据
                    page.processData.currentTime = 0;
                    page.processData.finishedTime = 1;
                    page.processData.error = false;
                    page.processData.textDescription = QStringLiteral("0%");

                    project.pages.append(page);
                }
            }
        });

        if(project.pages.size() > 0){
            auto iter = std::find_if(m_axureProjList.begin(),m_axureProjList.end(),[&project](const AxtureProject & proj){return proj.projectPath == project.projectPath;});
            if(iter != m_axureProjList.end()){
                (*iter).pages.append(project.pages);
            }else{
                m_axureProjList.append(project);
            }

            updateTableModel();
        }
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

/**
 * @brief 是否已经将文件加入队列
 * @param[in] filePath 待判断文件全路径
 * @return true:文件已经存在
 */
bool ClientOperate::isRepeatedFile(QString filePath)
{
    for(int i = 0; i< m_axureProjList.size(); i++)
    {
        const AxtureProject & t_axureProj = m_axureProjList.at(i);

        auto iter =std::find_if(t_axureProj.pages.begin(),t_axureProj.pages.end(),[&filePath](const AxturePage & page){
            return page.htmlFilePath == filePath;
        });

        if(iter != t_axureProj.pages.end()){
            appendWarningRecord(QString(QStringLiteral("该文件已经存在[%1]")).arg((*iter).htmlFileName));
            return true;
        }
    }
    return false;
}

/**
 * @brief 更新列表信息
 */
void ClientOperate::updateTableModel()
{
    m_model->setModelData(m_axureProjList);
    m_model->refreshModel();
}

/**
 * @brief 开始转换指定html文件
 */
void ClientOperate::switchLineHtmlFile(QString htmlFilePath)
{
    if(!m_signalFile)
    {
        m_signalFile = new StartThreads();
        connect(m_signalFile,SIGNAL(updateProgress(QString)),this,SLOT(appendRecord(QString)));
        connect(m_signalFile,SIGNAL(updateErrorProgress(QString)),this,SLOT(appendErrorRecord(QString)));
        connect(m_signalFile,SIGNAL(updateWaringProgress(QString)),this,SLOT(appendWarningRecord(QString)));
        connect(m_signalFile,SIGNAL(updataProgressBar(AxturePage)),this,SLOT(updateProcess(AxturePage)));

        connect(this,SIGNAL(switchObjFile(AxturePage,QString,QString)),m_signalFile,SLOT(threadProcessingData(AxturePage,QString,QString)));
    }
    for(int i = 0; i<m_axureProjList.size();i++)
    {
        AxtureProject t_project = m_axureProjList.at(i);
        for(int j = 0;j<t_project.pages.size();j++)
        {
            AxturePage fileInfo = t_project.pages.at(j);
            if(fileInfo.htmlFilePath == htmlFilePath)
            {
                QString outputDir;
                QString projectPath = t_project.projectPath;
                if(ui->createOutDirRadioBtn->isChecked())
                {
                    ui->createOutDirLedit->text();
                    outputDir = QString("%1%2%3%4").arg(ui->createOutDirLedit->text())
                            .arg(QDir::separator())
                            .arg(t_project.projectName).arg(QStringLiteral("_qt"));
                }
                else
                    outputDir = projectPath + QStringLiteral("_qt");
                emit switchObjFile(fileInfo,outputDir,projectPath);
            }
        }
    }
}

/**
 * @brief 开始转换所有文件
 */
void ClientOperate::startSwitchFiles()
{
    if(m_axureProjList.size() == 0){
        showWarnings(QStringLiteral("转换列表为空，请选择axure工程。"));
        return;
    }

    if(ui->createOutDirRadioBtn->isChecked()){
        QFileInfo fileInfo(ui->createOutDirLedit->text());
        if(!fileInfo.exists()){
            showWarnings(QStringLiteral("自定义导出目录不存在，请重新选择!"));
            return;
        }
    }



    {
        //设置控件失能
//        controlWidget(false);

//        connect(sonThread,SIGNAL(updateProgress(QString)),this,SLOT(appendRecord(QString)));
//        connect(sonThread,SIGNAL(updateErrorProgress(QString)),this,SLOT(appendErrorRecord(QString)));
//        connect(sonThread,SIGNAL(updateWaringProgress(QString)),this,SLOT(appendWarningRecord(QString)));

//        connect(sonThread,SIGNAL(updataProgressBar(AxturePage)),this,SLOT(updateProcess(AxturePage)));

//        connect(sonThread,SIGNAL(updataWidgetState(bool)),this,SLOT(controlWidget(bool)));


//        for(int i = 0;i<m_axureProjList.size();i++)
//        {
//            AxtureProject t_rootDirInfo = m_axureProjList.at(i);
//            QString  t_customizedPath;
//            if(ui->createOutDirRadioBtn->isChecked())
//            {
//                t_customizedPath = QString("%1%2%3").arg(ui->createOutDirLedit->text()).arg(QDir::separator()).arg(t_rootDirInfo.projectName);
//                sonThread->getFileInfo(t_rootDirInfo,t_customizedPath);
//            }
//            else
//            {
//                sonThread->getFileInfo(t_rootDirInfo,t_rootDirInfo.projectPath);
//            }
//        }
//        sonThread->threadStart();
    }
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
        m_axureProjList.clear();

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
void ClientOperate::deletFileData(QString filePath)
{
    int result = QMessageBox::question(this,QStringLiteral("提示"),QStringLiteral("是否删除此记录?"),QMessageBox::Yes | QMessageBox::No,QMessageBox::No);

    if(result == QMessageBox::Yes){
        for(int i = 0;i < m_axureProjList.size(); i++)
        {
            AxtureProject & project = m_axureProjList.operator [](i);

            auto iter =std::find_if(project.pages.begin(),project.pages.end(),[&filePath](const AxturePage & page){
                return page.htmlFilePath == filePath;
            });

            if(iter != project.pages.end()){
                project.pages.erase(iter);
                updateTableModel();
                break;
            }
        }
    }
}

/**
 * @brief 更新日志记录(错误消息)
 * @param record更新数据
 */
void ClientOperate::appendErrorRecord(QString record)
{
    ui->textBrowser->append("<font color=\"#FF0000\">" +RUtil::getTimeStamp()+":"+record + "</font>");
}

/**
 * @brief 更新日志记录(警告消息)
 * @param record日志记录
 */
void ClientOperate::appendWarningRecord(QString record)
{
    ui->textBrowser->append("<font color=\"#A2AA00\">" +RUtil::getTimeStamp()+":"+record + "</font>");
}

/**
 * @brief 更新日志记录(正常消息)
 * @param record日志记录
 */
void ClientOperate::appendRecord(QString record)
{
    ui->textBrowser->append("<font color=\"#00FF00\">" +RUtil::getTimeStamp()+":"+record + "</font>");
}

/**
 * @brief 更新进度条
 */
void ClientOperate::updateProcess(AxturePage newProcessInfo)
{
    for(int i = 0;i<m_axureProjList.size();i++)
    {
        AxtureProject project = m_axureProjList.at(i);
        for(int j = 0;j<project.pages.size();j++)
        {
            AxturePage fileInfo = project.pages.at(j);
            if(fileInfo.htmlFilePath == newProcessInfo.htmlFilePath)
            {
                project.pages.replace(j,newProcessInfo);
                m_axureProjList.replace(i,project);
            }
        }
    }

    updateTableModel();
}
