#include "gumboparsemethod.h"

#include <QDebug>

namespace Html{

NodeHtml G_NodeHtml;

GumboParseMethod::GumboParseMethod():m_gumboParser(nullptr)
{
    m_custControl.insert(QStringLiteral("单选按钮"),RRADIO_BUTTON);
    m_custControl.insert(QStringLiteral("复选框"),RCHECKBOX);
    m_custControl.insert(QStringLiteral("开关按钮"),R_CUSTOM_SWITCH_BUTTON);
//    m_custControl.insert(QStringLiteral("开关按钮"),RSWITCH_BUTTON);
//    m_custControl.insert(QStringLiteral("折叠信息区"),RTREE);
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

    //自定义类控件
    m_custControl.insert(QStringLiteral("下拉框按钮"),R_CUSTOM_RBUTTON);
    m_custControl.insert(QStringLiteral("带有虚拟键盘的输入框"),R_CUSTOM_KEYBOARD_RFIELD);
    m_custControl.insert(QStringLiteral("双向滑块"),R_CUSTOM_BIDIRECTIONAL_SLIDER);
    m_custControl.insert(QStringLiteral("悬浮窗"),R_CUSTOM_FLOATING_WINDOW);
    m_custControl.insert(QStringLiteral("折叠信息区"),R_CUSTOM_FOLDINGCONTROLS);
//    m_custControl.insert(QStringLiteral("开关按钮"),R_CUSTOM_SWITCH_BUTTON);
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

void GumboParseMethod::parseDiv(GumboNodeWrapper divNode, DomNode *parentNode)
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

            parseNodeData(childEle,ttype,node,parentNode);
            establishRelation(parentNode,node);

            QString dataLabel = divNode.data_label();
            QString childDataLabel = childEle.data_label();

            if(i == 0 && (ttype == RLABEL || ttype == RIMAGE)
                    && (divNode.clazz().contains("ax_default_hidden") || dataLabel.contains(QStringLiteral("弹窗")))
                    &&!dataLabel.contains(QStringLiteral("二级弹出"))){
                parentNode = node;
            }

            if(ttype == RGROUP){
                parseDiv(childEle,node);
            }else if(ttype == R_CUSTOM_VIRTUAL_CONTAINER){
                parseDiv(childEle,parentNode);
            }
            //自制控件子级有组合控件继续解析
            else if(ttype == RLABEL ||ttype == RBOX||ttype == RBUTTON){
                if(!(childEle.firstChild().clazz().contains("text")
					 || childEle.firstChild().clazz().contains("selected")
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
NodeType GumboParseMethod::getNodeType(GumboNodeWrapper element, GumboNodeWrapper parentElement)
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
            QString dataLabel = element.data_label();
            if(!dataLabel.contains(QStringLiteral("时间栏")))
                m_classInfo.clear();

            QString selectTionGroup = element.attribute(G_NodeHtml.SELECTIONGROUP);

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
            else if(dataLabel.contains(QStringLiteral("触发弹窗")) || dataLabel.contains(QStringLiteral("提示框体")) ||
                    dataLabel.contains(QStringLiteral("全局控制栏")) || dataLabel.contains(QStringLiteral("表格菜单")) ||
                    dataLabel.contains(QStringLiteral("触发二级菜单")) || dataLabel.contains(QStringLiteral("右键垂直菜单"))||
                    dataLabel.contains(QStringLiteral("触发二级弹框")))
                return R_CUSTOM_VIRTUAL_CONTAINER;
            else if(dataLabel.contains(QStringLiteral("二次确认弹窗"))){
                return RGROUP;
            }else if(dataLabel.contains(QStringLiteral("弹窗")))
                return RLABEL;

            if(dataLabel.contains(QStringLiteral("按钮"))||dataLabel.contains(QStringLiteral("文件夹"))
                    ||dataLabel.contains(QStringLiteral("复位"))||dataLabel.contains(QStringLiteral("快进"))
                    ||dataLabel.contains(QStringLiteral("快退"))||dataLabel.contains(QStringLiteral("播放"))
                    ||dataLabel.contains(QStringLiteral("暂停"))||dataLabel.contains(QStringLiteral("台位切换按钮"))
                    ||dataLabel.contains(QStringLiteral("显控设置按钮"))||dataLabel.contains(QStringLiteral("左"))
                    ||dataLabel.contains(QStringLiteral("右"))||dataLabel.contains(QStringLiteral("选项"))
                    ||dataLabel.contains(QStringLiteral("训练"))
                    ||dataLabel.contains(QStringLiteral("下拉三角"))&& !dataLabel.contains(QStringLiteral("单选按钮"))){

                if(classInfo.contains("box_1") || classInfo.contains("box_2") || classInfo.contains("box_3")
                        || classInfo.contains("label")||classInfo.contains(QStringLiteral("图片"))||classInfo.contains("ellipse")
                        ||classInfo.contains("image")||classInfo.contains("selected")||classInfo.contains(QStringLiteral("形状"))){
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

            if(element.hasAttribute(G_NodeHtml.DATA_LABEL)&&(element.data_label().contains(QStringLiteral("信息区"))))
                return RGROUP;
            if(classInfo.contains("radio_button"))
                return RRADIO_BUTTON;
            else if(classInfo.contains("text_field"))
                return RTEXT_FIELD;
            else if((classInfo.contains("label") || classInfo.contains("text")||
                     classInfo.contains("ellipse")||classInfo.contains("paragraph"))
                    && (!classInfo.contains("text_area")))
            {
                return RLABEL;
            }
            else if(classInfo.contains(QStringLiteral("级标题")) || classInfo.contains(QStringLiteral("形状")))
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
            else if(classInfo.contains("text_area") || classInfo.contains(QStringLiteral("文本段落")))
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
 * @param[in] parentNode 元素父节点
 */
void GumboParseMethod::parseNodeData(GumboNodeWrapper &element, NodeType type, DomNode *node, DomNode *parentNode)
{
    switch(type){
    case RCONTAINER:parseContainerNodeData(element,node,parentNode);break;
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
    case R_CUSTOM_RBUTTON:parseCustomButton(element,node);break;
    case R_CUSTOM_KEYBOARD_RFIELD:parseCustomKeyBoardField(element,node);break;
    case R_CUSTOM_BIDIRECTIONAL_SLIDER:parseCustomScrollBarField(element,node);break;
    case R_CUSTOM_FLOATING_WINDOW:parseCustomFloatingWindowField(element,node);break;
    case R_CUSTOM_FOLDINGCONTROLS:parseCustomFoldingControlsField(element,node);break;
    case R_CUSTOM_SWITCH_BUTTON:parseSwitchButtonNodeData(element,node);break;

    default:break;
    }
}

void GumboParseMethod::parseContainerNodeData(GumboNodeWrapper &element, DomNode *node,DomNode *parentNode)
{
    //解析定制控件‘系统控制区’中第一个子div的data-label值为‘外框’
    BaseData * data = new BaseData();

    QString dataLabel = element.data_label();
    if(dataLabel.contains(QStringLiteral("外框")) || dataLabel.contains(QStringLiteral("背景"))
            || dataLabel.contains(QStringLiteral("触发弹窗"))){
        data->m_srcImage = element.firstChild().attribute(G_NodeHtml.SRC);
        if(!(data->m_srcImage.isEmpty())){
            data->m_srcImageId = element.firstChild().id();

            //NOTE 20201229 右键垂直菜单的尺寸依赖“外框”和其子图形节点
            if(element.parent().valid()){
                QString dlabel = element.parent().data_label();
                //右键垂直菜单以及二级菜单框
                if(dlabel.contains(QStringLiteral("右键垂直菜单"))){
                    data->m_geometryDepend.enable = true;
                    data->m_geometryDepend.dependGeometryId = data->m_srcImageId;
                    data->m_geometryDepend.operates.insert(P_LEFT,CustomPositionOperate(true,O_ADD));
                    data->m_geometryDepend.operates.insert(P_TOP,CustomPositionOperate(true,O_ADD));
                    data->m_geometryDepend.operates.insert(P_WIDTH,CustomPositionOperate(true,O_REPLACE));
                    data->m_geometryDepend.operates.insert(P_HEIGHT,CustomPositionOperate(true,O_REPLACE));
                }else if(dlabel.contains(QStringLiteral("二级菜单"))){
                    data->m_geometryDepend.enable = true;
                    data->m_geometryDepend.dependGeometryId = data->m_srcImageId;
                    data->m_geometryDepend.operates.insert(P_LEFT,CustomPositionOperate(false,O_ADD,0));
                    data->m_geometryDepend.operates.insert(P_TOP,CustomPositionOperate(false,O_ADD,0));
                    data->m_geometryDepend.operates.insert(P_WIDTH,CustomPositionOperate(true,O_REPLACE));
                    data->m_geometryDepend.operates.insert(P_HEIGHT,CustomPositionOperate(true,O_REPLACE));
                }
            }
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
        QString dataLabel = childs.at(i).data_label();
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
            if (headPanel.clazz().contains("selected")) {
                selectedTabPageName = headPanel.secondChild().firstChild().firstChild().firstChild().text();
                tabData->m_tabSelectedImage = headPanel.firstChild().attribute(G_NodeHtml.SRC);
                if (tabData->m_tabSelectedImage.isEmpty())
                    tabData->m_tabBackStyleID = headPanel.firstChild().id();
                tabData->m_tabBarId = headPanel.id();
            }
            else if (headPanel.hasAttribute("selectiongroup")) {
                tabData->m_tabNormalImage = headPanel.firstChild().attribute(G_NodeHtml.SRC);
            }

            //将分割线作为Tabwidget的border-right-image
            if(headPanel.data_label().contains(QStringLiteral("分割线"))){
                tabData->m_tabRightImage = headPanel.firstChild().attribute(G_NodeHtml.SRC);
            }
        }

        //根据内容面板创建页面
        GumboNodeWrapperList contentPanels = contentNode.children();
        for(int i = 0; i < contentPanels.size(); i++){
            GumboNodeWrapper contentPanel = contentPanels.at(i);

            BaseData * pageData = new BaseData();
            pageData->m_text = contentPanel.data_label();

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
 * @brief 解析自定义按钮
 * @details 1.对应元件打样.rp中的开关按钮。
 *          2.解析时，开关按钮由开和关两种状态分别位于面板的两个状态页，将两种状态分别提取出来作为按钮的开关状态。
 *
 */
void GumboParseMethod::parseCustomButton(GumboNodeWrapper &element,DomNode *node)
{
    GumboNodeWrapperList childs = element.children();

    GumboNodeWrapper closeNode;
    GumboNodeWrapper openNode;

    std::for_each(childs.begin(),childs.end(),[&](GumboNodeWrapper subChild){
        QString dataLabel = subChild.data_label();
        if(dataLabel.contains(QStringLiteral("关"))){
            closeNode = subChild;
        }else if(dataLabel.contains(QStringLiteral("开"))){
            openNode = subChild;
        }
    });

    ButtonData * buttonData = new ButtonData();

    CXX::DropDownButtonData * customButtonData = new CXX::DropDownButtonData();
    customButtonData->m_ID = element.id();

    Html::ControlImproveInfo t_controlImproveInfo;
    t_controlImproveInfo.m_newClass = "MyPushButton";
    t_controlImproveInfo.m_extends = "QPushButton";
    t_controlImproveInfo.m_headFileName = "mypushbutton.h";
    buttonData->m_controlImproveInfos.append(t_controlImproveInfo);

    auto setLocation = [&](GumboNodeWrapper curNode){

        CXX::Location t_location;
        t_location.m_height = curNode.attribute(G_NodeHtml.DATA_HEIGHT).toInt();
        t_location.m_width = curNode.attribute(G_NodeHtml.DATA_WIDTH).toInt();
        t_location.m_top = curNode.attribute(G_NodeHtml.DATA_TOP).toInt();
        t_location.m_left = curNode.attribute(G_NodeHtml.DATA_LEFT).toInt();

        return t_location;
    };

    auto setBaseInfo = [&](GumboNodeWrapper curNode){

        CXX::BaseInfo bassInfo;
        bassInfo.m_ID = curNode.id();
        bassInfo.m_textInfo.m_text = curNode.secondChild().firstChild().firstChild().firstChild().text();
        bassInfo.m_srcImg = curNode.firstChild().attribute(G_NodeHtml.SRC);
        bassInfo.m_dataLabel = curNode.data_label();

        return bassInfo;
    };

    if(closeNode.valid() && openNode.valid()){
        GumboNodeWrapperList openChildList = openNode.firstChild().children();
        GumboNodeWrapperList closeChildList = closeNode.firstChild().children();

        std::for_each(closeChildList.begin(),closeChildList.end(),[&](GumboNodeWrapper closeChild){

            QString dataLabel = closeChild.data_label();
            if(dataLabel.contains(QStringLiteral("选项弹出框"))){

                customButtonData->m_optionPopUp.m_location = setLocation(closeChild);
                customButtonData->m_optionPopUp.m_ID = closeChild.id();

                GumboNodeWrapperList optionPopupNodes = closeChild.children();

                std::for_each(optionPopupNodes.begin(),optionPopupNodes.end(),[&](GumboNodeWrapper subChild){
                    QString dataLabel = subChild.data_label();
                    if(dataLabel.contains(QStringLiteral("边框"))){

                        CXX::TheBorder theBorder;
                        theBorder.m_baseInfo = setBaseInfo(subChild);
                        customButtonData->m_optionPopUp.m_border= theBorder;

                    }else if(dataLabel.contains(QStringLiteral("文本标题"))){

                        CXX::TitleInfo titleInfo;
                        titleInfo.m_baseInfo = setBaseInfo(subChild);
                        customButtonData->m_optionPopUp.m_titleInfo = titleInfo;

                    }else if(dataLabel.contains(QStringLiteral("选项按钮组"))){
                        GumboNodeWrapperList sonNodes = subChild.children();

                        customButtonData->m_optionPopUp.m_optionBtnGroup.m_location = setLocation(subChild);
                        customButtonData->m_optionPopUp.m_optionBtnGroup.m_ID = subChild.id();

                        std::for_each(sonNodes.begin(),sonNodes.end(),[&](GumboNodeWrapper sonSonChild){

                            CXX::ButtonInfo buttonInfo;

                            buttonInfo.m_baseInfo = setBaseInfo(sonSonChild);
                            customButtonData->m_optionPopUp.m_optionBtnGroup.m_buttons.append(buttonInfo);

                        });

                    }else if(dataLabel.contains(QStringLiteral("确定按钮组"))){

                        customButtonData->m_optionPopUp.m_exitBtnGroup.m_location = setLocation(subChild);

                        customButtonData->m_optionPopUp.m_exitBtnGroup.m_ID = subChild.id();

                        GumboNodeWrapperList sonNodes = subChild.children();
                        std::for_each(sonNodes.begin(),sonNodes.end(),[&](GumboNodeWrapper sonSonChild){

                            CXX::ButtonInfo btnInfo;
                            btnInfo.m_baseInfo = setBaseInfo(sonSonChild);
                            customButtonData->m_optionPopUp.m_exitBtnGroup.m_buttons.append(btnInfo);

                        });
                    }
                });
            }else if(dataLabel.contains(QStringLiteral("下拉框按钮"))){

                customButtonData->m_triggerBtnClose.m_location = setLocation(closeChild);

                customButtonData->m_triggerBtnClose.m_ID = closeChild.id();

                GumboNodeWrapperList dropDownNodes = closeChild.children();
                for(GumboNodeWrapper sonSonChild : dropDownNodes){
                    QString dataLabel = sonSonChild.data_label();
                    if(dataLabel.contains(QStringLiteral("按钮")))
                    {
                        buttonData->m_checkedId = sonSonChild.id();
                        customButtonData->m_triggerBtnClose.m_buttonInfo = setBaseInfo(sonSonChild);

                    }else if(dataLabel.contains(QStringLiteral("下拉箭头"))){
                        customButtonData->m_triggerBtnClose.m_iconInfo = setBaseInfo(sonSonChild);
                    }
                }
            }

        });
        std::for_each(openChildList.begin(),openChildList.end(),[&](GumboNodeWrapper openChild){
            QString dataLabel = openChild.data_label();
            if(dataLabel.contains(QStringLiteral("下拉框按钮"))){

                buttonData->m_width = openChild.attribute(G_NodeHtml.DATA_WIDTH).toInt();

                buttonData->m_height = openChild.attribute(G_NodeHtml.DATA_HEIGHT).toInt();

                customButtonData->m_triggerBtnOpen.m_location = setLocation(openChild);

                customButtonData->m_triggerBtnOpen.m_ID = openChild.id();

                GumboNodeWrapperList dropDownNodes = openChild.children();
                for(GumboNodeWrapper sonSonChild : dropDownNodes){
                    QString dataLabel = sonSonChild.data_label();
                    if(dataLabel.contains(QStringLiteral("按钮")))
                    {
                        buttonData->m_text = sonSonChild.secondChild().firstChild().firstChild().firstChild().text();
                        buttonData->m_textId = sonSonChild.id();
                        customButtonData->m_triggerBtnOpen.m_buttonInfo = setBaseInfo(sonSonChild);

                    }else if(dataLabel.contains(QStringLiteral("下拉箭头"))){
                        customButtonData->m_triggerBtnOpen.m_iconInfo = setBaseInfo(sonSonChild);
                    }
                }
            }
        });
    }

    {
        buttonData->m_codeData = customButtonData;
    }

    node->m_data = buttonData;

}

void GumboParseMethod::parseCustomKeyBoardField(GumboNodeWrapper &element,DomNode *node)
{
    GumboNodeWrapper sonNode = element.firstChild().firstChild();

//    KeyBoardInputBox * t_keyBoardInputBox = new KeyBoardInputBox();
    TextFieldData * data = new TextFieldData();

    data->m_specialTextId = element.id();

    Html::ControlImproveInfo t_controlImproveInfo;
    t_controlImproveInfo.m_newClass = "MyLineEdit";
    t_controlImproveInfo.m_extends = "QLineEdit";
    t_controlImproveInfo.m_headFileName = "mylineedit.h";
    data->m_controlImproveInfos.append(t_controlImproveInfo);

    CXX::KeyBoardInputBoxData *  keyBoardInputBoxData = new CXX::KeyBoardInputBoxData();

    keyBoardInputBoxData->m_inputBoxId = element.id();

    if(sonNode.valid()){
        GumboNodeWrapperList sonChilds = sonNode.children();

        std::for_each(sonChilds.begin(),sonChilds.end(),[&](GumboNodeWrapper sonChild){

            QString dataLabel = sonChild.data_label();
            if(dataLabel.contains(QStringLiteral("虚拟键盘"))){

                CXX::Location t_location;
                t_location.m_height = sonChild.attribute(G_NodeHtml.DATA_HEIGHT).toInt();
                t_location.m_width = sonChild.attribute(G_NodeHtml.DATA_WIDTH).toInt();
                t_location.m_top = sonChild.attribute(G_NodeHtml.DATA_TOP).toInt();
                t_location.m_left = sonChild.attribute(G_NodeHtml.DATA_LEFT).toInt();

                keyBoardInputBoxData->m_virtualKeyGroup.m_location = t_location;
                keyBoardInputBoxData->m_virtualKeyGroup.m_ID = sonChild.id();

                GumboNodeWrapperList keyGroupChilds = sonChild.children();

                std::for_each(keyGroupChilds.begin(),keyGroupChilds.end(),[&](GumboNodeWrapper keyGruopChild){

                    QString dataLabel = keyGruopChild.data_label();

                    if(dataLabel.contains(QStringLiteral("取消确定"))){

                        keyBoardInputBoxData->m_virtualKeyGroup.m_tailGroup.m_baseAtrribute.m_ID = keyGruopChild.id();
                        t_location.m_width = keyGruopChild.attribute(G_NodeHtml.DATA_WIDTH).toInt();
                        t_location.m_height = keyGruopChild.attribute(G_NodeHtml.DATA_HEIGHT).toInt();
                        t_location.m_top = keyGruopChild.attribute(G_NodeHtml.DATA_TOP).toInt();
                        t_location.m_left = keyGruopChild.attribute(G_NodeHtml.DATA_LEFT).toInt();

                        keyBoardInputBoxData->m_virtualKeyGroup.m_tailGroup.m_location = t_location;

                        GumboNodeWrapperList tailChilds = keyGruopChild.children();
                        std::for_each(tailChilds.begin(),tailChilds.end(),[&](GumboNodeWrapper tailChild){

                            CXX::BUTTON t_btnInfo;
                            t_btnInfo.m_baseAtrribute.m_ID = tailChild.id();
                            t_btnInfo.m_baseAtrribute.m_srcImage = tailChild.firstChild().attribute(G_NodeHtml.SRC);
                            t_btnInfo.m_baseAtrribute.m_text = tailChild.secondChild().firstChild().firstChild().firstChild().text();

                            keyBoardInputBoxData->m_virtualKeyGroup.m_tailGroup.m_quitBtns.append(t_btnInfo);
                        });


                    }else if(dataLabel.contains(QStringLiteral("背景"))){

                        GumboNodeWrapperList backChilds = keyGruopChild.children();

                        std::for_each(backChilds.begin(),backChilds.end(),[&](GumboNodeWrapper backChild){
                            QString dataLabel = backChild.data_label();
                            if(dataLabel.contains(QStringLiteral("背景"))){
                                keyBoardInputBoxData->m_virtualKeyGroup.m_baseAtrribute.m_ID = backChild.id();
                                keyBoardInputBoxData->m_virtualKeyGroup.m_baseAtrribute.m_srcImage = backChild.firstChild().attribute(G_NodeHtml.SRC);
                            }
                        });

                    }else if(dataLabel == QStringLiteral("键盘")){

                        t_location.m_width = keyGruopChild.attribute(G_NodeHtml.DATA_WIDTH).toInt();
                        t_location.m_height = keyGruopChild.attribute(G_NodeHtml.DATA_HEIGHT).toInt();
                        t_location.m_top = keyGruopChild.attribute(G_NodeHtml.DATA_TOP).toInt();
                        t_location.m_left = keyGruopChild.attribute(G_NodeHtml.DATA_LEFT).toInt();

                        keyBoardInputBoxData->m_virtualKeyGroup.m_keyGroup.m_location = t_location;
                        keyBoardInputBoxData->m_virtualKeyGroup.m_keyGroup.m_baseAtrribute.m_ID = keyGruopChild.id();

                        GumboNodeWrapperList keyChilds = keyGruopChild.children();

                        std::for_each(keyChilds.begin(),keyChilds.end(),[&](GumboNodeWrapper keyChild){

                            QString dataLabel = keyChild.data_label();
                            QString classLabel = keyChild.clazz();
                            if(dataLabel.contains(QStringLiteral("删除"))){

                                CXX::BUTTON t_deleteBtn;
                                t_deleteBtn.deleteBtn = true;
                                GumboNodeWrapperList deleteChilds = keyChild.children();
                                for(GumboNodeWrapper deleteChild : deleteChilds){

                                    QString classLabel = deleteChild.clazz();


                                    if(classLabel.contains(QStringLiteral("形状"))){
                                        t_deleteBtn.m_baseAtrribute.m_ID = deleteChild.id();
                                        t_deleteBtn.m_baseAtrribute.m_text = deleteChild.secondChild().firstChild().firstChild().firstChild().text();
                                    }else if(classLabel.contains(QStringLiteral("图片"))){
                                        t_deleteBtn.m_baseAtrribute.m_iconInfo.m_srcIcon = deleteChild.firstChild().attribute(G_NodeHtml.SRC);
                                        t_deleteBtn.m_baseAtrribute.m_iconInfo.m_ID = deleteChild.id();
                                    }
                                }

                                keyBoardInputBoxData->m_virtualKeyGroup.m_keyGroup.m_keyBtns.append(t_deleteBtn);

                            }
                            if(classLabel.contains(QStringLiteral("形状")) && !dataLabel.contains("删除")){
                                CXX::BUTTON t_deleteBtn;
                                t_deleteBtn.m_baseAtrribute.m_ID = keyChild.id();
                                t_deleteBtn.m_baseAtrribute.m_text = keyChild.secondChild().firstChild().firstChild().firstChild().text();
                                t_deleteBtn.m_baseAtrribute.m_srcImage = keyChild.firstChild().attribute(G_NodeHtml.SRC);

                                keyBoardInputBoxData->m_virtualKeyGroup.m_keyGroup.m_keyBtns.append(t_deleteBtn);

                            }
                        });

                    }else if(dataLabel.contains(QStringLiteral("键盘输入框"))){

                        keyBoardInputBoxData->m_virtualKeyGroup.m_title.m_baseAtrribute.m_ID = keyGruopChild.id();
                        keyBoardInputBoxData->m_virtualKeyGroup.m_title.m_baseAtrribute.m_text = keyGruopChild.secondChild().attribute(G_NodeHtml.VALUE);
                        keyBoardInputBoxData->m_virtualKeyGroup.m_title.m_baseAtrribute.m_srcImage = keyGruopChild.firstChild().attribute(G_NodeHtml.SRC);

                    }

                });
            }else if(dataLabel.contains(QStringLiteral("触发编辑输入框"))){

                keyBoardInputBoxData->m_triggerInputBox.m_baseAtrribute.m_ID = sonChild.id();
                data->m_textId = keyBoardInputBoxData->m_triggerInputBox.m_baseAtrribute.m_ID;
                keyBoardInputBoxData->m_triggerInputBox.m_baseAtrribute.m_text = sonChild.secondChild().attribute(G_NodeHtml.VALUE);
                keyBoardInputBoxData->m_triggerInputBox.m_baseAtrribute.m_srcImage = sonChild.firstChild().attribute(G_NodeHtml.SRC);
            }

        });
    }

    data->m_text = keyBoardInputBoxData->m_triggerInputBox.m_baseAtrribute.m_text;

    data->m_codeData = keyBoardInputBoxData;


    node->m_data = data;

}

/**
 * @brief 获取自定义双向滚动条html属性
 * @param element结点
 * @param node
 */
void GumboParseMethod::parseCustomScrollBarField(GumboNodeWrapper &element,DomNode *node)
{
    GumboNodeWrapperList BidirectionalSliderNodes = element.children();

    GroupData * customScrolldata = new GroupData();
    customScrolldata->m_left = element.attribute(G_NodeHtml.DATA_LEFT).toInt();
    customScrolldata->m_top = element.attribute(G_NodeHtml.DATA_TOP).toInt();
    customScrolldata->m_width = element.attribute(G_NodeHtml.DATA_WIDTH).toInt();
    customScrolldata->m_height = element.attribute(G_NodeHtml.DATA_HEIGHT).toInt();

    CXX::Location t_location;

    t_location.m_left = element.attribute(G_NodeHtml.DATA_LEFT).toInt();
    t_location.m_top = element.attribute(G_NodeHtml.DATA_TOP).toInt();
    t_location.m_width = element.attribute(G_NodeHtml.DATA_WIDTH).toInt();
    t_location.m_height = element.attribute(G_NodeHtml.DATA_HEIGHT).toInt();

    Html::ControlImproveInfo t_controlImproveInfo;
    t_controlImproveInfo.m_newClass = "MyScrollBar";
    t_controlImproveInfo.m_extends = "QWidget";
    t_controlImproveInfo.m_headFileName = "myscrollbar.h";
    customScrolldata->m_controlImproveInfos.append(t_controlImproveInfo);

    CXX::BidirectionalSlider *  t_BidirectionalSliderData = new CXX::BidirectionalSlider();

    t_BidirectionalSliderData->m_ID = element.id();
    t_BidirectionalSliderData->m_location = t_location;

    std::for_each(BidirectionalSliderNodes.begin(),BidirectionalSliderNodes.end(),[&](GumboNodeWrapper sonChild){
        QString datalabel = sonChild.data_label();

        auto scrollBarInfo = [&](GumboNodeWrapper sonChild){
            GumboNodeWrapper scrollBarChild = sonChild.firstChild().firstChild().firstChild();
            QString dalabel = scrollBarChild.data_label();
            if(dalabel.contains(QStringLiteral("滑块"))){
                CXX::Location t_location;
                t_location.m_width = scrollBarChild.attribute(G_NodeHtml.DATA_WIDTH).toInt();
                t_location.m_height = scrollBarChild.attribute(G_NodeHtml.DATA_HEIGHT).toInt();
                t_location.m_top = scrollBarChild.attribute(G_NodeHtml.DATA_TOP).toInt();
                t_location.m_left = scrollBarChild.attribute(G_NodeHtml.DATA_LEFT).toInt();

                CXX::DScrollBar t_srollBar;
                t_srollBar.m_location = t_location;

                GumboNodeWrapperList scrollBarChilds = scrollBarChild.children();
                std::for_each(scrollBarChilds.begin(),scrollBarChilds.end(),[&](GumboNodeWrapper sonChild){
                    CXX::BaseInfo t_srcImgInfo;
                    t_srcImgInfo.m_ID = sonChild.id();
                    t_srcImgInfo.m_className = sonChild.clazz();
                    t_srcImgInfo.m_dataLabel = sonChild.data_label();
                    t_srcImgInfo.m_srcImg = sonChild.firstChild().attribute(G_NodeHtml.SRC);

                    t_srollBar.m_srcImgs.append(t_srcImgInfo);

                });
                return t_srollBar;
            }
        };
        if(datalabel.contains(QStringLiteral("左滑块"))){
            t_BidirectionalSliderData->m_leftScrollBar.m_id = sonChild.id();
			t_BidirectionalSliderData->m_leftScrollBar = scrollBarInfo(sonChild);

        }else if(datalabel.contains(QStringLiteral("右滑块"))){
            t_BidirectionalSliderData->m_rightScrollBar.m_id = sonChild.id();
			t_BidirectionalSliderData->m_rightScrollBar = scrollBarInfo(sonChild);
        }else{
            CXX::BaseInfo t_baseInfo;
            t_baseInfo.m_ID = sonChild.id();
            t_baseInfo.m_srcImg = sonChild.firstChild().attribute(G_NodeHtml.SRC);
            t_baseInfo.m_className = sonChild.clazz();
            t_baseInfo.m_dataLabel = sonChild.data_label();
            t_baseInfo.m_textInfo.m_text = sonChild.secondChild().firstChild().firstChild().firstChild().text();

            t_BidirectionalSliderData->m_rectangularsInfo.append(t_baseInfo);
        }

    });

    customScrolldata->m_textId = element.id();
    customScrolldata->m_codeData = t_BidirectionalSliderData;

    node->m_data = customScrolldata;
}

/**
 * @brief 针对自制悬浮穿解析提升为自定义类
 * @param element
 * @param node
 */
void GumboParseMethod::parseCustomFloatingWindowField(GumboNodeWrapper &element,DomNode *node)
{
    //悬浮窗目前默认使用动态面板的第一个页面属性
    GumboNodeWrapper firstChild = element.firstChild();

    GroupData * customFloatingWindowData = new GroupData();

    CXX::FloatingWindow * floatingWindow = new CXX::FloatingWindow();
    floatingWindow->m_ID = element.id();

    GumboNodeWrapper sonChild = firstChild.firstChild();

    GumboNodeWrapperList floatingWindowNodes = sonChild.children();

    std::for_each(floatingWindowNodes.begin(),floatingWindowNodes.end(),[&](GumboNodeWrapper sonChild){
        QString className = sonChild.clazz();
        QString dataLabel = sonChild.data_label();

        if(className == QString("ax_default")){

            GumboNodeWrapperList groupChilds = sonChild.children();

            std::for_each(groupChilds.begin(),groupChilds.end(),[&](GumboNodeWrapper groupChild){

                auto setBaseInfo = [&](GumboNodeWrapper child){
                    CXX::BaseInfo t_baseInfo;
                    t_baseInfo.m_ID = child.id();
                    t_baseInfo.m_srcImg = child.firstChild().attribute(G_NodeHtml.SRC);

                    return t_baseInfo;
                };

                QString className = groupChild.clazz();

                if(className == QString("ax_default")){

					CXX::Location t_location;
					t_location.m_width = sonChild.attribute(G_NodeHtml.DATA_WIDTH).toInt();
					t_location.m_height = sonChild.attribute(G_NodeHtml.DATA_HEIGHT).toInt();
					t_location.m_top = sonChild.attribute(G_NodeHtml.DATA_TOP).toInt();
					t_location.m_left = sonChild.attribute(G_NodeHtml.DATA_LEFT).toInt();

					floatingWindow->m_floatButton.m_location = t_location;

                    GumboNodeWrapperList groupChilds = groupChild.children();

                    std::for_each(groupChilds.begin(),groupChilds.end(),[&](GumboNodeWrapper groupChild){

                        QString className = groupChild.clazz();

                        if(className.contains(QStringLiteral("形状"))){
                            floatingWindow->m_floatButton.m_srcImgs.append(setBaseInfo(groupChild));
                        }
                    });
                }else if(className.contains(QStringLiteral("形状"))){
                    floatingWindow->m_floatButton.m_srcImgs.append(setBaseInfo(groupChild));
                }

            });
        }
        if(dataLabel.contains(QStringLiteral("底板"))){
            floatingWindow->m_mainWidget.m_srcImg = sonChild.firstChild().attribute(G_NodeHtml.SRC);
            floatingWindow->m_mainWidget.m_id = sonChild.id();
        }else if(dataLabel.contains(QStringLiteral("开关按钮"))){

            CXX::SwitchButton t_switchButton;
            t_switchButton.m_id = sonChild.id();

            GumboNodeWrapperList buttonsNodes = sonChild.children();

            std::for_each(buttonsNodes.begin(),buttonsNodes.end(),[&](GumboNodeWrapper buttonsNode){
                QString dataLabel = buttonsNode.data_label();

                if(dataLabel.contains(QStringLiteral("关"))){
                    GumboNodeWrapper t_child = buttonsNode.firstChild().firstChild();
                    t_switchButton.m_closeState.m_ID = t_child.id();
                    t_switchButton.m_closeState.m_srcImg = t_child.firstChild().attribute(G_NodeHtml.SRC);
                    t_switchButton.m_closeState.m_textInfo.m_text = t_child.secondChild().firstChild().firstChild().firstChild().text();

                }else if(dataLabel.contains(QStringLiteral("开"))){
                    GumboNodeWrapper t_child = buttonsNode.firstChild().firstChild();
                    t_switchButton.m_openState.m_ID = t_child.id();
                    t_switchButton.m_openState.m_srcImg = t_child.firstChild().attribute(G_NodeHtml.SRC);
                    t_switchButton.m_openState.m_textInfo.m_text = t_child.secondChild().firstChild().firstChild().firstChild().text();
                }
            });

            floatingWindow->m_mainWidget.m_switchButtons.append(t_switchButton);
        }
    });

    Html::ControlImproveInfo t_controlImproveInfo;
    t_controlImproveInfo.m_newClass = "MyFloatingWindow";
    t_controlImproveInfo.m_extends = "QWidget";
    t_controlImproveInfo.m_headFileName = "myfloatingwindow.h";
    customFloatingWindowData->m_controlImproveInfos.append(t_controlImproveInfo);


    customFloatingWindowData->m_textId = element.id();

    customFloatingWindowData->m_codeData = floatingWindow;

    node->m_data = customFloatingWindowData;
}

/**
 * @brief 自定义折叠控件
 */
void GumboParseMethod::parseCustomFoldingControlsField(GumboNodeWrapper &element,DomNode *node)
{
    GroupData * customFoldingControlData = new GroupData();

    customFoldingControlData->m_textId = element.id();

    CXX::FoldingControls *foldingControls = new CXX::FoldingControls();
    foldingControls->m_ID = element.id();

    //获取到需要的层级动态面板结点
    GumboNodeWrapper pageChild = element.firstChild().firstChild().firstChild().firstChild().firstChild().firstChild();

    GumboNodeWrapperList foldingInfoGroups = pageChild.children();

    std::for_each(foldingInfoGroups.begin(),foldingInfoGroups.end(),[&](GumboNodeWrapper foldingInfoGroup){
        QString className = foldingInfoGroup.clazz();

        if(className == QString("ax_default")){
            CXX::Information t_information;

            auto getLocatonInfo = [&](GumboNodeWrapper curNode){
                CXX::Location t_location;
                t_location.m_width = curNode.attribute(G_NodeHtml.DATA_WIDTH).toInt();
                t_location.m_height = curNode.attribute(G_NodeHtml.DATA_HEIGHT).toInt();
                t_location.m_top = curNode.attribute(G_NodeHtml.DATA_TOP).toInt();
                t_location.m_left = curNode.attribute(G_NodeHtml.DATA_LEFT).toInt();

                return t_location;
            };

            t_information.m_ID = foldingInfoGroup.id();
            t_information.m_location = getLocatonInfo(foldingInfoGroup);

            GumboNodeWrapperList nodeInfos = foldingInfoGroup.children();

            std::for_each(nodeInfos.begin(),nodeInfos.end(),[&](GumboNodeWrapper nodeInfo){

                auto getBaseInfo = [&](GumboNodeWrapper curNode){
                    CXX::BaseInfo t_baseInfo;
                    t_baseInfo.m_ID = curNode.id();
                    t_baseInfo.m_srcImg = curNode.firstChild().attribute(G_NodeHtml.SRC);
                    t_baseInfo.m_dataLabel = curNode.data_label();
                    t_baseInfo.m_textInfo.m_text = curNode.secondChild().firstChild().firstChild().firstChild().text();

                    return t_baseInfo;

				};
                QString dataLabel = nodeInfo.data_label();
                if(dataLabel.contains(QStringLiteral("折叠信息"))){
                    CXX::FoldingInfo t_foldingInfo;
                    t_foldingInfo.m_ID = nodeInfo.id();
                    t_foldingInfo.m_location = getLocatonInfo(nodeInfo);

                    GumboNodeWrapperList  foldingInfos = nodeInfo.children();
                    std::for_each(foldingInfos.begin(),foldingInfos.end(),[&](GumboNodeWrapper foldingInfo){

                        t_foldingInfo.m_information.append(getBaseInfo(foldingInfo));
                    });

                    t_information.m_foldingInfo = t_foldingInfo;
                }else if(dataLabel.contains(QStringLiteral("展开信息"))){
                    CXX::UnFoldInfo t_unFoldInfo;
                    t_unFoldInfo.m_parentID = nodeInfo.id();
                    t_unFoldInfo.m_ID = nodeInfo.firstChild().firstChild().firstChild().id();
                    t_unFoldInfo.m_location = getLocatonInfo(nodeInfo.firstChild().firstChild().firstChild());

                    GumboNodeWrapperList  unfoldingInfos = nodeInfo.firstChild().firstChild().firstChild().children();

                    std::for_each(unfoldingInfos.begin(),unfoldingInfos.end(),[&](GumboNodeWrapper unfoldingInfo){

                        t_unFoldInfo.m_information.append(getBaseInfo(unfoldingInfo));
                    });
                    t_information.m_unFoldInfo = t_unFoldInfo;

                }
            });
            foldingControls->m_informations.append(t_information);
        }
    });

    Html::ControlImproveInfo t_controlImproveInfo;
    t_controlImproveInfo.m_newClass = "MyFoldingControl";
    t_controlImproveInfo.m_extends = "QWidget";
    t_controlImproveInfo.m_headFileName = "myfoldingcontrol.h";
    customFoldingControlData->m_controlImproveInfos.append(t_controlImproveInfo);

    customFoldingControlData->m_codeData = foldingControls;
    node->m_data = customFoldingControlData;

}

/*!
 * @brief 针对弹出菜单、弹窗等，其外层DIV不需要转换，但需要用来保存信号槽信息
 * @attention 在第一层节点之上虚拟了一个节点，此节点不用于显示，只负责维护节点直接关系
 */
void GumboParseMethod::parseCustomVirtualContainer(GumboNodeWrapper &element, DomNode *node)
{
    BaseData * data = new BaseData();

    QString dataLabel = element.data_label();

    if(dataLabel == QStringLiteral("触发弹窗")){
        GumboNodeWrapperList childs = element.children();
        Html::SignalSlotInfo signalInfo;
        for(GumboNodeWrapper node : childs){
            if(node.data_label().contains(QStringLiteral("按钮"))){
                signalInfo.m_sender = node.id();
                signalInfo.m_signal = "clicked(bool)";
            }else if(node.data_label().contains(QStringLiteral("窗体"))){
                signalInfo.m_receiver = node.id();
                signalInfo.m_slot = "setVisible(bool)";
            }
        }
        data->m_signals.append(signalInfo);
    }else if(dataLabel.contains(QStringLiteral("提示框体"))){       //‘信息提示’组件
        GumboNodeWrapperList childs = element.children();
        Html::SignalSlotInfo signalInfo;
        for(GumboNodeWrapper node : childs){
            if(node.data_label().contains(QStringLiteral("详情"))){
                signalInfo.m_receiver = node.id();
                signalInfo.m_slot = "setVisible(bool)";
            }else if(node.data_label().contains(QStringLiteral("折叠"))){

                GumboNodeWrapperList grandChilds = node.children();
                for(int i = 0; i < grandChilds.size(); i++){
                    GumboNodeWrapper grandChild = grandChilds.at(i);
                    if(grandChild.data_label().contains(QStringLiteral("下拉三角"))){
                        signalInfo.m_sender = grandChild.id();
                        signalInfo.m_signal = "clicked(bool)";
                    }
                }
            }
        }
        data->m_signals.append(signalInfo);
    }else if(dataLabel.contains(QStringLiteral("全局控制栏"))){       //'全局控制栏'组件
        GumboNodeWrapperList childs = element.children();

        QString xkId;           /*!< 线控设置id */
        QString xkPopId;

        QString twId;           /*!< 台位切换id */
        QString twPopId;

        QString twTitleId;      /*!< 台位标题 */
        QList<QPair<QString,QString>> buttSwitchIds;      /*!< 台位切换按钮组:key:id,value:索引编号 */

        for(GumboNodeWrapper node : childs){
            if(node.data_label().contains(QStringLiteral("显控设置按钮"))){
                xkId = node.id();
            }else if(node.data_label().contains(QStringLiteral("显控设置弹出"))){
                xkPopId = node.id();
            }else if(node.data_label().contains(QStringLiteral("台位切换按钮"))){
                twId = node.id();
            }else if(node.data_label().contains(QStringLiteral("台位切换弹出"))){
                twPopId = node.id();

                GumboNodeWrapperList grandChilds = node.firstChild().firstChild().children();
                for(GumboNodeWrapper grandChild : grandChilds){
                    if(grandChild.attribute(G_NodeHtml.SELECTIONGROUP).contains(QStringLiteral("按钮"))){
                        QRegExp exp("(\\d+)");
                        int index = 0;
                        if((index = exp.indexIn(grandChild.data_label(),index) != -1)){
                            buttSwitchIds.append(qMakePair(grandChild.id(),exp.cap(1)));
                        }
                    }
                }
            }else if(node.data_label().contains(QStringLiteral("台位标题"))){
                twTitleId = node.id();
            }
        }

        {
            Html::SignalSlotInfo signalInfo;
            signalInfo.m_sender = xkId;
            signalInfo.m_signal = "clicked(bool)";
            signalInfo.m_receiver = xkPopId;
            signalInfo.m_slot = "setVisible(bool)";
            data->m_signals.append(signalInfo);
        }

        {
            Html::SignalSlotInfo signalInfo;
            signalInfo.m_sender = twId;
            signalInfo.m_signal = "clicked(bool)";
            signalInfo.m_receiver = twPopId;
            signalInfo.m_slot = "setVisible(bool)";
            data->m_signals.append(signalInfo);
        }

        {
            CXX::TWSwitchCodeData * twData = new CXX::TWSwitchCodeData();
            data->m_codeData = twData;

            twData->m_twPopButtId = twId;
            twData->m_buttContainerId = twPopId;
            twData->m_twContainerId = twTitleId;
            twData->m_buttIds = buttSwitchIds;
        }

    }else if(dataLabel.contains(QStringLiteral("表格菜单"))){
        GumboNodeWrapperList children = element.children();

        SignalSlotInfo sinfo;

        for(int i = 0; i< children.size(); i++){
            GumboNodeWrapper child = children.at(i);
            if(child.data_label().contains(QStringLiteral("选项"))){
                sinfo.m_receiver = child.id();
                sinfo.m_slot = "setVisible(bool)";
            }else if(child.data_label().contains(QStringLiteral("菜单按钮"))){
                sinfo.m_sender = child.id();
                sinfo.m_signal = "clicked(bool)";
            }
        }

        data->m_signals.append(sinfo);
    }else if(dataLabel.contains(QStringLiteral("触发二级菜单"))){
        GumboNodeWrapperList children = element.children();

        SignalSlotInfo sinfo;

        for(int i = 0; i< children.size(); i++){
            GumboNodeWrapper child = children.at(i);
            if(child.data_label().contains(QStringLiteral("二级菜单"))){
                sinfo.m_receiver = child.id();
                sinfo.m_slot = "setVisible(bool)";
            }else if(child.data_label().contains(QStringLiteral("菜单项"))){
                GumboNodeWrapperList grandChilds = child.children();
                for(GumboNodeWrapper grand : grandChilds){
                    if(grand.data_label().contains(QStringLiteral("选项"))){
                        sinfo.m_sender = grand.id();
                        sinfo.m_signal = "clicked(bool)";
                    }
                }
            }
        }

        data->m_signals.append(sinfo);
    }else if(dataLabel.contains(QStringLiteral("右键垂直菜单"))){   //右键垂直菜单
        data->m_visible = true;
        data->m_dataLabel = element.data_label();

        GumboNodeWrapperList children = element.children();
        for(int i = 0; i< children.size(); i++){
            GumboNodeWrapper child = children.at(i);
            if(child.data_label().contains(QStringLiteral("外框"))){
                //                data->m_geometryReferenceId = child.id();
            }
        }
    }else if(dataLabel.contains(QStringLiteral("触发二级弹框"))){
        GumboNodeWrapperList children = element.firstChild().firstChild().firstChild().children();

        SignalSlotInfo sinfo;

        for(GumboNodeWrapper child : children){
            QString dataLabel = child.data_label();
            if(dataLabel.contains(QStringLiteral("训练"))){
                sinfo.m_sender = child.id();
                sinfo.m_signal = "clicked(bool)";
            }else if(dataLabel.contains(QStringLiteral("二级弹出"))){
                sinfo.m_receiver = child.id();
                sinfo.m_slot = "setHidden(bool)";

                GumboNodeWrapperList btnChildren = child.children();

                for(GumboNodeWrapper sonChild : btnChildren){
                    QString dataLabel = sonChild.data_label();
                    if(dataLabel.contains(QStringLiteral("选项"))){
                        SignalSlotInfo sinfo;
                        sinfo.m_sender = sonChild.id();
                        sinfo.m_signal = "pressed()";

                        sinfo.m_receiver = child.id();
                        sinfo.m_slot = "hide()";

                        data->m_signals.append(sinfo);
                    }
                }
            }
        }
        data->m_signals.append(sinfo);
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
        QString dataLabel = child.data_label();

        if(dataLabel.contains(QStringLiteral("进度条"))){
            GumboNodeWrapperList grandChilds = child.firstChild().firstChild().children();
            for(GumboNodeWrapper grandChild : grandChilds){
                QString grandDataLabel = grandChild.data_label();
                if(grandDataLabel.contains(QStringLiteral("进度槽"))){   //add-page
                    data->m_progressSlotId = grandChild.id();
                }else if(grandDataLabel.contains(QStringLiteral("进度条"))){ //sub-page
                    data->m_progressBarId = grandChild.id();
                }
            }
        }else if(dataLabel.contains(QStringLiteral("进度球"))){
            GumboNodeWrapperList grandChilds = child.children();
            for(GumboNodeWrapper grandChild : grandChilds){
                QString grandDataLabel = grandChild.data_label();
                if(grandDataLabel.contains(QStringLiteral("默认"))){   //handle
                    data->m_handleId = grandChild.firstChild().firstChild().id();
                    data->m_srcImage = grandChild.firstChild().firstChild().firstChild().attribute(G_NodeHtml.SRC);
                }else if(grandDataLabel.contains(QStringLiteral("选中"))){ //handle:hover
                    //WARNING 20201230 此处需要axure页面中设置一个标识，用来提示较大图片
                    data->m_pressedHandleId = grandChild.firstChild().secondChild().id();
                    data->m_checkedImage = grandChild.firstChild().secondChild().firstChild().attribute(G_NodeHtml.SRC);
                }
            }
        }
    }

    node->m_data = data;
}

void GumboParseMethod::parseScrollBarNodeData(GumboNodeWrapper &element,DomNode *node)
{
    ScrollBarData * data = new ScrollBarData();

    data->m_left = element.attribute(G_NodeHtml.DATA_LEFT).toInt();
    data->m_top = element.attribute(G_NodeHtml.DATA_TOP).toInt();
    data->m_width = element.attribute(G_NodeHtml.DATA_WIDTH).toInt();
    data->m_height = element.attribute(G_NodeHtml.DATA_HEIGHT).toInt();

    GumboNodeWrapperList childs = element.children();
    for(int i = 0; i < childs.size(); i++)
    {
        GumboNodeWrapper child = childs.at(i);

        if(child.data_label().contains(QStringLiteral("滚动槽"))){
            data->m_scrollSlotId = child.id();
        }

        if(child.data_label().contains(QStringLiteral("滚动条")))
        {
            data->m_scrollBarId = child.firstChild().firstChild().firstChild().id();
        }

        if(child.data_label().contains(QStringLiteral("下翻")))
        {
            GumboNodeWrapperList subChilds = child.children();
            for(GumboNodeWrapper subChild : subChilds){
                if(subChild.firstChild().hasAttribute(G_NodeHtml.SRC)){
                    data->m_downArrowImage = subChild.firstChild().attribute(G_NodeHtml.SRC);
                }else{
                    data->m_addLineId = subChild.id();
                }
            }
        }

        if(child.data_label().contains(QStringLiteral("上翻")))
        {
            GumboNodeWrapperList subChilds = child.children();
            for(GumboNodeWrapper subChild : subChilds){
                if(subChild.firstChild().hasAttribute(G_NodeHtml.SRC)){
                    data->m_upArrowImage = subChild.firstChild().attribute(G_NodeHtml.SRC);
                }else{
                    data->m_subLineId = subChild.id();
                }
            }
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
        if(child.data_label().contains(QStringLiteral("输入框")))
        {
            data->m_spinBoxId = child.firstChild().id();
            data->m_sinBoxTextId = child.secondChild().id();
        }
        else if(child.data_label().contains(QStringLiteral("加减箭头")))
        {
            GumboNodeWrapperList grandChilds = child.children();
            for(int j = 0; j < grandChilds.size(); j++){
                GumboNodeWrapper gchild = grandChilds.at(j);
                if(gchild.data_label().contains(QStringLiteral("默认"))){
                    GumboNodeWrapperList arrows = gchild.firstChild().children();
                    for(GumboNodeWrapper wrapper : arrows){
                        if(wrapper.data_label().contains(QStringLiteral("上三角"))){
                            data->m_upArrowImage = wrapper.firstChild().attribute(G_NodeHtml.SRC);
                        }else{
                            data->m_downArrowImage = wrapper.firstChild().attribute(G_NodeHtml.SRC);
                        }
                    }
                }else if(gchild.data_label().contains(QStringLiteral("得到焦点"))){
                    GumboNodeWrapperList arrows = gchild.firstChild().children();
                    for(GumboNodeWrapper wrapper : arrows){
                        if(wrapper.data_label().contains(QStringLiteral("上三角"))){
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
            if(child.data_label().contains(QStringLiteral("下拉菜单")))
            {
                GumboNodeWrapperList childs = child.firstChild().firstChild().children();
                for(int j = 0; j < childs.size(); j++)
                {
                    GumboNodeWrapper child = childs.at(j);
                    if(child.data_label().contains(QStringLiteral("下拉菜单背景")))
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
            if(child.data_label().contains(QStringLiteral("下拉框部分")))
            {
                GumboNodeWrapperList childs = child.children();
                for(int j = 0; j < childs.size(); j++)
                {
                    GumboNodeWrapper child = childs.at(j);
                    if(child.data_label().contains(QStringLiteral("下拉箭头-默认")))
                    {
                        data->m_arrowImageSrc = child.firstChild().attribute(G_NodeHtml.SRC);

                        QStringList imageList = data->m_arrowImageSrc.split(".");
                        if(imageList.size() == 2){
                            data->m_arrowImageOn = imageList.at(0) + "_selected." + imageList.at(1);
                        }
                    }
                    else if(child.data_label().contains(QStringLiteral("下拉框背景")))
                    {
                        data->m_widgetSizeId = child.id();
                    }
                    else if(child.data_label().contains(QStringLiteral("下拉框文本")))
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

    data->m_dataLabel = element.data_label();
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
        if(data->m_text.isEmpty()){
            data->m_text = element.secondChild().attribute(G_NodeHtml.VALUE);
        }
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

    QString dataLabel = element.data_label();
    data->m_dataLabel = dataLabel;

    if(dataLabel.contains(QStringLiteral("标签按钮"))){
        data->m_bChecked = true;

//        SignalSlotInfo sinfo;
//        sinfo.m_sender = element.id();
//        sinfo.m_signal = "clicked(bool)";
//        sinfo.m_receiver = element.id();
//        sinfo.m_slot = "setDisabled(bool)";
//        data->m_signals.append(sinfo);
    }else if(dataLabel.contains(QStringLiteral("关闭按钮"))){
        GumboNodeWrapperList chils = element.children();
        for(int j = 0; j < chils.size(); j++){
            GumboNodeWrapper child = chils.at(j);
            if(chils.at(j).data_label().contains(QStringLiteral("背景按钮"))){
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
    }else if(dataLabel.contains(QStringLiteral("播放")) || dataLabel.contains(QStringLiteral("暂停"))
             || dataLabel.contains(QStringLiteral("显控设置按钮")) || dataLabel.contains(QStringLiteral("台位切换按钮"))
             || dataLabel.contains(QStringLiteral("菜单按钮")) || dataLabel.contains(QStringLiteral("选项"))){
        data->m_needMousePressed = false;
        data->m_needMouseChecked = false;
    }else if(dataLabel.contains(QStringLiteral("页码"))){     //‘翻页器’
        data->m_needMousePressed = false;
        data->m_bChecked = true;
    }else if(dataLabel.contains(QStringLiteral("快退")) || dataLabel.contains(QStringLiteral("快进"))
             || dataLabel.contains(QStringLiteral("复位")) || dataLabel.contains(QStringLiteral("左"))
             || dataLabel.contains(QStringLiteral("右")) || dataLabel.contains(QStringLiteral("文件夹"))){
        data->m_needMouseChecked = false;
    }else if(dataLabel.contains(QStringLiteral("训练"))){
        data->m_needMouseChecked = false;
        data->m_needMouseOver = false;
    }else if(dataLabel.contains(QStringLiteral("下拉按钮"))){
        data->m_needMouseChecked = false;
        data->m_needMouseOver = false;
    }
    node->m_data = data;
}

/**
 * @brief 解析动态面板组成的开关按钮，点击切换状态
 */
void GumboParseMethod::parseSwitchButtonNodeData(GumboNodeWrapper &element, DomNode *node)
{
    GroupData * customSwitchData = new GroupData();

    customSwitchData->m_textId = element.id();

    CXX::customSwitchButton * switchBtnData = new CXX::customSwitchButton();

    switchBtnData->m_ID = element.id();

    GumboNodeWrapperList buttonStates = element.children();

    auto setBaseInfo = [&](GumboNodeWrapper child){

        CXX::BaseInfo t_textInfo;
        t_textInfo.m_ID = child.id();
        t_textInfo.m_srcImg = child.firstChild().attribute(G_NodeHtml.SRC);
        t_textInfo.m_textInfo.m_text = child.secondChild().firstChild().firstChild().firstChild().text();

        return t_textInfo;
    };

    for(GumboNodeWrapper curState : buttonStates){
        QString dataLabel = curState.data_label();
        if(dataLabel.contains(QStringLiteral("关"))){
//            GumboNodeWrapper closeChild = curState.firstChild().firstChild();
            GumboNodeWrapperList closeChilds = curState.firstChild().children();

            for(GumboNodeWrapper clild : closeChilds){

                QString className = clild.clazz();

                if(className.contains(QStringLiteral("形状"))){

                    switchBtnData->m_closeState = setBaseInfo(clild);
                    customSwitchData->m_checkedId = clild.id();//有些自定义开关按钮需要使用里面按钮的位置信息

                }else if(className.contains(QStringLiteral("文本段落")))
                {

                    switchBtnData->m_textParagraphs.append(setBaseInfo(clild));

                }else if(className.contains(QStringLiteral("text_field"))){

                    switchBtnData->m_lineEdits.append(setBaseInfo(clild));
                }
            }
        }else if(dataLabel.contains(QStringLiteral("开"))){

            GumboNodeWrapperList openChilds = curState.firstChild().children();

            for(GumboNodeWrapper clild : openChilds){
                QString className = clild.clazz();
                if(className.contains(QStringLiteral("形状"))){
                    switchBtnData->m_openState = setBaseInfo(clild);

                }
            }
        }
    }

    /*!< 特殊的开关按钮需要单独自定义类处理，目前还未处理 */
    Html::ControlImproveInfo t_controlImproveInfo;
    t_controlImproveInfo.m_newClass = "MySwitchButton";
    t_controlImproveInfo.m_extends = "QWidget";
    t_controlImproveInfo.m_headFileName = "myswitchbutton.h";
    customSwitchData->m_controlImproveInfos.append(t_controlImproveInfo);


    customSwitchData->m_codeData = switchBtnData;

    node->m_data = customSwitchData;
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
        if(child.data_label() == QStringLiteral("未选中"))
        {
            data->m_unCheckedImage = child.firstChild().firstChild().firstChild().firstChild().attribute(G_NodeHtml.SRC);
        }
        else if(child.data_label() == QStringLiteral("选中"))
        {
            data->m_checkedImage = child.firstChild().firstChild().firstChild().firstChild().attribute(G_NodeHtml.SRC);
        }
        else if(child.data_label() == QStringLiteral("半选中"))
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
        data->m_panelDataLabel = element.data_label();

        if(element.data_label().contains(QStringLiteral("台位切换弹出"))
                || element.data_label().contains(QStringLiteral("显控设置弹出"))){
            data->m_visible = false;
        }else if(element.data_label().contains(QStringLiteral("播放按钮"))){        //回放控制页面切换按钮

            CXX::PlayControlCodeData * playData = new CXX::PlayControlCodeData();
            playData->m_stackedWidgetId = element.id();
            data->m_codeData = playData;

            GumboNodeWrapperList childs = element.children();
            for(int i = 0; i < childs.size(); i++)
            {
                GumboNodeWrapper child = childs.at(i);
                GumboNodeWrapperList grandChilds = child.firstChild().children();
                for(int j = 0; j < grandChilds.size(); j++){
                    GumboNodeWrapper grandChild = grandChilds.at(j);
                    QString dataLabel = grandChild.data_label();
                    if(dataLabel.contains(QStringLiteral("播放")) || dataLabel.contains(QStringLiteral("暂停"))){
                        playData->m_modelIds.append(grandChild.id());
                    }
                }
            }
        }else if(element.data_label() == QStringLiteral("下拉按钮")){

            data->m_dataLabel = element.data_label();

            CXX::DropDownBoxData *dropDownBoxData = new CXX::DropDownBoxData();

            data->m_codeData = dropDownBoxData;

            data->m_childPageID = element.firstChild().id();

            GumboNodeWrapperList children = element.firstChild().firstChild().children();

            for(GumboNodeWrapper child : children){
                QString dataLabel = child.data_label();


                if(dataLabel.contains(QStringLiteral("按钮"))){
                    dropDownBoxData->m_buttonID = child.id();

                }else if(dataLabel.contains(QStringLiteral("下拉框"))){

                    data->m_left = child.attribute(G_NodeHtml.DATA_LEFT).toInt();
                    data->m_top = child.attribute(G_NodeHtml.DATA_TOP).toInt();
                    data->m_width = child.attribute(G_NodeHtml.DATA_WIDTH).toInt();
                    data->m_height = child.attribute(G_NodeHtml.DATA_HEIGHT).toInt();

                    dropDownBoxData->m_widgetID = child.id();


                    GumboNodeWrapperList boxChildren = child.children();

                    for(GumboNodeWrapper sonChild : boxChildren){
                        QString dataLabel = sonChild.data_label();
                        if(dataLabel.contains(QStringLiteral("选项"))){
                           dropDownBoxData->m_optionIdList.append(sonChild.id());
                        }
                    }
                }
            }

        }else if(element.data_label() == QStringLiteral("开关下拉按钮")
                 || element.data_label() == QStringLiteral("选择器")){
            GumboNodeWrapperList children = element.children();

            data->m_dataLabel = element.data_label();

            CXX::SwitchPullDownButtonData *switchPullDownBtnData = new CXX::SwitchPullDownButtonData();
            switchPullDownBtnData->m_datalabel = element.data_label();

            switchPullDownBtnData->m_stackedWidgetID = element.id();
            data->m_codeData = switchPullDownBtnData;

            for(GumboNodeWrapper child : children){
                QString datalabel = child.data_label();
                if(datalabel.contains(QStringLiteral("开"))
                        ||datalabel.contains(QStringLiteral("默认"))){

                    switchPullDownBtnData->m_openPageID = child.id();

                    GumboNodeWrapperList openChildren = child.firstChild().children();

                    for(GumboNodeWrapper openChild : openChildren){
                        QString datalabel = openChild.data_label();
                        if(datalabel.contains(QStringLiteral("按钮"))){
                            switchPullDownBtnData->m_openBtnID = openChild.id();
                        }else if(datalabel.contains(QStringLiteral("下拉"))){
                            GumboNodeWrapperList imgChildren = openChild.children();
                            for(GumboNodeWrapper imgChild : imgChildren){
                                if(imgChild.firstChild().hasAttribute(G_NodeHtml.SRC)){
                                   switchPullDownBtnData->m_icon =  imgChild.firstChild().attribute(G_NodeHtml.SRC);
                                }
                            }
                        }
                    }

                }else if(datalabel.contains(QStringLiteral("关"))
                         ||datalabel.contains(QStringLiteral("按下"))){

                    data->m_childPageID = child.id();

                    switchPullDownBtnData->m_closePageID = child.id();

                    GumboNodeWrapperList closeChildren = child.firstChild().children();

                    for(GumboNodeWrapper closeChild : closeChildren){
                        QString datalabel = closeChild.data_label();
                        if(datalabel.contains(QStringLiteral("按钮"))){
                            switchPullDownBtnData->m_closeBtnID = closeChild.id();
                        }else if(datalabel.contains(QStringLiteral("下拉"))){
                             GumboNodeWrapperList pullDownChildren = closeChild.children();
                             for(GumboNodeWrapper pullDownChild : pullDownChildren){
                                 QString datalabel = pullDownChild.data_label();
                                 if(datalabel.contains(QStringLiteral("按钮"))){
                                     switchPullDownBtnData->m_triggerPopupBtnID = pullDownChild.id();
                                 }else if(datalabel.contains(QStringLiteral("图标"))){
                                     switchPullDownBtnData->m_icon = pullDownChild.firstChild().attribute(G_NodeHtml.SRC);
                                 }
                             }
                        }else if(datalabel.contains(QStringLiteral("单模块弹出"))){

                            data->m_left = closeChild.attribute(G_NodeHtml.DATA_LEFT).toInt();
                            data->m_top = closeChild.attribute(G_NodeHtml.DATA_TOP).toInt();
                            data->m_width = closeChild.attribute(G_NodeHtml.DATA_WIDTH).toInt();
                            data->m_height = closeChild.attribute(G_NodeHtml.DATA_HEIGHT).toInt();

                            switchPullDownBtnData->m_popupWidgetID = closeChild.id();

                            GumboNodeWrapperList popupChildren = closeChild.children();

                            for(GumboNodeWrapper popupChild : popupChildren){

                                QString datalabel = popupChild.data_label();
                                if(datalabel.contains(QStringLiteral("选项"))){

                                    switchPullDownBtnData->m_optionIdList.append(popupChild.id());

                                }

                            }
                        }
                    }
                }
            }
        }
    }

    GumboNodeWrapperList childs = element.children();
    for(int i = 0; i < childs.size(); i++)
    {
        GumboNodeWrapper child = childs.at(i);

        if(child.clazz() == "panel_state"){
            data->m_srcImageId.clear();

            //定制化控件‘开关’
            if(element.data_label().contains("开关")){
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

    if(element.firstChild().clazz().contains("img")){
       data->m_srcImage =  element.firstChild().attribute(G_NodeHtml.SRC);
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

/*!
 * @brief 解析Axure中表格控件
 * @attention 确保所有列的宽度保持一致，否则计算列的时候会出现错误。
 * @details Axure表格中没有表头的概念，但用户默认第一行和第一列作为表头。
 */
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

        CellData cellData;
        cellData.id = child.id();
        cellData.imageSrc = child.firstChild().attribute(G_NodeHtml.SRC);
        cellData.text = child.secondChild().firstChild().firstChild().firstChild().text();

        data->m_cells.append(cellData);

        GumboNodeWrapperList childImage = child.children();
        if(childImage.size() > 0){
            GumboNodeWrapper image = childImage.at(0);
            if(image.clazz().contains("img")){
                data->m_srcImage = image.attribute(G_NodeHtml.SRC);

                if(data->m_srcImageId.isEmpty()){
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
 * @attention
 */
void GumboParseMethod::parseTreeNodeData(GumboNodeWrapper &element, DomNode *node)
{
    TreeData * treeData = new TreeData();

    treeData->m_left = element.attribute(G_NodeHtml.DATA_LEFT).toInt();
    treeData->m_top = element.attribute(G_NodeHtml.DATA_TOP).toInt();
    treeData->m_width = element.attribute(G_NodeHtml.DATA_WIDTH).toInt();
    treeData->m_height = element.attribute(G_NodeHtml.DATA_HEIGHT).toInt();

    GumboNodeWrapperList childs = element.children();

    TreeNodeData tdata;
    for(int i = 0; i < childs.size(); i++){
        GumboNodeWrapper child = childs.at(i);

        if(child.data_label().contains(QStringLiteral("展开信息"))){
            GumboNodeWrapperList grandChilds = child.firstChild().firstChild().firstChild().children();
            for(GumboNodeWrapper grand : grandChilds){
                if(grand.data_label().contains(QStringLiteral("详情内容"))){
                    tdata.detailInfo = grand.secondChild().firstChild().firstChild().firstChild().text();
                }
            }
        }else if(child.data_label().contains(QStringLiteral("折叠信息"))){
            GumboNodeWrapperList grandChilds = child.children();
            for(GumboNodeWrapper grand : grandChilds){
                if(grand.data_label().contains(QStringLiteral("时间"))){
                    tdata.timestamp = grand.secondChild().firstChild().firstChild().firstChild().text();
                }else if(grand.data_label().contains(QStringLiteral("缩略信息"))){
                    tdata.simpleInfo = grand.secondChild().firstChild().firstChild().firstChild().text();
                    tdata.simpleInfoId = grand.id();
                }else if(grand.data_label().contains(QStringLiteral("悬浮显示"))){
                    treeData->m_srcImage = grand.firstChild().attribute(G_NodeHtml.SRC);
                }
            }
        }

        if(tdata.valid()){
            treeData->m_treeDatas.append(tdata);
            tdata = TreeNodeData();
        }
    }

    if(element.clazz() == "ax_default"){
        node->m_id = element.firstChild().id();
    }

    node->m_data = treeData;
}

void GumboParseMethod::parseGroupNodeData(GumboNodeWrapper &element, DomNode *node)
{
    GroupData * data = new GroupData();

    QString dataLabel = element.data_label();

    //自定义控件窗体
    if(dataLabel.contains(QStringLiteral("窗体")))
    {
        if(!dataLabel.contains(QStringLiteral("可见"))){
            data->m_visible = false;
        }

        GumboNodeWrapper closeButtWrapper;
        GumboNodeWrapperList children = element.children();
        for(int i = 0; i< children.size(); i++){
            GumboNodeWrapper child = children.at(i);
            if(child.data_label().contains(QStringLiteral("框"))){
                data->m_geometryReferenceId = child.id();
            }else if(child.data_label().contains(QStringLiteral("关闭按钮"))){
                GumboNodeWrapperList grandChils = child.children();
                for(int j = 0; j < grandChils.size(); j++){
                    if(grandChils.at(j).data_label().contains(QStringLiteral("背景按钮"))){
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
                if(child.data_label().contains(QStringLiteral("普通按钮"))
                        || child.data_label().contains(QStringLiteral("取消按钮"))){
                    SignalSlotInfo sinfo;
                    sinfo.m_sender = child.id();
                    sinfo.m_signal = "pressed()";
                    sinfo.m_receiver = element.id();
                    sinfo.m_slot = "hide()";
                    data->m_signals.append(sinfo);
                }else if(child.data_label().contains(QStringLiteral("关闭按钮"))){
                    GumboNodeWrapperList grandChils = child.children();
                    for(int j = 0; j < grandChils.size(); j++){
                        if(grandChils.at(j).data_label().contains(QStringLiteral("背景按钮"))){
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
        }else if(dataLabel.contains(QStringLiteral("选项"))){     //表格2
            data->m_visible = false;

            GumboNodeWrapperList children = element.children();
            for(int i = 0; i< children.size(); i++){
                GumboNodeWrapper child = children.at(i);
                if(child.data_label().contains(QStringLiteral("外框"))){
                    data->m_geometryReferenceId = child.id();
                }
            }
        }
        else if(dataLabel.contains(QStringLiteral("二级菜单"))){       //菜单选项(触发二级菜单)
            data->m_visible = false;

            GumboNodeWrapperList children = element.children();
            for(int i = 0; i< children.size(); i++){
                GumboNodeWrapper child = children.at(i);
                if(child.data_label().contains(QStringLiteral("外框"))){
                    data->m_geometryReferenceId = child.id();

                    if(!(child.firstChild().attribute(G_NodeHtml.SRC).isEmpty())){
                        data->m_geometryDepend.enable = true;
                        data->m_geometryDepend.dependGeometryId = child.firstChild().id();
                        data->m_geometryDepend.operates.insert(P_LEFT,CustomPositionOperate(true,O_ADD));
                        data->m_geometryDepend.operates.insert(P_TOP,CustomPositionOperate(true,O_ADD));
                        data->m_geometryDepend.operates.insert(P_WIDTH,CustomPositionOperate(false,O_ADD));
                        data->m_geometryDepend.operates.insert(P_HEIGHT,CustomPositionOperate(true,O_REPLACE));
                    }
                }
            }
        }else if(dataLabel.contains(QStringLiteral("菜单项"))){        //菜单选项(触发二级菜单)
            GumboNodeWrapperList children = element.children();
            for(int i = 0; i< children.size(); i++){
                GumboNodeWrapper child = children.at(i);
                if(child.data_label().contains(QStringLiteral("选项"))){
                    data->m_geometryReferenceId = child.id();
                }
            }

            //只对二级菜单有效，一级菜单不需设置
            if(!element.parent().data_label().contains(QStringLiteral("右键垂直菜单"))){
                //NOTE 20201229 二级菜单项需要加上偏移量
                GumboNodeWrapperList brothers = element.parent().children();
                for(int i = 0; i < brothers.size(); i++){
                    GumboNodeWrapper tmpBro = brothers.at(i);
                    if(tmpBro.data_label().contains(QStringLiteral("外框"))){
                        if(!(tmpBro.firstChild().attribute(G_NodeHtml.SRC).isEmpty())){
                            data->m_geometryDepend.enable = true;
                            data->m_geometryDepend.dependGeometryId = tmpBro.firstChild().id();
                            data->m_geometryDepend.operates.insert(P_LEFT,CustomPositionOperate(true,O_SUB));
                            data->m_geometryDepend.operates.insert(P_TOP,CustomPositionOperate(true,O_IGNORE));
                            data->m_geometryDepend.operates.insert(P_WIDTH,CustomPositionOperate(false,O_ADD,0));
                            data->m_geometryDepend.operates.insert(P_HEIGHT,CustomPositionOperate(false,O_ADD,0));
                        }
                        break;
                    }
                }
            }
        }else if(dataLabel.contains(QStringLiteral("分割线"))){        //菜单选项(触发二级菜单)
            data->m_geometryReferenceId = element.firstChild().id();
        }else if(dataLabel.contains(QStringLiteral("模式切换"))){
            GumboNodeWrapperList children = element.children();

            CXX::ModelSwitchCodeData * codeData = new CXX::ModelSwitchCodeData;
            data->m_codeData = codeData;

            for(int i = 0; i< children.size(); i++){
                GumboNodeWrapper child = children.at(i);
                QString selectionGroup = child.attribute(G_NodeHtml.SELECTIONGROUP);
                if(!selectionGroup.isEmpty() && selectionGroup.contains(QStringLiteral("按钮"))){
                    codeData->m_modelIds.append(child.id());
                }
            }
        }else if(dataLabel.contains(QStringLiteral("分页"))){     //分页控件
            CXX::PageSwitchCodeData * pdata = new CXX::PageSwitchCodeData;
            data->m_codeData = pdata;

            GumboNodeWrapperList children = element.children();

            for(int i = 0; i< children.size(); i++){
                GumboNodeWrapper child = children.at(i);
                QString selectionGroup = child.attribute(G_NodeHtml.SELECTIONGROUP);
                if(!selectionGroup.isEmpty() && selectionGroup.contains(QStringLiteral("页码"))){
                    pdata->m_pageIds.append(child.id());
                }else if(child.data_label().contains(QStringLiteral("左翻"))){
                    GumboNodeWrapperList grandChilds = child.children();
                    for(int j = 0; j < grandChilds.size(); j++){
                        GumboNodeWrapper grand = grandChilds.at(j);
                        if(grand.data_label().contains(QStringLiteral("左"))){
                            pdata->m_leftPage = grand.id();
                        }
                    }
                }else if(child.data_label().contains(QStringLiteral("右翻"))){
                    GumboNodeWrapperList grandChilds = child.children();
                    for(int j = 0; j < grandChilds.size(); j++){
                        GumboNodeWrapper grand = grandChilds.at(j);
                        if(grand.data_label().contains(QStringLiteral("右"))){
                            pdata->m_rightPage = grand.id();
                        }
                    }
                }
            }
        }else if(dataLabel.contains(QStringLiteral("互斥按钮"))){
            GumboNodeWrapperList children = element.children();

            CXX::MutexButtonCodeData * mutexData = new CXX::MutexButtonCodeData;
            data->m_codeData = mutexData;

            for(int i = 0; i< children.size(); i++){
                GumboNodeWrapper child = children.at(i);
                mutexData->m_buttIds.append(child.id());
            }
        }else if(dataLabel.contains(QStringLiteral("二级弹出")) ||dataLabel.contains(QStringLiteral("下拉框"))
                 ||dataLabel.contains(QStringLiteral("单模块弹出"))){
            QString m_className = element.clazz();
            if(m_className.contains("hidden"))
                data->m_visible = false;
        }else if(dataLabel.contains(QStringLiteral("操作栏"))){
            GumboNodeWrapperList children = element.children();

            CXX::ButtonGroupCodeData * btnGroupData = new CXX::ButtonGroupCodeData;
            data->m_codeData = btnGroupData;

            for(int i = 0; i< children.size(); i++){
                GumboNodeWrapper child = children.at(i);
                if(child.data_label().contains(QStringLiteral("按钮")))
                    btnGroupData->m_buttIds.append(child.id());
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
