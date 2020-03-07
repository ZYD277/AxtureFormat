#include "qtparsemethod.h"

#include <QDebug>
#include <QRect>
#include "../util/rutil.h"

namespace RQt{

QtParseMethod::QtParseMethod()
{
    m_htmlMapToQtWidget.insert(Html::RCONTAINER,"QWidget");
    m_htmlMapToQtWidget.insert(Html::RGROUP,"QWidget");
    m_htmlMapToQtWidget.insert(Html::RBUTTON,"QPushButton");
    m_htmlMapToQtWidget.insert(Html::RDYNAMIC_PANEL,"QStackedWidget");
    m_htmlMapToQtWidget.insert(Html::RTEXT_FIELD,"QLineEdit");
    m_htmlMapToQtWidget.insert(Html::RRADIO_BUTTON,"QRadioButton");
    m_htmlMapToQtWidget.insert(Html::RTABLE,"QTableWidget");
    m_htmlMapToQtWidget.insert(Html::RIMAGE,"QLabel");
    m_htmlMapToQtWidget.insert(Html::RLABEL,"QLabel");
    m_htmlMapToQtWidget.insert(Html::RTREE,"QTreeWidget");
}

/*!
 * @brief 更新数据源
 */
void QtParseMethod::setDataSource(DomHtmlPtr ptr)
{
    m_dataSrc = ptr;
}

void QtParseMethod::setCssMap(CSS::CssMap globalCss, CSS::CssMap pageCss)
{
    m_globalCss = globalCss;
    m_pageCss = pageCss;
}

void QtParseMethod::setResFile(QString resFile)
{
    m_resFileName = resFile;
}

bool QtParseMethod::startSave(QDomDocument &doc)
{
    if(m_dataSrc != nullptr){
        m_resources.clear();

        DocWrapper docWrapper(&doc);
        docWrapper = docWrapper.createElement("ui",true).setAttribute("version","4.0");

        generateDomClass(doc,docWrapper.data(),"Form1");

        Html::DomNode * bodyNode = m_dataSrc->body;
        bodyNode->m_class = "body";

        generateNodeToUI(doc,docWrapper.data(),bodyNode,QRect(QPoint(0,0),RUtil::screenSize()));

        generateResources(doc,docWrapper.data());

        return true;
    }

    return false;
}

void QtParseMethod::generateDomClass(QDomDocument &doc,QDomElement parent,QString nodeText)
{
    QDomElement child = doc.createElement("class");
    QDomText textNode = doc.createTextNode(nodeText);
    child.appendChild(textNode);
    parent.appendChild(child);
}

void QtParseMethod::generateResources(QDomDocument &doc, QDomElement parent)
{
    QDomElement child = doc.createElement("resources");
    QDomElement includeNode = doc.createElement("include");
    includeNode.setAttribute("location",m_resFileName);
    child.appendChild(includeNode);
    parent.appendChild(child);
}

/*!
 * @brief 递归转换控件
 * @details 1.提取当前节点的位置属性
 *          2.根据元素的类型，制定转换策略
 * @param[in] doc xml文档节点
 * @param[in] parent 待转换节点的父节点
 * @param[in] node 待转换节点
 * @param[in] parentGlobalRect 父节点可用区域(全局坐标系，直接子节点的x、y对应需要减去此值的x、y，从而转换成相对位置)
 */
void QtParseMethod::generateNodeToUI(QDomDocument &doc,QDomElement parent,Html::DomNode *node,QRect parentGlobalRect)
{
    //[1]
    QMap<QString, QString> cssMap = extractCssRule(node);

    QRect rect(removePxUnit(cssMap.value("left")),removePxUnit(cssMap.value("top")),
               removePxUnit(cssMap.value("width")),removePxUnit(cssMap.value("height")));

    bool tGeometryExisted = false;
    if(node->m_type == Html::RGROUP){
        Html::GroupData * gdata = dynamic_cast<Html::GroupData*>(node->m_data);
        rect = QRect(gdata->m_left,gdata->m_top,gdata->m_width,gdata->m_height);
        tGeometryExisted = true;
    }


    //转换成父窗口的相对坐标
    rect.moveLeft(rect.x() - parentGlobalRect.x());
    rect.moveTop(rect.y() - parentGlobalRect.y());

    //[2]
    switch(node->m_type){
        case Html::RCONTAINER:{
            QDomElement child = doc.createElement("widget");
            child.setAttribute("class",m_htmlMapToQtWidget.value(node->m_type));
            child.setAttribute("name",node->m_id);

            if(node->m_class != "panel_state")
                generateRect(doc,child,rect);

            parent.appendChild(child);

            if(node->m_childs.size() > 0){
                for(int i = 0;i < node->m_childs.size();i++){
                    generateNodeToUI(doc,child,node->m_childs.at(i),rect);
                }
            }

            break;
        }
        case Html::RGROUP:{
            QDomElement child = doc.createElement("widget");
            child.setAttribute("class",m_htmlMapToQtWidget.value(node->m_type));
            child.setAttribute("name",node->m_id);

            generateRect(doc,child,rect);

            Html::GroupData * gdata = dynamic_cast<Html::GroupData*>(node->m_data);
            rect = QRect(gdata->m_left,gdata->m_top,gdata->m_width,gdata->m_height);

            parent.appendChild(child);


            if(node->m_childs.size() > 0){
                for(int i = 0;i < node->m_childs.size();i++){
                    generateNodeToUI(doc,child,node->m_childs.at(i),rect);
                }
            }

            break;
        }
        case Html::RTABLE:{
            QDomElement child = doc.createElement("widget");
            child.setAttribute("class",m_htmlMapToQtWidget.value(node->m_type));
            child.setAttribute("name",node->m_id);

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

                generateRect(doc,child,rect);

                child.appendChild(createChildElement(RAttribute,RBool,doc,"horizontalHeaderVisible","false"));
                child.appendChild(createChildElement(RAttribute,RBool,doc,"verticalHeaderVisible","false"));

                for(int i = 0; i < rowCount; i++){
                    QDomElement row = doc.createElement("row");

                    row.appendChild(createChildElement(RProperty,RString,doc,"text",QString("row%1").arg(i)));

                    child.appendChild(row);
                }

                for(int i = 0; i < columnCount; i++){
                    QDomElement column = doc.createElement("column");

                    column.appendChild(createChildElement(RProperty,RString,doc,"text",QString("column%1").arg(i)));

                    child.appendChild(column);
                }

                Html::TableData * tdata = dynamic_cast<Html::TableData *>(node->m_data);

                for(int i = 0; i < rowCount; i++){
                    for(int j = 0; j < columnCount; j++){
                        QDomElement item = doc.createElement("item");
                        item.setAttribute("row",QString::number(i));
                        item.setAttribute("column",QString::number(j));

                        item.appendChild(createChildElement(RProperty,RString,doc,"text",QString(tdata->m_items.at(i*columnCount + j))));

                        child.appendChild(item);
                    }
                }
                parent.appendChild(child);
            }
            break;
        }
        case Html::RDYNAMIC_PANEL:{
            QDomElement child = doc.createElement("widget");
            child.setAttribute("class",m_htmlMapToQtWidget.value(node->m_type));
            child.setAttribute("name",node->m_id);

            generateRect(doc,child,rect);

            parent.appendChild(child);

            if(node->m_childs.size() > 0){
                for(int i = 0;i < node->m_childs.size();i++){
                    generateNodeToUI(doc,child,node->m_childs.at(i),rect);
                }
            }
            break;

        }
        case Html::RTEXT_FIELD:
        case Html::RRADIO_BUTTON:
        case Html::RLABEL:
        case Html::RBUTTON:{
            QDomElement child = doc.createElement("widget");
            child.setAttribute("class",m_htmlMapToQtWidget.value(node->m_type));
            child.setAttribute("name",node->m_id);

            generateRect(doc,child,rect);

            Html::PanelData * data = dynamic_cast<Html::PanelData *>(node->m_data);
            if(data){
                QString panellabel = data->m_data_label;
            }

            child.appendChild(createChildElement(RProperty,RString,doc,"text",node->m_data->m_text));

            parent.appendChild(child);

            break;
        }

        case Html::RIMAGE:{
            QDomElement child = doc.createElement("widget");
            child.setAttribute("class",m_htmlMapToQtWidget.value(node->m_type));
            child.setAttribute("name",node->m_id);

            generateRect(doc,child,rect);

            Html::ImageData * data = dynamic_cast<Html::ImageData *>(node->m_data);
            if(data){
                //src中包含了所属页面信息，目前所有images目录下直接是图片，需要移除所属页面信息
                QString imageSrc = data->m_src;
                int firstSplitPos = imageSrc.indexOf("/");
                int secondSplitPos = imageSrc.indexOf("/",firstSplitPos + 1);

                imageSrc = imageSrc.remove(firstSplitPos,secondSplitPos - firstSplitPos);

                m_originalResources.append(data->m_src);
                m_resources.append(imageSrc);

                QDomElement attr = doc.createElement("property");
                attr.setAttribute("name","styleSheet");

                QDomElement tbool = doc.createElement("string");
                tbool.setAttribute("notr","true");

                //WARNING 此处的prefix被指定为/
                tbool.appendChild(doc.createTextNode(QString("background-image:url(:/%1);color:%2;").arg(imageSrc).arg(cssMap.value("color"))));

                attr.appendChild(tbool);

                child.appendChild(attr);
            }

            child.appendChild(createChildElement(RProperty,RString,doc,"text",node->m_data->m_text));
            child.appendChild(createChildElement(RProperty,RSet,doc,"alignment","Qt::AlignCenter"));

            parent.appendChild(child);

            break;
        }
        case Html::RLINE:{
            QDomElement child = doc.createElement("widget");
            child.setAttribute("class","Line");
            child.setAttribute("name",node->m_id);

            generateRect(doc,child,rect);

            Html::BaseData * data = dynamic_cast<Html::BaseData *>(node->m_data);

            if(removePxUnit(cssMap.value("width"))>removePxUnit(cssMap.value("height"))){
                if(!data->m_toolTip.isEmpty())
                    child.appendChild(createChildElement(RProperty,RString,doc,"toolTip",QString("%1").arg(data->m_toolTip)));
                if(data->m_bDisabled)
                    child.appendChild(createChildElement(RProperty,RString,doc,"enabled",QString("false")));
                child.appendChild(createChildElement(RProperty,RString,doc,"lineWidth",QString("%1").arg(removePxUnit(cssMap.value("height")))));
                child.appendChild(createChildElement(RProperty,RString,doc,"orientation",QString("Qt::Horizontal")));
            }
            else{
                if(!data->m_toolTip.isEmpty())
                    child.appendChild(createChildElement(RProperty,RString,doc,"toolTip",QString("%1").arg(data->m_toolTip)));
                if(data->m_bDisabled)
                    child.appendChild(createChildElement(RProperty,RString,doc,"enabled",QString("false")));
                child.appendChild(createChildElement(RProperty,RString,doc,"lineWidth",QString("%1").arg(removePxUnit(cssMap.value("width")))));
                child.appendChild(createChildElement(RProperty,RString,doc,"orientation",QString("Qt::Vertical")));
            }
            parent.appendChild(child);
            break;
        }
        case Html::RTREE:{
            QRect rectTree(0,0,parentGlobalRect.width(),parentGlobalRect.height());

            QDomElement child = doc.createElement("widget");
            child.setAttribute("class",m_htmlMapToQtWidget.value(node->m_type));
            child.setAttribute("name",node->m_id);
            generateRect(doc,child,rectTree);

            QDomElement column = doc.createElement("column");
            column.appendChild(createChildElement(RProperty,RString,doc,"text",node->m_treeText));
            child.appendChild(column);

            parent.appendChild(child);
            if(node->m_childs.size() > 0){
                for(int i = 0;i < node->m_childs.size();i++){
                    generateNodeToUI(doc,child,node->m_childs.at(i),rectTree);
                }
            }
            break;
        }
        case Html::RTREE_CHILDNODE:{

            QDomElement child = doc.createElement("item");

            child.appendChild(createChildElement(RProperty,RString,doc,"text",node->m_treeText));

            parent.appendChild(child);
            if(node->m_childs.size() > 0){
                for(int i = 0;i < node->m_childs.size();i++){
                    generateNodeToUI(doc,child,node->m_childs.at(i),rect);
                }
            }
            break;
        }
        default:break;
    }
}

