#include "tablewidgetstyletemplate.h"

#include "../cppgenerate.h"

namespace CXX{

TableWidgetStyleTemplate::TableWidgetStyleTemplate():AbstractCodeTemplate(CUSTOM_TABLE_WIDGET)
  ,m_tableData(nullptr)
{

}

void TableWidgetStyleTemplate::prepareOutput(CppGenerate *generate)
{
    generate->addInclude("#include <QHeaderView>");

    generate->addConstructInitCode(QString("ui->%1->setStyleSheet(\"%2\");").arg(m_tableData->m_tableId).arg(m_tableData->m_tableStyle));
    generate->addConstructInitCode(QString("ui->%1->horizontalHeader()->setStyleSheet(\"%2\");").arg(m_tableData->m_tableId).arg(m_tableData->m_horizontalStyle));
    generate->addConstructInitCode(QString("ui->%1->verticalHeader()->setStyleSheet(\"%2\");").arg(m_tableData->m_tableId).arg(m_tableData->m_verticalStyle));

    generate->addConstructInitCode(NEW_EMPTY);

    for(TabRowInfo t_rowInfo : m_tableData->m_tabRowInfos){
        generate->addConstructInitCode(QString("ui->%1->setRowHeight(%2,%3);").arg(m_tableData->m_tableId).arg(t_rowInfo.m_rowNumbers).arg(t_rowInfo.m_height));
    }
    generate->addConstructInitCode(NEW_EMPTY);

    for(TabColumnInfo t_columnInfo : m_tableData->m_tabColumnInfos){

        generate->addConstructInitCode(QString("ui->%1->setColumnWidth(%2,%3);").arg(m_tableData->m_tableId).arg(t_columnInfo.m_columnNumbers).arg(t_columnInfo.m_width));
    }
    generate->addConstructInitCode(NEW_EMPTY);

}

} //namespace CXX
