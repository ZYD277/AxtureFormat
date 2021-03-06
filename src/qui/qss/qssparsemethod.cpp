﻿#include "qssparsemethod.h"

#include <QDebug>
#include <QTextStream>
#include "qui/formatproperty.h"
#include "html/htmlstruct.h"

#include <QTime>
#include <QRgb>

namespace RQt{

QSSParseMethod::QSSParseMethod()
{

}

/*!
 * @brief 添加普通的样式
 */
void QSSParseMethod::setCommonStyle(const CSS::CssMap& globalCss,const CSS::CssMap& pageCss,SelectorTypeMap selectorType)
{
    m_globalCss = globalCss;
    m_selectorType = selectorType;

    //整合控件用到的所有样式，并过滤其中重复的属性
    auto iter = pageCss.begin();
    while(iter != pageCss.end()){
        QString name = iter.key();
        CSS::CssSegment seg = iter.value();

        if(name.contains("_div") || name.contains("_input"))
        {
            QString otherName;
            if(name.contains("_div"))
            {
                name.remove("_div");
                otherName = name + "_input";
            }
            else
            {
                name.remove("_input");
                otherName = name + "_div";
            }

            auto findSrc = pageCss.find(name);
            auto findOther = pageCss.find(otherName);

            if((findSrc != pageCss.end())&&(findOther != pageCss.end()))
            {
                CSS::CssSegment srcSeg = findSrc.value();
                CSS::CssSegment otherSeg = findOther.value();
                seg.rules = srcSeg.rules + seg.rules + otherSeg.rules;
            }
            else if((findSrc != pageCss.end())||(findOther != pageCss.end()))
            {
                CSS::CssSegment tmpSeg = (findSrc != pageCss.end()) ? findSrc.value() : findOther.value();
                seg.rules = tmpSeg.rules + seg.rules;
            }
        }

        seg.rules = filterDuplicateData(seg.rules);     //过滤重复属性

        m_pageCss.insert(name,seg);
        iter++;
    }

    //处理像table这样特殊控件的样式重复问题
    for(int i = 0; i < m_selectorType.size(); i++)
    {
        QString tmpKey = m_selectorType.keys().at(i);
        if(tmpKey.contains("_div_"))
        {
            QStringList selectorNames = tmpKey.split("_div_");
            if(selectorNames.size() > 1)
            {
                QString tmpName = selectorNames.at(1);
                m_pageCss.remove(tmpName);
            }
        }
    }
}

bool QSSParseMethod::startSave(RTextFile *file)
{
    generateCss(file,m_globalCss);

    generateCss(file,m_pageCss);

    return true;
}

/**
 * @brief 过滤重复样式
 * @param rules待处理数据表
 * @return 返回处理完的数据
 */
CSS::Rules QSSParseMethod::filterDuplicateData(CSS::Rules rules)
{
    QMap<QString,QString> tmpRules;
    std::for_each(rules.begin(),rules.end(),[&](CSS::CssRule cssRule){
        if(cssRule.value.contains("rgba") && cssRule.name.contains("color")){
            int index1 = cssRule.value.lastIndexOf(" ");
            int index2 = cssRule.value.lastIndexOf(")");
            QString t_ruleValue;
            QString tmp = cssRule.value.right(index2-index1);
            QString tp = tmp.left( tmp.lastIndexOf(")"));
            if(tp.toDouble() >= 0 && tp.toDouble() <= 1){
                int b = tp.toDouble() * 255;
                t_ruleValue = cssRule.value.left(cssRule.value.size() - (index2-index1)) + QString::number(b) + ")";
            }
            cssRule.value = t_ruleValue;
        }
        tmpRules.insert(cssRule.name,cssRule.value);
    });

    rules.clear();

    for(int i = 0; i < tmpRules.size(); i++)
    {
        CSS::CssRule cssRule;
        cssRule.name = tmpRules.keys().at(i);
        cssRule.value = tmpRules.values().at(i);
        rules.append(cssRule);
    }
    return rules;
}

void QSSParseMethod::generateCss(RTextFile *file,CSS::CssMap &cssMap)
{
    QTextStream stream(file);

    QString newLine = "\r\n";

    //qss中无用属性
    QStringList deprecatedRulesName;
    deprecatedRulesName<<"position"<<"left"<<"top"<<"width"<<"height"
            <<"-moz-box-shadow"<<"-webkit-box-shadow"
           <<"box-shadow"<<"box-sizing"<<"-ms-overflow-x"
          <<"-ms-overflow-y"<<"overflow-y"<<"overflow-x"
         <<"visibility"<<"z-index"<<"touch-action"<<"overflow"
        <<"word-wrap"<<"-ms-transform"<<"transform"<<"-webkit-transform"
       <<"-moz-transform"<<"-webkit-overflow-scrolling"
      <<"letter-spacing"<<"resize"<<"display";

    auto iter = cssMap.begin();

    FormatProperty formatProperty;

    while(iter != cssMap.end()){
        CSS::CssSegment & seg = iter.value();

        m_ruleSize = seg.rules.size();

        for(int i = 0; i < seg.rules.size(); i++){
            QString ruleName = seg.rules.at(i).name;
            QString ruleValue = seg.rules.at(i).value;

            if(ruleName == "border-color" && ruleValue == "transparent"){
                ruleName = "border";
                ruleValue = "none";
            }

            /*!< m_ruleSize计算剩余属性数量*/
            if(deprecatedRulesName.contains(ruleName))
            {
                m_ruleSize--;
            }
        }

        /*!< 对tree和table的样式通过获取其子节点的添加*/
        QStringList qssList = m_selectorType.keys();
        QStringList divList = qssList.filter("_div_");

        for(int i = 0;i < divList.size(); i++)
        {
            if(divList.at(i).contains(iter.key()))
            {
                if(qssList.size() > qssList.indexOf(divList.at(i)))
                {
                    if(m_ruleSize != 0 && (m_selectorType.values().at(qssList.indexOf(divList.at(i))) == Html::RTREE
                                           ||m_selectorType.values().at(qssList.indexOf(divList.at(i)))  == Html::RTABLE)){

                        /*!< 设计的子节点和主节点的查找 */
                        QStringList selectorNames = divList.at(i).split("_div_");
                        QString selectorName = seg.selectorName;

                        if(selectorNames.size() > 1){
                            if((selectorNames.at(0) + "_div" == seg.selectorName) || (selectorNames.at(0) == seg.selectorName))
                            {

                                selectorName = formatProperty.getTypeName(Html::RTABLE) + "#" + selectorNames.at(1);
                                if(m_selectorType.values().at(qssList.indexOf(divList.at(i))) == Html::RTREE)
                                {
                                    if(selectorNames.at(1).contains("back"))
                                    {
                                        QString back = selectorNames.at(1);
                                        selectorName = formatProperty.getTypeName(Html::RTREE) + "#" + back.remove("back") + "::item";
                                    }
                                    else if(selectorNames.at(1).contains("text"))
                                    {
                                        QString text = selectorNames.at(1);
                                        selectorName = formatProperty.getTypeName(Html::RTREE) + "#" + text.remove("text");
                                    }
                                }
                            }else if(seg.selectorName.contains(selectorNames.at(0)+":")){
                                selectorName = "#" + selectorName.replace(selectorNames.at(0),selectorNames.at(1) + "::item");
                            }else{
                                break;
                            }

                            stream<<selectorName<<" {"<<newLine;

                            foreach(const CSS::CssRule & rule,seg.rules){
                                if(!deprecatedRulesName.contains(rule.name)){
                                    stream<<"\t"<<rule.name<<":"<<rule.value<<";"<<newLine;
                                }
                            }
                            stream<<"}"<<newLine<<newLine;
                        }
                    }
                    /*!< 自制控件组合样式 */
                    else if(m_ruleSize != 0 && (m_selectorType.values().at(qssList.indexOf(divList.at(i)))  == Html::RCHECKBOX
                                           ||m_selectorType.values().at(qssList.indexOf(divList.at(i)))  == Html::RRADIO_BUTTON
                                           ||m_selectorType.values().at(qssList.indexOf(divList.at(i)))  == Html::RDROPLIST
                                           ||m_selectorType.values().at(qssList.indexOf(divList.at(i)))  == Html::RPROGRESSBAR
                                           ||m_selectorType.values().at(qssList.indexOf(divList.at(i)))  == Html::RSCROLLBAR
                                           ||m_selectorType.values().at(qssList.indexOf(divList.at(i)))  == Html::RTABWIDGET))
                    {
                        /*!< 设计的子节点和主节点的查找*/
                        QStringList selectorNames = divList.at(i).split("_div_");
                        QString selectorName = seg.selectorName;
                        if(selectorNames.size() > 1){
                            if((selectorNames.at(0) + "_div" == seg.selectorName) || (selectorNames.at(0) == seg.selectorName)
                                    ||(selectorNames.at(0) + "_input" == seg.selectorName))
                            {
                                switch(m_selectorType.values().at(qssList.indexOf(divList.at(i))))
                                {
                                case Html::RCHECKBOX:{
                                    selectorName = formatProperty.getTypeName(Html::RCHECKBOX) + "#" + selectorNames.at(1);
                                    break;
                                }
                                case Html::RRADIO_BUTTON:{
                                    selectorName =formatProperty.getTypeName(Html::RRADIO_BUTTON) + "#" + selectorNames.at(1);
                                    break;
                                }
                                case Html::RDROPLIST:{
                                    if(selectorNames.at(1).contains("option"))
                                    {
                                        QString option = selectorNames.at(1);
                                        selectorName = formatProperty.getTypeName(Html::RDROPLIST) + "#" + option.remove("option") + " QAbstractItemView";
                                    }
                                    else if(selectorNames.at(1).contains("arrow"))
                                    {
                                        QString arrow = selectorNames.at(1);
                                        selectorName = formatProperty.getTypeName(Html::RDROPLIST) + "#" + arrow.remove("arrow") + "::drop-down";
                                    }
                                    else if(selectorNames.at(1).contains("back"))
                                    {
                                        QString back = selectorNames.at(1);
                                        selectorName = formatProperty.getTypeName(Html::RDROPLIST) + "#" + back.remove("back");
                                    }
                                    break;
                                }
                                case Html::RPROGRESSBAR:{
                                    if(selectorNames.at(1).contains("bar"))
                                    {
                                        QString widgetId = selectorNames.at(1);
                                        selectorName = formatProperty.getTypeName(Html::RPROGRESSBAR) +  "#" + widgetId.remove("bar") + "::chunk";
                                    }
                                    else if(selectorNames.at(1).contains("slot"))
                                    {
                                        QString widgetId = selectorNames.at(1);
                                        selectorName = formatProperty.getTypeName(Html::RPROGRESSBAR) +  "#" + widgetId.remove("slot");
                                    }
                                    break;
                                }
                                default:break;
                                }
                            }
                            else
                                break;

                            stream<<selectorName<<" {"<<newLine;

                            foreach(const CSS::CssRule & rule,seg.rules){
                                if(!deprecatedRulesName.contains(rule.name)){
                                    if(!rule.value.contains("transparent"))
                                    {
                                        /*!< 处理控件的简单渐变背景*/
                                        if(rule.value.contains("gradient")){
                                            stream<<"\t"<<rule.name<<":"<< formatProperty.gradientSwitchToQss(rule.value)<<";"<<newLine;
                                        }
                                        else
                                            stream<<"\t"<<rule.name<<":"<<rule.value<<";"<<newLine;
                                    }
                                }
                            }
                            stream<<"}"<<newLine<<newLine;
                        }
                    }
                }
                break;
            }
        }

        /*!< 针对鼠标触发信号的样式处理修改*/
        QString nowkey = iter.key();
        if(nowkey.contains(":checked") || nowkey.contains(":disabled") || nowkey.contains(":hover") || nowkey.contains(":pressed")){
            QStringList nowkeyList = nowkey.split(":");
            if(nowkeyList.size() > 0)
                nowkey = nowkeyList.at(0);
        }

        if(m_ruleSize != 0 && (m_selectorType.keys().contains(nowkey) || seg.selectorName == "base"))
        {
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

            /*!< 控件在样式文件的图片信息*/
            int fontCount = seg.rules.size();
            foreach(const CSS::CssRule & rule,seg.rules){
                if(rule.name != "font-size")
                    fontCount--;

                if(!deprecatedRulesName.contains(rule.name)){
                    if(rule.name == "background-image" && rule.value != "none"){
                        QStringList imageValues = rule.value.split("/");
                        if(imageValues.size() > 0){
                            QString imageValue = imageValues.at(imageValues.size() - 1);
                            QString imageSrc = "images/"+imageValue;
                            imageSrc = imageSrc.remove("')");
                            imageValue = "url(:/images/"+imageValue;
                            imageValue = imageValue.remove("'");
                            if(!m_resources.contains(imageSrc))
                                m_resources.append(imageSrc);

                            stream<<"\t"<<"border-image"<<":"<<imageValue<<";"<<newLine;
                        }
                    }else{
                        /*!< 处理控件的简单渐变背景*/
                        if(rule.value.contains("gradient")){
                            stream<<"\t"<<rule.name<<":"<< formatProperty.gradientSwitchToQss(rule.value)<<";"<<newLine;
                        }
                        else{
                            stream<<"\t"<<rule.name<<":"<<rule.value<<";"<<newLine;
                        }
                    }
                }
            }

            /*!< 获取标题的默认大小*/
            if(fontCount == 0)
            {
                for(int i = 0; i < divList.size(); i++)
                {
                    if(m_selectorType.values().at(qssList.indexOf(divList.at(i))) == Html::RLABEL){
                        QStringList selectorNames = divList.at(i).split("_div_");
                        if(selectorNames.size() > 1){
                            if(selectorNames.at(0) == seg.selectorName){
                                stream<<"\t"<<"font-size:"<<selectorNames.at(1)<<";"<<newLine;
                                break;
                            }
                        }
                    }
                }
            }
            stream<<"}"<<newLine<<newLine;
        }

        /*!< 对下拉框下拉状态的样式处理*/
        if(m_ruleSize != 0 && qssList.contains(seg.selectorName)){
            if(m_selectorType.values().at(qssList.indexOf(seg.selectorName)) == Html::RDROPLIST){
                seg.selectorName = formatProperty.getTypeName(Html::RDROPLIST) + "#" + seg.selectorName + " QAbstractItemView";
                stream<<seg.selectorName<<" {"<<newLine;
                foreach(const CSS::CssRule & rule,seg.rules)
                {
                    if(!deprecatedRulesName.contains(rule.name)){
                        stream<<"\t"<<rule.name<<":"<<rule.value<<";"<<newLine;
                    }
                }
                stream<<"}"<<newLine<<newLine;
            }
        }
        iter++;
    }
}

} //namespace RQt
