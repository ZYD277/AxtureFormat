#include "mdomresource.h"

namespace RQt{

MDomResource::MDomResource()
{

}

MDomResource::~MDomResource()
{

}

void MDomResource::write(QXmlStreamWriter &writer, QString tagName)
{
    writer.writeStartElement(tagName.isEmpty() ? "resources" : tagName);

    foreach(const QString & res,m_ress){
        writer.writeStartElement("include");
        writer.writeAttribute("location",res);
        writer.writeEndElement();
    }

    writer.writeEndElement();
}

void MDomResource::addResource(QString resourcePath)
{
    m_ress.append(resourcePath);
}

} //namespace RQt
