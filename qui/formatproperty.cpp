﻿#include "formatproperty.h"

#include "../head.h"
#include "../util/rutil.h"
#include "props/mdomwidget.h"
#include "props/mproperty.h"
#include "props/mrow.h"
#include "props/mcolumn.h"
#include "props/mitem.h"

namespace RQt{

FormatProperty::FormatProperty()
{

}

/*!
 * @brief 更新数据源
 */
void FormatProperty::setDataSource(DomHtmlPtr ptr)
{
    m_dataSrc = ptr;
}

void FormatProperty::setCssMap(CSS::CssMap globalCss, CSS::CssMap pageCss)
{
    m_globalCss = globalCss;
    m_pageCss = pageCss;
}

/*!
 * @brief 格式转换
 */
bool FormatProperty::formart()
{
    Check_Return(m_dataSrc.isNull() || m_pageCss.size() ==0,false);

    RDomWidget * rootWidget = new RDomWidget();
    Html::DomNode * root = m_dataSrc->body;
    rootWidget->setAttributeClass(getTypeName(root->m_type));
    rootWidget->setAttributeName(root->m_id);

    std::for_each(root->m_childs.begin(),root->m_childs.end(),[&](Html::DomNode * element){
        createDomWidget(rootWidget,element,QRect(QPoint(0,0),RUtil::screenSize()));
    });

    return true;
}

/*!
 * @brief 获取节点对应的Qt控件名
 * @param[in] type 节点类型
 * @return qt控件名
 */
QString FormatProperty::getTypeName(Html::NodeType type)
{
    switch(type){
        case Html::RCONTAINER:
        case Html::RGROUP:return QString("QWidget");break;
        case Html::RBUTTON:return QString("QPushButton");break;
        case Html::RDYNAMIC_PANEL:return QString("QStackedWidget");break;
        case Html::RTEXT_FIELD:return QString("QLineEdit");break;
        case Html::RRADIO_BUTTON:return QString("QRadioButton");break;
        case Html::RTABLE:return QString("QTableWidget");break;
        case Html::RIMAGE:
        case Html::RLABEL:return QString("QLabel");break;
        case Html::RTREE:return QString("QTreeWidget");break;

        default:return QString();break;
    }
}

/*!
 * @brief 递归转换控件
 * @details 1.提取当前节点的位置属性
 *          2.根据元素的类型，制定转换策略
 * @param[in] parent 待转换节点的父节点
 * @param[in] node 待转换节点
 * @param[in] parentRect 父节点可用区域(全局坐标系，直接子节点的x、y对应需要减去此值的x、y，从而转换成相对位置)
 */
void FormatProperty::createDomWidget(RDomWidget * parentWidget,Html::DomNode *node,QRect parentRect)
{
    RDomWidget * domWidget = new RDomWidget();
    domWidget->setAttributeClass(getTypeName(node->m_type));
    domWidget->setAttributeName(node->m_id);

    StyleMap cssMap = extractCssRule(node);

    MProperty * rectProp = new MProperty();
    rectProp->setAttributeName("geometry");
    MRect * rrect = new MRect();
    QRect rect = calculateGeomerty(cssMap,node,parentRect);
    rrect->setRect(rect);
    rectProp->setPropRect(rrect);

    domWidget->addProperty(rectProp);

    parentWidget->addWidget(domWidget);

    switch(node->m_type){
        case Html::RCONTAINER:{

            break;
        }
        case Html::RGROUP:{
            Html::GroupData * gdata = dynamic_cast<Html::GroupData*>(node->m_data);
            rect = QRect(gdata->m_left,gdata->m_top,gdata->m_width,gdata->m_height);
            break;
        }
        case Html::RTABLE:{
            //需根据表格宽度与单元格宽度相除结果，作为列数
            int cWidth = 0;
            if(node->m_childs.size() > 0){
                QString subCellId = node->m_childs.first()->m_id;
                CSS::CssSegment cellSegment = m_pageCss.value(subCellId);
                if(cellSegment.rules.size() > 0){
                    auto fresult = std::find_if(cellSegment.rules.begin(),cellSegment.rules.end(),[&](const CSS::CssRule & rule ){
                        return rule.name.toLower() == "width";
                    });

                    if(fresult != cellSegment.rules.end()){
                        cWidth = removePxUnit(fresult->value);
                    }
                }
            }

            if(cWidth > 0){
                int rowCount = rect.width() / cWidth;
                int columnCount = node->m_childs.size() / rowCount;

                MProperty * hprop = new MProperty();
                hprop->setAttributeName("horizontalHeaderVisible");
                hprop->setPropBool("false");

                MProperty * vprop = new MProperty();
                vprop->setAttributeName("verticalHeaderVisible");
                vprop->setPropBool("false");

                for(int i = 0; i < rowCount; i++){
                    MRow  * row = new MRow();

                    MProperty * rowProp = new MProperty();
                    rowProp->setPropString(QString("row%1").arg(i));
                    row->addProperty(rowProp);

                    domWidget->addRow(row);
                }

                for(int i = 0; i < columnCount; i++){
                    MColumn  * column = new MColumn();

                    MProperty * rowProp = new MProperty();
                    rowProp->setPropString(QString("column%1").arg(i));
                    column->addProperty(rowProp);

                    domWidget->addColumn(column);
                }

                Html::TableData * tdata = dynamic_cast<Html::TableData *>(node->m_data);

                for(int i = 0; i < rowCount; i++){
                    for(int j = 0; j < columnCount; j++){
                        MItem * item = new MItem();

                        item->setAttributeRow(QString::number(i));
                        item->setAttributeColumn(QString::number(j));
                        MProperty * prop = new MProperty();
                        prop->setAttributeName("text");
                        prop->setPropString(QString(tdata->m_items.at(i*columnCount + j)));
                        item->setProperty(prop);

                        domWidget->addItem(item);
                    }
                }
            }
            break;
        }

        case Html::RTEXT_FIELD:{
            Html::TextFieldData * textData = dynamic_cast<Html::TextFieldData *>(node->m_data);

            createTextProp(domWidget,textData->m_text);
            createReadonlyProp(domWidget,textData->m_bReadOnly);
            createEnableProp(domWidget,textData->m_bDisabled);
            createToolTipProp(domWidget,textData->m_toolTip);

            MProperty * echoModeProp = new MProperty;
            echoModeProp->setAttributeName("echoMode");
            if(textData->m_type == "password"){
                echoModeProp->setPropEnum("QLineEdit::Password");
            }else{
                echoModeProp->setPropEnum("QLineEdit::Normal");
            }
            domWidget->addProperty(echoModeProp);

            if(textData->m_maxLength != 32767){
                MProperty * maxLenProp = new MProperty;
                maxLenProp->setAttributeName("maxLength");
                maxLenProp->setPropNumber(QString::number(textData->m_maxLength));
                domWidget->addProperty(maxLenProp);
            }

            if(textData->m_toolTip.size() > 0)
                createToolTipProp(domWidget,textData->m_toolTip);

            break;
        }

        case Html::RRADIO_BUTTON:{
            createCheckedProp(domWidget,node->m_data->m_bChecked);
            createEnableProp(domWidget,node->m_data->m_bDisabled);
            createTextProp(domWidget,node->m_data->m_text);
            createLayoutDirectionProp(domWidget,node->m_data->m_bLeftToRight);

            if(node->m_data->m_toolTip.size() > 0)
                createToolTipProp(domWidget,node->m_data->m_toolTip);

            break;
        }

        case Html::RLABEL:{

            createEnableProp(domWidget,node->m_data->m_bDisabled);
            createTextProp(domWidget,node->m_data->m_text);

            if(node->m_data->m_toolTip.size() > 0)
                createToolTipProp(domWidget,node->m_data->m_toolTip);

            break;
        }

        case Html::RBUTTON:{
            createCheckedProp(domWidget,node->m_data->m_bChecked);
            createEnableProp(domWidget,node->m_data->m_bDisabled);
            createTextProp(domWidget,node->m_data->m_text);

            if(node->m_data->m_toolTip.size() > 0)
                createToolTipProp(domWidget,node->m_data->m_toolTip);
            break;
        }

        default:break;
    }

    if(node->m_childs.size() > 0 && node->m_type != Html::RTABLE){
        std::for_each(node->m_childs.begin(),node->m_childs.end(),[&](Html::DomNode * element){
            createDomWidget(domWidget,element,rect);
        });
    }
}

/*!
 * @brief 分别从全局和当前页面样式中，将当前节点相关样式整合
 * @attention 1.根据class提取全局样式;
 *            2.根据class和id提取当前页面样式;
 * @param[in] node 待提取样式节点
 * @return 当前控件的全部样式信息
 */
FormatProperty::StyleMap FormatProperty::extractCssRule(Html::DomNode *node)
{
    auto addCss = [&](const CSS::CssSegment & cssSegment,QMap<QString,QString> & finalRuleMap){
        if(cssSegment.rules.size() > 0){
            foreach(const CSS::CssRule & crule,cssSegment.rules){
                finalRuleMap.insert(crule.name,crule.value);
            }
        }
    };

    //[1] 从全局css中提取对应class的样式,body属性不在全局样式中
    QStringList nodeClassList = node->m_class.trimmed().split(QRegExp("\\s+"));
    QMap<QString,QString> finalRuleMap;
    std::for_each(nodeClassList.begin(),nodeClassList.end(),[&](const QString clazz){
        addCss(m_globalCss.value(clazz),finalRuleMap);
    });

    //[2] 从当前页面中提取对应class和id的样式
    if(node->m_id.size() > 0 || node->m_class.size() > 0){
        addCss(m_pageCss.value(node->m_class),finalRuleMap);

        addCss(m_pageCss.value(node->m_id),finalRuleMap);
    }

    return finalRuleMap;
}

/*!
 * @brief 计算当前节点位置信息
 * @attention 位置计算属于使用相对计算方式，需要在父窗口基础上做出一定的偏移
 * @param[in] cssMap 节点所有样式集合
 * @param[in] node 当前节点
 * @param[in] parentRect 节点父窗口尺寸
 * @return 当前节点相对父窗口的偏移位置
 */
QRect FormatProperty::calculateGeomerty(FormatProperty::StyleMap &cssMap, Html::DomNode *node, QRect &parentRect)
{
    QRect rect(removePxUnit(cssMap.value("left")),removePxUnit(cssMap.value("top")),
               removePxUnit(cssMap.value("width")),removePxUnit(cssMap.value("height")));

    bool tGeometryExisted = false;
    if(node->m_type == Html::RGROUP){
        Html::GroupData * gdata = dynamic_cast<Html::GroupData*>(node->m_data);
        rect = QRect(gdata->m_left,gdata->m_top,gdata->m_width,gdata->m_height);
        tGeometryExisted = true;
    }

    //转换成父窗口的相对坐标
    rect.moveLeft(rect.x() - parentRect.x());
    rect.moveTop(rect.y() - parentRect.y());

    return rect;
}

int FormatProperty::removePxUnit(QString valueWithUnit)
{
    if(valueWithUnit.contains("px")){
        return valueWithUnit.left(valueWithUnit.indexOf("px")).toInt();
    }
    return valueWithUnit.toInt();
}

void FormatProperty::createTextProp(RDomWidget *domWidget, QString text)
{
    MProperty * textProp = new MProperty;
    textProp->setAttributeName("text");
    textProp->setPropString(text);
    domWidget->addProperty(textProp);
}

void FormatProperty::createReadonlyProp(RDomWidget *domWidget, bool readonly)
{
    MProperty * readOnlyProp = new MProperty;
    readOnlyProp->setAttributeName("readOnly");
    readOnlyProp->setPropBool(readonly?"true":"false");
    domWidget->addProperty(readOnlyProp);
}

void FormatProperty::createEnableProp(RDomWidget *domWidget, bool enable)
{
    MProperty * enableProp = new MProperty;
    enableProp->setAttributeName("enabled");
    enableProp->setPropBool(enable?"true":"false");
    domWidget->addProperty(enableProp);
}

/*!
 * @brief 创建可选中树形
 * @attention checkbox、radiobutton的checked属性依赖checkable属性，可同时创建这两个属性.
 *            axure默认不支持checkable属性，因此默认将checkable属性设置为true。
 */
void FormatProperty::createCheckedProp(RDomWidget *domWidget, bool checked)
{
    MProperty * checkableProp = new MProperty;
    checkableProp->setAttributeName("checkable");
    checkableProp->setPropBool("true");
    domWidget->addProperty(checkableProp);

    MProperty * checkedProp = new MProperty;
    checkedProp->setAttributeName("checked");
    checkedProp->setPropBool(checked?"true":"false");
    domWidget->addProperty(checkedProp);
}

void FormatProperty::createLayoutDirectionProp(RDomWidget *domWidget, bool leftToRight)
{
    MProperty * layoutProp = new MProperty;
    layoutProp->setAttributeName("layoutDirection");
    layoutProp->setPropEnum(leftToRight?"Qt::LeftToRight":"Qt::RightToLeft");
    domWidget->addProperty(layoutProp);
}

void FormatProperty::createToolTipProp(RDomWidget *domWidget, QString toolTip)
{
    if(toolTip.size() != 0){
        MProperty * tipProp = new MProperty;
        tipProp->setAttributeName("statusTip");
        tipProp->setPropString(toolTip);
        domWidget->addProperty(tipProp);
    }
}

} //namespace RQt