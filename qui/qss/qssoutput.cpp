#include "qssoutput.h"

#include "qssparsemethod.h"

namespace RQt{

QSSOutput::QSSOutput()
{

}

void QSSOutput::setStyle(CSS::CssMap globalCss, CSS::CssMap pageCss)
{
    m_globalCss = globalCss;
    m_pageCss = pageCss;
}

bool QSSOutput::save(QString fullPath)
{
    RTextFile file(fullPath);

    QSSParseMethod method;
    method.setStyle(m_globalCss,m_pageCss);

    file.setParseMethod(&method,false);
    return file.startSave(QFile::WriteOnly | QFile::Truncate | QFile::Text);
}

} //namespace RQt
