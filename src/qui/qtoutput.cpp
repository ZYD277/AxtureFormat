#include "qtoutput.h"

#include <QFileInfo>
#include <QDir>
#include "qrc/qrcoutput.h"
#include "qss/qssoutput.h"
#include "qss/qssparsemethod.h"
#include "formatproperty.h"
#include "exportui.h"

namespace RQt{

QtOutput::QtOutput()
{

}

QtOutput::~QtOutput()
{

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
    FormatProperty propFormat;
    propFormat.setDataSource(ptr);
    propFormat.setCssMap(globalCss,pageCss);

    RDomWidget * root = propFormat.formart();
    if(root){
        QFile file(fullPath);
        if(!file.open(QFile::WriteOnly)){
            return false;
        }

        //[1]
        ExportUi ui;
        ui.beginWrite(&file);

        ui.setAttributeVersion("4.0");
        ui.setElementClass("Form");
        ui.setDomWidget(root);

        MDomResource * resc = new MDomResource;
        resc->addResource("res.qrc");
        ui.setDomResource(resc);

        ui.endWrite();

        m_selectorType = propFormat.getHtmlParsedResult();
        //[3]
        QSSOutput qss;

        qss.setCommonStyle(globalCss,pageCss,m_selectorType);

        QFileInfo uiPathQss(fullPath);
        QString qssPath = uiPathQss.path() + QDir::separator() + "qss";

        if(qss.save(qssPath)){
            //[2]
            QrcOutput qrc;
            qrc.addResources("/",propFormat.getResources() + qss.getResources());

            m_originalResoucesLinks = propFormat.getOriginalResources();

            QFileInfo uiPath(fullPath);
            QString resFile = "res.qrc";
            QString qrcPath = uiPath.path() + QDir::separator() + resFile;
            return qrc.save(qrcPath);
        }
    }
    return false;
}

} //namespace RQt
