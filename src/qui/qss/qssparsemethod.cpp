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
//    for(int i = 0; i < m_selectorType.size(); i++){
//        auto iter = pageCss.begin();
//        while(iter != pageCss.end()){
//            QString name = iter.key();
//            CSS::CssSegment seg = iter.value();
//            if(name.contains(m_selectorType.keys().at(i))&&(name.contains("_div")||name.contains("_input"))){
//                name = m_selectorType.keys().at(i);
//            }
//            m_pageCss.insert(name,seg);
//            iter++;
//        }
//    }
    CSS::CssMap pageCssSelect;
    pageCssSelect = m_pageCss;
    for(int i = 0; i < m_selectorType.size(); i++){
        auto iter = pageCssSelect.begin();
        while(iter != pageCssSelect.end()){
            QString name = iter.key();
            if(name.contains(m_selectorType.keys().at(i))&&(name.contains("_div")||name.contains("_input"))){
                m_pageCss.remove(m_selectorType.keys().at(i));
            }
            iter++;
        }
    }
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
                                    if(!m_resources.contains(imageSrc))
                                        m_resources.append(imageSrc);
                                    stream<<"\t"<<"border-image"<<":"<<imageValue<<";"<<newLine;
                                }
                            }
                            else
                            {
                                if(rule.value.contains("gradient"))
                                {
                                    stream<<"\t"<<rule.name<<":"<<getQssGraduatedColour(rule.value)<<";"<<newLine;
                                }
                                else
                                    stream<<"\t"<<rule.name<<":"<<rule.value<<";"<<newLine;
                            }
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
