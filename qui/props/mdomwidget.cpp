#include "mdomwidget.h"

#include "mproperty.h"
#include "mrow.h"
#include "mcolumn.h"
#include "mitem.h"

namespace RQt{

RDomWidget::RDomWidget():m_bHasAttrName(false),m_bHasAttrClass(false)
{

}

RDomWidget::~RDomWidget()
{
    qDeleteAll(m_props);
    m_props.clear();

    qDeleteAll(m_widgets);
    m_widgets.clear();

    qDeleteAll(m_rows);
    m_rows.clear();

    qDeleteAll(m_columns);
    m_columns.clear();

    qDeleteAll(m_items);
    m_items.clear();
}

void RDomWidget::addProperty(MProperty *prop)
{
    m_props.push_back(prop);
}

void RDomWidget::addWidget(RDomWidget *widget)
{
    m_widgets.push_back(widget);
}

void RDomWidget::addRow(MRow *row)
{
    m_rows.push_back(row);
}

void RDomWidget::addColumn(MColumn *column)
{
    m_columns.push_back(column);
}

void RDomWidget::addItem(MItem *item)
{
    m_items.push_back(item);
}

} //namespace RQt
