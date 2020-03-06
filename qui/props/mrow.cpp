#include "mrow.h"

#include "mproperty.h"

namespace RQt{

MRow::MRow()
{

}

MRow::~MRow()
{
    qDeleteAll(m_props);
    m_props.clear();
}

void MRow::write(QXmlStreamWriter &writer, QString tagName)
{
    writer.writeStartElement(tagName.isEmpty()?"row":tagName);

    foreach(MProperty * hh,m_props)
        hh->write(writer,"property");

    writer.writeEndElement();
}

void MRow::addProperty(MProperty *prop)
{
    m_props.append(prop);
}

} //namespace RQt
