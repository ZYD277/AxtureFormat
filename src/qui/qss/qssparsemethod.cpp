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

            QStringList rulesName;
            rulesName<<"position"<<"left"<<"top"<<"width"<<"height"
                     <<"-moz-box-shadow"<<"-webkit-box-shadow"
                     <<"box-shadow"<<"box-sizing"<<"-ms-overflow-x"
                     <<"-ms-overflow-y"<<"overflow-y"<<"overflow-x"
                     <<"visibility";
            m_ruleSize = seg.rules.size();

            for(int i=0;i<seg.rules.size();i++){
                m_name = seg.rules.at(i).name;
                m_value = seg.rules.at(i).value;
                if(m_name == "border-color" && m_value == "transparent"){
                    m_name = "border";
                    m_value = "none";
                }
                for(int j=0;j<rulesName.size();j++)
                    if(m_name == rulesName.at(j)){
                        m_name.clear();
                        m_value.clear();
                        m_ruleSize--;
                        break;
                    }
            }
            for(int i=0;i<m_selectorType.size();i++){
                if(m_selectorType.keys().at(i).contains("_div_")
                        &&(m_selectorType.values().at(i) == Html::RTREE
                        ||m_selectorType.values().at(i) == Html::RTABLE)
                        &&m_ruleSize != 0){
                    QStringList selectorNames = m_selectorType.keys().at(i).split("_div_");
                    QString selectorName = seg.selectorName;
                    if(selectorNames.size()>1){
                        if((selectorNames.at(0)+"_div" == seg.selectorName&&m_selectorType.values().at(i) == Html::RTREE)
                            ||(selectorNames.at(0) == seg.selectorName&&m_selectorType.values().at(i) == Html::RTABLE))
                            selectorName ="#" + selectorNames.at(1) + "::item";
                        else if(seg.selectorName.contains(selectorNames.at(0)+":"))
                            selectorName ="#" + selectorName.replace(selectorNames.at(0),selectorNames.at(1) + "::item");
                        else
                            break;
                        stream<<selectorName<<" {"<<newLine;
                        foreach(const CSS::CssRule & rule,seg.rules){
                            int j;
                            for(j=0;j<rulesName.size();j++)
                                if(rule.name == rulesName.at(j))
                                    break;
                            if(j == rulesName.size())
                                stream<<"\t"<<rule.name<<":"<<rule.value<<";"<<newLine;
                        }
                        stream<<"}"<<newLine<<newLine;
                        break;
                    }
                }
            }
            for(int i=0;i<m_selectorType.size();i++){
                if((seg.selectorName == m_selectorType.keys().at(i)
                    || seg.selectorName.contains(m_selectorType.keys().at(i) + " ")
                    || seg.selectorName.contains(m_selectorType.keys().at(i) + "_")
                    || seg.selectorName.contains(m_selectorType.keys().at(i) + ".")
                    || seg.selectorName.contains(m_selectorType.keys().at(i) + ":")
                    || seg.selectorName == "base")&&m_ruleSize != 0){
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

                    int fontCount = seg.rules.size();
                    foreach(const CSS::CssRule & rule,seg.rules){
                        if(rule.name != "font-size")
                            fontCount--;
                        int j;
                        for(j=0;j<rulesName.size();j++)
                            if(rule.name == rulesName.at(j))
                                break;
                        if(j == rulesName.size()){
                            if(rule.name == "background-image" && rule.value != "none"){
                                QStringList imageValues = rule.value.split("/");
                                if(imageValues.size() > 0){
                                    QString imageValue = imageValues.at(imageValues.size()-1);
                                    QString imageSrc = "images/"+imageValue;
                                    imageSrc = imageSrc.remove("')");
                                    imageValue = "url(:/images/"+imageValue;
                                    imageValue = imageValue.remove("'");
                                    m_resources.append(imageSrc);
                                    stream<<"\t"<<"border-image"<<":"<<imageValue<<";"<<newLine;
                                }
                            }
                            else
                                stream<<"\t"<<rule.name<<":"<<rule.value<<";"<<newLine;
                        }
                    }
                    if(fontCount == 0){
                        for(int i=0;i<m_selectorType.size();i++){
                            if(m_selectorType.keys().at(i).contains("_div_")
                                    &&m_selectorType.values().at(i) == Html::RLABEL){
                                QStringList selectorNames = m_selectorType.keys().at(i).split("_div_");
                                if(selectorNames.size()>1){
                                    if(selectorNames.at(0) == seg.selectorName){
                                        stream<<"\t"<<"font-size:"<<selectorNames.at(1)<<";"<<newLine;
                                        break;
                                    }
                                }
                            }
                        }
                    }
                    stream<<"}"<<newLine<<newLine;
                    break;
                }
            }
            for(int i=0;i<m_selectorType.size();i++){
                if(m_selectorType.keys().at(i) == seg.selectorName && m_ruleSize != 0){
                    if(m_selectorType.values().at(i) == Html::RDROPLIST){
                        seg.selectorName ="#" + seg.selectorName + " QAbstractItemView";
                        stream<<seg.selectorName<<" {"<<newLine;
                        foreach(const CSS::CssRule & rule,seg.rules){
                            int j;
                            for(j=0;j<rulesName.size();j++)
                                if(rule.name == rulesName.at(j))
                                    break;
                            if(j == rulesName.size())
                                stream<<"\t"<<rule.name<<":"<<rule.value<<";"<<newLine;
                        }
                        stream<<"}"<<newLine<<newLine;
                        break;
                    }
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
