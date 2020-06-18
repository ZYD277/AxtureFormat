#include "qtoutput.h"

#include <QDebug>
#include <QFileInfo>
#include <QDir>
#include "qrc/qrcoutput.h"
#include "qss/qssparsemethod.h"
#include "exportui.h"
#include "props/mdomwidget.h"
#include "props/mconnections.h"
#include "props/mconnection.h"

namespace RQt{

QtOutput::QtOutput()
{

}

QtOutput::~QtOutput()
{

}

/*!
 * @brief 1.将html节点保存至指定的xml文件 2.创建对应的资源文件 3.创建样式文件
 * @param[in] className 生成的UI控件窗口objectname
 * @param[in] ptr 待转换的html节点
 * @param[in] globalCss 全局的css信息
 * @param[in] pageCss 当前页面的css信息
 * @param[in] fullPath 文件保存全路径
 * @return true:保存成功
 */
bool QtOutput::save(QString className,DomHtmlPtr ptr, CSS::CssMap globalCss, CSS::CssMap pageCss, QString fullPath)
{
    propFormat.setDataSource(ptr);
    propFormat.setCssMap(globalCss,pageCss);

    RDomWidget * root = propFormat.formart();
    if(root){
        m_saveFullFilePath = fullPath;
        QFile file(m_saveFullFilePath);
        if(!file.open(QFile::WriteOnly)){
            return false;
        }

        //[1]
        ExportUi ui;
        ui.beginWrite(&file);

        ui.setAttributeVersion("4.0");
        ui.setElementClass(className);
        root->setAttributeName(className);
        ui.setDomWidget(root);

        MDomResource * resc = new MDomResource;
        resc->addResource("res.qrc");
        ui.setDomResource(resc);

        MConnections * conns = propFormat.connections();
        if(conns){
            ui.setConnections(conns);
        }

        ui.endWrite();

        m_originalResoucesLinks = propFormat.getOriginalResources();
        m_codeDatas = propFormat.getCodeDatas();

        return true;
    }
    return false;
}

/*!
 * @brief 保存QSS文件
 * @param[in] cssFileName qss文件名
 * @return
 */
bool QtOutput::saveQss(QString cssFileName)
{
    m_qssFileName = cssFileName;
    m_selectorType = propFormat.getHtmlParsedResult();
    //[3]
    qssOutput.setCommonStyle(propFormat.getGlobalCss(),propFormat.getPageCss(),m_selectorType);

    QFileInfo uiPathQss(m_saveFullFilePath);
    QString qssPath = uiPathQss.path() + QDir::separator() + m_qssFileName;

    return qssOutput.save(qssPath);
}

/*!
 * @brief 保存资源文件
 */
bool QtOutput::saveQrc()
{
    QrcOutput qrc;
    qrc.addResources("/",propFormat.getResources() + qssOutput.getResources());
    qrc.addResources("/style",QStringList(m_qssFileName));

    QFileInfo uiPath(m_saveFullFilePath);
    QString resFile = "res.qrc";
    QString qrcPath = uiPath.path() + QDir::separator() + resFile;
    return qrc.save(uiPath.path(),qrcPath);
}

} //namespace RQt
