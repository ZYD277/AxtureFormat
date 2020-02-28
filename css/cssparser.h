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
