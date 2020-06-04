#include "gumboparsemethod.h"

#include <QDebug>

namespace Html{

NodeHtml G_NodeHtml;

GumboParseMethod::GumboParseMethod():m_gumboParser(nullptr)
{
    m_custControl.insert(QStringLiteral("单选按钮"),RRADIO_BUTTON);
    m_custControl.insert(QStringLiteral("复选框"),RCHECKBOX);
//    m_custControl.insert(RTREE,QStringLiteral("折叠信息"));
    m_custControl.insert(QStringLiteral("下拉列表框"),RDROPLIST);
    m_custControl.insert(QStringLiteral("加减输入框"),RSPINBOX);
    m_custControl.insert(QStringLiteral("滚动条"),RSCROLLBAR);
    m_custControl.insert(QStringLiteral("进度条"),RPROGRESSBAR);
    m_custControl.insert(QStringLiteral("tab页"),RTABWIDGET);
    m_custControl.insert(QStringLiteral("菜单选项（无标识触发）"),RUNMENUBUTTON);

    m_custControl.insert(QStringLiteral("输入框-默认"),R_CUSTOM_TEXT_FIELD);
    m_custControl.insert(QStringLiteral("输入框-禁用"),R_CUSTOM_TEXT_FIELD);
    m_custControl.insert(QStringLiteral("输入框-警告"),R_CUSTOM_TEXT_FIELD);
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

        if(ttype != RINVALID){
            DomNode * node = new DomNode(ttype);
            node->m_id = childEle.id();
            node->m_class = childEle.clazz();
            node->m_style = childEle.style();

            parseNodeData(childEle,ttype,node);
            establishRelation(parentNode,node);

            QString dataLabel = divNode.attribute(G_NodeHtml.DATA_LABEL);
            QString childDataLabel = childEle.attribute(G_NodeHtml.DATA_LABEL);

            if(i == 0 && (ttype == RLABEL || ttype == RIMAGE)
                    && (divNode.clazz().contains("ax_default_hidden")||dataLabel.contains(QStringLiteral("弹窗")))){
                parentNode = node;
            }

            if(ttype == RGROUP){
                parseDiv(childEle,node);
            }
            //自制控件子级有组合控件继续解析
            else if(ttype == RLABEL||ttype == RBOX||ttype == RBUTTON){
                if(!(childEle.firstChild().clazz().contains("text")
                     ||childEle.firstChild().clazz().contains("img")
                     ||childEle.firstChild().clazz().isEmpty()
                     ||childDataLabel.contains(QStringLiteral("关闭按钮"))))
                {
                    parseDiv(childEle,parentNode);
                }
            }
        }else{
            parseDiv(childEle,parentNode);
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
        if(element.firstChild().tagName() == "iframe"){
            return RINLINE_FRAME;
        }

        QString classInfo = element.clazz();
        if(classInfo == m_classInfo && classInfo == "ax_default"){
            return RGROUP;
        }

        m_classInfo = classInfo;

        //解析定制化控件(部分官方控件会包含data-label属性，需要过滤)
        if(element.hasAttribute(G_NodeHtml.DATA_LABEL)){
            QString dataLabel = element.attribute(G_NodeHtml.DATA_LABEL);
            QString selectTionGroup = element.attribute(G_NodeHtml.SELECTIONGROUP);
            if(!dataLabel.contains(QStringLiteral("时间栏")))
                m_classInfo.clear();

            if(selectTionGroup.contains(QStringLiteral("按钮"))||selectTionGroup.contains(QStringLiteral("页码"))
                    ||selectTionGroup.contains(QStringLiteral("联机脱机")))
            {
                if(classInfo.contains("box"))
                {
                    return RBUTTON;
                }
            }
            if(dataLabel.contains(QStringLiteral("关闭按钮")))
                return RBUTTON;
            else if(dataLabel.contains(QStringLiteral("弹窗")))
                return RLABEL;
            if(dataLabel.contains(QStringLiteral("按钮"))||dataLabel.contains(QStringLiteral("文件夹"))
                    ||dataLabel.contains(QStringLiteral("复位"))||dataLabel.contains(QStringLiteral("快进"))
                    ||dataLabel.contains(QStringLiteral("快退"))||dataLabel.contains(QStringLiteral("播放"))
                    ||dataLabel.contains(QStringLiteral("暂停"))||dataLabel.contains(QStringLiteral("台位切换按钮"))
                    ||dataLabel.contains(QStringLiteral("显控设置"))||dataLabel.contains(QStringLiteral("左"))
                    ||dataLabel.contains(QStringLiteral("右"))||dataLabel.contains(QStringLiteral("选项"))
                    ||dataLabel.contains(QStringLiteral("下拉三角"))
                    && !dataLabel.contains(QStringLiteral("单选按钮"))){
                if(classInfo.contains("box_1") || classInfo.contains("box_2") || classInfo.contains("box_3")
                    || classInfo.contains("label")||classInfo.contains(QStringLiteral("图片"))||classInfo.contains("ellipse")
                    ||classInfo.contains("image")){
                    return RBUTTON;
                }
            }

            auto iter = m_custControl.begin();

            while(iter != m_custControl.end()){
                if(dataLabel.contains(iter.key())){
                    return iter.value();
                }
                else if((classInfo.contains("box_1")||classInfo.contains("box_2")
                        ||classInfo.contains("box_3")||classInfo.contains("label"))&&!dataLabel.contains(QStringLiteral("菜单选项（无标识触发）")))
                {
                    return RLABEL;
                }

                ++iter;
            }
        }

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
            if(element.hasAttribute(G_NodeHtml.DATA_LEFT) && element.hasAttribute(G_NodeHtml.DATA_LEFT) && element.hasAttribute(G_NodeHtml.DATA_LEFT))
                return RGROUP;

            if(element.hasAttribute(G_NodeHtml.DATA_LABEL)&&element.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("信息区")))
                return RGROUP;
            if(classInfo.contains("radio_button"))
                return RRADIO_BUTTON;
            else if(classInfo.contains("text_field"))
                return RTEXT_FIELD;
            else if((classInfo.contains("label") || classInfo.contains("text")||
                     classInfo.contains("ellipse")||classInfo.contains("paragraph")||classInfo.contains(QStringLiteral("文本段落")))
                    && (!classInfo.contains("text_area")))
            {
                return RLABEL;
            }
            else if(classInfo.contains(QStringLiteral("级标题")))
                return RLABEL;
            else if(classInfo.contains("button") || classInfo.contains("primary_button"))
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
                    {
                        return RIMAGE;

                    }
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
        case RUNMENUBUTTON:
        case RBUTTON:parseButtonNodeData(element,node);break;
        case RCHECKBOX:
        case RRADIO_BUTTON:parseRadioButtonNodeData(element,node);break;
        case RDYNAMIC_PANEL:parserDynamicPanelNodeData(element,node);break;
        case RTEXT_FIELD:parseTextFieldNodeData(element,node);break;
        case RIMAGE:parseImageNodeData(element,node);break;
        case RTABLE:parseTableNodeData(element,node);break;
        case RGROUP:parseGroupNodeData(element,node);break;
        case RLABEL:parseLabelNodeData(element,node);break;
        case RLIST_BOX:
        case RDROPLIST:parseListNodeData(element,node);break;
        case RTEXT_AREA:parseTextAreaNodeData(element,node);break;
        case RINLINE_FRAME:parseInlineFrameNodeData(element,node);break;
        case RBOX:parseBoxNodeData(element,node);break;
        case RLINE:parseLineNodeData(element,node);break;
        case RTREE:parseTreeNodeData(element,node);break;
        case RSPINBOX:parseSpinBoxNodeData(element,node);break;
        case RSCROLLBAR:parseScrollBarNodeData(element,node);break;
        case RPROGRESSBAR:parseProgreesBarNodeData(element,node);break;
        case RTABWIDGET:parseTabWidgetNodeData(element,node);break;

        case R_CUSTOM_TEXT_FIELD:parseCustomInputEdit(element,node);break;
        default:break;
    }
}

