#include "clientoperate.h"
#include "ui_clientoperate.h"
#include "util/rutil.h"

#include <QFileDialog>
#include <QDebug>
#include <QDir>
#include <QProgressBar>
#include <QDateTime>
#include <QMessageBox>
#include<QDesktopServices>
#include<QUrl>
#include <QTextCodec>
#include <QProcess>
#include <QPainter>
#include <QItemDelegate>

#define PROPERTY_CURRENT_ROW "currentRow"
#define PROPERTY_FLIE_NAME "fileName"
#define PROPERTY_FLIE_PATH "filePath"

ClientOperate::ClientOperate(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ClientOperate)
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("Axture转Qt工具-RGCompany"));

    QtVsionButtonGroup = nullptr;
    DirPathButtonGroup = nullptr;
    m_model = nullptr;
    m_viewDelegate = nullptr;
    m_widget = nullptr;

    radioBtnSign = false;
    //    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    //    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    //    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    //    ui->tableWidget->setFocusPolicy(Qt::NoFocus);
    //    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    //    //设置各列宽度
    //    ui->tableWidget->setColumnWidth(0,10);
    //    ui->tableWidget->setColumnWidth(1,500);
    //    ui->tableWidget->setColumnWidth(2,50);
    //    ui->tableWidget->setColumnWidth(3,10);
    //    ui->tableWidget->setColumnWidth(4,10);
    //    ui->tableWidget->setShowGrid(false);
    //    QStringList t_headList;
    //    t_headList<<QStringLiteral("编号")<<QStringLiteral("文件名称")<<QStringLiteral("状态")
    //             <<QStringLiteral("打开")<<QStringLiteral("删除")<<QStringLiteral("转换");
    //    QHeaderView* t_headerView = ui->tableWidget->verticalHeader();
    //    t_headerView->setHidden(true);
    //    ui->tableWidget->setColumnCount(t_headList.size());
    //    ui->tableWidget->setHorizontalHeaderLabels(t_headList);
    //moudel测试

    connect(ui->startChangeBtn,SIGNAL(clicked(bool)),this,SLOT(startSwitchFiles()));
    innitSwitchNeedMsgStru();
}

