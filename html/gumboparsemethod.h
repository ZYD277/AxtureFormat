/*!
 *  @brief     使用gumbo库解析html，生成控件树
 *  @details   google的gumbo库可以很好的解决html解析问题，规避了通过xml方式解析失败的问题
 *  @author    wey
 *  @version   1.0
 *  @date      2020.03.03
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef GUMBOPARSEMETHOD_H
#define GUMBOPARSEMETHOD_H

#include <QSharedPointer>
#include <gumbo.h>
#include "../util/fileutils.h"
#include "htmlstruct.h"
#include "gumbonodewrapper.h"

namespace Html{

class GumboParseMethod : public RTextParseMethod
{
public:
    GumboParseMethod();
    ~GumboParseMethod();

    bool startParse(RTextFile * file) override;

    DomHtmlPtr getParsedResult(){return m_result;}

private:
    bool parseFile(RTextFile * file);

private:
    DomHtmlPtr m_result;

    QString m_errorMsg;
    GumboOutput * m_gumboParser;
};

} //namespace Html

#endif // GUMBOPARSEMETHOD_H
