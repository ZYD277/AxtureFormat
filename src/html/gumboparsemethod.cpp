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
    m_custControl.insert(QStringLiteral("滑动条"),RSLIDER);
    m_custControl.insert(QStringLiteral("tab页"),RTABWIDGET);
    m_custControl.insert(QStringLiteral("菜单选项(无标识触发)"),RUNMENUBUTTON);

    m_custControl.insert(QStringLiteral("输入-默认"),R_CUSTOM_TEXT_FIELD);
    m_custControl.insert(QStringLiteral("输入-禁用"),R_CUSTOM_TEXT_FIELD);
    m_custControl.insert(QStringLiteral("输入-警告"),R_CUSTOM_TEXT_FIELD);

    m_custControl.insert(QStringLiteral("外框"),RCONTAINER);          //'系统控制区'中外框
    m_custControl.insert(QStringLiteral("背景"),RCONTAINER);
    m_custControl.insert(QStringLiteral("触发弹窗"),R_CUSTOM_VIRTUAL_CONTAINER);

    m_custControl.insert(QStringLiteral("窗体"),RGROUP);
    m_custControl.insert(QStringLiteral("二次确认弹窗"),RGROUP);
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
                    && (divNode.clazz().contains("ax_default_hidden") || dataLabel.contains(QStringLiteral("弹窗")))){
                parentNode = node;
            }

            if(ttype == RGROUP){
                parseDiv(childEle,node);
            }else if(ttype == R_CUSTOM_VIRTUAL_CONTAINER){
                parseDiv(childEle,parentNode);
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
            else if(dataLabel.contains(QStringLiteral("触发弹窗")) || dataLabel.contains(QStringLiteral("提示框体")))
                return R_CUSTOM_VIRTUAL_CONTAINER;
            else if(dataLabel.contains(QStringLiteral("二次确认弹窗"))){
                return RGROUP;
            }else if(dataLabel.contains(QStringLiteral("弹窗")))
                return RLABEL;

            if(dataLabel.contains(QStringLiteral("按钮"))||dataLabel.contains(QStringLiteral("文件夹"))
                    ||dataLabel.contains(QStringLiteral("复位"))||dataLabel.contains(QStringLiteral("快进"))
                    ||dataLabel.contains(QStringLiteral("快退"))||dataLabel.contains(QStringLiteral("播放"))
                    ||dataLabel.contains(QStringLiteral("暂停"))||dataLabel.contains(QStringLiteral("台位切换按钮"))
                    ||dataLabel.contains(QStringLiteral("显控设置"))||dataLabel.contains(QStringLiteral("左"))
                    ||dataLabel.contains(QStringLiteral("右"))||dataLabel.contains(QStringLiteral("选项"))
                    ||dataLabel.contains(QStringLiteral("下拉三角"))&& !dataLabel.contains(QStringLiteral("单选按钮"))){

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
                else if((classInfo.contains("box_1") || classInfo.contains("box_2")
                        ||classInfo.contains("box_3") || classInfo.contains("label"))
                        && !dataLabel.contains(QStringLiteral("菜单选项(无标识触发)"))
                        && !dataLabel.contains(QStringLiteral("外框"))         //’系统控制区‘第一个子div
                        && !dataLabel.contains(QStringLiteral("背景"))
                        && !dataLabel.contains(QStringLiteral("触发弹窗")))
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
        case RCONTAINER:parseContainerNodeData(element,node);break;
        case RUNMENUBUTTON:
        case RBUTTON:parseButtonNodeData(element,node);break;
        case RCHECKBOX:
        case RRADIO_BUTTON:parseRadioButtonNodeData(element,node);break;
        case RDYNAMIC_PANEL:parseDynamicPanelNodeData(element,node);break;
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
        case RSLIDER:parseProgreesBarNodeData(element,node);break;
        case RTABWIDGET:parseTabWidgetNodeData(element,node);break;

        case R_CUSTOM_TEXT_FIELD:parseCustomInputEdit(element,node);break;
        case R_CUSTOM_VIRTUAL_CONTAINER:parseCustomVirtualContainer(element,node);break;
        default:break;
    }
}

