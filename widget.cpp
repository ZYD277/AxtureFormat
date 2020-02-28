#include "widget.h"
#include "ui_widget.h"

#include <QDebug>
#include <QFileDialog>
#include <QDir>
#include <QMessageBox>

#include "head.h"
#include "html/htmlstruct.h"
#include "util/rutil.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget),cssBaseFileName("styles.css"),jsBaseFileName("document.js"),jsSinglePageFileName("data.js")
{
    ui->setupUi(this);
    setWindowTitle(QStringLiteral("Axture转Qt工具-RGCompany"));
    connect(ui->chooseAxtureProject,SIGNAL(pressed()),this,SLOT(chooseAxtureProject()));
    connect(this,SIGNAL(updateProgress(QString)),this,SLOT(appendRecord(QString)));
}

Widget::~Widget()
{
    delete ui;
}

/*!
 * @brief 分析axture导出工程
 * @details 1.标准的axture工程包含html文件、data文件夹(通用样式表)、files文件夹(以各个html文件命名的样式文件夹)
 *          2.解析通用css样式文件
 *          3.逐个解析单个页面(先解析页面的css样式，再解析html(结合css计算样式))
 *          4.将依赖的image文件夹拷贝至目的目录
 */
void Widget::chooseAxtureProject()
{
    QString projectPath = QFileDialog::getExistingDirectory(this,QStringLiteral("选择Axture工程目录"),QDir::homePath());
    QDir pDir(projectPath);
    if(projectPath.size() > 0 && pDir.exists()){
        ui->progressRecord->clear();

        emit updateProgress(QString(QStringLiteral("开始解析[%1]")).arg(pDir.path()));

        AxtureProject project;
        project.projectName = pDir.dirName();

        QString basePath = projectPath + QDir::separator() + "data";
        if(!checkJsCssExisted(basePath,false)){
            showWarnings(QStringLiteral("通用样式表或js文件不存在!"));
            return;
        }

        emit updateProgress(QStringLiteral("检测通用样式."));

        QPair<QString,QString> jsCssPair = getJsCssFile(basePath,false);
        project.baseJsFilePath = jsCssPair.first;
        project.baseCssFilePath = jsCssPair.second;

        QFileInfoList htmlFiles = pDir.entryInfoList(QDir::Files);
        std::for_each(htmlFiles.begin(),htmlFiles.end(),[&](const QFileInfo & fileInfo){
            AxturePage page;
            page.htmlFilePath = fileInfo.filePath();
            QString pageJsCssPath = fileInfo.absoluteDir().path() + QDir::separator() + "files" + QDir::separator() + fileInfo.baseName();

            if(checkJsCssExisted(pageJsCssPath)){
                jsCssPair = getJsCssFile(pageJsCssPath);
                page.jsFilePath = jsCssPair.first;
                page.cssFilePath = jsCssPair.second;

                emit updateProgress(QString(QStringLiteral("检测到页面[%1].")).arg(fileInfo.fileName()));

                project.pages.append(page);
            }
        });

        if(project.pages.size() > 0){
            //[2]
            CSS::CssParser cssParser;
            CSS::CssMap globalCssMap;
            CSS::ErrorMsg errorMsg;
            if(cssParser.parseFile(project.baseCssFilePath)){
                emit updateProgress(QString(QStringLiteral("解析通用样式成功.")));
                globalCssMap = cssParser.getParsedResult();
            }else{
                emit updateProgress(QString(QStringLiteral("解析通用样式失败,流程终止!")));
                errorMsg = cssParser.getParsedErrorMsg();
                emit updateProgress(errorMsg.getErrorMsg);
                return;
            }

            //[3]
            QString outputDir = projectPath + "_qt";
            std::for_each(project.pages.begin(),project.pages.end(),[&](const AxturePage & page){
                if(cssParser.parseFile(page.cssFilePath)){
                    if(m_htmlParser.parseHtmlFile(page.htmlFilePath)){
                        QString htmlName = QFileInfo(page.htmlFilePath).baseName();
                        emit updateProgress(QString(QStringLiteral("解析Html[%1]结束.")).arg(htmlName));

                        QString pageDir = outputDir + QDir::separator() + htmlName;
                        if(RUtil::createDir(outputDir) && RUtil::createDir(pageDir)){
                            QString pageUiName = pageDir + QDir::separator() + htmlName  +".ui";
                            m_qtOutput.save(m_htmlParser.getParsedResult(),globalCssMap,cssParser.getParsedResult(),pageUiName);

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
                                emit updateProgress(QString(QStringLiteral("拷贝图片资源[%1]结束.")).arg(htmlName));
                            }
                        }
                    }
                }
            });
        }
    }
}

void Widget::appendRecord(QString record)
{
    ui->progressRecord->append(RUtil::getTimeStamp()+":"+record);
}

void Widget::showWarnings(QString content)
{
    QMessageBox::warning(this,QStringLiteral("警告"),content,QMessageBox::Yes);
}

void Widget::showInfomation(QString content)
{
    QMessageBox::information(this,QStringLiteral("提示"),content,QMessageBox::Yes);
}

/*!
 * @brief 检查指定目录下js和css文件是否存在
 * @param[in] path 待检测的目录
 * @param[in] isSinglePage 是否为单个html页面，单个HTML页面和通用的js文件名不一致
 * @return true:都存在；false:至少有一个不存在
 */
bool Widget::checkJsCssExisted(QString path,bool isSinglePage)
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
QPair<QString, QString> Widget::getJsCssFile(QString path,bool isSinglePage)
{
    return qMakePair((isSinglePage?path + QDir::separator()+ jsSinglePageFileName:path + QDir::separator()+ jsBaseFileName),
                     path + QDir::separator()+ cssBaseFileName);
}
