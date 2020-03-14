#include "threadswitch.h"

#include <QFileDialog>
#include <QDebug>
#include "head.h"
#include "html/htmlstruct.h"
#include "util/rutil.h"

#include <QTime>
#include <QDateTime>

ThreadSwitch::ThreadSwitch()
{
    threadSymbol = false;
    qRegisterMetaType<AxturePage>("AxturePage");//在线程中传递信号时参数默认放到队列中需要注册
}

ThreadSwitch::~ThreadSwitch()
{

}

/**
 * @brief 开启线程
 */
void ThreadSwitch::threadStart()
{
    if(threadSymbol)
    {

    }
    else
    {
        threadSymbol = true;
        this->start();
    }
}

/**
 * @brief 关闭线程
 */
void ThreadSwitch::threadStop()
{
    if(threadSymbol)
    {
        threadSymbol = false;
        //        this->terminate();
        this->quit();
        this->wait();
    }
}

/**
 * @brief 获取数据
 */
void ThreadSwitch::getFileInfo(AxtureProject project,QString targetProPath)
{
    if(project.pages.size() > 0)
    {
        //[2]
        CSS::CssParser cssParser;
        CSS::CssMap globalCssMap;
        CSS::ErrorMsg errorMsg;
        if(cssParser.parseFile(project.baseCssFilePath)){
            emit updateProgress(QString(QStringLiteral("解析通用样式成功.")));
            globalCssMap = cssParser.getParsedResult();
        }else{
            emit updateErrorProgress(QString(QStringLiteral("解析通用样式失败,流程终止!")));
            errorMsg = cssParser.getParsedErrorMsg();
            emit updateErrorProgress(errorMsg.errorMsg);
            return;
        }
        outputDir = targetProPath + QStringLiteral("_qt");
        projectPath = project.projectPath;
        for(int i = 0;i<project.pages.size();i++)
        {
            AxturePage page = project.pages.at(i);
            QMutexLocker t_locker(&t_lock);
            myQueue.enqueue(page);
            condition.wakeAll();
        }
    }
}

/**
 * @brief 子线程执行转换
 */
void ThreadSwitch::run()
{
    while(threadSymbol)
    {
        while(myQueue.isEmpty())
        {
            t_lock.lock();
            condition.wait(&t_lock);
            t_lock.unlock();
        }
        if(myQueue.count()>0)
        {
            QMutexLocker t_locker(&t_lock);
            AxturePage page = myQueue.dequeue();
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
        if(myQueue.count()<=0)
        {
            emit updataWidgetState(true);
            threadStop();
        }
    }
}
/**
 * @brief 更新进度条
 * @param page单个文件结构体
 */
void ThreadSwitch::updataProcessBar(AxturePage page,int curentProcess,bool symbol)
{
    //更新进度条数据
    page.processData.currentTime = curentProcess;
    page.processData.finishedTime = 4;
    page.processData.error = symbol;
    float t_data = (float)page.processData.currentTime/(float)page.processData.finishedTime;
    if(symbol)
    {
        page.processData.textDescription = QString(QStringLiteral("正在转换%1%")).arg(QString::number(t_data*100));
    }
    else
        page.processData.textDescription = QString(QStringLiteral("转换失败%1%")).arg(QString::number(t_data*100));
    emit updataProgressBar(page);
}