void GumboParseMethod::parseContainerNodeData(GumboNodeWrapper &element, DomNode *node)
{
    //解析定制控件‘系统控制区’中第一个子div的data-label值为‘外框’
    BaseData * data = new BaseData();

    QString dataLabel = element.attribute(G_NodeHtml.DATA_LABEL);
    if(dataLabel.contains(QStringLiteral("外框")) || dataLabel.contains(QStringLiteral("背景"))
            || dataLabel.contains(QStringLiteral("触发弹窗"))){
        data->m_srcImage = element.firstChild().attribute(G_NodeHtml.SRC);
        if(!(data->m_srcImage.isEmpty())){
            data->m_srcImageId = element.firstChild().id();
        }
    }

    node->m_data = data;
}

/*!
 * @brief 解析TAB标签页
 * @attention TAB标签页的结构:
 *            +[div]
 *              +[div data-label="内容"]
 *              +[div data-label="选项卡"]
 *          通过针对性识别两个子div获取类型，需要注意的是“内容”中面板名称的顺序和“选项卡”中面板名称顺序相反。
 *          【因‘内容’面板中每个panel都带了data-label属性，与对应的选项卡名称一致，所以就把每个data-label属性作为tab页面名称】
 */
void GumboParseMethod::parseTabWidgetNodeData(GumboNodeWrapper &element,DomNode *node)
{
    GumboNodeWrapperList childs = element.children();

    GumboNodeWrapper headNode;
    GumboNodeWrapper contentNode;

    for(int i = 0; i < childs.size(); i++){
        QString dataLabel = childs.at(i).attribute(G_NodeHtml.DATA_LABEL);
        if(dataLabel.contains(QStringLiteral("选项卡"))){
            headNode = childs.at(i);
        }else if(dataLabel.contains(QStringLiteral("内容"))){
            contentNode = childs.at(i);
        }
    }

    if(headNode.valid() && contentNode.valid()){
        TabWidgetData * tabData = new TabWidgetData();

        //整个QTabWidget的位置尺寸信息
        tabData->m_left = element.attribute(G_NodeHtml.DATA_LEFT).toInt();
        tabData->m_top = element.attribute(G_NodeHtml.DATA_TOP).toInt();
        tabData->m_width = element.attribute(G_NodeHtml.DATA_WIDTH).toInt();
        tabData->m_height = element.attribute(G_NodeHtml.DATA_HEIGHT).toInt();

        //根据选项卡中包含‘selected’的项，将其作为默认选中的index
        GumboNodeWrapperList headPanels = headNode.children();
        QString selectedTabPageName;
        for(int i = 0; i < headPanels.size(); i++){
            GumboNodeWrapper headPanel = headPanels.at(i);
            if(headPanel.clazz().contains("selected")){
                selectedTabPageName = headPanel.secondChild().firstChild().firstChild().firstChild().text();
                tabData->m_tabSelectedImage = headPanel.firstChild().attribute(G_NodeHtml.SRC);
                tabData->m_tabBarId = headPanel.id();
            }else if(headPanel.hasAttribute("selectiongroup")){
                tabData->m_tabNormalImage = headPanel.firstChild().attribute(G_NodeHtml.SRC);
            }
        }

        //根据内容面板创建页面
        GumboNodeWrapperList contentPanels = contentNode.children();
        for(int i = 0; i < contentPanels.size(); i++){
            GumboNodeWrapper contentPanel = contentPanels.at(i);

            BaseData * pageData = new BaseData();
            pageData->m_text = contentPanel.attribute(G_NodeHtml.DATA_LABEL);

            if(pageData->m_text == selectedTabPageName){
                tabData->m_selectedPageIndex = i;
            }

            DomNode * page = new DomNode(RTABWIDGET_PAGE);
            page->m_id = contentPanel.id();
            page->m_class = contentPanel.clazz();
            page->m_style = contentPanel.style();
            page->m_data = pageData;
            establishRelation(node,page);

            parseDiv(contentPanel.firstChild(),page);
        }

        node->m_data = tabData;
    }
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

/*!
 * @brief 针对弹出菜单、弹窗等，其外层DIV不需要转换，但需要用来保存信号槽信息
 */
void GumboParseMethod::parseCustomVirtualContainer(GumboNodeWrapper &element, DomNode *node)
{
    BaseData * data = new BaseData();

    QString dataLabel = element.attribute(G_NodeHtml.DATA_LABEL);

    if(dataLabel == QStringLiteral("触发弹窗")){
        GumboNodeWrapperList childs = element.children();
        Html::SignalSlotInfo signalInfo;
        for(GumboNodeWrapper node : childs){
            if(node.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("按钮"))){
                signalInfo.m_sender = node.id();
                signalInfo.m_signal = "clicked(bool)";
            }else if(node.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("窗体"))){
                signalInfo.m_receiver = node.id();
                signalInfo.m_slot = "setVisible(bool)";
            }
        }
        data->m_signals.append(signalInfo);
    }else if(dataLabel.contains(QStringLiteral("提示框体"))){       //‘信息提示’组件
        GumboNodeWrapperList childs = element.children();
        Html::SignalSlotInfo signalInfo;
        for(GumboNodeWrapper node : childs){
            if(node.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("详情"))){
                signalInfo.m_receiver = node.id();
                signalInfo.m_slot = "setVisible(bool)";
            }else if(node.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("折叠"))){

                GumboNodeWrapperList grandChilds = node.children();
                for(int i = 0; i < grandChilds.size(); i++){
                    GumboNodeWrapper grandChild = grandChilds.at(i);
                    if(grandChild.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("下拉三角"))){
                        signalInfo.m_sender = grandChild.id();
                        signalInfo.m_signal = "clicked(bool)";
                    }
                }
            }
        }
        data->m_signals.append(signalInfo);
    }

    node->m_data = data;
}

