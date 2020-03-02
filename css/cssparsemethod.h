#ifndef CSSPARSEMETHOD_H
#define CSSPARSEMETHOD_H

#include <QSharedPointer>
#include "../util/fileutils.h"
#include "cssstruct.h"

namespace CSS{

class CssParseMethod : public RTextParseMethod
{
public:
    CssParseMethod();

    bool  startParse(RTextFile * file) override;

    CssMap getParsedResult(){return m_cssMap;}

    ErrorMsg getParsedErrorMsg(){return m_errorMsg;}

private:
    QString specialDis(const QString &character, int startPosition, int endPosition);    /*!< 处理数据中的符号*/
    bool dataKey(const QString &keyData);      /*!< key的特殊处理*/

private:
    CssMap m_cssMap;
    ErrorMsg m_errorMsg;

};

} //namespace Css

#endif // CSSPARSEMETHOD_H
