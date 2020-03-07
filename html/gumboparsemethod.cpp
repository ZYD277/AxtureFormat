#include "gumboparsemethod.h"

#include <QDebug>

namespace Html{

NodeHtml G_NodeHtml;

GumboParseMethod::GumboParseMethod():m_gumboParser(nullptr)
{

}

GumboParseMethod::~GumboParseMethod()
{

}

bool GumboParseMethod::startParse(RTextFile *file)
{
    Check_Return(!parseFile(file),false);

    GumboNodeWrapper html(m_gumboParser->root);
    Check_Return(!html.valid(),false);

    if(html.valid()){
        GumboNodeWrapper bodyNode = html.elementByTagName(G_NodeHtml.BODY);
        Check_Return(!bodyNode.valid(),false);

        m_htmlResultPtr = DomHtmlPtr(new DomHtml);
        parseBody(bodyNode);

//        printBody(m_htmlResultPtr->body);
    }

    return true;
}

/*!
 * @brief 若文件包含bom头，文件指针跳过3字节
 * @attention bom格式开头的文件前三个字节分别为0xEF 0xBB 0xBF，若包含则移除这三个字节
 * @param[in] file 打开文件句柄
 */
void GumboParseMethod::skipBomHead(RTextFile *file)
{
    uchar buff[3] = {0};
    file->read((char *)buff,3);
    if(buff[0] == 0xEF && buff[1] == 0xBB && buff[2] == 0xBF){
        return;
    }
    file->seek(0);
}

bool GumboParseMethod::parseFile(RTextFile *file)
{
    if(file == nullptr){
        m_errorMsg = QStringLiteral("文件句柄空!");
        return false;
    }

    skipBomHead(file);

    QByteArray allFileData = file->readAll();
    Check_Return(allFileData.size() == 0,false);

    m_gumboParser = gumbo_parse(allFileData.data());

    Check_Return_Cb(m_gumboParser->errors.length > 0,false,[&](){gumbo_destroy_output(&kGumboDefaultOptions,m_gumboParser);});

    return true;
}

void GumboParseMethod::parseBody(GumboNodeWrapper &bodyNode)
{
    DomNode * body = new DomNode(RCONTAINER);
    m_htmlResultPtr->body = body;

    GumboNodeWrapper baseDiv = bodyNode.firstChild();
    Check_Return(!baseDiv.valid(),);

    body->m_id = baseDiv.id();
    body->m_class = baseDiv.clazz();

    parseDiv(baseDiv,body);
}

void GumboParseMethod::parseDiv(GumboNodeWrapper &divNode, DomNode *parentNode)
{
    GumboNodeWrapperList childNodeList = divNode.children();
    for(int i = 0; i < childNodeList.size(); i++){
        GumboNodeWrapper childEle = childNodeList.at(i);
        NodeType ttype = getNodeType(childEle,GumboNodeWrapper());
        NodeType ttypeNew = ttype;
        if(ttype == RTREE)
            ttypeNew = RCONTAINER;

        if(ttypeNew != RINVALID){
            DomNode * node = new DomNode(ttypeNew);
            node->m_id = childEle.id();
            node->m_class = childEle.clazz();
            node->m_style = childEle.style();

            if(ttype == RTREE)
                parseNodeData(childEle,ttype,node);
            else
                parseNodeData(childEle,ttypeNew,node);

            establishRelation(parentNode,node);

            if(ttypeNew == RGROUP){
                parseDiv(childEle,node);
            }
        }else{

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
NodeType GumboParseMethod::getNodeType(GumboNodeWrapper &element, GumboNodeWrapper parentElement)
{
    //TODO 20200112 容器节点如何检测？？？？
    if(element.valid()){
        QString classInfo = element.clazz();
        if(classInfo == m_classInfo && classInfo == "ax_default"){
            return RGROUP;
        }
        m_classInfo = classInfo;
        if(classInfo.isEmpty()){
            if(!parentElement.valid()){
                GumboNodeWrapper secondElement = parentElement.secondChild();
                if(secondElement.valid()){
                    return getNodeType(secondElement,element);
                }else{
                    QString pClassInfo = parentElement.clazz();
                    if(pClassInfo.contains("box_1") || pClassInfo.contains("box_2") || pClassInfo.contains("box_3"))
                        return RCONTAINER;
                }
            }

            GumboNodeWrapper firstElement = element.firstChild();
            if(firstElement.valid()){
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
            else if(classInfo.contains("panel_state"))
                return RDYNAMIC_PANEL;
            else if(classInfo.contains("table_cell"))
                return RTABLE;
            else if(classInfo.contains("line"))
                return RLINE;
            else if(classInfo.contains("treeroot"))
                return RTREE;
            else{
                //img标签和box_1需要结合parentElement
                if(parentElement.valid()){
                    QString pClassInfo = parentElement.clazz();

                    bool parentContainBox = (pClassInfo.contains("box_1") ||  pClassInfo.contains("box_2") || pClassInfo.contains("box_3"));
                    //父节点为box_1，第一个子节点class属性为img
                    if(classInfo.contains("img") || (parentContainBox && classInfo.contains("img")))
                        return RIMAGE;
                    else{
                        if(parentContainBox){
                            return RCONTAINER;
                        }else{
                            //父节点为box_1，第一个子节点class属性为空，检测第二个子节点class属性
                            GumboNodeWrapper secondElement = parentElement.secondChild();
                            if(secondElement.valid()){
                                return getNodeType(secondElement,element);
                            }
                        }
                    }
                }else{
                    return getNodeType(element.firstChild(),element);
                }

            }
        }
    }
    return RINVALID;
}

void GumboParseMethod::printBody(DomNode *node)
{
    qDebug()<<node->m_id<<node->m_type;

    if(node->m_childs.size() > 0){
        foreach(DomNode * child,node->m_childs){
            printBody(child);
        }
    }
}

/*!
 * @brief 建立父子节点之间关系
 * @param[in] parentNode 父节点
 * @param[in] childNode 子节点
 */
void GumboParseMethod::establishRelation(DomNode *parentNode, DomNode *childNode)
{
    if(parentNode && childNode){
        parentNode->m_childs.append(childNode);
        childNode->m_parent = parentNode;
    }
}

/*!
 * @brief 解析元素中包含的数据信息
 * @param[in] element 待解析的确定的元素节点
 * @param[in] type 元素类型
 * @param[in] node 元素节点
 */
void GumboParseMethod::parseNodeData(GumboNodeWrapper &element, NodeType type, DomNode *node)
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
        case RLINE:parseLineNodeData(element,node);break;
        case RTREE:parseTreeNodeData(element,node);break;
        default:break;
    }
}

void GumboParseMethod::parseButtonNodeData(GumboNodeWrapper &element, DomNode *node)
{
    BaseData * data = new BaseData();
    data->m_text = element.secondChild().firstChild().firstChild().firstChild().text();
    data->m_toolTip = element.attribute(G_NodeHtml.TITLE);
    data->m_bChecked = element.firstChild().clazz().contains("selected");
    data->m_bDisabled = element.clazz().contains(G_NodeHtml.DISABLED);
    node->m_data = data;
}

void GumboParseMethod::parseRadioButtonNodeData(GumboNodeWrapper &element, DomNode *node)
{
    BaseData * data = new BaseData();
    data->m_text = element.firstChild().firstChild().firstChild().firstChild().firstChild().text();
    data->m_bChecked = element.secondChild().hasAttribute(G_NodeHtml.CHECKED);
    data->m_bDisabled = element.secondChild().hasAttribute(G_NodeHtml.DISABLED);
    data->m_toolTip = element.attribute(G_NodeHtml.TITLE);
    node->m_data = data;
}

/**
 * @brief 解析rdynamicPanel
 * @param element
 * @param node
 */
void GumboParseMethod::parserdynamicPanelNodeData(GumboNodeWrapper &element, DomNode *node)
{
    GumboNodeWrapperList childs = element.children();
    for(int i = 0; i < childs.size(); i++){
        GumboNodeWrapper child = childs.at(i);
        if(child.clazz() == "panel_state"){
            DomNode * nodeChild = new DomNode(RCONTAINER);
            nodeChild->m_id = child.id();
            nodeChild->m_class = child.clazz();
            nodeChild->m_style = child.style();

            establishRelation(node,nodeChild);
            parseDiv(child.firstChild(),nodeChild);
        }
    }
}

void GumboParseMethod::parseTextFieldNodeData(GumboNodeWrapper &element, DomNode *node)
{
    TextFieldData * data = new TextFieldData();
    data->m_bReadOnly = element.firstChild().hasAttribute(G_NodeHtml.READONLY);
    data->m_bDisabled = element.firstChild().hasAttribute(G_NodeHtml.DISABLED);
    data->m_text = element.firstChild().attribute(G_NodeHtml.VALUE);
    data->m_type = element.firstChild().attribute(G_NodeHtml.TYPE);
    if(element.firstChild().hasAttribute(G_NodeHtml.MAX_LEN))
        data->m_maxLength = element.firstChild().attribute(G_NodeHtml.MAX_LEN).toInt();
    data->m_toolTip = element.attribute(G_NodeHtml.TITLE);
    node->m_data = data;
}

void GumboParseMethod::parseImageNodeData(GumboNodeWrapper &element, DomNode *node)
{
    ImageData * data = new ImageData();
    data->m_src = element.firstChild().attribute(G_NodeHtml.SRC);
    data->m_toolTip = element.attribute(G_NodeHtml.TITLE);
    data->m_bDisabled = element.clazz().contains(G_NodeHtml.DISABLED);
    data->m_text = element.secondChild().firstChild().firstChild().firstChild().text();
    node->m_data = data;
}

void GumboParseMethod::parseTableNodeData(GumboNodeWrapper &element, DomNode *node)
{
    GumboNodeWrapperList childs = element.children();
    TableData * data = new TableData();
    node->m_data = data;
    for(int i = 0; i < childs.size(); i++){
        GumboNodeWrapper child = childs.at(i);
        parseTableCellNodeData(child,node);

        data->m_items.append(child.secondChild().firstChild().firstChild().firstChild().text());
    }
}

void GumboParseMethod::parseTableCellNodeData(GumboNodeWrapper &element, DomNode *parentNode)
{
    DomNode * node = new DomNode(RTABLE_CELL);
    node->m_id = element.attribute(G_NodeHtml.ID);
    node->m_class = element.attribute(G_NodeHtml.CLASS);
    node->m_style = element.attribute(G_NodeHtml.STYLE);
    establishRelation(parentNode,node);
}

void GumboParseMethod::parseLineNodeData(GumboNodeWrapper &element, DomNode *node)
{
    BaseData * data = new BaseData();
    data->m_toolTip = element.attribute(G_NodeHtml.TITLE);
    data->m_bDisabled = element.clazz().contains(G_NodeHtml.DISABLED);
    node->m_data = data;
}

/**
 * @brief 解析tree控件
 * @param element
 * @param node
 */
void GumboParseMethod::parseTreeNodeData(GumboNodeWrapper &element, DomNode *node)
{
    GumboNodeWrapperList childs = element.children();
    TreeData * data = new TreeData();

    for(int i = 0; i < childs.size(); i++){
        GumboNodeWrapper child = childs.at(i);
        if(!child.clazz().contains("treenode"))
        {
            parseTreeNodeData(child,node);
        }
        else
        {
            NodeType ttype;
            if(node->m_class.contains("treeroot"))
                ttype = RTREE;
            else
                ttype = RTREE_CHILDNODE;
            DomNode * childNode = new DomNode(ttype);
            childNode->m_id = child.id();
            childNode->m_class = child.clazz();

            parseTreeDataNodeData(child,childNode);
            childNode->m_treeText = m_treeDatas;
            establishRelation(node,childNode);
            parseTreeNodeData(child,childNode);
        }
    }
    node->m_data = data;
}

void GumboParseMethod::parseTreeDataNodeData(GumboNodeWrapper &element, DomNode *node)
{
    GumboNodeWrapperList childs = element.children();

    for(int i = 0; i < childs.size(); i++){
        GumboNodeWrapper child = childs.at(i);
        GumboNodeWrapperList secondChilds = child.children();
        for(int j = 0; j < secondChilds.size(); j++){
            GumboNodeWrapper secondChild = secondChilds.at(j);
            QString columsData = secondChild.firstChild().firstChild().firstChild().text();
            if(!columsData.isEmpty()){
                m_treeDatas = columsData;
            }
        }
    }
}

void GumboParseMethod::parseGroupNodeData(GumboNodeWrapper &element, DomNode *node)
{
    GroupData * data = new GroupData();
    data->m_left = element.attribute("data-left").toInt();
    data->m_top = element.attribute("data-top").toInt();
    data->m_width = element.attribute("data-width").toInt();
    data->m_height = element.attribute("data-height").toInt();
    node->m_data = data;
}

void GumboParseMethod::parseLabelNodeData(GumboNodeWrapper &element, DomNode *node)
{
    BaseData * data = new BaseData();
    data->m_text = element.secondChild().firstChild().firstChild().firstChild().text();
    data->m_bDisabled = element.clazz().contains(G_NodeHtml.DISABLED);
    data->m_bChecked = element.firstChild().clazz().contains("selected");
    data->m_toolTip = element.attribute(G_NodeHtml.TITLE);
    node->m_data = data;
}


} //namespace Html
