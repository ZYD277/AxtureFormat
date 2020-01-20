#ifndef QSSPARSEMETHOD_H
#define QSSPARSEMETHOD_H

#include <QStringList>

#include "../../util/fileutils.h"
#include "../../css/cssstruct.h"

namespace RQt{

class QSSParseMethod : public RTextParseMethod
{
public:
    QSSParseMethod();

    void setStyle(CSS::CssMap globalCss,CSS::CssMap pageCss);

    bool  startSave(RTextFile * file) override;

private:
    CSS::CssMap m_globalCss;
    CSS::CssMap m_pageCss;
};

} //namespace RQt

#endif // QSSPARSEMETHOD_H
