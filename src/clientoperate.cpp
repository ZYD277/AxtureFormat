#include "clientoperate.h"
#include "ui_clientoperate.h"
#include "util/rutil.h"

#include <QFileDialog>
#include <QDebug>
#include <QDir>
#include <QProgressBar>
#include <QDateTime>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QTextCodec>
#include <QProcess>
#include <QPainter>
#include <QItemDelegate>

#include <QThreadPool>
#include <QApplication>

ClientOperate::ClientOperate(QWidget *parent) :
    QWidget(parent),cssBaseFileName("styles.css"),jsBaseFileName("document.js"),jsSinglePageFileName("data.js"),
    ui(new Ui::ClientOperate)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("Axture转Qt工具-RGCompany"));


    QtVsionButtonGroup = nullptr;
    DirPathButtonGroup = nullptr;
    m_model = nullptr;
    m_viewDelegate = nullptr;

    radioBtnSign = false;

    sonThread = nullptr;
    m_signalFile = nullptr;

    if(!sonThread)
    {
        sonThread = new ThreadSwitch();
    }

    connect(ui->startChangeBtn,SIGNAL(clicked(bool)),this,SLOT(startSwitchFiles()));
    innitSwitchNeedMsgStru();
}

ClientOperate::~ClientOperate()
{
    delete ui;
    delete m_model;
    delete m_viewDelegate;

    delete sonThread;
    delete m_signalFile;
}

void ClientOperate::innitSwitchNeedMsgStru()
{
    if(!m_model)
    {
        m_model = new ViewMoudel();

    }
    if(!m_viewDelegate)
    {
        m_viewDelegate = new ViewDelegate();
    }
    if(!QtVsionButtonGroup)
    {
        QtVsionButtonGroup = new QButtonGroup(this);
    }
    QtVsionButtonGroup->addButton(ui->Qt4RadioBtn,0);
    QtVsionButtonGroup->addButton(ui->Qt5RadioBtn,1);
    QtVsionButtonGroup->setExclusive(true);
    ui->Qt4RadioBtn->setChecked(true);

    if(!DirPathButtonGroup)
    {
        DirPathButtonGroup = new QButtonGroup(this);
    }
    DirPathButtonGroup->addButton(ui->soureseDirRadioBtn,0);
    DirPathButtonGroup->addButton(ui->createOutDirRadioBtn,1);
    DirPathButtonGroup->setExclusive(true);
    ui->soureseDirRadioBtn->setChecked(true);
    ui->createOutDirLedit->setEnabled(false);
    ui->selectDirBtn->setEnabled(false);

    connect(QtVsionButtonGroup, SIGNAL(buttonClicked(int)),this,SLOT(slot_QtVisonGroupClicked(int)));
    connect(DirPathButtonGroup, SIGNAL(buttonClicked(int)),this,SLOT(slot_DirPathGroupClicked(int)));
}

/**
 * @brief 获取qt版本
 */
void ClientOperate::slot_QtVisonGroupClicked(int)
{
    int t_radioId = QtVsionButtonGroup->checkedId();
    switch(t_radioId)
    {
    case 0:
        break;
    case 1:
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
    int t_radioId = DirPathButtonGroup->checkedId();
    switch(t_radioId)
    {
    case 0:
        ui->createOutDirLedit->setEnabled(false);
        ui->selectDirBtn->setEnabled(false);
        break;
    case 1:
        ui->createOutDirLedit->setEnabled(true);
        ui->selectDirBtn->setEnabled(true);
        break;
    default:
        break;
    }
}
/**
 * @brief 点击按钮获取文件夹根目录
 */
void ClientOperate::on_openAxPrjBtn_clicked()
{

    ui->textBrowser->clear();
    QString projectPath = QFileDialog::getExistingDirectory(this,QStringLiteral("选择Axture工程目录"),QDir::homePath());
    if (projectPath.isEmpty())
    {
        return;
    }
    QDir pDir(projectPath);
    if(projectPath.size() > 0 && pDir.exists()){
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

        QFileInfoList htmlFiles = pDir.entryInfoList(QDir::Files);
        std::for_each(htmlFiles.begin(),htmlFiles.end(),[&](const QFileInfo & fileInfo){
            AxturePage page;
            if(!project.pages.isEmpty())
            {
                if(findHadHtmlFile(fileInfo.filePath()))
                    return;
            }
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
                page.processData.switchState = true;
                page.processData.textDescription = QStringLiteral("0%");

                project.pages.append(page);
            }
        });
        if(updateBranch(project.projectName,project))
        {
            updateListInformation();
        }
        else
        {

            rootPathList.append(project);
            updateListInformation();
        }
    }
}

