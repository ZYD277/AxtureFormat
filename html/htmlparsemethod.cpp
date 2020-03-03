#include "htmlparsemethod.h"

#include <QDebug>

namespace Html{


QDomElement DomWrapper::getIndexChildElement(const QDomElement &element, int index)
{
    QDomNodeList childs = element.childNodes();
    if(index >= 0 && index < childs.size()){
        int loop = -1;
        for(int i = 0;i < childs.size(); i++){
            if(childs.at(i).isElement()){
                if(++loop == index)
                    return childs.at(i).toElement();
            }
        }
    }
    return QDomElement();
}



HtmlParseMethod::HtmlParseMethod()
{

}

bool HtmlParseMethod::startParse(QDomNode &rootNode)
{
    if(rootNode.nodeName() == m_nodeType.HTML){
        m_result = DomHtmlPtr(new DomHtml);

        //TODO 20200112解析head标签，获取css属性

        //解析body标签
        QDomNodeList bodyList = rootNode.toElement().elementsByTagName(m_nodeType.BODY);
        if(bodyList.size() == 1){
            QDomElement body = bodyList.at(0).toElement();
            parseBody(body,m_result.data());

            //            printBody(m_result->body);
        }
        return true;
    }

    return false;
}

/*!
 * @brief 解析html中body标签
 * @details axture的body标签只有一个子节点div，
 * @param[in] element body节点
 * @param[in] parentNode body父节点
 * @return true:解析成功；false:解析失败
 */
void HtmlParseMethod::parseBody(const QDomElement & bodyElement,DomHtml * parentNode)
{
    DomNode * body = new DomNode(RCONTAINER);
    parentNode->body = body;

    QDomElement div = firstChildElement(bodyElement);
    body->m_id = div.attribute(m_nodeType.ID);
    body->m_class = div.attribute(m_nodeType.CLASS);

    parseDiv(div,body);
}

/*!
 * @brief 解析DIV标签
 * @details axture用div标签将各个控件包裹起来，可通过div的class属性、第一个子节点等方式来判定元素的类型
 * @param[in] divElement 待解析的div信息
 * @param[in] parentNode 父节点
 */
void HtmlParseMethod::parseDiv(const QDomElement &divElement, DomNode *parentNode)
{
    QDomNodeList childNodeList = divElement.childNodes();
    for(int i = 0; i < childNodeList.size(); i++){
        if(childNodeList.at(i).isElement()){
            QDomElement childEle = childNodeList.at(i).toElement();

            NodeType ttype = getNodeType(childEle,QDomElement());

            if(ttype != RINVALID){
                DomNode * node = new DomNode(ttype);
                node->m_id = childEle.attribute(m_nodeType.ID);
                node->m_class = childEle.attribute(m_nodeType.CLASS);
                node->m_style = childEle.attribute(m_nodeType.STYLE);

                parseNodeData(childEle,ttype,node);

                establishRelation(parentNode,node);

                if(ttype == RGROUP){
                    parseDiv(childEle,node);
                }
            }else{

            }
        }
    }
}

/*!
 * @brief 根据class属性提取控件类型
 * @attention 会存在在一级节点中无法直接判断类型，需要在其子节点中判断类型
 * @param[in] divElement 节点
 * @param[in] parentElement 父节点
 * @return 若能解析则返回对应控件类型，否则返回无效
 */
NodeType HtmlParseMethod::getNodeType(const QDomElement &element,QDomElement parentElement)
{
    //TODO 20200112 容器节点如何检测？？？？
    if(!element.isNull()){
        QString classInfo = element.attribute(m_nodeType.CLASS);
        if(classInfo.isEmpty()){
            if(!parentElement.isNull()){
                QDomElement secondElement = secondChildElement(parentElement);
                if(!secondElement.isNull()){
                    return getNodeType(secondElement,element);
                }else{
                    QString pClassInfo = parentElement.attribute(m_nodeType.CLASS);
                    if(pClassInfo.contains("box_1") || pClassInfo.contains("box_2") || pClassInfo.contains("box_3"))
                        return RCONTAINER;
                }
            }

            QDomElement firstElement = firstChildElement(element);
            if(!firstElement.isNull()){
                return getNodeType(firstElement,element);
            }
        }else{

            //NOTE 匹配顺序按照优先级排列
            if(element.hasAttribute("data-left") && element.hasAttribute("data-left") && element.hasAttribute("data-left"))
                return RGROUP;

            if(classInfo.contains("radio_button"))
                return RRADIO_BUTTON;
            else if(classInfo.contains("text_field"))
                return RTEXT_FIELD;
            else if(classInfo.contains("label") || classInfo.contains("text"))
                return RLABEL;
            else if(classInfo.contains("button") || classInfo.contains("primary_button"))
                return RBUTTON;
            else if(classInfo.contains("panel_state")&&!classInfo.contains("panel_state_content"))
                return RDYNAMIC_PANEL;
            else if(classInfo.contains("table_cell"))
                return RTABLE;
            else if(classInfo.contains("tree_node"))
                return RTREE;
            else{
                //img标签和box_1需要结合parentElement
                if(!parentElement.isNull()){
                    QString pClassInfo = parentElement.attribute(m_nodeType.CLASS);

                    bool parentContainBox = (pClassInfo.contains("box_1") ||  pClassInfo.contains("box_2") || pClassInfo.contains("box_3"));
                    //父节点为box_1，第一个子节点class属性为img
                    if(classInfo.contains("img") || (parentContainBox && classInfo.contains("img")))
                        return RIMAGE;
                    else{
                        if(parentContainBox){
                            return RCONTAINER;
                        }else{
                            //父节点为box_1，第一个子节点class属性为空，检测第二个子节点class属性
                            QDomElement secondElement = secondChildElement(parentElement);
                            if(!secondElement.isNull()){
                                return getNodeType(secondElement,element);
                            }
                        }
                    }
                }else{
                    return getNodeType(firstChildElement(element),element);
                }

            }
        }
    }
    return RINVALID;
}

/*!
 * @brief 返回第一个类型为element的节点
 * @attention xml中注释也是一个节点，但这里需要的是如<div>这样的元素节点
 * @param[in] element 父节点
 * @return 若子元素中没有类型为element的则返回空节点，否则返回第一个element节点
 */
QDomElement HtmlParseMethod::firstChildElement(const QDomElement &element)
{
    return getIndexChildElement(element,0);
}

QDomElement HtmlParseMethod::secondChildElement(const QDomElement &element)
{
    return getIndexChildElement(element,1);
}

/*!
 * @brief 获取节点第index个子节点，若没有则返回空节点
 * @param[in] element 操作的节点
 * @param[in] index 子节点顺序
 * @return 若子元素中没有类型为element的则返回空节点，否则返回第index个element节点
 */
QDomElement HtmlParseMethod::getIndexChildElement(const QDomElement &element, int index)
{
    QDomNodeList childs = element.childNodes();
    if(index >= 0 && index < childs.size()){
        int loop = -1;
        for(int i = 0;i < childs.size(); i++){
            if(childs.at(i).isElement()){
                if(++loop == index)
                    return childs.at(i).toElement();
            }
        }
    }
    return QDomElement();
}

/*!
 * @brief 建立父子节点之间关系
 * @param[in] parentNode 父节点
 * @param[in] childNode 子节点
 */
void HtmlParseMethod::establishRelation(DomNode *parentNode, DomNode *childNode)
{
    if(parentNode && childNode){
        parentNode->m_childs.append(childNode);
        childNode->m_parent = parentNode;
    }
}

void HtmlParseMethod::printBody(DomNode *node)
{
    qDebug()<<node->m_id<<node->m_type;

    if(node->m_childs.size() > 0){
        foreach(DomNode * child,node->m_childs){
            printBody(child);
        }
    }
}

/*!
 * @brief 解析元素中包含的数据信息
 * @param[in] element 待解析的确定的元素节点
 * @param[in] type 元素类型
 * @param[in] node 元素节点
 * @return
 */
void HtmlParseMethod::parseNodeData(QDomElement &element, NodeType type, DomNode * node)
{
    switch(type){
        case RBUTTON:parseButtonNodeData(element,node);break;
        case RRADIO_BUTTON:parseRadioButtonNodeData(element,node);break;
        case RDYNAMIC_PANEL:parserdynamicPanelNodeData(element,node);break;
        case RTEXT_FIELD:parseTextFieldNodeData(element,node);break;
        case RIMAGE:parseImageNodeData(element,node);break;
        case RTABLE:parseTableNodeData(element,node);break;
        case RGROUP:parseGroupNodeData(element,node);break;
        case RLABEL:parseLabelNodeData(element,node);break;
        case RTREE:parseTreeNodeData(element,node);break;
        default:break;
    }
}

/*!
 * @brief 解析单button控件
 * @attention 此处需要依赖于button控件在axture中代码格式
 */
void HtmlParseMethod::parseButtonNodeData(QDomElement &element, DomNode *node)
{
    DomWrapper wrapper(element);

    BaseData * data = new BaseData();
    data->m_text = wrapper.secondChild().firstChild().firstChild().data().text();
    node->m_data = data;
}

/*!
 * @brief 解析radio button控件
 * @attention 此处需要依赖于radio button控件在axture中代码格式
 */
void HtmlParseMethod::parseRadioButtonNodeData(QDomElement &element, DomNode *node)
{
    DomWrapper wrapper(element);

    BaseData * data = new BaseData();
    data->m_text = wrapper.firstChild().firstChild().firstChild().firstChild().data().text();
    node->m_data = data;
}

/**
 * @brief 解析rdynamicPanel
 * @param element
 * @param node
 */
void HtmlParseMethod::parserdynamicPanelNodeData(QDomElement &element, DomNode *parentNode)
{
    QDomNodeList childs = element.childNodes();
    for(int i = 0; i < childs.size(); i++){
        if(childs.at(i).isElement()){
            if(childs.at(i).toElement().attribute(m_nodeType.CLASS) == "panel_state"){
                DomNode * node = new DomNode(RCONTAINER);
                node->m_id = childs.at(i).toElement().attribute(m_nodeType.ID);
                node->m_class = childs.at(i).toElement().attribute(m_nodeType.CLASS);
                node->m_style = childs.at(i).toElement().attribute(m_nodeType.STYLE);

                establishRelation(parentNode,node);
            }
            else{
                parserdynamicPanelNodeData(childs.at(i).toElement(),parentNode);
            }
        }
    }
}

void HtmlParseMethod::parseLabelNodeData(QDomElement &element, DomNode *node)
{
    DomWrapper wrapper(element);

    BaseData * data = new BaseData();
    data->m_text = wrapper.secondChild().firstChild().firstChild().data().text();
    node->m_data = data;
}

/*!
 * @brief 解析单行文本输入框控件
 * @attention 此处需要依赖于radio button控件在axture中代码格式
 */
void HtmlParseMethod::parseTextFieldNodeData(QDomElement &element, DomNode *node)
{
    DomWrapper wrapper(element);

    BaseData * data = new BaseData();
    data->m_text = wrapper.firstChild().attribute(m_nodeType.VALUE);
    node->m_data = data;
}

/*!
 * @brief 解析image控件
 * @attention 此处需要依赖于image控件在axture中代码格式
 */
void HtmlParseMethod::parseImageNodeData(QDomElement &element, DomNode *node)
{
    DomWrapper wrapper(element);

    ImageData * data = new ImageData();
    data->m_src = wrapper.firstChild().attribute(m_nodeType.SRC);

    DomWrapper wrapper2(element);
    data->m_text = wrapper2.secondChild().firstChild().firstChild().data().text();

    node->m_data = data;
}

/*!
 * @brief 解析table控件
 * @attention 此处需要依赖于table控件在axture中代码格式
 * @warning 无法通过此来判断行和列的关系，需要结合css样式表
 */
void HtmlParseMethod::parseTableNodeData(QDomElement &element, DomNode *node)
{
    QDomNodeList childs = element.childNodes();
    TableData * data = new TableData();
    node->m_data = data;
    for(int i = 0; i < childs.size(); i++){
        if(childs.at(i).isElement()){
            DomWrapper wrapper(childs.at(i).toElement());
            parseTableCellNodeData(childs.at(i).toElement(),node);

            data->m_items.append(wrapper.secondChild().firstChild().firstChild().data().text());
        }
    }
}

/**
 * @brief 解析tree控件
 * @param element
 * @param node
 */
void HtmlParseMethod::parseTreeNodeData(QDomElement &element, DomNode *node)
{
    if(element.toElement().attribute(m_nodeType.CLASS).contains("treeroot")){
        QDomNodeList childs = element.childNodes().at(0).toElement().childNodes().at(1).toElement().childNodes();
        TreeData * data = new TreeData();

        for(int i = 1; i < childs.size(); i++){

            QDomElement childEle = childs.at(i).toElement();

            DomWrapper wrapper(childEle.toElement());

            QString columsData = wrapper.data().text();
            if(!columsData.isEmpty()){
                data->m_dataCoIt.append(columsData);
            }
        }
        QString colums = data->m_dataCoIt[0];
        QString items = data->m_dataCoIt[1];

        data->m_colums = colums.split("Item");
        data->m_items = items.split("Item");

        node->m_data = data;
    }
}

/*!
 * @brief 解析table的cell控件
 */
void HtmlParseMethod::parseTableCellNodeData(QDomElement &element, DomNode * parentNode)
{
    DomNode * node = new DomNode(RTABLE_CELL);
    node->m_id = element.attribute(m_nodeType.ID);
    node->m_class = element.attribute(m_nodeType.CLASS);
    node->m_style = element.attribute(m_nodeType.STYLE);
    establishRelation(parentNode,node);
}

/*!
 * @brief 解析分组控件
 * @attention 此处需要依赖于group分组在axture中代码格式
 */
void HtmlParseMethod::parseGroupNodeData(QDomElement &element, DomNode *node)
{
    GroupData * data = new GroupData();
    data->m_left = element.attribute("data-left").toInt();
    data->m_top = element.attribute("data-top").toInt();
    data->m_width = element.attribute("data-width").toInt();
    data->m_height = element.attribute("data-height").toInt();
    node->m_data = data;
}

} //namespace Html
