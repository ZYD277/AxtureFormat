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
#include <QPainter>
#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QPropertyAnimation>
#include <QMenu>
#include <QAction>

#include "util/rutil.h"
#include "util/threadpool.h"
#include "viewdelegate.h"
#include "viewmodel.h"
#include "switchtask.h"

#define IMG_FIXED_WIDTH 120

ClientOperate::ClientOperate(QWidget *parent) :
    QMainWindow(parent),cssBaseFileName("styles.css"),jsBaseFileName("document.js"),jsSinglePageFileName("data.js"),
    ui(new Ui::ClientOperate),m_dirPathButtGroup(nullptr),m_viewDelegate(nullptr),m_model(nullptr),
    m_pool(new ThreadPool(5)),m_b_mouseActive(false)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("原型转换工具"));
    qRegisterMetaType<SwitchProgress>("SwitchProgress");

    if(m_normalPix.load(":/icon/image/normal.png")){
        m_normalPix = m_normalPix.scaled(IMG_FIXED_WIDTH,IMG_FIXED_WIDTH);
    }

    if(m_activePix.load(":/icon/image/active.png")){
        m_activePix = m_activePix.scaled(IMG_FIXED_WIDTH,IMG_FIXED_WIDTH);
    }

    if(m_waterMark.load(":/icon/image/watermark.png")){
        m_waterMark = m_waterMark.scaled(24,24);
    }

    initView();
}

ClientOperate::~ClientOperate()
{
    delete ui;
    delete m_model;
    delete m_viewDelegate;
    delete m_pool;
}

bool ClientOperate::eventFilter(QObject *watched, QEvent *event)
{
    if(watched == ui->dropArea){
        switch(event->type()){
            case QEvent::Paint:{
                QPainter  painter(ui->dropArea);
                painter.setRenderHints(QPainter::SmoothPixmapTransform,true);
                QRect availRect =  ui->dropArea->rect();

                int x = (availRect.width() - IMG_FIXED_WIDTH)/2;
                int y = (availRect.height() - IMG_FIXED_WIDTH)/3;

                if(m_b_mouseActive){
                    painter.drawPixmap(QRect(x,y,IMG_FIXED_WIDTH,IMG_FIXED_WIDTH),m_activePix);
                    painter.setPen(QColor(17,149,219));
                }else{
                    painter.drawPixmap(QRect(x,y,IMG_FIXED_WIDTH,IMG_FIXED_WIDTH),m_normalPix);
                    painter.setPen(QColor(205,205,205));
                }

                int t_textHeight = 30;

                QFont  font = painter.font();
                font.setPixelSize(18);
                font.setBold(true);
                painter.setFont(font);
                painter.drawText(QRect(0,availRect.height() / 3 * 2,availRect.width(),t_textHeight),Qt::AlignCenter,QStringLiteral("点击添加文件夹 / 推拽到此区域"));

                break;
            }
            case QEvent::Enter:{
                m_b_mouseActive = true;
                update();
                break;
            }
            case QEvent::Leave:{
                m_b_mouseActive = false;
                update();
                break;
            }
            case QEvent::MouseButtonPress:{
                QString projectPath = chooseAxureProject();
                if(!projectPath.isEmpty()){
                    parseAxureProject(projectPath);

                    switchMainWidget();
                }
            }
            case QEvent::DragEnter:{
                QDragEnterEvent * dragEnter = dynamic_cast<QDragEnterEvent *>(event);
                if(dragEnter){
                    if(dragEnter->mimeData()->hasUrls()){
                        QList<QUrl> urls = dragEnter->mimeData()->urls();
                        auto iter = std::find_if(urls.begin(),urls.end(),[&](QUrl & url){
                            QString path = url.path();
                            if(path.startsWith("/"))
                                path = path.remove(0,1);

                            return QFileInfo(path).isDir();
                        });

                        if(iter != urls.end()){
                            m_b_mouseActive = true;
                            update();

                            dragEnter->setDropAction(Qt::MoveAction);
                            dragEnter->accept();

                            return true;
                        }
                    }

                    dragEnter->acceptProposedAction();
                }
                break;
            }
            case QEvent::DragMove:{
                QDragMoveEvent * dragMove = dynamic_cast<QDragMoveEvent *>(event);
                if(dragMove->mimeData()->hasUrls()){
                    dragMove->setDropAction(Qt::MoveAction);
                    dragMove->accept();
                }else{
                    dragMove->acceptProposedAction();
                }
                break;
            }
            case QEvent::DragLeave:{
                m_b_mouseActive = false;
                update();

                break;
            }
            case QEvent::Drop:{
                QDropEvent * drop = dynamic_cast<QDropEvent *>(event);
                if(drop->mimeData()->hasUrls()){
                    bool t_bHasDir = false;
                    QList<QUrl> urls = drop->mimeData()->urls();
                    std::for_each(urls.begin(),urls.end(),[&](QUrl & url){
                        QString path = url.path();
                        if(path.startsWith("/"))
                            path = path.remove(0,1);

                        if(QFileInfo(path).isDir()){
                            t_bHasDir = true;
                            parseAxureProject(path);
                        }
                    });
                    if(t_bHasDir){
                        switchMainWidget();
                    }
                }
                break;
            }
            default:break;
        }
    }else if(watched == ui->widget_2){
        switch(event->type()){
            case QEvent::Paint:{
                QPainter  painter(ui->widget_2);
                painter.setRenderHints(QPainter::SmoothPixmapTransform,true);
                QRect availRect =  ui->widget_2->rect();

                painter.drawPixmap(QRect(QPoint((availRect.width() - m_waterMark.width())/2,availRect.height() - m_waterMark.height() * 1.5),m_waterMark.size())
                                   ,m_waterMark);

            }
            default:break;
        }
    }
    return QWidget::eventFilter(watched,event);
}

