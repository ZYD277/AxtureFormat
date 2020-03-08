/*!
 *  @brief     CSS解析器
 *  @details   解析指定的css文件，返回样式列表
 *  @author    wey
 *  @version   1.0
 *  @date      2020.02.26
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef CSSPARSER_H
#define CSSPARSER_H

#include <QObject>
#include "cssstruct.h"
#include "cssparsemethod.h"

namespace CSS{

class CssParser : public QObject
{
    Q_OBJECT
public:
    CssParser();

    bool parseFile(QString fullPath);

    CssMap getParsedResult(){return m_parseMethod.getParsedResult();}

    ErrorMsg getParsedErrorMsg(){return m_parseMethod.getParsedErrorMsg();}

private:
    CssParseMethod m_parseMethod;
};

} //namespace CSS

#endif // CSSPARSER_H
