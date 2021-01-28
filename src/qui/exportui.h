/*!
 *  @brief     将内存节点导出成xml的ui文件
 *  @details   使用sax方式生成xml文件
 *  @author    wey
 *  @version   1.0
 *  @date      2020.03.04
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef EXPORTUI_H
#define EXPORTUI_H

#include <QString>
#include <QXmlStreamWriter>

#include "props/mdomresource.h"

namespace RQt{

class RDomWidget;
class MDomResource;
class MConnections;
class McontrolImproves;
class ExportUi
{
public:
    ExportUi();
    ~ExportUi();

    bool beginWrite(QIODevice * device);
    void endWrite();

    void setAttributeVersion(QString version){m_attrVersion = version;m_bHasAttributeVersion = true;}
    inline bool hasAttributeVersion()const {return m_bHasAttributeVersion;}
    QString attributeVersion()const{return m_attrVersion;}

    void setElementClass(QString clazzName);
    void setDomWidget(RDomWidget * domWidget);
    void setDomResource(MDomResource * domResource);
    void setConnections(MConnections * conns);
    void setCustomControls(McontrolImproves *controlImproves);

private:
    ExportUi & operator=(const ExportUi &) = delete;
    ExportUi(const ExportUi &) = delete;

    void write(QXmlStreamWriter & writer);

private:
    enum Child {
        Author = 1,
        Comment = 2,
        ExportMacro = 4,
        Class = 8,
        Widget = 16,
        LayoutDefault = 32,
        LayoutFunction = 64,
        PixmapFunction = 128,
        CustomWidgets = 256,
        TabStops = 512,
        Images = 1024,
        Includes = 2048,
        Resources = 4096,
        Connections = 8192,
        Designerdata = 16384,
        Slots = 32768,
        ButtonGroups = 65536
    };

private:
    QXmlStreamWriter m_xmlWriter;
    bool m_initDevice;

    bool m_bHasAttributeVersion;
    QString m_attrVersion;

    uint m_children;            /*!< 子节点集合 */

    RDomWidget * m_domWidget;
    MDomResource * m_domResource;
    MConnections * m_conns;
    McontrolImproves * m_controlImproves;
    QString m_className;
};

} //namespace RQt

#endif // EXPORTUI_H
