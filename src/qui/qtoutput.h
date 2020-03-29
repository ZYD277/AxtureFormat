#ifndef QTOUTPUT_H
#define QTOUTPUT_H

#include "../html/htmlstruct.h"
#include "../css/cssstruct.h"
#include "../qui/formatproperty.h"

namespace RQt{

class QtOutput
{
public:
    QtOutput();
    ~QtOutput();

    bool save(QString className, QString cssFileName, DomHtmlPtr ptr, CSS::CssMap globalCss, CSS::CssMap pageCss, QString fullPath);
    QStringList getOriginalResouces(){return m_originalResoucesLinks;}

private:
    QStringList m_originalResoucesLinks;
    SelectorTypeMap m_selectorType;

};

} //namespace RQt

#endif // QTOUTPUT_H