void GumboParseMethod::parseTabWidgetNodeData(GumboNodeWrapper &element,DomNode *node)
{
    TabWidgetData *data = new TabWidgetData();
    GumboNodeWrapperList childs = element.children();

    std::for_each(childs.begin(),childs.end(),[&](GumboNodeWrapper child){
        if(child.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("选项卡")))
        {
            GumboNodeWrapperList childPages = child.children();
            std::for_each(childPages.begin(),childPages.end(),[&](GumboNodeWrapper childPage){
                if(childPage.attribute("selectiongroup").contains(QStringLiteral("选项组")))
                {
                    if(childPage.clazz().contains("selected"))
                    {
                        data->m_selectedImage = childPage.firstChild().attribute(G_NodeHtml.SRC);
                    }
                    else
                        data->m_srcImage = childPage.firstChild().attribute(G_NodeHtml.SRC);

                    data->m_tabBarId = childPage.id();
                    QString dataLabel = childPage.attribute(G_NodeHtml.DATA_LABEL);
                    BaseData *childData = new BaseData();
                    childData->m_text = childPage.secondChild().firstChild().firstChild().firstChild().text();
                    DomNode * nodeChild = new DomNode(RTABWIDGET_PAGE);
                    nodeChild->m_id = childPage.id();
                    nodeChild->m_class = childPage.clazz();
                    nodeChild->m_style = childPage.style();
                    nodeChild->m_data = childData;
                    establishRelation(node,nodeChild);
                    std::for_each(childs.begin(),childs.end(),[&](GumboNodeWrapper child){
                        if(child.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("内容")))
                        {
                            GumboNodeWrapperList pageChilds = child.children();
                            std::for_each(pageChilds.begin(),pageChilds.end(),[&](GumboNodeWrapper pageChild){
                                if(pageChild.attribute(G_NodeHtml.DATA_LABEL).contains(dataLabel))
                                {
                                    data->m_srcImageId = pageChild.firstChild().firstChild().id();
                                    parseDiv(pageChild.firstChild(),nodeChild);
                                }
                            });
                        }
                    });
                }

            });
        }
    });
    node->m_data = data;
}

