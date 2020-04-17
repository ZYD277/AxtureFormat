#include "cssparser.h"
#include <QDebug>
namespace CSS{

CssParser::CssParser()
{

}

bool CssParser::parseFile(QString fullPath)
{
    RTextFile textFile(fullPath);
    textFile.setParseMethod(&m_parseMethod,false);
    if(textFile.startParse(QFile::ReadOnly)){
        return true;
    }
    return false;
}

} //namespace CSS
