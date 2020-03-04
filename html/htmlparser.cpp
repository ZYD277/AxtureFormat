#include "htmlparser.h"

#include "util/fileutils.h"
#include "gumboparsemethod.h"

namespace Html{

HtmlParser::HtmlParser():m_parseMethod(nullptr)
{

}

HtmlParser::~HtmlParser()
{
    if(m_parseMethod)
        delete m_parseMethod;
}

bool HtmlParser::parseHtmlFile(QString fullPath)
{
    if(m_parseMethod == nullptr){
        m_parseMethod = new GumboParseMethod();
    }

    RTextFile textFile(fullPath);
    textFile.setParseMethod(m_parseMethod,false);
    if(textFile.startParse()){
        m_parseResult = m_parseMethod->getParsedResult();
        return true;
    }

    return false;
}

DomHtmlPtr HtmlParser::getParsedResult()
{
    return m_parseResult;
}

} //namespace Html