/*!
 * @brief 解析自定义输入框
 * @details 1.对应元件.rp中信息输入组件，输入框-默认、输入框-禁用、输入框-警告三种
 *          2.解析时，子元素中包含Rectangele和Text field两个组件，其中Rectangle负责维护样式，Text field只负责输入(不管理样式)
 *            解析后使用QLineEdit控件，需要将Rectangle控件的样式拷贝至QLineEdit上。
 */
void GumboParseMethod::parseCustomInputEdit(GumboNodeWrapper &element, DomNode *node)
{
    GumboNodeWrapperList childs = element.children();

    GumboNodeWrapper boxNode;
    GumboNodeWrapper textFieldNode;

    for(int i = 0; i < childs.size(); i++)
    {
        QString childClazz = childs.at(i).clazz();
        if(childClazz.contains("text_field")){
            textFieldNode = childs.at(i);
        }else if(childClazz.contains("box_")){
            boxNode = childs.at(i);
        }
    }

    if(textFieldNode.valid() && boxNode.valid())
    {
        parseTextFieldNodeData(textFieldNode,node);

        //因样式信息都在Rectangle中
        node->m_id = textFieldNode.id();

        if(node->m_data){
            QStringList referenceIds;
            referenceIds << boxNode.id();      //"uXX_div"，css中样式都是对内层div设置的
            node->m_data->m_referenceIds = referenceIds;
        }
    }
}

void GumboParseMethod::parseProgreesBarNodeData(GumboNodeWrapper &element,DomNode *node)
{
    ProgressBarData *data = new ProgressBarData();
    GumboNodeWrapperList childs = element.firstChild().firstChild().children();
    for(int i = 0; i < childs.size(); i++)
    {
        GumboNodeWrapper child = childs.at(i);
        if(child.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("进度槽")))
        {
           data->m_ProgressSlotId = child.id();
        }
        if(child.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("进度条")))
        {
           data->m_progressBarId = child.id();
        }
    }

    node->m_data = data;
}

