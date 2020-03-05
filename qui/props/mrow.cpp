#include "mrow.h"

namespace RQt{

MRow::MRow()
{

}

MRow::~MRow()
{
    qDeleteAll(m_props);
    m_props.clear();
}

void MRow::addProperty(MProperty *prop)
{
    m_props.append(prop);
}

} //namespace RQt
