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

private:
    CssMap m_cssMap;

};

} //namespace Css

#endif // CSSPARSEMETHOD_H
