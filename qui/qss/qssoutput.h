#ifndef QSSOUTPUT_H
#define QSSOUTPUT_H

#include <QString>
#include <QStringList>
#include "../../css/cssstruct.h"

namespace RQt{

class QssParseMethod;

class QSSOutput
{
public:
    QSSOutput();

    void setStyle(CSS::CssMap globalCss,CSS::CssMap pageCss);
    bool save(QString fullPath);

private:
    QssParseMethod * m_parseMethod;

    CSS::CssMap m_globalCss;
    CSS::CssMap m_pageCss;
};

} //namespace RQt

#endif // QSSOUTPUT_H
