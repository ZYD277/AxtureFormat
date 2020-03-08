#ifndef QTOUTPUT_H
#define QTOUTPUT_H

#include "../html/htmlstruct.h"
#include "../css/cssstruct.h"

namespace RQt{

class QtOutput
{
public:
    QtOutput();
    ~QtOutput();

    bool save(DomHtmlPtr ptr, CSS::CssMap globalCss,CSS::CssMap pageCss,QString fullPath);
    QStringList getOriginalResouces(){return m_originalResoucesLinks;}

private:
    QStringList m_originalResoucesLinks;

};

} //namespace RQt

#endif // QTOUTPUT_H
