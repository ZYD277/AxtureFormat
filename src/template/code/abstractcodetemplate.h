/*!
 *  @brief     代码模板基类
 *  @details
 *  @author    wey
 *  @version   1.0
 *  @date      2020.06.17
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef ABSTRACTCODETEMPLATE_H
#define ABSTRACTCODETEMPLATE_H

#include <QString>
#include <QStringList>

#include "../cppstruct.h"

extern QString NEW_EMPTY;
extern QString New_Line;
extern QString New_Space;
extern QString ConcatNewLine(QString text);

namespace CXX{

class CppGenerate;

class AbstractCodeTemplate
{
public:
    AbstractCodeTemplate(CodeType type);

    virtual void prepareOutput(CppGenerate * generate) = 0;

    void setSameTypeIndex(int index){m_sameTypeIndex = index;}

protected:
    int m_sameTypeIndex;        /*!< 当同一个页面出现多个重复模板时，按照_1、_2进行编号 */
    CodeType m_type;

};

} //namespace CXX

#endif // ABSTRACTCODETEMPLATE_H
