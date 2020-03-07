#include "mitem.h"

#include "mproperty.h"

namespace RQt{

MItem::MItem():m_hasAttrRow(false),m_hasAttrColumn(false),m_prop(nullptr)
{

}

MItem::~MItem()
{
    delete m_prop;
    m_prop = nullptr;

    foreach(auto hh,m_items)
        delete hh;

    m_items.clear();
}

void MItem::write(QXmlStreamWriter &writer, QString tagName)
{
    writer.writeStartElement(tagName.isEmpty()?"item":tagName);

    if(hasAttributeRow())
        writer.writeAttribute("row",attributeRow());

    if(hasAttributeColumn())
        writer.writeAttribute("column",attributeColumn());

    m_prop->write(writer,"property");

    foreach(auto hh,m_items){
        hh->write(writer,"item");
    }

    writer.writeEndElement();
}

} //namespace RQt