void GumboParseMethod::parseProgreesBarNodeData(GumboNodeWrapper &element,DomNode *node)
{
    SliderData * data = new SliderData();

    data->m_left = element.attribute(G_NodeHtml.DATA_LEFT).toInt();
    data->m_top = element.attribute(G_NodeHtml.DATA_TOP).toInt();
    data->m_width = element.attribute(G_NodeHtml.DATA_WIDTH).toInt();
    data->m_height = element.attribute(G_NodeHtml.DATA_HEIGHT).toInt();

    GumboNodeWrapperList childs = element.children();

    for(int i = 0; i < childs.size(); i++)
    {
        GumboNodeWrapper child = childs.at(i);
        QString dataLabel = child.attribute(G_NodeHtml.DATA_LABEL);

        if(dataLabel.contains(QStringLiteral("进度条"))){
            GumboNodeWrapperList grandChilds = child.firstChild().firstChild().children();
            for(GumboNodeWrapper grandChild : grandChilds){
                  QString grandDataLabel = grandChild.attribute(G_NodeHtml.DATA_LABEL);
                  if(grandDataLabel.contains(QStringLiteral("进度槽"))){   //add-page
                      data->m_progressSlotId = grandChild.id();
                  }else if(grandDataLabel.contains(QStringLiteral("进度条"))){ //sub-page
                      data->m_progressBarId = grandChild.id();
                  }
            }
        }else if(dataLabel.contains(QStringLiteral("进度球"))){
            GumboNodeWrapperList grandChilds = child.children();
            for(GumboNodeWrapper grandChild : grandChilds){
                  QString grandDataLabel = grandChild.attribute(G_NodeHtml.DATA_LABEL);
                  if(grandDataLabel.contains(QStringLiteral("默认"))){   //handle
                      data->m_handleId = grandChild.firstChild().firstChild().id();
                      data->m_srcImage = grandChild.firstChild().firstChild().firstChild().attribute(G_NodeHtml.SRC);
                  }else if(grandDataLabel.contains(QStringLiteral("选中"))){ //handle:hover
                      data->m_pressedHandleId = grandChild.firstChild().firstChild().id();
                      data->m_checkedImage = grandChild.firstChild().firstChild().firstChild().attribute(G_NodeHtml.SRC);
                  }
            }
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
    SpinboxData * data = new SpinboxData();
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
            GumboNodeWrapperList grandChilds = child.children();
            for(int j = 0; j < grandChilds.size(); j++){
                GumboNodeWrapper gchild = grandChilds.at(j);
                if(gchild.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("默认"))){
                    GumboNodeWrapperList arrows = gchild.firstChild().children();
                    for(GumboNodeWrapper wrapper : arrows){
                        if(wrapper.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("上三角"))){
                            data->m_upArrowImage = wrapper.firstChild().attribute(G_NodeHtml.SRC);
                        }else{
                            data->m_downArrowImage = wrapper.firstChild().attribute(G_NodeHtml.SRC);
                        }
                    }
                }else if(gchild.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("得到焦点"))){
                    GumboNodeWrapperList arrows = gchild.firstChild().children();
                    for(GumboNodeWrapper wrapper : arrows){
                        if(wrapper.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("上三角"))){
                            data->m_upArrowMouseOverImage = wrapper.firstChild().attribute(G_NodeHtml.SRC);
                        }else{
                            data->m_downArrowMouseOverImage = wrapper.firstChild().attribute(G_NodeHtml.SRC);
                        }
                    }
                }
            }
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
                      if(imageList.size() == 2){
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
    ButtonData * data = new ButtonData();

    data->m_text = element.secondChild().firstChild().firstChild().firstChild().text();
    data->m_toolTip = element.attribute(G_NodeHtml.TITLE);
    data->m_bChecked = element.clazz().contains(G_NodeHtml.SELECTED);
    data->m_bDisabled = element.clazz().contains(G_NodeHtml.DISABLED);
    data->m_srcImage = element.firstChild().attribute(G_NodeHtml.SRC);

    QString dataLabel = element.attribute(G_NodeHtml.DATA_LABEL);
    if(dataLabel.contains(QStringLiteral("标签按钮"))){
        data->m_bChecked = true;
        data->m_dataLabel = dataLabel;

        SignalSlotInfo sinfo;
        sinfo.m_sender = element.id();
        sinfo.m_signal = "clicked(bool)";
        sinfo.m_receiver = element.id();
        sinfo.m_slot = "setDisabled(bool)";
        data->m_signals.append(sinfo);
    }else if(dataLabel.contains(QStringLiteral("关闭按钮"))){
        GumboNodeWrapperList chils = element.children();
        for(int j = 0; j < chils.size(); j++){
            GumboNodeWrapper child = chils.at(j);
            if(chils.at(j).attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("背景按钮"))){
                data->m_srcImage = child.firstChild().attribute(G_NodeHtml.SRC);
                node->m_id = child.id();
                break;
            }
        }
    }else if(dataLabel.contains(QStringLiteral("开关按钮")) || dataLabel.contains(QStringLiteral("模式切换按钮背景"))
             || dataLabel.contains(QStringLiteral("下拉三角"))){
        data->m_needMouseOver = false;
        data->m_needMousePressed = false;
        data->m_needMouseChecked = false;
    }else if(dataLabel.contains(QStringLiteral("播放")) || dataLabel.contains(QStringLiteral("暂停"))){
        data->m_needMousePressed = false;
        data->m_needMouseChecked = false;
    }else if(dataLabel.contains(QStringLiteral("页码"))){
        data->m_bChecked = true;
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
        else if(child.attribute(G_NodeHtml.DATA_LABEL) == QStringLiteral("半选中"))
        {
            data->m_partiallyCheckedImage = child.firstChild().firstChild().firstChild().firstChild().attribute(G_NodeHtml.SRC);
        }
    }
    data->m_width = element.firstChild().firstChild().firstChild().attribute(G_NodeHtml.DATA_WIDTH).toInt();
    data->m_height = element.firstChild().firstChild().firstChild().attribute(G_NodeHtml.DATA_HEIGHT).toInt();

    data->m_bChecked = element.secondChild().hasAttribute(G_NodeHtml.CHECKED);
    data->m_bDisabled = element.secondChild().hasAttribute(G_NodeHtml.DISABLED);
    data->m_toolTip = element.attribute(G_NodeHtml.TITLE);
    node->m_data = data;
}

