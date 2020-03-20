#include "mattribute.h"

namespace RQt{

MAttribute::MAttribute():m_kind(Unknown)
{

}

MAttribute::~MAttribute()
{

}

void MAttribute::write(QXmlStreamWriter &writer, QString tagName)
{
    writer.writeStartElement(tagName.isEmpty() ? "attribute" : tagName);

    if(hasAtributeName())
        writer.writeAttribute("name",attributeName());
    if(m_kind == Bool)
        writer.writeTextElement("bool",m_propString);
    else if(m_kind == Number)
        writer.writeTextElement("number",m_propString);

    writer.writeEndElement();
}

void MAttribute::setAttributeBool(QString value)
{
    m_kind = Bool;
    m_propString = value;
}

void MAttribute::setPropNumber(QString value)
{
    m_kind = Number;
    m_propString = value;
}

} //namespace RQt
