#include "mcontrolimprove.h"
namespace RQt{

McontrolImprove::McontrolImprove()
{

}
McontrolImprove::~McontrolImprove()
{

}

void McontrolImprove::write(QXmlStreamWriter &writer, QString tagName)
{
    writer.writeStartElement(tagName.isEmpty() ? "customwidget" : tagName);

    writer.writeTextElement("class",m_class);
    writer.writeTextElement("extends",m_extends);
    writer.writeStartElement("header");
    writer.writeAttribute("location", "global");
    writer.writeCharacters(m_header);
    writer.writeEndElement();

    writer.writeEndElement();
}

void McontrolImprove::setClass(QString className)
{
    m_class = className;
}
void McontrolImprove::setExtends(QString extendsName)
{
    m_extends = extendsName;
}
void McontrolImprove::setHeader(QString headerName)
{
    m_header = headerName;
}

} //namespace RQt