void GumboParseMethod::parseScrollBarNodeData(GumboNodeWrapper &element,DomNode *node)
{
    ScrollBarData *data = new ScrollBarData();
    GumboNodeWrapperList childs = element.children();
    for(int i = 0; i < childs.size(); i++)
    {
        GumboNodeWrapper child = childs.at(i);
        if(child.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("滚动槽"))){
            node->m_id = child.id();
        }
        if(child.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("滚动条")))
        {
            data->m_scrollBarId = child.firstChild().firstChild().firstChild().id();
        }
        if(child.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("下翻")))
        {
            data->m_addLine = child.firstChild().id();
            data->m_downArrow = child.secondChild().id();
        }
        if(child.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("上翻")))
        {
            data->m_subLine = child.firstChild().id();
            data->m_upArrow = child.secondChild().id();
        }
    }
    node->m_data = data;
}

void GumboParseMethod::parseSpinBoxNodeData(GumboNodeWrapper &element,DomNode *node)
{
    GroupData * data = new GroupData();
    data->m_left = element.attribute(G_NodeHtml.DATA_LEFT).toInt();
    data->m_top = element.attribute(G_NodeHtml.DATA_TOP).toInt();
    data->m_width = element.attribute(G_NodeHtml.DATA_WIDTH).toInt();
    data->m_height = element.attribute(G_NodeHtml.DATA_HEIGHT).toInt();
    node->m_data = data;

    GumboNodeWrapperList childs = element.children();
    for(int i = 0; i < childs.size(); i++)
    {
        GumboNodeWrapper child = childs.at(i);
        if(child.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("输入框")))
        {
            data->m_spinBoxId = child.firstChild().id();
            data->m_sinBoxTextId = child.secondChild().id();
        }
        else if(child.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("加减箭头")))
        {

        }
    }
}

void GumboParseMethod::parseListNodeData(GumboNodeWrapper &element,DomNode *node)
{
    ListData *data = new ListData();

    GumboNodeWrapperList childs;
    //Axure8
    if(element.firstChild().id().contains("_input"))
    {

        childs = element.firstChild().children();
        data->m_bDisabled = element.firstChild().hasAttribute(G_NodeHtml.DISABLED);
    }
    //Axure9.0.0
    else if(element.secondChild().id().contains("_input"))
    {

        childs  = element.secondChild().children();
        data->m_bDisabled = element.secondChild().hasAttribute(G_NodeHtml.DISABLED);
    }
    if(!childs.isEmpty())
    {
        for(int i = 0;i < childs.size();i++)
        {
            GumboNodeWrapper child = childs.at(i);
            if(child.hasAttribute("selected")){
                data->m_selectedValue = child.attribute(G_NodeHtml.VALUE);
            }
            data->m_itemList.append(child.attribute("value"));
        }
    }
    else {
        childs = element.children();
        for(int i = 0; i < childs.size(); i++)
        {
            GumboNodeWrapper child = childs.at(i);
            if(child.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("下拉菜单")))
            {
                 GumboNodeWrapperList childs = child.firstChild().firstChild().children();
                 for(int j = 0; j < childs.size(); j++)
                 {
                     GumboNodeWrapper child = childs.at(j);
                    if(child.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("下拉菜单背景")))
                    {
                        data->m_srcImage = child.firstChild().attribute(G_NodeHtml.SRC);
                    }
                    else if(child.attribute("selectiongroup").contains(QStringLiteral("下拉菜单选项组")))
                    {
                        data->m_backGroundId = child.id();
                        data->m_itemList.append(child.secondChild().firstChild().firstChild().firstChild().text());
                    }
                 }
            }
            if(child.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("下拉框部分")))
            {
                GumboNodeWrapperList childs = child.children();
                for(int j = 0; j < childs.size(); j++)
                {
                    GumboNodeWrapper child = childs.at(j);
                   if(child.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("下拉箭头-默认")))
                   {
                      data->m_arrowImageSrc = child.firstChild().attribute(G_NodeHtml.SRC);
                      QStringList imageList = data->m_arrowImageSrc.split(".");
                      if(imageList.size() == 2)
                      {
                          data->m_arrowImageOn = imageList.at(0) + "_selected." + imageList.at(1);
                      }
                   }
                   else if(child.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("下拉框背景")))
                   {
                       data->m_widgetSizeId = child.id();
                   }
                   else if(child.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("下拉框文本")))
                   {
                       data->m_textId = child.id();
                       data->m_selectedValue = child.secondChild().firstChild().firstChild().firstChild().text();
                   }
                }
            }
        }
    }
    if(element.clazz() == "ax_default"){
        node->m_id = element.firstChild().id();
    }
    data->m_toolTip = element.attribute(G_NodeHtml.TITLE);
    node->m_data = data;

}