void QtParseMethod::generateRect(QDomDocument &doc,QDomElement parent,QRect rect)
{
    QDomElement property = doc.createElement("property");
    property.setAttribute("name","geometry");

    QDomElement rectEle = doc.createElement("rect");
    QDomElement x = doc.createElement("x");
    x.appendChild(doc.createTextNode(QString::number(rect.x())));

    QDomElement y = doc.createElement("y");
    y.appendChild(doc.createTextNode(QString::number(rect.y())));

    QDomElement width = doc.createElement("width");
    width.appendChild(doc.createTextNode(QString::number(rect.width())));

    QDomElement height = doc.createElement("height");
    height.appendChild(doc.createTextNode(QString::number(rect.height())));

    rectEle.appendChild(x);
    rectEle.appendChild(y);
    rectEle.appendChild(width);
    rectEle.appendChild(height);

    property.appendChild(rectEle);

    parent.appendChild(property);
}

/*!
 * @brief 分别从全局和当前页面样式中，将当前节点相关样式整合
 * @attention 1.根据class提取全局样式;
 *            2.根据class和id提取当前页面样式;
 * @param[in] node 待提取样式节点
 * @return 当前控件的全部样式信息
 */
QMap<QString,QString> QtParseMethod::extractCssRule(Html::DomNode *node)
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

