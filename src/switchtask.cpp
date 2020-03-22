#include "switchtask.h"

#include <iostream>
#include <thread>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QDateTime>
#include <QDebug>

#include "css/cssparser.h"
#include "util/rutil.h"

SwitchTask::SwitchTask()
{

}

SwitchTask::~SwitchTask()
{

}

void SwitchTask::initTask(AxurePage page, QString outputDir)
{
    m_page = page;

    Progress tproj = P_CssParse;
    bool error = false;
    QString errorMsg = QStringLiteral("转换失败");

    do{
        CSS::CssParser cssParser;
        CSS::CssMap globalCssMap;

        if(cssParser.parseFile(page.cssFilePath)){

            updataProcessBar(tproj,error,QStringLiteral("CSS解析成功"));

            if(m_htmlParser.parseHtmlFile(page.htmlFilePath)){
                tproj = P_HtmlParse;
                updataProcessBar(tproj,error,QStringLiteral("Html解析成功"));

                QString htmlName = QFileInfo(page.htmlFilePath).baseName();
                m_outputDir = outputDir + QDir::separator() + htmlName;

                if(RUtil::createDir(outputDir) && RUtil::createDir(m_outputDir)){
                    QString pageUiName = m_outputDir + QDir::separator() + htmlName  +".ui";
                    m_qtOutput.save(m_htmlParser.getParsedResult(),globalCssMap,cssParser.getParsedResult(),pageUiName);

                    tproj = P_CopyFile;
                    updataProcessBar(tproj,error,QStringLiteral("转换ui结束"));

                    //[4]
                    QFileInfo dir(page.htmlFilePath);

                    QString baseImagePath = dir.path() + QDir::separator() + "images" + QDir::separator();
                    QString srcImagePath = baseImagePath + QDir::separator() + htmlName;
                    QDir srcImageDir(srcImagePath);
                    QString dstImagePath = m_outputDir + QDir::separator() + "images";
                    if(RUtil::createDir(dstImagePath)){
                        //根据图片引用的资源链接去拷贝
                        QStringList resourcesLinks = m_qtOutput.getOriginalResouces();
                        foreach(const QString & links,resourcesLinks){
                            QString imgPath = dir.path() + QDir::separator() + links;
                            QString dstImageFullPath = dstImagePath + QDir::separator() + QFileInfo(imgPath).fileName();
                            QFile::copy(imgPath,dstImageFullPath);
                        }

                        //拷贝对应目录下所有文件
                        QFileInfoList srcFileInfos = srcImageDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);
                        std::for_each(srcFileInfos.begin(),srcFileInfos.end(),[&](QFileInfo & finfo){
                            QString dstImageFullPath = dstImagePath + QDir::separator() + finfo.fileName();
                            QFile::copy(finfo.filePath(),dstImageFullPath);
                        });

                        tproj = P_Finish;
                        updataProcessBar(tproj,error,QStringLiteral("转换结束"));
                        return;
                    }else{
                        errorMsg = QStringLiteral("创建图片导出目录失败");
                    }
                }else{
                    errorMsg = QStringLiteral("创建导出目录失败");
                }
            }else{
                errorMsg = QStringLiteral("解析Html失败");
            }
        }else{
            errorMsg = QStringLiteral("解析CSS样式失败");
        }
    }while(0);

    updataProcessBar(tproj,true,errorMsg);
}

void SwitchTask::updataProcessBar(Progress curentProcess, bool hasError,QString description)
{
    SwitchProgress prog;

    prog.pageName = m_page.htmlFileName;
    prog.pageId = m_page.id;
    prog.outputDir = m_outputDir;
    prog.progress.error = hasError;

    int proj = (int)curentProcess/(float)P_Finish * 100;

    prog.progress.m_progress = proj;
    prog.progress.textDescription = description;

    emit updateProgress(prog);
}
