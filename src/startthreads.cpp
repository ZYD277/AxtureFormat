#include "startthreads.h"

#include <QDebug>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>

#include <QThreadPool>
#include <QThread>
#include <QRunnable>
#include <QMutex>
#include <QFuture>
#include <QTime>

#include <QQueue>

#include "head.h"
#include "html/htmlstruct.h"
#include "util/rutil.h"

#include <QMetaType>
StartThreads::StartThreads(QWidget *parent) : QWidget(parent)
{
    qRegisterMetaType<AxturePage>("AxturePage");//在线程中传递信号时参数默认放到队列中需要注册
}

/**
 * @brief 处理单个文件
 * @param page单个文件信息
 * @param outputDir输出目录
 * @param projectPath文件根目录
 */
void StartThreads::threadProcessingData(AxturePage page,QString outputDir,QString projectPath)
{
    CSS::CssParser cssParser;
    CSS::CssMap globalCssMap;
    if(cssParser.parseFile(page.cssFilePath)){

        updataProcessBar(page,1,true);//更新进度条

        if(m_htmlParser.parseHtmlFile(page.htmlFilePath)){

            updataProcessBar(page,2,true);

            QString htmlName = QFileInfo(page.htmlFilePath).baseName();
            emit updateProgress(QString(QStringLiteral("解析Html[%1]结束.")).arg(htmlName));
            QString pageDir = outputDir + QDir::separator() + htmlName;
            if(RUtil::createDir(outputDir) && RUtil::createDir(pageDir)){
                QString pageUiName = pageDir + QDir::separator() + htmlName  +".ui";
                m_qtOutput.save(m_htmlParser.getParsedResult(),globalCssMap,cssParser.getParsedResult(),pageUiName);

                updataProcessBar(page,3,true);

                //[4]
                QString baseImagePath = projectPath + QDir::separator() + "images" + QDir::separator();
                QString srcImagePath = baseImagePath + QDir::separator() + htmlName;
                QDir srcImageDir(srcImagePath);
                QString dstImagePath = pageDir + QDir::separator() + "images";
                if(RUtil::createDir(dstImagePath)){
                    //根据图片引用的资源链接去拷贝
                    QStringList resourcesLinks = m_qtOutput.getOriginalResouces();
                    foreach(const QString & links,resourcesLinks){
                        QString imgPath = projectPath + QDir::separator() + links;
                        QString dstImageFullPath = dstImagePath + QDir::separator() + QFileInfo(imgPath).fileName();
                        QFile::copy(imgPath,dstImageFullPath);
                    }

                    //拷贝对应目录下所有文件
                    QFileInfoList srcFileInfos = srcImageDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
                    std::for_each(srcFileInfos.begin(),srcFileInfos.end(),[&](QFileInfo & finfo){
                        QString dstImageFullPath = dstImagePath + QDir::separator() + finfo.fileName();
                        QFile::copy(finfo.filePath(),dstImageFullPath);
                    });
                    updataProcessBar(page,4,true);
                }
                else
                    updataProcessBar(page,4,false);
            }
            else
               updataProcessBar(page,3,false);
        }
        else
           updataProcessBar(page,2,false);
    }
    else
       updataProcessBar(page,1,false);
}

/**
 * @brief 更新进度条
 * @param page单个文件结构体
 */
void StartThreads::updataProcessBar(AxturePage page,int curentProcess,bool success)
{
    //更新进度条数据
    page.processData.currentTime = curentProcess;
    page.processData.finishedTime = 4;
    page.processData.switchState = success;
    float t_data = (float)page.processData.currentTime/(float)page.processData.finishedTime;
    if(success)
    {
        page.processData.textDescription = QString(QStringLiteral("正在转换%1%")).arg(QString::number(t_data*100));
    }
    else
        page.processData.textDescription = QString(QStringLiteral("转换失败%1%")).arg(QString::number(t_data*100));
    emit updataProgressBar(page);
}
//void StartThreads::chooseAxtureProject(AxtureProject project,QString targetProPath)
//{
//    //[3]
//    //最大允许开启十个线程
//    //            QThreadPool mythreadpool;
//    //            mythreadpool.setMaxThreadCount(THREAD_NUM);
//    //        QString outputDir = targetProPath + "_qt";

