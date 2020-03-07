#include "mcolumn.h"

#include "mproperty.h"

namespace RQt{

MColumn::MColumn()
{

}

MColumn::~MColumn()
{
    qDeleteAll(m_props);
    m_props.clear();
}

void MColumn::write(QXmlStreamWriter &writer, QString tagName)
{
    writer.writeStartElement(tagName.isEmpty()?"row":tagName);

    foreach(MProperty * hh,m_props)
        hh->write(writer,"property");

    writer.writeEndElement();
}

void MColumn::addProperty(MProperty *prop)
{
    m_props.append(prop);
}


} //namespace RQt
