#include "qssoutput.h"

#include "qssparsemethod.h"

namespace RQt{

QSSOutput::QSSOutput()
{

}

void QSSOutput::setCommonStyle(const CSS::CssMap &globalCss, const CSS::CssMap &pageCss, SelectorTypeMap selectorType)
{
    m_globalCss = globalCss;
    m_pageCss = pageCss;
    m_selectorType = selectorType;
}

bool QSSOutput::save(QString fullPath)
{
    RTextFile file(fullPath);

    QSSParseMethod method;
    method.setCommonStyle(m_globalCss,m_pageCss,m_selectorType);

    file.setParseMethod(&method,false);

    if(file.startSave(QFile::WriteOnly | QFile::Truncate | QFile::Text)){
        m_resources = method.getResources();
        return true;
    }
    else
        return false;
}

} //namespace RQt
