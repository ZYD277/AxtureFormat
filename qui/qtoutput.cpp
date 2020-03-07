#include "qtoutput.h"

#include <QFileInfo>
#include <QDir>
#include "qtparsemethod.h"
#include "qrc/qrcoutput.h"
#include "qss/qssoutput.h"
#include "formatproperty.h"
#include "exportui.h"

namespace RQt{

QtOutput::QtOutput():m_parseMethod(nullptr)
{

}

QtOutput::~QtOutput()
{
    if(m_parseMethod)
        delete m_parseMethod;
}

/*!
 * @brief 1.将html节点保存至指定的xml文件 2.创建对应的资源文件 3.创建样式文件
 * @param[in] ptr 待转换的html节点
 * @param[in] globalCss 全局的css信息
 * @param[in] pageCss 当前页面的css信息
 * @param[in] fullPath 文件保存全路径
 * @return true:保存成功
 */
bool QtOutput::save(DomHtmlPtr ptr, CSS::CssMap globalCss, CSS::CssMap pageCss, QString fullPath)
{
    if(m_parseMethod == nullptr){
        m_parseMethod = new QtParseMethod();
    }

    m_parseMethod->setDataSource(ptr);
    m_parseMethod->setCssMap(globalCss,pageCss);

    QString resFile = "res.qrc";
    m_parseMethod->setResFile(resFile);

    FormatProperty propFormat;
    propFormat.setDataSource(ptr);
    propFormat.setCssMap(globalCss,pageCss);
    RDomWidget * root = propFormat.formart();
    if(root){
        QFile file("d:/aaa.ui");
        if(!file.open(QFile::WriteOnly)){
            return false;
        }

        ExportUi ui;
        ui.beginWrite(&file);

        ui.setAttributeVersion("4.0");
        ui.setElementClass("Form");
        ui.setDomWidget(root);

        MDomResource * resc = new MDomResource;
        resc->addResource("res.qrc");
        ui.setDomResource(resc);

        ui.endWrite();
    }

    //[1]
    RXmlFile xmlFile(fullPath);
    xmlFile.disableAutoAddXmlSuffix();
    xmlFile.setParseMethod(m_parseMethod,false);

    if(xmlFile.startSave()){

        //[2]
        QrcOutput qrc;
        qrc.addResources("/",m_parseMethod->getResources());

        m_originalResoucesLinks = m_parseMethod->getOriginalResources();

        QFileInfo uiPath(fullPath);
        QString qrcPath = uiPath.path() + QDir::separator() + resFile;

        if(qrc.save(qrcPath)){

            //[3]
            QSSOutput qss;
            qss.setCommonStyle(globalCss,pageCss);

            QFileInfo uiPath(fullPath);
            QString qssPath = uiPath.path() + QDir::separator() + "qss";

            return qss.save(qssPath);
        }
    }
    return false;
}

} //namespace RQt
