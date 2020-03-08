#include "mrect.h"

namespace RQt{

MRect::MRect()
{

}

MRect::~MRect()
{

}

void MRect::write(QXmlStreamWriter &writer, QString tagName)
{
    writer.writeStartElement(tagName.isEmpty() ? "rect":tagName);

    writer.writeTextElement("x",QString::number(m_rect.x()));
    writer.writeTextElement("y",QString::number(m_rect.y()));
    writer.writeTextElement("width",QString::number(m_rect.width()));
    writer.writeTextElement("height",QString::number(m_rect.height()));

    writer.writeEndElement();
}

void MRect::setRect(QRect rect)
{
    m_rect = rect;
}

const QRect &MRect::rect() const
{
    return m_rect;
}
} //namespace RQt
