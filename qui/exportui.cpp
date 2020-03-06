#include "exportui.h"

#include "props/mdomwidget.h"
#include "props/mdomresource.h"

namespace RQt{

ExportUi::ExportUi():m_bHasAttributeVersion(false),m_domWidget(nullptr),
    m_domResource(nullptr),m_children(0),m_initDevice(false)
{

}

ExportUi::~ExportUi()
{
    if(m_domWidget)
        delete m_domWidget;

    if(m_domResource)
        delete m_domResource;
}

bool ExportUi::beginWrite(QIODevice *device)
{
    if(device == nullptr || !device->isOpen())
        return false;

    m_xmlWriter.setDevice(device);
    m_xmlWriter.setAutoFormatting(true);
    m_xmlWriter.setAutoFormattingIndent(1);
    m_xmlWriter.writeStartDocument();
    m_initDevice = true;
    return true;
}

void ExportUi::endWrite()
{
    if(m_initDevice){
        write(m_xmlWriter);
        m_xmlWriter.writeEndDocument();
    }
    m_initDevice = false;
}

void ExportUi::write(QXmlStreamWriter &writer)
{
    writer.writeStartElement("ui");

    if(hasAttributeVersion())
        writer.writeAttribute("version",attributeVersion());

    if(m_children & Class)
        writer.writeTextElement("class",m_className);

    if(m_children & Widget)
        m_domWidget->write(writer,"widget");

    if(m_children & Resources)
        m_domResource->write(writer,"resources");


    writer.writeEndElement();
}

void ExportUi::setElementClass(QString clazzName)
{
    m_children |= Class;
    m_className = clazzName;
}

void ExportUi::setDomWidget(RDomWidget *domWidget)
{
    m_children |= Widget;
    m_domWidget = domWidget;
}

void ExportUi::setDomResource(MDomResource *domResource)
{
    m_children |= Resources;
    m_domResource = domResource;
}


} //namespace RQt