int QtParseMethod::removePxUnit(QString valueWithUnit)
{
    if(valueWithUnit.contains("px")){
        return valueWithUnit.left(valueWithUnit.indexOf("px")).toInt();
    }
    return valueWithUnit.toInt();
}

QDomElement QtParseMethod::createChildElement(PropertyType ptype,PropValueType vtype,QDomDocument &doc,QString prpoName, QString value)
{
    QString propTagName;
    switch(ptype){
        case RProperty:propTagName = "property";break;
        case RAttribute:propTagName = "attribute";break;
        case RRect:propTagName = "rect";break;
        case RX:propTagName = "x";break;
        case RY:propTagName = "y";break;
        case RWidth:propTagName = "width";break;
        case RHeight:propTagName = "height";break;
        default:break;
    }

    QDomElement attr = doc.createElement(propTagName);
    attr.setAttribute("name",prpoName);

    QString valueTagName;
    switch(vtype){
        case RString:valueTagName = "string";break;
        case RBool:valueTagName = "bool";break;
        case RSet:valueTagName = "set";break;
        default:break;
    }

    QDomElement tbool = doc.createElement(valueTagName);
    tbool.appendChild(doc.createTextNode(value));

    attr.appendChild(tbool);

    return attr;
}

} //namespace RQt
