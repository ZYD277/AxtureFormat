#include "qssparsemethod.h"

#include <QDebug>
#include <QTextStream>
#include "qui/formatproperty.h"
#include "html/htmlstruct.h"

namespace RQt{

QSSParseMethod::QSSParseMethod()
{

}

/*!
 * @brief 添加普通的样式
 * @attention
 * @param[in]
 * @return
 */
void QSSParseMethod::setCommonStyle(const CSS::CssMap& globalCss,const CSS::CssMap& pageCss,SelectorTypeMap selectorType)
{
    m_globalCss = globalCss;
    m_pageCss = pageCss;
    m_selectorType = selectorType;
}

bool QSSParseMethod::startSave(RTextFile *file)
{
    QTextStream stream(file);

    QString newLine = "\r";

    auto generateCss = [&](CSS::CssMap& cssMap){
        auto iter = cssMap.begin();
        while(iter != cssMap.end()){
            CSS::CssSegment & seg = iter.value();

            SelectorTypeMap selectorType;
            FormatProperty formatProperty;
            selectorType = formatProperty.getHtmlParsedResult();

            if(seg.type == CSS::Clazz){
                stream<<".";
            }else if(seg.type == CSS::Id || seg.type == CSS::DynamicType){
                stream<<"#";
            }

            if(seg.selectorName.contains("_div"))
                seg.selectorName = seg.selectorName.remove("_div");
            else if(seg.selectorName.contains("_input"))
                seg.selectorName = seg.selectorName.remove("_input");
            stream<<seg.selectorName<<" {"<<newLine;

            foreach(const CSS::CssRule & rule,seg.rules){
                if(rule.name == "border-color" && rule.value == "transparent"){
                    QString name = "border";
                    QString value = "none";
                    stream<<"\t"<<name<<":"<<value<<";"<<newLine;
                }
                else
                    stream<<"\t"<<rule.name<<":"<<rule.value<<";"<<newLine;
            }

            stream<<"}"<<newLine<<newLine;

            for(int i=0;i<m_selectorType.size();i++){
                if(m_selectorType.keys().at(i) == seg.selectorName){
                    if(m_selectorType.values().at(i) == Html::RDROPLIST)
                        seg.selectorName ="#" + seg.selectorName + " QAbstractItemView";
//                    else if(m_selectorType.values().at(i) == Html::RTREE)
//                        seg.selectorName ="#" + seg.selectorName + "::item";
                    stream<<seg.selectorName<<" {"<<newLine;
                    foreach(const CSS::CssRule & rule,seg.rules)
                       stream<<"\t"<<rule.name<<":"<<rule.value<<";"<<newLine;
                    stream<<"}"<<newLine<<newLine;
                    break;
                }
            }

            iter++;
        }
    };

    generateCss(m_globalCss);

    generateCss(m_pageCss);

    return true;
}
} //namespace RQt