/*!
 * @brief 显示表格右键操作菜单
 */
void ClientOperate::showTableContextMenu(QPoint point)
{
    Q_UNUSED(point);

    QModelIndex modelIndex = ui->tableView->indexAt(point);
    if(!modelIndex.isValid() || modelIndex.row() < 0 && modelIndex.row() >= m_pageList.size())
        return;

    AxurePage page = m_pageList.at(modelIndex.row());

    QMenu * menu = new QMenu(ui->tableView);

    QAction * openHtmlAction = new QAction();
    openHtmlAction->setData(page.htmlFilePath);
    openHtmlAction->setIcon(QIcon(":/icon/image/html.png"));
    openHtmlAction->setText(QStringLiteral("打开"));
    connect(openHtmlAction,SIGNAL(triggered(bool)),this,SLOT(openHtml(bool)));
    menu->addAction(openHtmlAction);

    QAction * openAxureAction = new QAction();
    openAxureAction->setData(page.axureProjectPath);
    openAxureAction->setIcon(QIcon(":/icon/image/project.png"));
    openAxureAction->setText(QStringLiteral("打开Axure工程"));
    connect(openAxureAction,SIGNAL(triggered(bool)),this,SLOT(openHtml(bool)));
    menu->addAction(openAxureAction);

    if(page.outputDir.size() > 0){
        menu->addSeparator();
        QAction * openOutputAction = new QAction();
        openOutputAction->setData(page.outputDir);
        openOutputAction->setIcon(QIcon(":/icon/image/qt.png"));
        openOutputAction->setText(QStringLiteral("查看转换文件"));
        connect(openOutputAction,SIGNAL(triggered(bool)),this,SLOT(openHtml(bool)));
        menu->addAction(openOutputAction);
    }

    menu->exec(QCursor::pos());
    menu->deleteLater();
}