void GumboParseMethod::parseTextAreaNodeData(GumboNodeWrapper &element,DomNode *node)
{
    BaseData *data = new BaseData();

    //Axure8
    if(element.firstChild().id().contains("_input"))
    {
        data->m_text = element.firstChild().firstChild().text();
        data->m_bReadOnly = element.firstChild().hasAttribute(G_NodeHtml.READONLY);
        data->m_bDisabled = element.firstChild().hasAttribute(G_NodeHtml.DISABLED);
    }
    //Axure9.0.0
    else if(element.secondChild().id().contains("_input"))
    {
        data->m_text = element.secondChild().firstChild().text();
        data->m_bReadOnly = element.secondChild().hasAttribute(G_NodeHtml.READONLY);
        data->m_bDisabled = element.secondChild().hasAttribute(G_NodeHtml.DISABLED);
    }

    data->m_toolTip = element.attribute(G_NodeHtml.TITLE);
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
    data->m_srcImage = element.firstChild().attribute(G_NodeHtml.SRC);
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
    data->m_srcImage = element.firstChild().attribute(G_NodeHtml.SRC);

    if(element.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("关闭按钮")))
    {
         data->m_srcImage = element.firstChild().firstChild().attribute(G_NodeHtml.SRC);
         node->m_id = element.firstChild().id();
    }

    node->m_data = data;
}

void GumboParseMethod::parseRadioButtonNodeData(GumboNodeWrapper &element, DomNode *node)
{
    BaseData * data = new BaseData();
    //Axure8
    if(element.firstChild().firstChild().clazz().contains("text"))
        data->m_text = element.firstChild().firstChild().firstChild().firstChild().firstChild().text();
    //Axure9.0.0
    else if(element.firstChild().secondChild().clazz().contains("text"))
        data->m_text = element.firstChild().secondChild().firstChild().firstChild().firstChild().text();
    //定制化单选按钮
    else if(element.firstChild().firstChild().firstChild().secondChild().secondChild().clazz().contains("text")){
        data->m_text = element.firstChild().firstChild().firstChild().secondChild().secondChild().firstChild().firstChild().firstChild().text();
        data->m_textId = element.firstChild().firstChild().firstChild().secondChild().id();
        data->m_specialTextId = data->m_textId;

    }
    else if(element.firstChild().firstChild().firstChild().firstChild().secondChild().clazz().contains("text")){
        data->m_text = element.firstChild().firstChild().firstChild().firstChild().secondChild().firstChild().firstChild().firstChild().text();
        data->m_textId = element.firstChild().firstChild().firstChild().firstChild().id();
    }

    GumboNodeWrapperList childs = element.children();
    for(int i = 0; i < childs.size(); i++)
    {
        GumboNodeWrapper child = childs.at(i);
        if(child.attribute(G_NodeHtml.DATA_LABEL) == QStringLiteral("未选中"))
        {
           data->m_unCheckedImage = child.firstChild().firstChild().firstChild().firstChild().attribute(G_NodeHtml.SRC);
        }
        else if(child.attribute(G_NodeHtml.DATA_LABEL) == QStringLiteral("选中"))
        {
            data->m_checkedImage = child.firstChild().firstChild().firstChild().firstChild().attribute(G_NodeHtml.SRC);
        }

    }
    data->m_widths = element.firstChild().firstChild().firstChild().attribute(G_NodeHtml.DATA_WIDTH).toInt();
    data->m_heights = element.firstChild().firstChild().firstChild().attribute(G_NodeHtml.DATA_HEIGHT).toInt();

    data->m_bChecked = element.secondChild().hasAttribute(G_NodeHtml.CHECKED);
    data->m_bDisabled = element.secondChild().hasAttribute(G_NodeHtml.DISABLED);
    data->m_toolTip = element.attribute(G_NodeHtml.TITLE);
    node->m_data = data;
}