//    //        QThreadPool::globalInstance()->setMaxThreadCount(THREAD_NUM);
//    //        std::for_each(project.pages.begin(),project.pages.end(),[&](const AxturePage & page){
//    //            QFuture<void> f = QtConcurrent::run(this,&StartThreads::threadProcessingData,page,outputDir,project.projectPath);
//    //        });
//    //        for(int i = 0;i<project.pages.size();i++)
//    //        {
//    //            AxturePage fileInfo = project.pages.at(i);
//    //            QtConcurrent::run(this,&StartThreads::threadProcessingData,fileInfo,outputDir,project.projectPath);
//    //        }
//    //        std::for_each(project.pages.begin(),project.pages.end(),[&](const AxturePage & page){
//    //            threadProcessingData(page,outputDir,project.projectPath);
//    //        });
//    if(project.pages.size() > 0){
//        //[2]
//        CSS::CssParser cssParser;
//        CSS::CssMap globalCssMap;
//        CSS::ErrorMsg errorMsg;
//        if(cssParser.parseFile(project.baseCssFilePath)){
//            emit updateProgress(QString(QStringLiteral("解析通用样式成功.")));
//            globalCssMap = cssParser.getParsedResult();
//        }else{
//            emit updateErrorProgress(QString(QStringLiteral("解析通用样式失败,流程终止!")));
//            errorMsg = cssParser.getParsedErrorMsg();
//            emit updateErrorProgress(errorMsg.errorMsg);
//            return;
//        }
//        QString outputDir = targetProPath + "_qt";
//        //        QString projectPath = project.projectPath;
//        //        QMutex threadLock;
//        //        QWaitCondition queueFull;
//        //        QWaitCondition queueEmply;
//        //        QQueue<AxturePage> myQueue;
//        QThreadPool mythreadpool;
//        QMutex threadLock;
//        mythreadpool.setMaxThreadCount(THREAD_NUM);
//        QtConcurrent::run(&mythreadpool,[&]()
//        {
//            if(project.pages.size() > THREAD_NUM)
//            {
//                int startNum = 0;
//                int endNum = project.pages.size()/THREAD_NUM;
//                for(int i = startNum;i<endNum;i++)
//                {
//                    AxturePage fileInfo = project.pages.at(i);
//                    threadLock.lock();
//                    threadProcessingData(fileInfo,outputDir,project.projectPath);
//                    threadLock.unlock();
//                }
//            }
//        });
//        QtConcurrent::run(&mythreadpool,[&]()
//        {
//            if(project.pages.size() > THREAD_NUM)
//            {
//                int startNum = (project.pages.size()/THREAD_NUM);
//                int endNum = (project.pages.size()/THREAD_NUM)*2;
//                for(int i = startNum;i<endNum;i++)
//                {
//                    AxturePage fileInfo = project.pages.at(i);
//                    threadLock.lock();
//                    threadProcessingData(fileInfo,outputDir,project.projectPath);
//                    threadLock.unlock();
//                }
//            }
//        });
//        QtConcurrent::run(&mythreadpool,[&]()
//        {
//            if(project.pages.size() > THREAD_NUM)
//            {
//                int startNum = (project.pages.size()/THREAD_NUM)*2;
//                int endNum = (project.pages.size()/THREAD_NUM)*3;
//                for(int i = startNum;i<endNum;i++)
//                {
//                    AxturePage fileInfo = project.pages.at(i);
//                    threadLock.lock();
//                    threadProcessingData(fileInfo,outputDir,project.projectPath);
//                    threadLock.unlock();
//                }
//            }
//        });
//        QtConcurrent::run(&mythreadpool,[&]()
//        {
//            if(project.pages.size() > THREAD_NUM)
//            {
//                int startNum = (project.pages.size()/THREAD_NUM)*3;
//                int endNum = project.pages.size();
//                for(int i = startNum;i<endNum;i++)
//                {
//                    AxturePage fileInfo = project.pages.at(i);
//                    threadLock.lock();
//                    threadProcessingData(fileInfo,outputDir,project.projectPath);
//                    threadLock.unlock();
//                }
//            }
//        });
//        mythreadpool.waitForDone(-1);
//    }
//}
