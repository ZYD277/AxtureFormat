/*!
 *  @brief     axure html文件解析
 *  @author    wey
 *  @version   1.0
 *  @date      2020.03.03
 *  @warning
 *  @copyright NanJing RenGu.
 *  @note 20200303:wey:使用gumbo替代xml方式解析html
 */
#ifndef HTMLPARSER_H
#define HTMLPARSER_H

#include <QObject>
#include "htmlstruct.h"

namespace Html{

class GumboParseMethod;

class HtmlParser : QObject
{
    Q_OBJECT
public:
    HtmlParser();
    ~HtmlParser();

    bool parseHtmlFile(QString fullPath);
    DomHtmlPtr getParsedResult();

private:
    GumboParseMethod * m_parseMethod;
    DomHtmlPtr m_parseResult;
};

} //namespace Html

#endif // HTMLPARSER_H