void GumboParseMethod::parserDynamicPanelNodeData(GumboNodeWrapper &element, DomNode *node)
{
    PanelData * data = new PanelData();
    data->m_toolTip = element.attribute(G_NodeHtml.TITLE);
    node->m_data = data;

    //获取定制控件文字描述
    if(element.hasAttribute(G_NodeHtml.DATA_LABEL)){
        data->m_panelDataLab = element.attribute(G_NodeHtml.DATA_LABEL);
    }

    GumboNodeWrapperList childs = element.children();
    for(int i = 0; i < childs.size(); i++)
    {
        GumboNodeWrapper child = childs.at(i);
        if(child.clazz() == "panel_state"){
            data->m_srcImageId.clear();
            DomNode * nodeChild = new DomNode(RDYNAMIC_PANEL_PAGE);
            nodeChild->m_id = child.id();
            nodeChild->m_class = child.clazz();
            nodeChild->m_style = child.style();   

            GumboNodeWrapper panel = child.firstChild();
            GumboNodeWrapperList panelChilds = panel.children();
            for(int j = 0; j < panelChilds.size(); j++)
            {
                GumboNodeWrapper panelChild = panelChilds.at(j);
                QStringList clazzList = panelChild.clazz().split(QRegExp("\\s+"));
                //class属性包含多个
                if(panelChild.clazz().contains("ax_default") && clazzList.size() > 1){
                    data->m_srcImageId = panelChild.id();
                    break;
                }

                //获取动态面板的子级动态面板id
                if(panelChild.clazz().contains("ax_default") && clazzList.size() == 1)
                {
                   if(panelChild.firstChild().clazz().contains("panel_state"))
                   {
                       data->m_sonPanelStateId = panelChild.id();
                   }
                }
            }

            //处理自制动态面板几种情况
            GumboNodeWrapper imageChild  = panel.firstChild().firstChild();
            GumboNodeWrapper textChild  = panel.firstChild().secondChild();
            if(data->m_srcImageId.isEmpty())
            {
                //获取自制动态面板下组合的第一个控件id
                if(!imageChild.clazz().isEmpty())
                {
                    data->m_srcImageId = imageChild.id();

                }
                //获取自制动态面板下组合的第二个控件id（一般只能处理两个控件组合的情况）
                if(!textChild.clazz().isEmpty())
                {
                    data->m_panelTextId = textChild.id();

                }

                //处理自制动态面板下含有二级子菜单，获取一级子菜单和二级子菜单背景id（右垂直菜单）
                GumboNodeWrapperList firstFloorGroupChilds = panel.firstChild().children();

                std::for_each(firstFloorGroupChilds.begin(),firstFloorGroupChilds.end(),
                              [&](GumboNodeWrapper firstFloorGroupChild){
                    if(firstFloorGroupChild.hasAttribute(G_NodeHtml.DATA_LABEL))
                    {
                        QString firstDataLabelText = firstFloorGroupChild.attribute(G_NodeHtml.DATA_LABEL);

                        if(firstDataLabelText.contains(QStringLiteral("背景")))
                        {
                            if(firstFloorGroupChild.firstChild().clazz().contains("img"))
                                data->m_srcImageId = firstFloorGroupChild.firstChild().id();
                        }
                        if(firstDataLabelText.contains(QStringLiteral("二级菜单")))
                        {
                            GumboNodeWrapperList secondFloorGroupChilds = firstFloorGroupChild.firstChild().children();
                            std::for_each(secondFloorGroupChilds.begin(),secondFloorGroupChilds.end(),
                                          [&](GumboNodeWrapper secondFloorGroupChild){
                                if(secondFloorGroupChild.hasAttribute(G_NodeHtml.DATA_LABEL))
                                {
                                    QString secondDataLabelText = secondFloorGroupChild.attribute(G_NodeHtml.DATA_LABEL);
                                    if(secondDataLabelText.contains(QStringLiteral("背景")))
                                    {
                                        if(secondFloorGroupChild.firstChild().clazz().contains("img"))
                                            data->m_secondSrcImageId = secondFloorGroupChild.firstChild().id();
                                    }
                                }
                            });
                        }

                    }
                });
            }

            if(imageChild.firstChild().clazz().contains("img"))
                data->m_srcImage = imageChild.firstChild().attribute(G_NodeHtml.SRC);

            establishRelation(node,nodeChild);
            parseDiv(child.firstChild(),nodeChild);
        }
    }
}