/**
 * @brief 更新分支内容
 * @param projectName分支根目录名
 * @return true分支已经存在不需添加，false分支不存在添加新分支
 */
bool ClientOperate::updateBranch(QString projectName,AxtureProject newProject)
{
    for(int i = 0;i<rootPathList.size();i++)
    {
        AxtureProject t_project = rootPathList.at(i);
        if(t_project.projectName == projectName)
        {
            for(int j = 0;j<newProject.pages.size();j++)
            {
                t_project.pages.append(newProject.pages.at(j));
            }
            return true;
        }
    }
    return false;
}
/** @brief 检查指定目录下js和css文件是否存在
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
 * @brief 过滤重复文件
 */
bool  ClientOperate::findHadHtmlFile(QString newFilePath)
{
    for(int i=0; i<rootPathList.size(); i++)
    {
        AxtureProject t_rootListInfo = rootPathList.at(i);
        for(int j = 0;j<t_rootListInfo.pages.size();j++)
        {
            AxturePage t_fileInfo = t_rootListInfo.pages.at(j);
            if(t_fileInfo.htmlFilePath == newFilePath)
            {
                QString t_logRecord = QString(QStringLiteral("该文件已经存在[%1]")).arg(t_fileInfo.htmlFileName);
                appendWarningRecord(t_logRecord);
                return true;
            }
        }
    }
    return false;
}

/**
 * @brief 更新列表信息
 */
void ClientOperate::updateListInformation()
{
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);//不能编辑
    ui->tableView->setFocusPolicy(Qt::NoFocus);//小部件不接受聚焦
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_model->setModalList(&rootPathList);
    ui->tableView->setModel(m_model);
    ui->tableView->setItemDelegate(m_viewDelegate);

    //设置指定列宽度
    ui->tableView->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->tableView->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    ui->tableView->horizontalHeader()->setSectionResizeMode(4, QHeaderView::Fixed);
    ui->tableView->horizontalHeader()->setSectionResizeMode(5, QHeaderView::Fixed);

    ui->tableView->setColumnWidth(0,50);
    ui->tableView->setColumnWidth(3,100);
    ui->tableView->setColumnWidth(4,100);
    ui->tableView->setColumnWidth(5,100);

    m_model->refrushModel();

    connect(m_viewDelegate,SIGNAL(deleteFileLine(QString)),this,SLOT(deletFileData(QString)));
    connect(m_viewDelegate,SIGNAL(switchSingleFile(QString)),this,SLOT(switchLineHtmlFile(QString)));
}

/**
 * @brief 开始转换指定html文件
 */
