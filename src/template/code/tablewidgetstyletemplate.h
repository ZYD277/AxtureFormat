/*!
 *  @brief     表格样式信息
 *  @details   因Qt4和Qt5中在Ui中对表格的样式支持不一致，但通过代码可实现统一效果
 *  @author    wey
 *  @version   1.0
 *  @date      2020.07.02
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef TABLEWIDGETSTYLETEMPLATE_H
#define TABLEWIDGETSTYLETEMPLATE_H

#include "abstractcodetemplate.h"

namespace CXX{

class TableWidgetStyleTemplate : public AbstractCodeTemplate
{
public:
    TableWidgetStyleTemplate();

    void prepareOutput(CppGenerate * generate);

    void setTableStyleData(TableStyleCodeData * data){m_tableData = data;}

private:
    TableStyleCodeData * m_tableData;
};

} //namespace CXX

#endif // TABLEWIDGETSTYLETEMPLATE_H