ClientOperate::~ClientOperate()
{
    delete ui;
    delete m_model;
    delete m_viewDelegate;
}
void ClientOperate::innitSwitchNeedMsgStru()
{
    if(!m_widget)
    {
        m_widget = new Widget();
    }
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
    connect(ui->createOutDirLedit,SIGNAL(textChanged(QString)),this,SLOT(getNewProjectPath(QString)));

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
void ClientOperate::slot_DirPathGroupClicked(int)
{
    int t_radioId = DirPathButtonGroup->checkedId();
    switch(t_radioId)
    {
    case 0:
        radioBtnSign = false;
        g_switchNeedMsg.m_newDirectory = g_switchNeedMsg.m_rootDirectory;
        ui->createOutDirLedit->setEnabled(false);
        ui->selectDirBtn->setEnabled(false);
        break;
    case 1:
        radioBtnSign = true;
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
    //清空表或初始化标志
    m_dirList.clear();
    m_fileList.clear();

    QString t_srcDirPath = QFileDialog::getExistingDirectory(this, "choose src Directory","/");
    if (t_srcDirPath.isEmpty())
    {
        return;
    }
    g_switchNeedMsg.m_rootDirectory = t_srcDirPath;
    g_switchNeedMsg.m_newDirectory = t_srcDirPath;
    getFolderMsg(t_srcDirPath);
    getSuitableHtmlFile();
}

/**
 * @brief 获取文件夹下文件和目录
 * @param path根目录路径
 */
void ClientOperate::getFolderMsg(QString path)
{
    QDir t_getDir(path);
    if(!t_getDir.exists())
    {
        return;
    }
    t_getDir.setFilter(QDir::Dirs|QDir::Files);//筛选目录和文件
    t_getDir.setSorting(QDir::DirsFirst);//优先显示目录
    QFileInfoList t_list = t_getDir.entryInfoList();//获取文件信息列表
    Global::DirInfo t_dir;
    Global::FileInfo t_file;
    for(int i=0;i<t_list.size();i++)
    {
        QFileInfo t_fileInfo = t_list.at(i);
        if(t_fileInfo.fileName() == "." | t_fileInfo.fileName() == "..")
            continue;
        if(t_fileInfo.isDir())
        {
            t_dir.m_dirName = t_fileInfo.fileName();
            t_dir.m_dirPath = t_fileInfo.filePath();

            m_dirList.append(t_dir);
        }
        if(t_fileInfo.isFile())
        {
            t_file.m_fileName = t_fileInfo.fileName();
            t_file.m_fielPath = t_fileInfo.filePath();
            t_file.m_fileBaseName = t_fileInfo.baseName();
            t_file.m_filesuffix = t_fileInfo.suffix();
            m_fileList.append(t_file);
        }
    }
}

/**
 * @brief 过滤重复文件
 */
bool  ClientOperate::findHadHtmlFile(QString newfileName)
{
    if(m_suitableFileList.isEmpty())
    {
        return false;
    }
    else
    {
        for(int i=0; i<m_suitableFileList.size(); i++)
        {
            Global::FileInfo t_listFileInfo = m_suitableFileList.at(i);
            if(t_listFileInfo.m_fileName == newfileName)
            {
                QString t_logRecord = QString(QStringLiteral("该文件已经存在[%1]")).arg(newfileName);
                appendWarningRecord(t_logRecord);
                return true;
            }

        }
        return false;
    }
}
/**
 * @brief 获取合适的html文件
 */
void ClientOperate::getSuitableHtmlFile()
{
    int t_htmlNumber = 0;
    for(int i=0;i<m_fileList.size();i++)
    {
        Global::FileInfo t_file = m_fileList.at(i);
        if(t_file.m_filesuffix == "html")
        {
            if(findHadHtmlFile(t_file.m_fileName))
                continue;
            t_htmlNumber++;
            for(int j=0;j<m_dirList.size();j++)
            {
                Global::DirInfo t_dir = m_dirList.at(j);
                if(t_dir.m_dirName == "files")
                {
                    m_css = false;
                    getSuitableFile(t_file.m_fileBaseName,t_dir.m_dirPath);
                }
                if(t_dir.m_dirName == "images")
                {
                    m_image = false;
                    getSuitableFile(t_file.m_fileBaseName,t_dir.m_dirPath);
                }
            }
            if(m_css&&m_image)
            {
                m_suitableFileList.append(t_file);
            }
            else
            {
                QString t_logRecord;
                if((!m_css)&&(m_image))
                {
                    t_logRecord = QString(QStringLiteral("没有发现.css文件[%1]")).arg(t_file.m_fileName);
                }
                else if(m_css&&(!m_image))
                {
                    t_logRecord = QString(QStringLiteral("没有发现image文件[%1]")).arg(t_file.m_fileName);
                }
                else
                {
                    t_logRecord = QString(QStringLiteral("没有发现.css文件和image文件[%1]")).arg(t_file.m_fileName);
                }
                appendErrorRecord(t_logRecord);
            }
        }
    }
    if(t_htmlNumber <= 0)
    {
        QMessageBox::information(NULL,QStringLiteral("提示"),QStringLiteral("没有发现.html文件"));
    }
    updateListInformation();
}

/**
 * @brief 筛选合适的文件
 * @param baseFileName不加后缀的文件名
 * @param dirPath存放css或image的路径
 */
void ClientOperate::getSuitableFile(QString baseFileName,QString dirPath)
{
    QDir t_getDir(dirPath);
    t_getDir.setFilter(QDir::Dirs|QDir::Files);
    t_getDir.setSorting(QDir::DirsFirst);
    QFileInfoList t_list = t_getDir.entryInfoList();
    for(int i=0;i<t_list.size();i++)
    {
        QFileInfo t_fileInfo = t_list.at(i);
        if(t_fileInfo.fileName() == "." | t_fileInfo.fileName() == "..")
            continue;
        if(t_fileInfo.isDir())
        {
            if(t_fileInfo.fileName() == baseFileName)
            {
                QDir t_getFile(t_fileInfo.filePath());
                t_getFile.setFilter(QDir::Files);
                QFileInfoList t_targetList = t_getFile.entryInfoList();
                for(int j = 0;j<t_targetList.size();j++)
                {
                    QFileInfo t_targetFileInfo = t_targetList.at(j);
                    if(t_targetFileInfo.suffix() == "css")
                    {
                        m_css = true;
                    }
                    if(t_targetFileInfo.suffix() == "png")
                    {
                        m_image = true;
                    }
                    if(t_targetFileInfo.suffix() == "jpg")
                    {
                        m_image = true;
                    }
                    if(t_targetFileInfo.suffix() == "jpeg")
                    {
                        m_image = true;
                    }
                    if(t_targetFileInfo.suffix() == "gif")
                    {
                        m_image = true;
                    }
                    if(t_targetFileInfo.suffix() == "tif")
                    {
                        m_image = true;
                    }
                    if(t_targetFileInfo.suffix() == "bmp")
                    {
                        m_image = true;
                    }
                }
            }
        }
    }
}

/**
 * @brief 更新列表信息
 */
void ClientOperate::updateListInformation()
{
    //    ui->tableWidget->setRowCount(m_suitableFileList.size());
    //    for(int i=0;i<m_suitableFileList.size();i++)
    //    {
    //        Global::FileInfo t_fileinfo = m_suitableFileList.at(i);
    //        QTableWidgetItem *rowNumber  = new QTableWidgetItem();
    //        rowNumber->setText(QString::number(i+1));
    //        QTableWidgetItem *fileName = new QTableWidgetItem();
    //        fileName->setText(t_fileinfo.m_fileName);
    //        ui->tableWidget->setItem(i,0,rowNumber);
    //        ui->tableWidget->setItem(i,1,fileName);
    //        //添加进度条
    //        QProgressBar *t_showProgress = new QProgressBar(ui->tableWidget);
    //        t_showProgress->setStyleSheet("QProgressBar{border:2px solid grey; border-radius: 5px; background-color: #FFFFFF;}"
    //                                      "QProgressBar::chunk { background-color: #05B8CC;width: 20px;}"
    //                                      "QProgressBar{border:2px solid grey;border-radius:5px;text-align: center;}"
    //                                      "");
    //        t_showProgress->setValue(10);
    //        ui->tableWidget->setCellWidget(i,2,t_showProgress);

    //        //添加打开按钮
    //        QPushButton *t_openFile = new QPushButton(ui->tableWidget);
    //        t_openFile->setIcon(QIcon(":/icon/image/open.jpg"));
    //        t_openFile->setStyleSheet("QPushButton{color:black; background-color:transparent;}"
    //                                  "QPushButton:hover{background-color:cyan;transparent;}"
    //                                  "QPushButton:pressed{background-color:red;transparent;}");
    //        t_openFile->setProperty(PROPERTY_FLIE_NAME,t_fileinfo.m_fileName);
    //        t_openFile->setProperty(PROPERTY_FLIE_PATH,t_fileinfo.m_fielPath);
    //        ui->tableWidget->setCellWidget(i,3,t_openFile);
    //        connect(t_openFile,SIGNAL(clicked(bool)),this,SLOT(openLineHtmlFile()));
    //        //添加删除按钮
    //        QPushButton *t_deleteFile = new QPushButton(ui->tableWidget);
    //        t_deleteFile->setIcon(QIcon(":/icon/image/delete.jfif"));
    //        t_deleteFile->setStyleSheet("QPushButton{color:black; background-color:transparent;}"
    //                                  "QPushButton:hover{background-color:cyan;transparent;}"
    //                                  "QPushButton:pressed{background-color:red;transparent;}");
    ////        t_deleteFile->setProperty(PROPERTY_CURRENT_ROW,QString::number(i));
    //        t_deleteFile->setProperty(PROPERTY_FLIE_NAME,t_fileinfo.m_fileName);
    //        t_deleteFile->setProperty(PROPERTY_FLIE_PATH,t_fileinfo.m_fielPath);
    //        ui->tableWidget->setCellWidget(i,4,t_deleteFile);
    //        connect(t_deleteFile,SIGNAL(clicked(bool)),this,SLOT(deleteLineHtmlFile()));


    //        //添加转换图标
    //        QPushButton *t_switchFile = new QPushButton(ui->tableWidget);
    //        t_switchFile->setIcon(QIcon(":/icon/image/switch.jfif"));
    //        t_switchFile->setStyleSheet("QPushButton{color:black; background-color:transparent;}"
    //                                  "QPushButton:hover{background-color:cyan;transparent;}"
    //                                  "QPushButton:pressed{background-color:red;transparent;}");
    //        ui->tableWidget->setCellWidget(i,5,t_switchFile);
    //        connect(t_switchFile,SIGNAL(clicked(bool)),this,SLOT(switchLineHtmlFile()));
    //    }
    //    m_suitableFileList.clear();
    ui->tableView->verticalHeader()->hide();
    //    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);//按行选择
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);//不能编辑
    ui->tableView->setFocusPolicy(Qt::NoFocus);//小部件不接受聚焦
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    m_model->setModalList(&m_suitableFileList);
    ui->tableView->setModel(m_model);
    ui->tableView->setItemDelegate(m_viewDelegate);

    m_model->refrushModel();

    connect(m_viewDelegate,SIGNAL(deleteFileLine(QString)),this,SLOT(deletFileData(QString)));
}

/**
 * @brief 打开本行html文件
 */
void ClientOperate::openLineHtmlFile()
{
//    QPushButton *t_pButton = qobject_cast<QPushButton *>(sender());
//    QString t_openFileName;
//    QByteArray t_openFilePath;
//    if(t_pButton)
//    {
//        t_openFileName = t_pButton->property(PROPERTY_FLIE_NAME).toString();
//        t_openFilePath = t_pButton->property(PROPERTY_FLIE_PATH).toByteArray();
//    }
//    //        QTextCodec::setCodecForLocale(QTextCodec::codecForName("UTF8"));
//    //        QString strMessage = QString::fromLocal8Bit(t_openFilePath);
//    //        QTextCodec *codec = QTextCodec::codecForName("utf-8");
//    //        QDesktopServices :: openUrl(QUrl(codec->toUnicode(t_openFilePath)));
//    QStringList arguments;
//    QString start_path = t_openFilePath;
//    arguments << "/c" << start_path;

//    QProcess* process = new QProcess;

//    qDebug() << start_path << endl;
//    process->start("cmd.exe",arguments);  //开启新进程打开文件
}

/**
 * @brief 删除本行数据
 */
void ClientOperate::deleteLineHtmlFile()
{
    //    QPushButton *t_pButton = qobject_cast<QPushButton *>(sender());
    //    if(t_pButton)
    //    {
    //      int t_row  = t_pButton->property(PROPERTY_CURRENT_ROW).toInt();
    //      qDebug()<<__FILE__<<__FUNCTION__<<__LINE__<<"\n"
    //             <<t_row
    //             <<"\n";
    //      ui->tableWidget->removeRow(t_row);

    //    }
    //    int t_row = ui->tableWidget->currentRow();
    //    if(t_row < 0)
    //    {
    //        QMessageBox::information(NULL,QStringLiteral("error"),QStringLiteral("请选中删除行"));
    //    }
    //    else
    //    {
    //        QMessageBox::StandardButton butt = QMessageBox::information(this,QStringLiteral("提示"),QStringLiteral("是否删除列表"),
    //                                                                        QMessageBox::Yes|QMessageBox::No,
    //                                                                        QMessageBox::Yes);
    //         if(butt == QMessageBox::Yes)
    //         {
    //              ui->tableWidget->removeRow(t_row);
    //              //删除链表中本条记录
    //              QPushButton *t_pButton = qobject_cast<QPushButton *>(sender());
    //              QString t_openFileName;
    //              QString t_openFilePath;
    //              if(t_pButton)
    //              {
    //                  t_openFileName = t_pButton->property(PROPERTY_FLIE_NAME).toString();
    //                  t_openFilePath = t_pButton->property(PROPERTY_FLIE_PATH).toString();
    //              }
    //              for(int i = 0;i<m_suitableFileList.size();i++)
    //              {
    //                  Global::FileInfo t_deletFlieInfo = m_suitableFileList.at(i);
    //                  if(t_deletFlieInfo.m_fileName == t_openFileName)
    //                  {
    //                       m_suitableFileList.removeAt(i);
    //                  }
    //              }
    //         }
    //    }

}

/**
 * @brief 开始转换本html文件
 */
void ClientOperate::switchLineHtmlFile()
{

}

/**
 * @brief 开始转换所有文件
 */
void ClientOperate::startSwitchFiles()
{
    if(radioBtnSign&&(ui->createOutDirLedit->text().isNull()))
    {
       QMessageBox::information(NULL,QStringLiteral("提示"),QStringLiteral("请选择文件存放路径"));
    }
    //设置控件失能
    ui->openAxPrjBtn->setDisabled(true);
    ui->deletListBtn->setDisabled(true);
    ui->AxureVerisionCbox->setDisabled(true);
    ui->Qt4RadioBtn->setDisabled(true);
    ui->Qt5RadioBtn->setDisabled(true);
    ui->soureseDirRadioBtn->setDisabled(true);
    ui->createOutDirRadioBtn->setDisabled(true);
    ui->createOutDirLedit->setDisabled(true);
    ui->selectDirBtn->setDisabled(true);
    ui->tableView->setEnabled(false);
    //信号连接槽
    connect(m_widget,SIGNAL(updateProgress(QString)),this,SLOT(appendRecord(QString)));
    connect(this,SIGNAL(getHomePath(QString,QString)),m_widget,SLOT(chooseAxtureProject(QString,QString)));

    emit getHomePath(g_switchNeedMsg.m_rootDirectory,g_switchNeedMsg.m_newDirectory);
}

/**
 * @brief 清空用户表格
 */
void ClientOperate::on_deletListBtn_clicked()
{
    //    ui->tableWidget->clearContents();
    //    ui->tableWidget->clear();
    //    QMessageBox::StandardButton butt = QMessageBox::information(this,QStringLiteral("提示"),QStringLiteral("是否删除列表"),
    //                                                                    QMessageBox::Yes|QMessageBox::No,
    //                                                                    QMessageBox::Yes);
    //     if(butt == QMessageBox::Yes)
    //     {
    //         for(;ui->tableWidget->rowCount()!=0;)
    //         {
    //             ui->tableWidget->removeRow(ui->tableWidget->rowCount()-1);
    //         }
    //     }
    //     m_suitableFileList.clear();
    if(m_suitableFileList.size()>0)
    {
        m_suitableFileList.clear();
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
void ClientOperate::deletFileData(QString filename)
{
    if(m_suitableFileList.size()>0)
    {
        for(int i=0; i<m_suitableFileList.size();i++)
        {
            Global::FileInfo t_fileInfo = m_suitableFileList.at(i);
            if(t_fileInfo.m_fileName == filename)
            {
               m_suitableFileList.removeAt(i);
            }
        }
        m_model->refrushModel();
    }
    else
    {
        QMessageBox::information(NULL,QStringLiteral("提示"),QStringLiteral("没有发现数据"));
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
void ClientOperate::getNewProjectPath(QString)
{
    g_switchNeedMsg.m_newDirectory = ui->createOutDirLedit->text();
}