void ClientOperate::initView()
{
    setBaseSize(480,680);
    setFocusPolicy(Qt::ClickFocus);
    ui->stackedWidget->setCurrentIndex(0);

    m_settings = new QSettings("config.ini",QSettings::IniFormat);

    static int fixedSize = 10;
    if(!m_settings->contains("maxrecent")){
        m_settings->setValue("maxrecent",fixedSize);
    }
    int tmp = m_settings->value("maxrecent",fixedSize).toInt();
    m_maxRecentSize = tmp <= 0 ? fixedSize : (tmp > fixedSize ? fixedSize: tmp);

    int recentCount =  m_settings->beginReadArray("recent");
    for(int i = 0; i < recentCount && i < m_maxRecentSize; i++){
        m_settings->setArrayIndex(i);
        m_recentProjects<<m_settings->value("name").toString();
    }
    m_settings->endArray();

    QMenuBar * mbar = menuBar();
    QMenu * fileMenu = new QMenu(QStringLiteral("文件(&F)"));

    m_recentMenu = new QMenu(QStringLiteral("最近打开的项目(&R)"));
    if(m_recentProjects.size() > 0){
        for(QString projPath:m_recentProjects){
            QAction * action = new QAction(projPath);
            action->setData(projPath);
            connect(action,SIGNAL(triggered(bool)),this,SLOT(showRecentProject(bool)));
            m_recentMenu->addAction(action);
            m_recentActions.append(action);
        }
        m_recentMenu->addSeparator();

        QAction * clearAction = new QAction(QStringLiteral("清除菜单"));
        connect(clearAction,SIGNAL(triggered(bool)),this,SLOT(clearRecentProjs(bool)));
        m_recentMenu->addAction(clearAction);
    }

    fileMenu->addMenu(m_recentMenu);

    QAction * quitAction = new QAction(QStringLiteral("退出(&X)"));
    connect(quitAction,&QAction::triggered,[&](bool flag){
        this->close();
    });

    fileMenu->addSeparator();
    fileMenu->addAction(quitAction);

    mbar->addMenu(fileMenu);

    ui->widget_2->installEventFilter(this);
    ui->dropArea->installEventFilter(this);
    ui->dropArea->setAcceptDrops(true);

    m_model = new ViewModel();
    connect(m_model,SIGNAL(updateItemClassName(int,QString)),this,SLOT(updateItemClassName(int,QString)));
    connect(m_model,SIGNAL(updateItemError()),this,SLOT(updateItemError()));
    m_viewDelegate = new ViewDelegate();

    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setFocusPolicy(Qt::NoFocus);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->tableView->setModel(m_model);
    ui->tableView->setItemDelegate(m_viewDelegate);
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tableView,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(showTableContextMenu(QPoint)));

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

    m_dirPathButtGroup = new QButtonGroup(this);
    m_dirPathButtGroup->addButton(ui->sourceeDirRadioBtn,Path_Current);
    m_dirPathButtGroup->addButton(ui->createOutDirRadioBtn,Path_SelfDefine);
    m_dirPathButtGroup->setExclusive(true);

    ui->sourceeDirRadioBtn->setChecked(true);
    ui->createOutDirLedit->setEnabled(false);
    ui->selectDirBtn->setEnabled(false);

    connect(m_dirPathButtGroup, SIGNAL(buttonClicked(int)),this,SLOT(chooseOutputPath(int)));

    connect(ui->openAxPrjBtn,SIGNAL(pressed()),this,SLOT(openAxureProject()));
    connect(ui->clearList,SIGNAL(pressed()),this,SLOT(clearAxureTable()));
    connect(ui->selectDirBtn,SIGNAL(pressed()),this,SLOT(chooseUserFilePath()));
    connect(ui->startChangeBtn,SIGNAL(clicked(bool)),this,SLOT(startSwitchFiles()));
    connect(ui->viewRecord,SIGNAL(pressed()),this,SLOT(showLogWindow()));

    m_logger = new LogOutputer(this);
    m_logger->hide();
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
    QString projectPath = chooseAxureProject();

    Check_Return(projectPath.isEmpty(),);

    parseAxureProject(projectPath);
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
    QString outDir = ui->createOutDirRadioBtn->isChecked() ? ui->createOutDirLedit->text() + outName:QFileInfo(page.htmlFilePath).path() + "_qt";
    SwitchTask * task = new SwitchTask;
    connect(task,SIGNAL(updateProgress(SwitchProgress)),this,SLOT(updateProgress(SwitchProgress)),Qt::QueuedConnection);
    m_pool->enqueue([&](SwitchTask * switchTask,AxurePage & pp,QString outPath){
        switchTask->initTask(pp,outPath);
        delete switchTask;
    },task,page,outDir);
}

