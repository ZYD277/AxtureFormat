#include "htmlparser.h"

#include "util/fileutils.h"
#include "htmlparsemethod.h"

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
        m_parseMethod = new HtmlParseMethod();
    }

    RXmlFile xmlFile(fullPath);
    xmlFile.setParseMethod(m_parseMethod,false);
    if(xmlFile.startParse()){
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