void GumboParseMethod::parseDynamicPanelNodeData(GumboNodeWrapper &element, DomNode *node)
{
    PanelData * data = new PanelData();
    data->m_toolTip = element.attribute(G_NodeHtml.TITLE);
    node->m_data = data;

    //获取定制控件文字描述
    if(element.hasAttribute(G_NodeHtml.DATA_LABEL)){
        data->m_panelDataLabel = element.attribute(G_NodeHtml.DATA_LABEL);
    }

    GumboNodeWrapperList childs = element.children();
    for(int i = 0; i < childs.size(); i++)
    {
        GumboNodeWrapper child = childs.at(i);
        if(child.clazz() == "panel_state"){
            data->m_srcImageId.clear();

            //定制化控件‘开关’
            if(element.attribute(G_NodeHtml.DATA_LABEL).contains("开关")){
                if(child.style().contains("hidden")){
                    data->m_currentIndex = i;
                }
            }

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

    QString dataLabel = element.attribute(G_NodeHtml.DATA_LABEL);

    //自定义控件窗体
    if(dataLabel.contains(QStringLiteral("窗体"))){
        if(!dataLabel.contains(QStringLiteral("可见"))){
            data->m_visible = false;
        }

        GumboNodeWrapper closeButtWrapper;
        GumboNodeWrapperList children = element.children();
        for(int i = 0; i< children.size(); i++){
            GumboNodeWrapper child = children.at(i);
            if(child.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("框"))){
                data->m_geometryReferenceId = child.id();
            }else if(child.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("关闭按钮"))){
                GumboNodeWrapperList grandChils = child.children();
                for(int j = 0; j < grandChils.size(); j++){
                    if(grandChils.at(j).attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("背景按钮"))){
                        closeButtWrapper = grandChils.at(j);
                    }
                }
            }
        }

        //连接‘关闭按钮’和‘窗体’信号
        if(closeButtWrapper.valid()){
            SignalSlotInfo sinfo;
            sinfo.m_sender = closeButtWrapper.id();
            sinfo.m_signal = "pressed()";
            sinfo.m_receiver = element.id();
            sinfo.m_slot = "hide()";
            data->m_signals.append(sinfo);
        }
    }else{
        if(dataLabel.contains(QStringLiteral("二次确认弹窗"))){

            GumboNodeWrapperList children = element.children();
            for(int i = 0; i< children.size(); i++){
                GumboNodeWrapper child = children.at(i);
                //连接‘确定’和‘取消’按钮点击后，窗体关闭
                if(child.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("普通按钮"))
                        || child.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("取消按钮"))){
                    SignalSlotInfo sinfo;
                    sinfo.m_sender = child.id();
                    sinfo.m_signal = "pressed()";
                    sinfo.m_receiver = element.id();
                    sinfo.m_slot = "hide()";
                    data->m_signals.append(sinfo);
                }else if(child.attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("关闭按钮"))){
                    GumboNodeWrapperList grandChils = child.children();
                    for(int j = 0; j < grandChils.size(); j++){
                        if(grandChils.at(j).attribute(G_NodeHtml.DATA_LABEL).contains(QStringLiteral("背景按钮"))){
                            SignalSlotInfo sinfo;
                            sinfo.m_sender = grandChils.at(j).id();
                            sinfo.m_signal = "pressed()";
                            sinfo.m_receiver = element.id();
                            sinfo.m_slot = "hide()";
                            data->m_signals.append(sinfo);
                        }
                    }
                }
            }
        }

        data->m_left = element.attribute(G_NodeHtml.DATA_LEFT).toInt();
        data->m_top = element.attribute(G_NodeHtml.DATA_TOP).toInt();
        data->m_width = element.attribute(G_NodeHtml.DATA_WIDTH).toInt();
        data->m_height = element.attribute(G_NodeHtml.DATA_HEIGHT).toInt();
    }

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
    data->m_bChecked = element.firstChild().clazz().contains(G_NodeHtml.SELECTED);
    data->m_toolTip = element.attribute(G_NodeHtml.TITLE);
    node->m_data = data;
}


} //namespace Html