/*!
 * @brief 选择打开axure工程
 * @return 若取消则返回空,否则返回打开的路径
 */
QString ClientOperate::chooseAxureProject()
{
    QDir parent = QDir(m_lastAxureProjectPath);
    parent.cdUp();

    QString projectPath = QFileDialog::getExistingDirectory(this,QStringLiteral("选择Axure工程目录"),
                                                            m_lastAxureProjectPath.isEmpty() ? QDir::homePath() : parent.path());
    return projectPath;
}

/*!
 * @brief 解析axure工程目录
 * @details 验证指定目录是否为axure工程目录,若是则将目录下的html文件添加到转换表格
 */
void ClientOperate::parseAxureProject(QString projectPath)
{
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

        if(!m_recentProjects.contains(projectPath)){
            m_recentProjects.prepend(projectPath);

            QAction * action = new QAction(projectPath);
            action->setData(projectPath);
            connect(action,SIGNAL(triggered(bool)),this,SLOT(showRecentProject(bool)));

            if(m_recentActions.size() == 0){
                m_recentMenu->addAction(action);
                m_recentMenu->addSeparator();

                QAction * clearAction = new QAction(QStringLiteral("清除菜单"));
                connect(clearAction,SIGNAL(triggered(bool)),this,SLOT(clearRecentProjs(bool)));
                m_recentMenu->addAction(clearAction);
            }else{
                m_recentMenu->insertAction(m_recentActions.first(),action);
            }

            if(m_recentProjects.size() > m_maxRecentSize){
                m_recentProjects.removeLast();
                m_recentMenu->removeAction(m_recentActions.last());
                m_recentActions.removeLast();
            }

            m_recentActions.prepend(action);

            saveConfigFile();
        }

        appendLog(LogNormal,QStringLiteral("检测通用样式."));
        QPair<QString,QString> jsCssPair = getJsCssFile(basePath,false);

        QStringList nameFilters;
        nameFilters<<"*.html";

        QFileInfoList htmlFiles = pDir.entryInfoList(nameFilters,QDir::Files | QDir::NoDotAndDotDot);
        std::for_each(htmlFiles.begin(),htmlFiles.end(),[&](const QFileInfo & fileInfo){
            if(!isRepeatedFile(fileInfo.filePath())){
                AxurePage page;

                page.axureProjectPath = projectPath;
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

void ClientOperate::switchMainWidget()
{
    ui->stackedWidget->setCurrentIndex(1);
//    QSize screen = RUtil::screenSize(0);
//    QSize initWindowSize(1000,740);
    //    setGeometry(QRect(QPoint((screen.width() - initWindowSize.width())/2,(screen.height() - initWindowSize.height())/2),initWindowSize));
}

void ClientOperate::saveConfigFile()
{
    m_settings->beginWriteArray("recent",m_recentProjects.size());
    for(int i = 0; i < m_recentProjects.size(); i++){
        m_settings->setArrayIndex(i);
        m_settings->setValue("name",m_recentProjects.at(i));
    }
    m_settings->endArray();
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
 * @details 20200530 将相同Axure工程的文件创建合并行显示工程名称
 */
void ClientOperate::updateTableModel()
{
    QList<int> sameProjectStatstics;          //相同工程路路径下文件个数
    QString lastProjectPath;

    for(AxurePage page : m_pageList){
        if(lastProjectPath == page.axureProjectPath){
            sameProjectStatstics.last()++;
        }else{
            sameProjectStatstics.append(1);
        }

        lastProjectPath = page.axureProjectPath;
    }

    ui->tableView->clearSpans();

    int t_startRow = 0;
    for(int spanRow : sameProjectStatstics){
        ui->tableView->setSpan(t_startRow,1,spanRow,1);
        t_startRow += spanRow;
    }

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

    m_logger->appendLog(QString("<font color=\"%1\">" + RUtil::getTimeStamp("hh:mm:ss")+":"+record + "</font>").arg(textColor));
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
    ui->sourceeDirRadioBtn->setEnabled(state);
    ui->createOutDirRadioBtn->setEnabled(state);
    ui->createOutDirLedit->setEnabled(state);
    ui->selectDirBtn->setEnabled(state);
    ui->tableView->setEnabled(state);
    ui->startChangeBtn->setEnabled(state);
}

void ClientOperate::showLogWindow()
{
    QSize logSize(this->rect().width() / 3, this->rect().height());

    QPropertyAnimation *animation = new QPropertyAnimation(m_logger,"geometry");
    animation->setKeyValueAt(0,QRect(QPoint(rect().width(),rect().height() - logSize.height()),logSize));
    animation->setKeyValueAt(1,QRect(QPoint(rect().width() - logSize.width(),rect().height() - logSize.height()),logSize));
    animation->setDuration(400);
    animation->setEasingCurve(QEasingCurve::OutQuad);
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    m_logger->show();
}

void ClientOperate::openHtml(bool flag)
{
    Q_UNUSED(flag);
    QAction * action = dynamic_cast<QAction *>(QObject::sender());
    if(action){
        viewFile(action->data().toString());
    }
}

void ClientOperate::clearRecentProjs(bool flag)
{
    Q_UNUSED(flag);
    m_recentMenu->clear();
    m_recentProjects.clear();
    m_recentActions.clear();

    saveConfigFile();
}

void ClientOperate::showRecentProject(bool flag)
{
    Q_UNUSED(flag);
    QAction * action = dynamic_cast<QAction *>(QObject::sender());
    if(action){
        QString recentProj = action->data().toString();
        parseAxureProject(recentProj);
        ui->stackedWidget->setCurrentIndex(1);
    }
}

/*!
 * @brief 更新指定行类名
 * @param[in] row 待更新行
 * @param[in] newValue 新类名
 */
void ClientOperate::updateItemClassName(int row, QString newValue)
{
    if(m_pageList.size() > row && row >= 0 && newValue.size() > 0){
        m_pageList.operator [](row).switchClassName = newValue;
        updateTableModel();
    }
}

void ClientOperate::updateItemError()
{
    QMessageBox::warning(this,QStringLiteral("警告"),QStringLiteral("类名不符合命名规则!\n[1]不能含有空白字符;     \n[2]不能以字母或下划线开头.   "),QMessageBox::Yes);
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
 * @param[in] filePath 待打开文件路径，可能是html文件，也可是文件夹
 */
void ClientOperate::viewFile(QString filePath)
{
    if(filePath.size() == 0)
        return;

    QFileInfo fileInfo(filePath);
    if(fileInfo.isDir()){
        filePath.replace("/","\\");
        QProcess::startDetached("explorer " + filePath);
    }else{
        QStringList arguments;
        arguments << "/c" << filePath;
        QProcess g_process;
        g_process.start("cmd.exe",arguments);
        g_process.waitForStarted();
        g_process.waitForFinished();
    }
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
            page.outputDir = proj.outputDir;
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
