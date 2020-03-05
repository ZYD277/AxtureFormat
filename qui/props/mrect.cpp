#include "mrect.h"

namespace RQt{

MRect::MRect()
{

}

MRect::~MRect()
{

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
