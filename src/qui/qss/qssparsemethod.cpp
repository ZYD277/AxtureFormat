#include "qssparsemethod.h"

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
 * @attention
 * @param[in]
 * @return
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

        if(name.contains("_div")||name.contains("_input"))
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

                seg.rules = filterDuplicateData(seg.rules);//过滤重复属性
            }
            else if((findSrc != pageCss.end())||(findOther != pageCss.end()))
            {
                CSS::CssSegment tmpSeg = findSrc != pageCss.end() ? findSrc.value() : findOther.value();
                seg.rules = tmpSeg.rules + seg.rules;

                seg.rules = filterDuplicateData(seg.rules);//过滤重复属性
            }
        }
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

/**
 * @brief 过滤重复样式
 * @param rules待处理数据表
 * @return 返回处理完的数据
 */
CSS::Rules QSSParseMethod::filterDuplicateData(CSS::Rules rules)
{
    QMap<QString,QString> tmpRules;
    std::for_each(rules.begin(),rules.end(),[&](CSS::CssRule cssRule){
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

bool QSSParseMethod::startSave(RTextFile *file)
{
    QTextStream stream(file);

    QString newLine = "\r";

    auto generateCss = [&](CSS::CssMap& cssMap){

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

        while(iter != cssMap.end()){
            CSS::CssSegment & seg = iter.value();

            SelectorTypeMap selectorType;
            FormatProperty formatProperty;
            selectorType = formatProperty.getHtmlParsedResult();

            m_ruleSize = seg.rules.size();

            for(int i = 0; i < seg.rules.size(); i++){
                QString ruleName = seg.rules.at(i).name;
                QString ruleValue = seg.rules.at(i).value;

                if(ruleName == "border-color" && ruleValue == "transparent"){
                    ruleName = "border";
                    ruleValue = "none";
                }
                else if(ruleValue.contains("rgba") && ruleName.contains("color")){
                    int index1 = ruleValue.lastIndexOf(" ");
                    int index2 = ruleValue.lastIndexOf(")");
                    QString t_ruleValue;
                    QString tmp = ruleValue.right(index2-index1);
                    QString tp = tmp.left( tmp.lastIndexOf(")"));
                    if(tp.toDouble() >= 0 && tp.toDouble() <= 1){
                        int b = tp.toDouble() * 255;
                        t_ruleValue = ruleValue.left(ruleValue.size() - (index2-index1)) + QString::number(b) + ")";
                    }
                    ruleValue = t_ruleValue;
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

                            /*!< 设计的子节点和主节点的查找*/
                            QStringList selectorNames = divList.at(i).split("_div_");
                            QString selectorName = seg.selectorName;

                            if(selectorNames.size() > 1){
                                if((selectorNames.at(0) + "_div" == seg.selectorName) || (selectorNames.at(0) == seg.selectorName))
                                {
                                    selectorName ="#" + selectorNames.at(1);
                                    if(m_selectorType.values().at(qssList.indexOf(divList.at(i))) == Html::RTREE)
                                    {
                                        selectorName ="#" + selectorNames.at(1) + "::item";
                                    }
                                }else if(seg.selectorName.contains(selectorNames.at(0)+":")){
                                    selectorName ="#" + selectorName.replace(selectorNames.at(0),selectorNames.at(1) + "::item");
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
                                stream<<"\t"<<rule.name<<":"<<getQssGraduatedColour(rule.value)<<";"<<newLine;
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
                    seg.selectorName ="#" + seg.selectorName + " QAbstractItemView";
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
    };

    generateCss(m_globalCss);

    generateCss(m_pageCss);

    return true;
}

/**
 * @brief 获取简单的渐变背景
 */
QString QSSParseMethod::getQssGraduatedColour(QString value)
{
    QStringList list = value.split(" ");
    QStringList m_rgbList;
    QString t_rgb;

    QStringList rangeList;
    QString rangel;

    QString gradientDirection;
    std::for_each(list.begin(),list.end(),[&](QString graduatedInfo){
        if(graduatedInfo.contains("gradient"))
        {
            if(graduatedInfo.contains(","))
            {
                graduatedInfo = graduatedInfo.remove(QChar(','),Qt::CaseInsensitive);
                QStringList t_filtration = graduatedInfo.split("(");
                std::for_each(t_filtration.begin(),t_filtration.end(),[&](QString filtrationDirection){
                    if(filtrationDirection.contains("deg"))
                    {
                        gradientDirection =  filtrationDirection;
                    }
                });
            }
        }
        else if(graduatedInfo.contains("%"))
        {
            if(graduatedInfo.contains(")"))
            {
                graduatedInfo.remove(QChar(')'),Qt::CaseInsensitive);
            }
            rangel = rangel + graduatedInfo;
        }
        else
        {
            if(graduatedInfo.contains(")"))
            {
                t_rgb = t_rgb + graduatedInfo;
                m_rgbList.append(t_rgb);
                t_rgb.clear();
            }
            else
                t_rgb = t_rgb + graduatedInfo;
        }
    });
    rangeList = rangel.split(",");

    QStringList rangeFloats;
    std::for_each(rangeList.begin(),rangeList.end(),[&](QString graduatedRange){
        if(graduatedRange.contains("%"))
        {
            float rangeFloat = (graduatedRange.left(graduatedRange.indexOf("%")).toInt())/float(100);
            rangeFloats.append(QString::number(rangeFloat));
        }
    });
    QString graduatedInfo;
    if(rangeFloats.size() == m_rgbList.size())
    {
        for(int i = 0;i<rangeFloats.size();i++)
        {
            QString t_stopFloat = rangeFloats.at(i);
            QString t_rgbInfo = m_rgbList.at(i);
            QString t_graduatedInfo = QString(" stop:%1 %2,").arg(t_stopFloat).arg(t_rgbInfo);
            graduatedInfo += t_graduatedInfo;
        }
    }

    int xDirection;
    int yDirection;
    if((gradientDirection == "90deg")||(gradientDirection == "270deg"))
    {
        xDirection = 1;
        yDirection = 0;
    }
    else if((gradientDirection == "180deg")||(gradientDirection == "0deg"))
    {
        xDirection = 0;
        yDirection = 1;
    }
    value = QString(" qlineargradient(spread:pad, x1:0, y1:0, x2:%1, y2:%2, %3)").arg(QString::number(xDirection))
            .arg(QString::number(yDirection)).arg(graduatedInfo);
    return value;
}
} //namespace RQt
