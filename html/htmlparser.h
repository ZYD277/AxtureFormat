#ifndef HTMLPARSER_H
#define HTMLPARSER_H

#include <QObject>
#include "htmlstruct.h"

namespace Html{

class HtmlParseMethod;

class HtmlParser : QObject
{
    Q_OBJECT
public:
    HtmlParser();
    ~HtmlParser();

    bool parseHtmlFile(QString fullPath);
    DomHtmlPtr getParsedResult();

private:
    HtmlParseMethod * m_parseMethod;
    DomHtmlPtr m_parseResult;
};

} //namespace Html

#endif // HTMLPARSER_H
