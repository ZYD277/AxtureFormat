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
        GumboNodeWrapperList childNodeListBox = childEle.children();
        GumboNodeWrapper childEleBox = childNodeListBox.at(0);
        if(childEle.clazz() == "ax_default" && childEleBox.clazz().contains("box"))
            parseDiv(childEle,parentNode);
        NodeType ttype = getNodeType(childEle,GumboNodeWrapper());

        if(ttype != RINVALID){
            DomNode * node = new DomNode(ttype);
            node->m_id = childEle.id();
            node->m_class = childEle.clazz();
            node->m_style = childEle.style();

            parseNodeData(childEle,ttype,node);

            establishRelation(parentNode,node);

            if(ttype == RGROUP){
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
        if(element.tagName() == "iframe")
            return RINLINE_FRAME;

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
            else if((classInfo.contains("label") || classInfo.contains("text")) && (!classInfo.contains("text_area")))
                return RLABEL;
            else if(classInfo.contains(QStringLiteral("级标题")))
                return RLABEL;
            else if(classInfo.contains("button") || classInfo.contains("primary_button") || element.attribute("data-label").contains(QStringLiteral("按钮")))
                return RBUTTON;
            else if(classInfo.contains("panel_state"))
                return RDYNAMIC_PANEL;
            else if(classInfo.contains("table_cell"))
                return RTABLE;
            else if(classInfo.contains("checkbox"))
                return RCHECKBOX;
            else if(classInfo.contains("list_box"))
                return RLIST_BOX;
            else if(classInfo.contains("droplist"))
                return RDROPLIST;
            else if(classInfo.contains("text_area"))
                return RTEXT_AREA;
            else if(classInfo.contains("box_1")||classInfo.contains("box_2")||classInfo.contains("box_3"))
                return RBOX;
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
        case RDYNAMIC_PANEL:parserDynamicPanelNodeData(element,node);break;
        case RTEXT_FIELD:parseTextFieldNodeData(element,node);break;
        case RIMAGE:parseImageNodeData(element,node);break;
        case RTABLE:parseTableNodeData(element,node);break;
        case RGROUP:parseGroupNodeData(element,node);break;
        case RLABEL:parseLabelNodeData(element,node);break;
        case RCHECKBOX:parseCheckBoxNodeData(element,node);break;
        case RLIST_BOX:
        case RDROPLIST:parseListNodeData(element,node);break;
        case RTEXT_AREA:parseTextAreaNodeData(element,node);break;
        case RINLINE_FRAME:parseInlineFrameNodeData(element,node);break;
        case RBOX:parseBoxNodeData(element,node);break;
        case RLINE:parseLineNodeData(element,node);break;
        case RTREE:parseTreeNodeData(element,node);break;
        default:break;
    }
}

void GumboParseMethod::parseCheckBoxNodeData(GumboNodeWrapper &element,DomNode *node)
{
    BaseData * data = new BaseData();
    data->m_text = element.firstChild().firstChild().firstChild().firstChild().firstChild().text();
    data->m_bChecked = element.secondChild().hasAttribute(G_NodeHtml.CHECKED);
    data->m_bDisabled = element.secondChild().hasAttribute(G_NodeHtml.DISABLED);
    data->m_toolTip = element.attribute(G_NodeHtml.TITLE);
    node->m_data = data;
}

void GumboParseMethod::parseListNodeData(GumboNodeWrapper &element,DomNode *node)
{
    ListData *data = new ListData();

    GumboNodeWrapperList chids = element.firstChild().children();
    for(int i = 0;i < chids.size();i++)
    {
       GumboNodeWrapper chid = chids.at(i);
       if(chid.hasAttribute("selected")){
           data->m_selectedValue = chid.attribute(G_NodeHtml.VALUE);
       }
       data->m_itemList.append(chid.attribute("value"));
    }
    data->m_bDisabled = element.firstChild().hasAttribute(G_NodeHtml.DISABLED);
    data->m_toolTip = element.attribute(G_NodeHtml.TITLE);
    node->m_data = data;

}

void GumboParseMethod::parseTextAreaNodeData(GumboNodeWrapper &element,DomNode *node)
{
    BaseData *data = new BaseData();
    data->m_text = element.firstChild().firstChild().text();
    data->m_bDisabled = element.firstChild().hasAttribute(G_NodeHtml.DISABLED);
    data->m_toolTip = element.attribute(G_NodeHtml.TITLE);
    data->m_bReadOnly = element.secondChild().hasAttribute(G_NodeHtml.READONLY);
    node->m_data = data;
}

void GumboParseMethod::parseInlineFrameNodeData(GumboNodeWrapper &element,DomNode *node)
{
    BaseData *data = new BaseData();
    data->m_bDisabled = element.secondChild().hasAttribute(G_NodeHtml.DISABLED);
    data->m_toolTip = element.attribute(G_NodeHtml.TITLE);
    //WARNING 无法从html中识别inlineframe引用的资源路径信息，需要web容器加载js后才能获取。
    node->m_data = data;
}

void GumboParseMethod::parseBoxNodeData(GumboNodeWrapper &element,DomNode *node)
{
    BaseData *data = new BaseData();
    data->m_bDisabled = element.clazz().contains(G_NodeHtml.DISABLED);
    data->m_toolTip = element.attribute(G_NodeHtml.TITLE);
    data->m_text = element.secondChild().firstChild().firstChild().firstChild().text();
    node->m_data = data;
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

void GumboParseMethod::parserDynamicPanelNodeData(GumboNodeWrapper &element, DomNode *node)
{
    BaseData * data = new BaseData();
    data->m_toolTip = element.attribute(G_NodeHtml.TITLE);
    node->m_data = data;

    GumboNodeWrapperList childs = element.children();
    for(int i = 0; i < childs.size(); i++){
        GumboNodeWrapper child = childs.at(i);
        if(child.clazz() == "panel_state"){
            DomNode * nodeChild = new DomNode(RDYNAMIC_PANEL_PAGE);
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

/*!
 * @attention 在第一层节点之上虚拟了一个节点，此节点不用于显示，只负责维护节点直接关系
 */
void GumboParseMethod::parseTreeNodeData(GumboNodeWrapper &element, DomNode *node)
{
    TreeItemData * virtualRoot = new TreeItemData();
    virtualRoot->m_text = "root";
    virtualRoot->m_parentItem = nullptr;

    GumboNodeWrapperList topLevelChilds = element.firstChild().children();
    for(int i = 0; i < topLevelChilds.size(); i++){
        parseSubTreeDataNodeData(topLevelChilds.at(i),virtualRoot);
    }

    node->m_data = virtualRoot;
}

void GumboParseMethod::parseSubTreeDataNodeData(GumboNodeWrapper element, TreeItemData *parentNode)
{
    GumboNodeWrapperList childs = element.children();

    TreeItemData * data = new TreeItemData();
    data->m_parentItem = parentNode;

    if(childs.size() > 1){
        for(int i = 0; i < childs.size();i++){
            GumboNodeWrapper tmpChild = childs.at(i);
            if(tmpChild.attribute("selectiongroup").contains("tree_group")){
                data->m_text = tmpChild.secondChild().firstChild().firstChild().firstChild().text();
            }else if(tmpChild.clazz().contains("children")){
                GumboNodeWrapperList subChilds = tmpChild.children();
                for(int j = 0; j < subChilds.size(); j++){
                    parseSubTreeDataNodeData(subChilds.at(j),data);
                }
            }
        }
    }else if(childs.size() == 1){
        data->m_text = element.firstChild().secondChild().firstChild().firstChild().firstChild().text();
    }
    parentNode->m_childItems.push_back(data);
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
