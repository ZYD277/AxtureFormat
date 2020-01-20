#include "qssparsemethod.h"

#include <QDebug>
#include <QTextStream>

namespace RQt{

QSSParseMethod::QSSParseMethod()
{

}

void QSSParseMethod::setStyle(CSS::CssMap globalCss, CSS::CssMap pageCss)
{
    m_globalCss = globalCss;
    m_pageCss = pageCss;
}

bool QSSParseMethod::startSave(RTextFile *file)
{
    QTextStream stream(file);

    QString newLine = "\r";

    auto generateCss = [&](CSS::CssMap& cssMap){
        auto iter = cssMap.begin();
        while(iter != cssMap.end()){
            CSS::CssSegment & seg = iter.value();

            if(seg.type == CSS::Clazz){
                stream<<".";
            }else if(seg.type == CSS::Id){
                stream<<"#";
            }
            stream<<seg.selectorName<<" {"<<newLine;

            foreach(const CSS::CssRule & rule,seg.rules){
                stream<<"\t"<<rule.name<<":"<<rule.value<<";"<<newLine;
            }

            stream<<"}"<<newLine<<newLine;

            iter++;
        }
    };

    generateCss(m_globalCss);
    generateCss(m_pageCss);


    return true;
}

} //namespace RQt