void ClientOperate::switchLineHtmlFile(QString htmlFilePath)
{
    if(!m_signalFile)
    {
        m_signalFile = new StartThreads();
    }
    connect(m_signalFile,SIGNAL(updateProgress(QString)),this,SLOT(appendRecord(QString)));
    connect(m_signalFile,SIGNAL(updateErrorProgress(QString)),this,SLOT(appendErrorRecord(QString)));
    connect(m_signalFile,SIGNAL(updateWaringProgress(QString)),this,SLOT(appendWarningRecord(QString)));

    connect(m_signalFile,SIGNAL(updataProgressBar(AxturePage)),this,SLOT(updateProcess(AxturePage)));

    connect(this,SIGNAL(switchObjFile(AxturePage,QString,QString)),m_signalFile,SLOT(threadProcessingData(AxturePage,QString,QString)));
    for(int i = 0; i<rootPathList.size();i++)
    {
        AxtureProject t_project = rootPathList.at(i);
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
    if((ui->createOutDirRadioBtn->isChecked())&&(ui->createOutDirLedit->text().isNull()))
    {
        QMessageBox::information(NULL,QStringLiteral("提示"),QStringLiteral("请选择文件存放路径"));
    }
    else
    {
        //设置控件失能
        controlWidget(false);

        connect(sonThread,SIGNAL(updateProgress(QString)),this,SLOT(appendRecord(QString)));
        connect(sonThread,SIGNAL(updateErrorProgress(QString)),this,SLOT(appendErrorRecord(QString)));
        connect(sonThread,SIGNAL(updateWaringProgress(QString)),this,SLOT(appendWarningRecord(QString)));

        connect(sonThread,SIGNAL(updataProgressBar(AxturePage)),this,SLOT(updateProcess(AxturePage)));

        connect(sonThread,SIGNAL(updataWidgetState(bool)),this,SLOT(controlWidget(bool)));


        for(int i = 0;i<rootPathList.size();i++)
        {
            AxtureProject t_rootDirInfo = rootPathList.at(i);
            QString  t_customizedPath;
            if(ui->createOutDirRadioBtn->isChecked())
            {
                t_customizedPath = QString("%1%2%3").arg(ui->createOutDirLedit->text()).arg(QDir::separator()).arg(t_rootDirInfo.projectName);
                sonThread->getFileInfo(t_rootDirInfo,t_customizedPath);
            }
            else
            {
                sonThread->getFileInfo(t_rootDirInfo,t_rootDirInfo.projectPath);
            }
        }
        sonThread->threadStart();
    }
}

/**
 * @brief 创建线程
 * @param page文件信息
 * @param outputDir输出目录
 * @param projectPath原工程目录
 */
void ClientOperate::createThreads(AxtureProject project,QString outputDir)
{

}
/**
 * @brief 控制控件使能情况
 * @param state控件使能状态
 */
void ClientOperate::controlWidget(bool state)
{
    ui->openAxPrjBtn->setEnabled(state);
    ui->deletListBtn->setEnabled(state);
    ui->AxureVerisionCbox->setEnabled(state);
    ui->Qt4RadioBtn->setEnabled(state);
    ui->Qt5RadioBtn->setEnabled(state);
    ui->soureseDirRadioBtn->setEnabled(state);
    ui->createOutDirRadioBtn->setEnabled(state);
    ui->createOutDirLedit->setEnabled(state);
    ui->selectDirBtn->setEnabled(state);
    ui->tableView->setEnabled(state);

}
/**
 * @brief 清空用户表格
 */
void ClientOperate::on_deletListBtn_clicked()
{
    if(rootPathList.size()>0)
    {
        rootPathList.clear();
        m_model->setModalList(&rootPathList);
        m_model->refrushModel();
    }
    else
    {
        QMessageBox::information(NULL,QStringLiteral("提示"),QStringLiteral("没有发现数据"));
    }
}

/**
 * @brief 获取自定义存放目录路径
 */
void ClientOperate::on_selectDirBtn_clicked()
{
    QString t_srcDirPath = QFileDialog::getExistingDirectory(this, "choose src Directory","/");
    if (t_srcDirPath.isEmpty())
    {
        return;
    }
    ui->createOutDirLedit->setText(t_srcDirPath);
}

/**
 * @brief 删除一行数据
 * @param filename删除行文件名
 */
void ClientOperate::deletFileData(QString htmlFilePath)
{
    for(int i = 0;i<rootPathList.size();i++)
    {
        AxtureProject project = rootPathList.at(i);
        for(int j = 0;j<project.pages.size();j++)
        {
            AxturePage fileInfo = project.pages.at(j);
            if(fileInfo.htmlFilePath == htmlFilePath)
            {
                project.pages.removeAt(j);
                rootPathList.replace(i,project);
                m_model->setModalList(&rootPathList);
                m_model->refrushModel();
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
    for(int i = 0;i<rootPathList.size();i++)
    {
        AxtureProject project = rootPathList.at(i);
        for(int j = 0;j<project.pages.size();j++)
        {
            AxturePage fileInfo = project.pages.at(j);
            if(fileInfo.htmlFilePath == newProcessInfo.htmlFilePath)
            {
                project.pages.replace(j,newProcessInfo);
                rootPathList.replace(i,project);
            }
        }
    }
    m_model->setModalList(&rootPathList);
    m_model->refrushModel();
    //    if(newProcessInfo.processData.currentTime == newProcessInfo.processData.finishedTime)
    //    {
    //        deletFileData(newProcessInfo.htmlFilePath);
    //    }
}