void GumboParseMethod::parseTextFieldNodeData(GumboNodeWrapper &element, DomNode *node)
{
    TextFieldData * data = new TextFieldData();
    //Axure8
    if(element.firstChild().id().contains("_input"))
    {
        data->m_text = element.firstChild().attribute(G_NodeHtml.VALUE);
        data->m_bReadOnly = element.firstChild().hasAttribute(G_NodeHtml.READONLY);
        data->m_bDisabled = element.firstChild().hasAttribute(G_NodeHtml.DISABLED);
    }
    //Axure9.0.0
    else if(element.secondChild().id().contains("_input"))
    {
        data->m_text = element.secondChild().attribute(G_NodeHtml.VALUE);
        data->m_bReadOnly = element.secondChild().hasAttribute(G_NodeHtml.READONLY);
        data->m_bDisabled = element.secondChild().hasAttribute(G_NodeHtml.DISABLED);
    }

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
    data->m_srcImage = data->m_src;
    if(!data->m_srcImage.isEmpty()){
        data->m_srcImageId = element.firstChild().id();
    }
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

        if(child.clazz().contains("table_cell"))
            data->m_itemId = child.id();
        data->m_items.append(child.secondChild().firstChild().firstChild().firstChild().text());

        GumboNodeWrapperList childImage = child.children();
        if(childImage.size()>0){
            GumboNodeWrapper image = childImage.at(0);
            if(image.clazz().contains("img")){
                data->m_srcImage = image.attribute(G_NodeHtml.SRC);
                if(data->m_srcImageId.isEmpty())
                {
                    data->m_srcImageId = image.id();
                }
            }
        }
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
    data->m_srcImage = element.firstChild().attribute(G_NodeHtml.SRC);
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

    if(element.clazz() == "ax_default"){
        node->m_id = element.firstChild().id();
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
                parentNode->m_childItemId = tmpChild.id();
                data->m_text = tmpChild.secondChild().firstChild().firstChild().firstChild().text();
            }else if(tmpChild.clazz().contains("children")){
                parentNode->m_childTextId = tmpChild.firstChild().id();
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
    data->m_left = element.attribute(G_NodeHtml.DATA_LEFT).toInt();
    data->m_top = element.attribute(G_NodeHtml.DATA_TOP).toInt();
    data->m_width = element.attribute(G_NodeHtml.DATA_WIDTH).toInt();
    data->m_height = element.attribute(G_NodeHtml.DATA_HEIGHT).toInt();
    node->m_data = data;
}

void GumboParseMethod::parseLabelNodeData(GumboNodeWrapper &element, DomNode *node)
{
    BaseData * data = new BaseData();
    data->m_srcImage = element.firstChild().attribute(G_NodeHtml.SRC);
    if(!(data->m_srcImage.isEmpty()))
    {
        data->m_srcImageId = element.firstChild().id();
    }

    GumboNodeWrapperList subChilds = element.secondChild().children();
    QString textStr;

    //获取标签含有多行文本
    if(subChilds.size() > 1)
    {
        std::for_each(subChilds.begin(),subChilds.end(),[&](GumboNodeWrapper subChild){
            if(!subChild.firstChild().firstChild().text().isEmpty())
            {
                textStr += subChild.firstChild().firstChild().text() + QStringLiteral("\n");
            }
        });
        data->m_text = textStr;
    }
    else
        data->m_text = element.secondChild().firstChild().firstChild().firstChild().text();
    data->m_bDisabled = element.clazz().contains(G_NodeHtml.DISABLED);
    data->m_bChecked = element.firstChild().clazz().contains("selected");
    data->m_toolTip = element.attribute(G_NodeHtml.TITLE);
    node->m_data = data;
}


} //namespace Html
