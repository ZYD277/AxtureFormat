#include "mcolumn.h"

namespace RQt{

MColumn::MColumn()
{

}

MColumn::~MColumn()
{
    qDeleteAll(m_props);
    m_props.clear();
}

void MColumn::addProperty(MProperty *prop)
{
    m_props.append(prop);
}


} //namespace RQt
