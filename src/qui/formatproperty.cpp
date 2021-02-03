#include "formatproperty.h"

#include <QDebug>
#include <QRegExp>
#include "../head.h"
#include "../util/rutil.h"
#include "props/mdomwidget.h"
#include "props/mproperty.h"
#include "props/mrow.h"
#include "props/mcolumn.h"
#include "props/mitem.h"
#include "props/mattribute.h"
#include "props/mconnection.h"
#include "props/mconnections.h"
#include "props/mcontrolimproves.h"
#include "props/mcontrolimprove.h"

namespace RQt{

QString G_NewLine = "\r\n";

FormatProperty::FormatProperty():m_conns(nullptr),m_controlImproves(nullptr)
{

}

/*!
 * @brief 更新数据源
 */
void FormatProperty::setDataSource(DomHtmlPtr ptr)
{
    m_dataSrc = ptr;
}

/*!
 * @brief 设置全局和当前页面的CSS
 * @details 20200604:因自定义控件中需要将控件A的样式挪动至控件B进行显示，因此需要对pageCss中原来属于A的移除，将B的属性改成A。
 * @param[in] globalCss 全局CSS
 * @param[in] pageCss 当前页面CSS
 */
void FormatProperty::setCssMap(CSS::CssMap globalCss, CSS::CssMap pageCss)
{
    m_globalCss = globalCss;
    m_pageCss = pageCss;
}

/*!
 * @brief 格式转换
 */
RDomWidget *FormatProperty::formart()
{
    Check_Return(m_dataSrc.isNull() || m_pageCss.size() ==0,nullptr);

    RDomWidget * rootWidget = new RDomWidget();
    Html::DomNode * root = m_dataSrc->body;
    rootWidget->setAttributeClass(getTypeName(root->m_type));
    rootWidget->setAttributeName(root->m_id);

    std::for_each(root->m_childs.begin(),root->m_childs.end(),[&](Html::DomNode * element){
        createDomWidget(rootWidget,element,QRect(QPoint(0,0),RUtil::screenSize()));
    });

    return rootWidget;
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
        case Html::RTABWIDGET_PAGE:
        case Html::RDYNAMIC_PANEL_PAGE:
        case Html::RGROUP:return QString("QWidget");break;
        case Html::RUNMENUBUTTON:
        case Html::RBUTTON:return QString("QPushButton");break;
        case Html::RDYNAMIC_PANEL:return QString("QStackedWidget");break;
        case Html::R_CUSTOM_TEXT_FIELD:
        case Html::RTEXT_FIELD:return QString("QLineEdit");break;
        case Html::RRADIO_BUTTON:return QString("QRadioButton");break;
        case Html::RTABLE:return QString("QTableWidget");break;
        case Html::RIMAGE:
        case Html::RLABEL:return QString("QLabel");break;
        case Html::RLINE:return QString("Line");break;
        case Html::RLIST_BOX:return QString("QListWidget");break;
        case Html::RTEXT_AREA:return QString("QPlainTextEdit");break;
        case Html::RCHECKBOX:return QString("QCheckBox");break;
        case Html::RDROPLIST:return QString("QComboBox");break;
        case Html::RINLINE_FRAME:
        case Html::RBOX:return QString("QLabel");break;
        case Html::RTREE:return QString("QTreeWidget");break;
        case Html::RSPINBOX:return QString("QSpinBox");break;
        case Html::RSCROLLBAR:return QString("QScrollBar");break;
        case Html::RPROGRESSBAR:return QString("QProgressBar");break;
        case Html::RSLIDER:return QString("QSlider");break;
        case Html::RTABWIDGET:return QString("QTabWidget");break;

        case Html::R_CUSTOM_RBUTTON:return QString("MyPushButton");break;
        case Html::R_CUSTOM_KEYBOARD_RFIELD:return QString("MyLineEdit");break;
        case Html::R_CUSTOM_BIDIRECTIONAL_SLIDER:return QString("MyScrollBar");break;
        case Html::R_CUSTOM_FLOATING_WINDOW:return QString("MyFloatingWindow");break;
        case Html::R_CUSTOM_FOLDINGCONTROLS:return QString("MyFoldingControl");break;
        case Html::R_CUSTOM_SWITCH_BUTTON:return QString("MySwitchButton");break;
        case Html::R_CUSTOM_SINGLE_SLIDING_BLOCK:return QString("MySingleScrollBar");break;
        case Html::R_CUSTOM_LABEL:return QString("MyLabel");break;
        case Html::R_CUSTOM_DRAWERD_CONTROL:return QString("MyFoldingControl");break;

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

    //自动代码生成
    createCodeDatas(node);

    //判断有无控件提升
    createControlImprove(node);

    //虚拟的容器无需创建UI界面，但可能携带信号槽
    if(node->m_type == Html::R_CUSTOM_VIRTUAL_CONTAINER){
        createConnections(node);
        return;
    }

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

    if(!node->m_id.isEmpty())
        m_selectorType.insert(node->m_id,node->m_type);

    switch(node->m_type){
        case Html::RCONTAINER:{
            Html::BaseData * baseData = node->m_data;

            if(!baseData->m_srcImage.isEmpty()){
                if(!baseData->m_srcImage.isEmpty())
                    createImageProp(domWidget,baseData->m_srcImage);
            }

            break;
        }
        case Html::R_CUSTOM_SINGLE_SLIDING_BLOCK:
        case Html::R_CUSTOM_BIDIRECTIONAL_SLIDER:
        case Html::RGROUP:{
            Html::GroupData * gdata = dynamic_cast<Html::GroupData*>(node->m_data);

            //NOTE 20200613 默认ui中控件都是可见的，visible属性可不用设置。
            if(!gdata->m_visible){
                createVisibleProp(domWidget,gdata->m_visible);
            }

            createConnections(node);

            //定制控件'触发弹窗'等
            if(!gdata->m_geometryReferenceId.isEmpty()){
                CSS::Rules rules = m_pageCss.value(gdata->m_geometryReferenceId).rules;
                int left = removePxUnit(findRuleByName(rules,"left").value);
                int top = removePxUnit(findRuleByName(rules,"top").value);
                int width = removePxUnit(findRuleByName(rules,"width").value);
                int height = removePxUnit(findRuleByName(rules,"height").value);

                rect = QRect(left,top,width,height);
            }else{
                rect = QRect(gdata->m_left,gdata->m_top,gdata->m_width,gdata->m_height);
            }
            break;
        }
        case Html::RDYNAMIC_PANEL:{

            Html::PanelData * panelData = dynamic_cast<Html::PanelData *>(node->m_data);

            if(!panelData->m_visible){
                createVisibleProp(domWidget,panelData->m_visible);
            }

            if(panelData->m_toolTip.size() > 0)
                createToolTipProp(domWidget,panelData->m_toolTip);

            QString imageSrc = panelData->m_srcImage;
            if(!imageSrc.isEmpty() && rect.width() < 1)
                createImageProp(domWidget,imageSrc);

            for(int i = 0; i < node->m_childs.size();i++){
                createDomWidget(domWidget,node->m_childs.at(i),rect);
            }

            if(panelData->m_dataLabel.contains(QStringLiteral("下拉按钮"))
                    ||panelData->m_dataLabel.contains(QStringLiteral("开关下拉按钮"))){
                createConnections(node);
            }

            createCurrentIndexProp(domWidget,panelData->m_currentIndex);

            break;
        }
        case Html::RLINE:{
            createEnableProp(domWidget,node->m_data->m_bDisabled);

            if(node->m_data->m_toolTip.size() > 0)
                createToolTipProp(domWidget,node->m_data->m_toolTip);

            //判断线条方向
            int width = removePxUnit(cssMap.value("width"));
            int height = removePxUnit(cssMap.value("height"));

            MProperty * directProp = new MProperty();
            MProperty * lineWidthProp = new MProperty();
            directProp->setAttributeName("orientation");
            lineWidthProp->setAttributeName("lineWidth");
            if((width > 5) || (height <= width)){
                directProp->setPropEnum("Qt::Horizontal");
                lineWidthProp->setPropNumber(QString::number(height));
            }else {
                directProp->setPropEnum("Qt::Vertical");
                lineWidthProp->setPropNumber(QString::number(width));
            }

            domWidget->addProperty(lineWidthProp);
            domWidget->addProperty(directProp);

            if(!node->m_data->m_srcImage.isEmpty())
            {
                 createImageProp(domWidget,node->m_data->m_srcImage);
            }
            break;
        }
        case Html::RTABLE:{
            Html::TableData * tdata = dynamic_cast<Html::TableData *>(node->m_data);

            //需根据表格宽度与单元格宽度相除结果，作为列数
//            int cWidth = 0;
//            if(node->m_childs.size() > 0){
//                QString subCellId = node->m_childs.first()->m_id;
//                CSS::CssSegment cellSegment = m_pageCss.value(subCellId);
//                if(cellSegment.rules.size() > 0){
//                    auto fresult = std::find_if(cellSegment.rules.begin(),cellSegment.rules.end(),[&](const CSS::CssRule & rule ){
//                        return rule.name.toLower() == "width";
//                    });

//                    if(fresult != cellSegment.rules.end()){
//                        cWidth = removePxUnit(fresult->value);
//                    }
//                }
//            }
            int maxColumnCount = 0;
            int lastTop = 0;
            for(int i = 0 ;i < node->m_childs.size(); i ++){
				CSS::CssSegment childSegment = m_pageCss.value(node->m_childs.at(i)->m_id);
                int top = removePxUnit(findRuleByName(childSegment.rules,"top").value);

                if(top != lastTop && i != 0){
                    maxColumnCount = i;
                    break;
                }
                lastTop = top;
            }

            //NOTE Axure中表格第一行和第一列作为Qt中的行、列表头
            //20210120修改表格为隐藏表格头，按行添加数据，表格的每行，每列宽度使用代码手动修改
            if(maxColumnCount > 0){
                int columnCount = maxColumnCount;
                int rowCount = node->m_childs.size() / columnCount;

                if(!tdata->m_itemId.isEmpty()){
                    tdata->m_itemId = tdata->m_itemId + "_div_" + node->m_id;
                    m_selectorType.insert(tdata->m_itemId,Html::RTABLE);
                }

                MAttribute *horizontalHeader = new MAttribute();
                horizontalHeader->setAttributeName("horizontalHeaderVisible");
                horizontalHeader->setAttributeBool("false");
                domWidget->addAttrinute(horizontalHeader);

                MAttribute *verticalHeader = new MAttribute();
                verticalHeader->setAttributeName("verticalHeaderVisible");
                verticalHeader->setAttributeBool("false");
                domWidget->addAttrinute(verticalHeader);

                QList<CXX::TabRowInfo> tabRowInfos;
                QList<CXX::TabColumnInfo> abColumnInfos;

                auto setRowHeight = [&](QString curID,int number){

                    CXX::TabRowInfo t_tabRowInfo;
                    t_tabRowInfo.m_rowNumbers = number;
                    t_tabRowInfo.m_height = removePxUnit(getCssStyle(curID,"height"));

                    return t_tabRowInfo;
                };

                auto setColumnWidth = [&](QString curID,int number){

                    CXX::TabColumnInfo t_tabColumnInfo;
                    t_tabColumnInfo.m_columnNumbers = number;
                    t_tabColumnInfo.m_width = removePxUnit(getCssStyle(curID,"width"));

                    return t_tabColumnInfo;

                };
                for(int i = 0; i < rowCount; i++){
                    for(int j = 0; j < columnCount; j++){

                        Html::CellData cell = tdata->m_cells.at(i*columnCount + j);

                        /*if(i == 0 && j == 0){
                            continue;
                        }else*/ if( i == 0){

                            abColumnInfos.append(setColumnWidth(cell.id,j));

                            MColumn  * column = new MColumn();

                            MProperty * columnProp = new MProperty();
                            columnProp->setAttributeName("text");
                            columnProp->setPropString(cell.text);
                            column->addProperty(columnProp);

                            domWidget->addColumn(column);
                        }if( j == 0){
                            MRow  * row = new MRow();

                            tabRowInfos.append(setRowHeight(cell.id,i));

                            MProperty * rowProp = new MProperty();
                            rowProp->setAttributeName("text");
                            rowProp->setPropString(cell.text);
                            row->addProperty(rowProp);

                            domWidget->addRow(row);
                        }/*else{
//                            MItem * item = new MItem();

//                            item->setAttributeRow(QString::number(i - 1));
//                            item->setAttributeColumn(QString::number(j - 1));
//                            MProperty * prop = new MProperty();
//                            prop->setAttributeName("text");
//                            prop->setPropString(cell.text);
//                            item->setProperty(prop);

//                            domWidget->addItem(item);
                        }*/

                        MItem * item = new MItem();

                        item->setAttributeRow(QString::number(i));
                        item->setAttributeColumn(QString::number(j ));
                        MProperty * prop = new MProperty();
                        prop->setAttributeName("text");
                        prop->setPropString(cell.text);
                        item->setProperty(prop);

                        domWidget->addItem(item);
                    }
                }

                CXX::TableStyleCodeData * tableData = new CXX::TableStyleCodeData();
                tableData->m_tableId = node->m_id;
                tableData->m_tabRowInfos = tabRowInfos;
                tableData->m_tabColumnInfos = abColumnInfos;

                QString m_cellWidth;
                QString m_cellHeight;

                if(!tdata->m_srcImageId.isEmpty()){
                    CSS::CssSegment cellSegment = m_pageCss.value(tdata->m_srcImageId);
                    for(int i = 0;i < cellSegment.rules.size(); i++){
                        if(cellSegment.rules.at(i).name == "width")
                            m_cellWidth = cellSegment.rules.at(i).value;
                        else if(cellSegment.rules.at(i).name == "height")
                            m_cellHeight = cellSegment.rules.at(i).value;
                    }
                    m_cellWidth = QString::number(removePxUnit(m_cellWidth));
                    m_cellHeight = QString::number(removePxUnit(m_cellHeight));
                }

                MAttribute * hSectionSize = new MAttribute();
                hSectionSize->setAttributeName("horizontalHeaderDefaultSectionSize");
                hSectionSize->setPropNumber(m_cellWidth);
                domWidget->addAttrinute(hSectionSize);

                MAttribute * vhSectionSize = new MAttribute();
                vhSectionSize->setAttributeName("verticalHeaderDefaultSectionSize");
                vhSectionSize->setPropNumber(m_cellHeight);
                domWidget->addAttrinute(vhSectionSize);

                if(!tdata->m_srcImage.isEmpty())
                    createTableCodeData(node,tdata,m_cellWidth,m_cellHeight,columnCount,tableData);

                m_codeDatas.append(tableData);
            }


            break;
        }

        case Html::RTREE:{
            Html::TreeData * treeData = dynamic_cast<Html::TreeData *>(node->m_data);

            //创建列(axure树只有一列，并且在qt ui中需要将水平表头隐藏)
            MColumn  * column = new MColumn();
            MProperty * rowProp = new MProperty();
            rowProp->setPropString(treeData->m_text);
            column->addProperty(rowProp);
            domWidget->addColumn(column);

            MAttribute * hVisibleAttribute = new MAttribute();
            hVisibleAttribute->setAttributeName("headerVisible");
            hVisibleAttribute->setAttributeBool("false");
            domWidget->addAttrinute(hVisibleAttribute);

            MProperty * animated = new MProperty();
            animated->setAttributeName("animated");
            animated->setPropBool("true");
            domWidget->addProperty(animated);

            MProperty * verticalScrollBar = new MProperty();
            verticalScrollBar->setAttributeName("verticalScrollBarPolicy");
            verticalScrollBar->setPropEnum("Qt::ScrollBarAlwaysOff");
            domWidget->addProperty(verticalScrollBar);

            MProperty * horizontalScrollBar = new MProperty();
            horizontalScrollBar->setAttributeName("horizontalScrollBarPolicy");
            horizontalScrollBar->setPropEnum("Qt::ScrollBarAlwaysOff");
            domWidget->addProperty(horizontalScrollBar);

            QString firstSimpleNodeId;
            for(int i = treeData->m_treeDatas.size() - 1; i >= 0; i--){
                Html::TreeNodeData nodeData = treeData->m_treeDatas.at(i);

                //概要信息
                MItem * item = new MItem();

                MProperty * prop = new MProperty();
                prop->setAttributeName("text");
                prop->setPropString(QString("%1 %2").arg(nodeData.timestamp).arg(nodeData.simpleInfo));
                item->setProperty(prop);

                if(firstSimpleNodeId.isEmpty())
                    firstSimpleNodeId = nodeData.simpleInfoId;

                //折叠信息
                {
                    MItem * detailItem = new MItem();

                    MProperty * detailProp = new MProperty();
                    detailProp->setAttributeName("text");
                    detailProp->setPropString(nodeData.detailInfo);
                    detailItem->setProperty(detailProp);

                    item->addItem(detailItem);
                }

                domWidget->addItem(item);
            }

            createTreeImageProp(domWidget,treeData,firstSimpleNodeId);

            break;
        }
        case Html::RTEXT_FIELD:
        case Html::R_CUSTOM_KEYBOARD_RFIELD:
        case Html::R_CUSTOM_TEXT_FIELD:{
            Html::TextFieldData * textData = dynamic_cast<Html::TextFieldData *>(node->m_data);

            createTextProp(domWidget,textData->m_text);
            createReadonlyProp(domWidget,textData->m_bReadOnly);
            createEnableProp(domWidget,textData->m_bDisabled);
            createToolTipProp(domWidget,textData->m_toolTip);

            if(!textData->m_srcImage.isEmpty()){
                createLineEditImageProp(domWidget,textData);
            }

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

        case Html::RCHECKBOX:{
            node->m_data->m_bLeftToRight = judgeLayoutDirection(node->m_id);

            createCheckedProp(domWidget,node->m_data->m_bChecked);
            createEnableProp(domWidget,node->m_data->m_bDisabled);
            createTextProp(domWidget,node->m_data->m_text);
            createLayoutDirectionProp(domWidget,node->m_data->m_bLeftToRight);

            MProperty * cursorShape = new MProperty;
            cursorShape->setAttributeName("cursor");
            cursorShape->setProCursor("PointingHandCursor");
            domWidget->addProperty(cursorShape);

            if(!node->m_data->m_partiallyCheckedImage.isEmpty()){
                createTristateProp(domWidget,true);
            }

            if(!node->m_data->m_checkedImage.isEmpty() && !node->m_data->m_unCheckedImage.isEmpty()){
                createRadioBtnImageProp(domWidget,node->m_data,"QCheckBox");
            }

            if(!node->m_data->m_specialTextId.isEmpty()){
                QString m_textId = node->m_data->m_specialTextId + "_div_" + node->m_id;
                m_selectorType.insert(m_textId,Html::RCHECKBOX);
            }

            if(node->m_data->m_toolTip.size() > 0)
                createToolTipProp(domWidget,node->m_data->m_toolTip);

            break;
        }

        case Html::RRADIO_BUTTON:{
            node->m_data->m_bLeftToRight = judgeLayoutDirection(node->m_id);

            createCheckedProp(domWidget,node->m_data->m_bChecked);
            createEnableProp(domWidget,node->m_data->m_bDisabled);
            createTextProp(domWidget,node->m_data->m_text);
            createLayoutDirectionProp(domWidget,node->m_data->m_bLeftToRight);

            MProperty * cursorShape = new MProperty;
            cursorShape->setAttributeName("cursor");
            cursorShape->setProCursor("PointingHandCursor");
            domWidget->addProperty(cursorShape);

            if(!node->m_data->m_checkedImage.isEmpty() && !node->m_data->m_unCheckedImage.isEmpty()){
                createRadioBtnImageProp(domWidget,node->m_data,"QRadioButton");
            }

            if(!node->m_data->m_specialTextId.isEmpty()){
                QString m_textId = node->m_data->m_specialTextId + "_div_" + node->m_id;
                m_selectorType.insert(m_textId,Html::RRADIO_BUTTON);
            }

            if(node->m_data->m_toolTip.size() > 0)
                createToolTipProp(domWidget,node->m_data->m_toolTip);

            break;
        }

        case Html::RIMAGE:{
            Html::ImageData * imgData = dynamic_cast<Html::ImageData *>(node->m_data);

            //src中包含了所属页面信息，目前所有images目录下直接是图片，需要移除所属页面信息
            QString imageSrc = imgData->m_src;
            if(!imageSrc.isEmpty())
                createImageProp(domWidget,imageSrc);

            createTextProp(domWidget,imgData->m_text);

            MProperty * prop = new MProperty();
            prop->setAttributeName("alignment");
            prop->setPropSet("Qt::AlignCenter");
            domWidget->addProperty(prop);

            MProperty *wordWrap = new MProperty();
            wordWrap->setAttributeName("wordWrap");
            wordWrap->setPropBool("true");
            domWidget->addProperty(wordWrap);

            if(imgData->m_toolTip.size() > 0)
                createToolTipProp(domWidget,imgData->m_toolTip);
            break;
        }

        case Html::R_CUSTOM_LABEL:
        case Html::RLABEL:{
            createEnableProp(domWidget,node->m_data->m_bDisabled);
            createTextProp(domWidget,node->m_data->m_text);

            MProperty * prop = new MProperty();
            prop->setAttributeName("alignment");
            prop->setPropSet("Qt::AlignCenter");
            domWidget->addProperty(prop);

            //添加标签的自动换行，处理自定义控件中的竖向文本
            MProperty *wordWrap = new MProperty();
            wordWrap->setAttributeName("wordWrap");
            wordWrap->setPropBool("true");
            domWidget->addProperty(wordWrap);

            //添加边距，调整标签文本显示
            QString textSize = QString("-%1").arg(QString::number(node->m_data->m_text.size()));
            MProperty *marginProp = new MProperty();
            marginProp->setAttributeName("margin");
            marginProp->setPropNumber(textSize);
            domWidget->addProperty(marginProp);

            QString imageSrc = node->m_data->m_srcImage;
            if(!imageSrc.isEmpty())
                createImageProp(domWidget,imageSrc);

            if(node->m_data->m_toolTip.size() > 0)
                createToolTipProp(domWidget,node->m_data->m_toolTip);

            if(node->m_class.contains(QStringLiteral("_一级标题")))
                m_selectorType.insert(node->m_id+"_div_32px",Html::RLABEL);
            else if(node->m_class.contains(QStringLiteral("_二级标题")))
                m_selectorType.insert(node->m_id+"_div_24px",Html::RLABEL);
            else if(node->m_class.contains(QStringLiteral("_三级标题")))
                m_selectorType.insert(node->m_id+"_div_18px",Html::RLABEL);
            else if(node->m_class.contains(QStringLiteral("_四级标题")))
                m_selectorType.insert(node->m_id+"_div_14px",Html::RLABEL);
            else if(node->m_class.contains(QStringLiteral("_五级标题")))
                m_selectorType.insert(node->m_id+"_div_13px",Html::RLABEL);
            else if(node->m_class.contains(QStringLiteral("_六级标题")))
                m_selectorType.insert(node->m_id+"_div_10px",Html::RLABEL);

            if(node->m_type == Html::R_CUSTOM_LABEL){

            }

            break;
        }
        case Html::RUNMENUBUTTON:
        case Html::RBUTTON:{

            Html::ButtonData * buttData = dynamic_cast<Html::ButtonData *>(node->m_data);

            MProperty * cursorShape = new MProperty;
            cursorShape->setAttributeName("cursor");
            cursorShape->setProCursor("PointingHandCursor");
            domWidget->addProperty(cursorShape);

            if(!buttData->m_srcImage.isEmpty()){
                createButtonImageProp(domWidget,buttData);
            }

            if(buttData->m_dataLabel.contains(QStringLiteral("标签按钮"))
                    || buttData->m_dataLabel.contains(QStringLiteral("页码"))){
                buttData->m_bChecked = false;

                createConnections(node);
            }

            createCheckedProp(domWidget,buttData->m_bChecked);
            createEnableProp(domWidget,buttData->m_bDisabled);
            createTextProp(domWidget,buttData->m_text);

            //若按钮设置了checked属性，则需要手动的添加checked:hover{}属性,使得在选中后，鼠标再移动时颜色不改变
            QString checkedSelector = node->m_id+":checked";
            CSS::CssSegment seg = m_pageCss.value(checkedSelector);
            if(seg.rules.size() > 0){
                seg.selectorName = checkedSelector + ":hover";
                m_pageCss.insert(seg.selectorName,seg);
            }

            if(buttData->m_toolTip.size() > 0)
                createToolTipProp(domWidget,buttData->m_toolTip);

            if(buttData->m_dataLabel.contains(QStringLiteral("带图标按钮"))){
                CXX::ButtonWithIcon * buttonWithIcon = dynamic_cast<CXX::ButtonWithIcon *>(node->m_data->m_codeData);

                addCustomControlBgImg(buttonWithIcon->m_srcIcon);
                buttonWithIcon->m_srcIcon = switchImageURL(buttonWithIcon->m_srcIcon);
            }

            break;
        }

        case Html::RDROPLIST:
        case Html::RLIST_BOX:{
            Html::ListData * listData = static_cast<Html::ListData *>(node->m_data);

            createEnableProp(domWidget,listData->m_bDisabled);

            if(listData->m_toolTip.size() > 0)
                createToolTipProp(domWidget,listData->m_toolTip);

            int t_selectedRow = -1;
            for(int i = 0;i < listData->m_itemList.size(); i++){
                MItem * item = new MItem();

                MProperty * prop = new MProperty();
                prop->setAttributeName("text");
                prop->setPropString(listData->m_itemList.at(i));
                item->setProperty(prop);

                domWidget->addItem(item);

                if(listData->m_selectedValue.size() > 0 && listData->m_selectedValue == listData->m_itemList.at(i)){
                    t_selectedRow = i;
                }
            }

            MProperty * currentRow = new MProperty();

            if(node->m_type == Html::RDROPLIST)
                currentRow->setAttributeName("currentIndex");
            else if(node->m_type == Html::RLIST_BOX)
                currentRow->setAttributeName("currentRow");

            currentRow->setPropNumber(QString::number(t_selectedRow));
            domWidget->addProperty(currentRow);

            if(!listData->m_srcImage.isEmpty() || !listData->m_arrowImageOn.isEmpty() || !listData->m_arrowImageSrc.isEmpty()){
                createComBoxImageProp(domWidget,listData->m_srcImage,listData->m_arrowImageSrc,listData->m_arrowImageOn);
            }

            if(!node->m_data->m_textId.isEmpty()){
                QString m_option = node->m_data->m_textId + "_div_option" + node->m_id;
                m_selectorType.insert(m_option,Html::RDROPLIST);

            }
            if(!listData->m_backGroundId.isEmpty())
            {
                QString m_back= listData->m_backGroundId + "_div_back" + node->m_id;
                m_selectorType.insert(m_back,Html::RDROPLIST);
            }
            if(!listData->m_widgetSizeId.isEmpty())
            {
                QString m_widgetSizeId = listData->m_widgetSizeId + "_div_arrow" + node->m_id;
                m_selectorType.insert(m_widgetSizeId,Html::RDROPLIST);
            }

            break;
        }

        case Html::RTEXT_AREA:{

        Html::BaseData * baseData = node->m_data;

            createReadonlyProp(domWidget,node->m_data->m_bReadOnly);

            MProperty * currentRow = new MProperty();
            currentRow->setAttributeName("plainText");
            currentRow->setPropString(node->m_data->m_text);
            domWidget->addProperty(currentRow);

            createEnableProp(domWidget,node->m_data->m_bDisabled);

            if(baseData->m_dataLabel.contains(QStringLiteral("设置"))){
                MProperty * VScrollBarHide = new MProperty();
                VScrollBarHide->setAttributeName("verticalScrollBarPolicy");
                VScrollBarHide->setPropEnum("Qt::ScrollBarAlwaysOff");
                domWidget->addProperty(VScrollBarHide);
            }

            if(node->m_data->m_toolTip.size() > 0)
                createToolTipProp(domWidget,node->m_data->m_toolTip);

            break;
        }

        case Html::RSPINBOX:{
            Html::SpinboxData *spinBox = static_cast<Html::SpinboxData *>(node->m_data);

            createSpinboxImageProp(domWidget,spinBox);

            MProperty * miniMum = new MProperty();
            miniMum->setAttributeName("minimum");
            miniMum->setPropNumber("-99");
            domWidget->addProperty(miniMum);

            break;
        }
        case Html::RSCROLLBAR:{
            Html::ScrollBarData * scrollbarData = static_cast<Html::ScrollBarData *>(node->m_data);

            QString scrollBarWidth = getCssStyle(node->m_id,"width");
            QString scrollBarHight = getCssStyle(node->m_id,"height");

            bool horizonal = (removePxUnit(scrollBarWidth) > removePxUnit(scrollBarHight) ? true : false);
            createOrientationProp(domWidget,horizonal);

            createSrollBarStyleProp(domWidget,scrollbarData,horizonal);

            break;
        }
       case Html::RSLIDER:{
            Html::SliderData * slideData = static_cast<Html::SliderData *>(node->m_data);

            createOrientationProp(domWidget,true);

            createProgressStyleProp(domWidget,slideData);

//            if(!progressBarData->m_progressBarId.isEmpty())
//            {
//                QString m_textId = progressBarData->m_progressBarId + "_div_bar" + node->m_id;
//                m_selectorType.insert(m_textId,Html::RPROGRESSBAR);
//            }

//            if(!progressBarData->m_ProgressSlotId.isEmpty())
//            {
//                QString m_textId = progressBarData->m_ProgressSlotId + "_div_slot" + node->m_id;
//                m_selectorType.insert(m_textId,Html::RPROGRESSBAR);
//            }

            break;
       }
       case Html::RTABWIDGET:{
            Html::TabWidgetData * tabData = static_cast<Html::TabWidgetData *>(node->m_data);
            if(tabData){
                MProperty * indexProp = new MProperty();
                indexProp->setAttributeName("currentIndex");
                indexProp->setPropNumber(QString::number(tabData->m_selectedPageIndex));
                domWidget->addProperty(indexProp);

                for(int i = 0;  i < node->m_childs.size(); i++){
                    createDomWidget(domWidget,node->m_childs.at(i),rect);
                }

				if(tabData->m_tabSelectedImage.isEmpty())
					createTabWidgetStyleSheet(domWidget,tabData);
				else
					createTabWidgetImageProp(domWidget,tabData);
            }
            break;
       }
        case Html::RTABWIDGET_PAGE:{
            MAttribute * hVisibleAttribute = new MAttribute();

            hVisibleAttribute->setAttributeName("title");
            hVisibleAttribute->setPropString(node->m_data->m_text);

            domWidget->addAttrinute(hVisibleAttribute);
            break;
        }
        case Html::RBOX:{
            createEnableProp(domWidget,node->m_data->m_bDisabled);

            QString imageSrc = node->m_data->m_srcImage;
            if(!imageSrc.isEmpty())
                createImageProp(domWidget,imageSrc);
            if(!node->m_data->m_text.isEmpty())
                createTextProp(domWidget,node->m_data->m_text);

            if(node->m_data->m_toolTip.size() > 0)
                createToolTipProp(domWidget,node->m_data->m_toolTip);

            break;
        }
        case Html::R_CUSTOM_RBUTTON:{
            Html::ButtonData * buttData = dynamic_cast<Html::ButtonData *>(node->m_data);

            MProperty * cursorShape = new MProperty;
            cursorShape->setAttributeName("cursor");
            cursorShape->setProCursor("PointingHandCursor");
            domWidget->addProperty(cursorShape);

            if(!buttData->m_srcImage.isEmpty()){
                createCustomButtonImageProp(domWidget,buttData);
            }

            createTextProp(domWidget,buttData->m_text);

                break;
           }

        default:break;
    }

    //20200604:对自定义控件样式进行合并处理
    processNodeStyle(node);

    if(node->m_childs.size() > 0 && node->m_type != Html::RTABLE && node->m_type != Html::RDYNAMIC_PANEL
            &&node->m_type != Html::RTABWIDGET){
        std::for_each(node->m_childs.begin(),node->m_childs.end(),[&](Html::DomNode * element){
            createDomWidget(domWidget,element,rect);
        });
    }
}

/*!
 * @brief 对每个控件样式进行特殊处理
 * @attention 【此操作会改变样式表中的内容，使得相关控件的样式被移除、修改】
 *              示例：在自定义控件中”输入框“，设计时采用组合模式拼接成一个输入框，但样式信息不在text filed控件中，而在其兄弟节点里，需要将其兄弟节点的样式拿过来，将名称替换成
 *              text field的相关信息，才可使用。
 * @param[in] 待处理节点
 */
void FormatProperty::processNodeStyle(Html::DomNode *node)
{
    if(node->m_data && node->m_data->m_referenceIds.size() > 0){
        QStringList referenceIds = node->m_data->m_referenceIds;

        switch(node->m_type){
            case Html::R_CUSTOM_TEXT_FIELD:{
                //在css解析时，已经将XX_div.selected替换成了XX:checked和XX:disabled
                QString selectedSuffix = ":checked";
                QString disabledSuffix = ":disabled";

                QString referenceId = referenceIds.at(0);

                //找出所有参考ID的样式,并用当前ID等价替换
                if(m_pageCss.contains(referenceId)){
                    m_pageCss.remove(referenceId);
                }

                QString selectedId = referenceId + selectedSuffix;
                if(m_pageCss.contains(selectedId)){
                    QString newSelectedId = node->m_id + ":hover";      //QLineEdit无checked属性，用hover代替
                    m_pageCss.operator [](selectedId).selectorName = newSelectedId;

                    m_pageCss.insert(newSelectedId,m_pageCss.value(selectedId));
                    m_pageCss.remove(selectedId);
                }

                QString disabledId = referenceId + disabledSuffix;
                if(m_pageCss.contains(disabledId)){
                    QString newDisabledId = node->m_id + disabledSuffix;
                    m_pageCss.operator [](disabledId).selectorName = newDisabledId;

                    m_pageCss.insert(newDisabledId,m_pageCss.value(disabledId));
                    m_pageCss.remove(disabledId);
                }

                //移除当前本身的样式(text filed不包含样式信息)
                m_pageCss.remove(node->m_id);
                QString inputId = node->m_id + "_input";

                CSS::CssRule colorRule;
                if(m_pageCss.contains(inputId)){
                    CSS::Rules inputIdStyle = m_pageCss.operator [](inputId).rules;
                    colorRule = findRuleByName(inputIdStyle,"color");
                }

                m_pageCss.remove(inputId);
				m_pageCss.remove(node->m_id + "_div");			//NOTE 20201230 在axure9中，比axure8多出来一个_div节点

                QString divId = referenceId + "_div";
                if(m_pageCss.contains(divId)){
                    m_pageCss.operator [](divId).selectorName = node->m_id;
                    m_pageCss.operator [](divId).rules.append(colorRule);

                    m_pageCss.insert(node->m_id,m_pageCss.value(divId));
                }

                break;
            }
            default:break;
        }
    }
}

/*!
 * @brief 分别从全局和当前页面样式中，将当前节点相关样式整合
 * @attention 1.根据class提取全局样式;
 *            2.根据class和id提取当前页面样式;
 *            3.若设置了样式参考ID，则拉取参考ID的的样式(针对定制化控件)
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

    //[3] 若设置了参考ID，则从参考ID中拉取样式作为己方的样式
    if(node->m_data && !node->m_data->m_referenceIds.isEmpty()){
        for(QString referenceId : node->m_data->m_referenceIds){
            addCss(m_pageCss.value(referenceId),finalRuleMap);
        }
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
    //处理尺寸依赖的计算
    auto processGeometryReference = [&](Html::DomNode * tmpNode,QRect & rectResult){
        Html::GeometryReferenceDesc geoDesc = tmpNode->m_data->m_geometryDepend;
        CSS::CssSegment dependCSS = m_pageCss.value(geoDesc.dependGeometryId);

        QMap<QString,QString> dependMap;
        foreach(const CSS::CssRule & crule,dependCSS.rules){
            dependMap.insert(crule.name,crule.value);
        }

        double dLeft = removePxUnit(dependMap.value("left"));
        double dTop = removePxUnit(dependMap.value("top"));
        double dWidth = removePxUnit(dependMap.value("width"));
        double dHeight = removePxUnit(dependMap.value("height"));

        auto calcVal = [](int myVal,int referenceVal,Html::CustomPositionOperate & operate){
            double val = myVal;

            double opereateVal = operate.useReferenceGeo ? referenceVal : operate.customGeoVal;

            if(operate.opereate == Html::O_ADD){
                val = myVal + opereateVal;
            }else if(operate.opereate == Html::O_SUB){
                val = myVal - opereateVal;
            }else if(operate.opereate == Html::O_REPLACE){
                val = opereateVal;
            }

            return val;
        };

        if(geoDesc.operates.contains(Html::P_LEFT)){
            Html::CustomPositionOperate operate = geoDesc.operates.value(Html::P_LEFT);
            rectResult.setX(calcVal(rectResult.x(),dLeft,operate));
        }

        if(geoDesc.operates.contains(Html::P_TOP)){
            Html::CustomPositionOperate operate = geoDesc.operates.value(Html::P_TOP);
            if(operate.opereate != Html::O_OFFSET){
                rectResult.setY(calcVal(rectResult.y(),dTop,operate));
            }else{
                rectResult.moveTopLeft(QPoint(rectResult.x(),rectResult.top() - dTop));
            }
        }

        if(geoDesc.operates.contains(Html::P_WIDTH)){
            Html::CustomPositionOperate operate = geoDesc.operates.value(Html::P_WIDTH);
            rectResult.setWidth(calcVal(rectResult.width(),dWidth,operate));
        }

        if(geoDesc.operates.contains(Html::P_HEIGHT)){
            Html::CustomPositionOperate operate = geoDesc.operates.value(Html::P_HEIGHT);
            rectResult.setHeight(calcVal(rectResult.height(),dHeight,operate));
        }
    };

    QRect rect(removePxUnit(cssMap.value("left")),removePxUnit(cssMap.value("top")),
               removePxUnit(cssMap.value("width")),removePxUnit(cssMap.value("height")));

    if(node->m_type == Html::RGROUP){
        Html::GroupData * gdata = dynamic_cast<Html::GroupData*>(node->m_data);
        if(!gdata->m_geometryReferenceId.isEmpty()){
            CSS::Rules rules = m_pageCss.value(gdata->m_geometryReferenceId).rules;
            int left = removePxUnit(findRuleByName(rules,"left").value);
            int top = removePxUnit(findRuleByName(rules,"top").value);
            int width = removePxUnit(findRuleByName(rules,"width").value);
            int height = removePxUnit(findRuleByName(rules,"height").value);

            rect = QRect(left,top,width,height);

            if(node->m_data->m_geometryDepend.enable){
                processGeometryReference(node,rect);
            }
        }else{
            Html::GroupData * gdata = dynamic_cast<Html::GroupData*>(node->m_data);
            rect = QRect(gdata->m_left,gdata->m_top,gdata->m_width,gdata->m_height);
        }
    }

    //转换成父窗口的相对坐标
    //NOTE dynamic panel中每个子页面容器已经是相对位置，不必计算相对位置。
    //tab widget中属性不在css中，因此获取的位置信息为0
    if(node->m_type != Html::RDYNAMIC_PANEL_PAGE && node->m_type != Html::RTABWIDGET){
        rect.moveLeft(rect.x() - parentRect.x());
        rect.moveTop(rect.y() - parentRect.y());
    }

    //自制动态面板只有位置没有大小
    if(node->m_type == Html::RDYNAMIC_PANEL && rect.width() < 1)
    {
        Html::PanelData * panelData = dynamic_cast<Html::PanelData *>(node->m_data);

        //定制化控件“输入框”:取第一个孩子面板中的width、height属性，作为自身的属性
        if(panelData->m_panelDataLabel.contains(QStringLiteral("输入框"))){
            if(node->m_childs.size() > 0){
                CSS::CssSegment childSegment = m_pageCss.value(node->m_childs.first()->m_id);
                rect.setWidth(removePxUnit(findRuleByName(childSegment.rules,"width").value));
                rect.setHeight(removePxUnit(findRuleByName(childSegment.rules,"height").value));
                return rect;
            }
        }

        if(!panelData->m_sonPanelStateId.isEmpty())
        {
            QString twidth = getCssStyle(panelData->m_sonPanelStateId,"width");
            QString theight = getCssStyle(panelData->m_sonPanelStateId,"height");

            rect.setWidth(removePxUnit(twidth));
            rect.setHeight(removePxUnit(theight));
        }
        //获取自制动态面板下第一个组合控件的尺寸位置
        else if(!panelData->m_srcImageId.isEmpty()){
            QString twidth = getCssStyle(panelData->m_srcImageId,"width");
            QString theight = getCssStyle(panelData->m_srcImageId,"height");
            QString timgx = getCssStyle(panelData->m_srcImageId,"left");

            rect.setWidth(removePxUnit(twidth));
            rect.setHeight(removePxUnit(theight));

            //获取两种控件组合的情况下，第二个控件位置尺寸
            if(!panelData->m_panelTextId.isEmpty())
            {
                QString textWidth = getCssStyle(panelData->m_panelTextId,"width");
                QString textheight = getCssStyle(panelData->m_panelTextId,"height");
                QString textX = getCssStyle(panelData->m_panelTextId,"left");

                //获取自定义复选框，单选框，半选中复选框等特殊自定义控件位置尺寸
                if((panelData->m_panelDataLabel.contains(QStringLiteral("复选"))) ||(panelData->m_panelDataLabel.contains(QStringLiteral("单选")))){
                    rect.setWidth((removePxUnit(timgx) > removePxUnit(textX)) ? (removePxUnit(twidth) + removePxUnit(timgx)) : (removePxUnit(textWidth) + removePxUnit(textX)));
                    rect.setHeight((removePxUnit(theight) > removePxUnit(textheight)) ? removePxUnit(theight) : removePxUnit(textheight));
                }
                //获取两个组合控件是包含关系时的位置尺寸
                else
                {
                   QRect textRect = rect;
                   textRect.setWidth(removePxUnit(textWidth));
                   textRect.setHeight(removePxUnit(textheight));

                   if(!rect.contains(textRect,false)){
                       rect = textRect;
                   }
                }
            }
        }
    }
    //下拉按钮等自制控件动态面板大小依据按钮和弹窗设置大小
    if(node->m_type == Html::RDYNAMIC_PANEL && !node->m_data->m_childPageID.isEmpty()){

        int twidth = removePxUnit(getCssStyle(node->m_data->m_childPageID,"width"));
        int theight = removePxUnit(getCssStyle(node->m_data->m_childPageID,"height"));

        rect.setWidth(twidth > node->m_data->m_width ? twidth: node->m_data->m_width);
        rect.setHeight(theight > node->m_data->m_height ? theight : theight + node->m_data->m_height);

    }
    //修改RIMAGE和RLABEL类型的位置尺寸为背景图片的
    else if(node->m_type == Html::RIMAGE || node->m_type == Html::RLABEL)
    {
        if(!node->m_data->m_srcImageId.isEmpty()){
            QString twidth = getCssStyle(node->m_data->m_srcImageId,"width");
            QString theight = getCssStyle(node->m_data->m_srcImageId,"height");
            QString tleft = getCssStyle(node->m_data->m_srcImageId,"left");
            QString ttop = getCssStyle(node->m_data->m_srcImageId,"top");

            rect.setLeft(rect.left() + removePxUnit(tleft));
            rect.setTop(rect.top() + removePxUnit(ttop));
            rect.setWidth(removePxUnit(twidth));
            rect.setHeight(removePxUnit(theight));
        }

    }

    else if(node->m_type == Html::RRADIO_BUTTON || node->m_type == Html::RCHECKBOX)
    {
        if(rect.width() == 0 || rect.height() == 0){
            if(node->m_data->m_width != 0 && node->m_data->m_height != 0){
                rect.setWidth(node->m_data->m_width);
                rect.setHeight(node->m_data->m_height);
            }else{
                QString textId = node->m_data->m_textId;
                rect.setWidth(removePxUnit(getCssStyle(textId,"width")) + 20);
                rect.setHeight(removePxUnit(getCssStyle(textId,"height")));
            }
        }
    }
    else if(node->m_type == Html::RDROPLIST)
    {
        if(!node->m_data->m_widgetSizeId.isEmpty())
        {
            QString textId = node->m_data->m_widgetSizeId;
            QRect dropListRect(removePxUnit(getCssStyle(textId,"left")),removePxUnit(getCssStyle(textId,"top")),
                       removePxUnit(getCssStyle(textId,"width")),removePxUnit(getCssStyle(textId,"height")));
            rect = dropListRect;
        }
    }
    else if(node->m_type == Html::RSPINBOX)
    {
        Html::SpinboxData * gdata = dynamic_cast<Html::SpinboxData*>(node->m_data);
        rect = QRect(gdata->m_left,gdata->m_top,gdata->m_width,gdata->m_height);
    }
    else if(node->m_type == Html::RSLIDER)
    {
        Html::SliderData * gdata = dynamic_cast<Html::SliderData*>(node->m_data);
        rect = QRect(gdata->m_left - parentRect.left(),gdata->m_top - parentRect.top(),gdata->m_width,gdata->m_height);
    }
    else if(node->m_type == Html::RSCROLLBAR)
    {
        Html::ScrollBarData * gdata = dynamic_cast<Html::ScrollBarData*>(node->m_data);
        rect = QRect(gdata->m_left,gdata->m_top - parentRect.top(),gdata->m_width,gdata->m_height);
    }
    else if(node->m_type == Html::RTABWIDGET)
    {
        Html::TabWidgetData * tabData = dynamic_cast<Html::TabWidgetData *>(node->m_data);
        if(tabData){
            rect.setTop(tabData->m_top - parentRect.y());
            rect.setWidth(tabData->m_width);
            rect.setHeight(tabData->m_height);
        }
    }else if(node->m_type == Html::RTREE){
        Html::TreeData * treeData = dynamic_cast<Html::TreeData *>(node->m_data);
        rect = QRect(treeData->m_left,treeData->m_top,treeData->m_width,treeData->m_height);
    }else if(node->m_type == Html::RCONTAINER){
        //NOTE 20201229 解决“右键垂直菜单”尺寸由子控件“外框”和背景图片两者计算产生
        if(node->m_data->m_geometryDepend.enable){

            processGeometryReference(node,rect);
        }
    }else if(node->m_type == Html::R_CUSTOM_RBUTTON){
        Html::ButtonData * buttonData = dynamic_cast<Html::ButtonData *>(node->m_data);
        if(buttonData){
            rect.setWidth(buttonData->m_width);
            rect.setHeight(buttonData->m_height);
        }
    }else if(node->m_type == Html::R_CUSTOM_KEYBOARD_RFIELD){
        Html::TextFieldData * inputBoxData = dynamic_cast<Html::TextFieldData *>(node->m_data);
        if(inputBoxData){

            QString twidth = getCssStyle(inputBoxData->m_textId,"width");
            QString theight = getCssStyle(inputBoxData->m_textId,"height");

            rect.setWidth(removePxUnit(twidth));
            rect.setHeight(removePxUnit(theight));
        }
    }else if(node->m_type == Html::R_CUSTOM_BIDIRECTIONAL_SLIDER
		|| node->m_type == Html::R_CUSTOM_SINGLE_SLIDING_BLOCK){
        Html::GroupData * customScrolldata = dynamic_cast<Html::GroupData *>(node->m_data);
        if(customScrolldata){

            rect.setLeft(customScrolldata->m_left + rect.left());
            rect.setTop(customScrolldata->m_top + rect.top());
            rect.setWidth(customScrolldata->m_width);
            rect.setHeight(customScrolldata->m_height);

        }
    }else if(node->m_type == Html::R_CUSTOM_SWITCH_BUTTON){
        Html::GroupData * button = dynamic_cast<Html::GroupData *>(node->m_data);
        if(button){

            QString twidth = getCssStyle(button->m_checkedId,"width");
            QString theight = getCssStyle(button->m_checkedId,"height");

            if(rect.width() <= 0)
                rect.setWidth(removePxUnit(twidth));
            if(rect.height() <=0)
                rect.setHeight(removePxUnit(theight));

        }
    }else if(node->m_type == Html::R_CUSTOM_LABEL){
        Html::BaseData * customLabelData = dynamic_cast<Html::BaseData *>(node->m_data);
        if(customLabelData){

            QString twidth = getCssStyle(customLabelData->m_srcImageId,"width");
            QString theight = getCssStyle(customLabelData->m_srcImageId,"height");

            rect.setWidth(removePxUnit(twidth));
            rect.setHeight(removePxUnit(theight));

        }
    }else if(node->m_type == Html::R_CUSTOM_DRAWERD_CONTROL){
        Html::GroupData * gdata = dynamic_cast<Html::GroupData*>(node->m_data);
        rect = QRect(gdata->m_left,gdata->m_top,gdata->m_width,gdata->m_height);
    }



    if(rect.left() < 0){
        rect.setWidth(rect.width() - rect.left());
        rect.setLeft(0);
    }
    if(rect.top() < 0){
        rect.setHeight(rect.height() - rect.top());
        rect.setTop(0);
    }

    getMaxWindowSize(rect);

    return rect;
}

/**
 * @brief 获取当前窗口的最大尺寸
 * @return较大的尺寸
 */
void FormatProperty::getMaxWindowSize(QRect rect)
{
    m_maxWindowSize.setHeight(m_maxWindowSize.height() > (rect.top() + rect.height()) ? m_maxWindowSize.height() : (rect.top() + rect.height()));
    m_maxWindowSize.setWidth(m_maxWindowSize.width() > (rect.left() + rect.width()) ? m_maxWindowSize.width() : (rect.left() + rect.width()));
}

int FormatProperty::removePxUnit(QString valueWithUnit)
{
    if(valueWithUnit.contains("px")){
        return valueWithUnit.left(valueWithUnit.indexOf("px")).toInt();
    }
    return valueWithUnit.toInt();
}

void FormatProperty::createImageProp(RDomWidget *domWidget, QString imageSrc)
{
    imageSrc = switchImageURL(imageSrc);

    if(!imageSrc.isEmpty()){
        MProperty * styleProp = new MProperty();
        styleProp->setAttributeName("styleSheet");
        styleProp->setPropString(QString("border-image:url(:/%1);").arg(imageSrc));
        domWidget->addProperty(styleProp);
    }
}

/*!
 * @attention 按钮若设置了背景图片，则需要添加正常背景、悬停、按下、选中四种状态背景；
 *            若按钮被选中，其背景图片已经加入了_selected，在计算其它状态图片时，需先移除
 */
void FormatProperty::createButtonImageProp(RDomWidget *domWidget, Html::ButtonData *baseData)
{
    QString normalImageSrc;
    QString mouseOverImageSrc;
    QString mouseDownImageSrc;
    QString mouseSelectedImageSrc;

    if(baseData->m_bChecked){
        QString selectedSuffix = "_selected";

        QStringList selectedNameList = baseData->m_srcImage.split(".");
        if(selectedNameList.size() == 2){
            QString selectedName = selectedNameList.at(0);
            QString normalName = selectedName.remove(selectedSuffix,Qt::CaseInsensitive);

            normalImageSrc = switchImageURL(normalName +"." + selectedNameList.at(1));
            mouseOverImageSrc = switchImageURL(normalName + "_mouseOver." + selectedNameList.at(1));
            mouseDownImageSrc = switchImageURL(normalName + "_mouseDown." + selectedNameList.at(1));

            if(baseData->m_srcImage.contains(selectedSuffix)){
                mouseSelectedImageSrc = switchImageURL(baseData->m_srcImage);
            }else{
                mouseSelectedImageSrc = switchImageURL(normalName + selectedSuffix + "." + selectedNameList.at(1));
            }
        }
    }else{
        normalImageSrc = switchImageURL(baseData->m_srcImage);
        //hover状态在正常状态加入_mouseOver、_mouseDown、_selected
        QStringList normalNameList = baseData->m_srcImage.split(".");
        if(normalNameList.size() == 2){
            mouseOverImageSrc = switchImageURL(normalNameList.at(0) + "_mouseOver." + normalNameList.at(1));
            mouseDownImageSrc = switchImageURL(normalNameList.at(0) + "_mouseDown." + normalNameList.at(1));
            mouseSelectedImageSrc = switchImageURL(normalNameList.at(0) + "_selected." + normalNameList.at(1));
        }
    }

    MProperty * styleProp = new MProperty();
    styleProp->setAttributeName("styleSheet");


    QString prop = QString("QPushButton {border-image: url(:/%1);}" + G_NewLine)
                        .arg(normalImageSrc);

    if(baseData->m_needMouseOver){
        prop += QString("QPushButton:hover {border-image: url(:/%1);}" + G_NewLine).arg(mouseOverImageSrc);
    }

    if(baseData->m_needMousePressed){
        prop += QString("QPushButton:pressed {border-image: url(:/%1)}" + G_NewLine).arg(mouseDownImageSrc);
    }

    if(baseData->m_needMouseChecked){
        prop += QString("QPushButton:checked {border-image: url(:/%1);}").arg(mouseSelectedImageSrc);
    }

    styleProp->setPropString(prop);

    domWidget->addProperty(styleProp);
}

/*!
 * @attention 按钮若设置了背景图片，则需要添加正常背景、悬停、按下、选中四种状态背景；
 *            若按钮被选中，其背景图片已经加入了_selected，在计算其它状态图片时，需先移除
 */
void FormatProperty::createCustomButtonImageProp(RDomWidget *domWidget, Html::ButtonData *baseData)
{
    QString normalImageSrc;
    QString mouseSelectedImageSrc;

    normalImageSrc = switchImageURL(baseData->m_srcImage);
    mouseSelectedImageSrc = switchImageURL(baseData->m_checkedImage);


    MProperty * styleProp = new MProperty();
    styleProp->setAttributeName("styleSheet");


    QString prop = QString("QPushButton {border-image: url(:/%1);}" + G_NewLine)
                        .arg(normalImageSrc);

    prop += QString("QPushButton:checked {border-image: url(:/%1);}").arg(mouseSelectedImageSrc);

    styleProp->setPropString(prop);

    domWidget->addProperty(styleProp);
}

/*!
 * @attention 部分自定义输入框如果设置了背景图片就手动添加
 */
void FormatProperty::createLineEditImageProp(RDomWidget *domWidget, Html::TextFieldData *baseData)
{
    QString normalImageSrc;

    normalImageSrc = switchImageURL(baseData->m_srcImage);

    MProperty * styleProp = new MProperty();
    styleProp->setAttributeName("styleSheet");


    QString prop = QString("QLineEdit {border-image: url(:/%1);}" + G_NewLine)
                        .arg(normalImageSrc);

    styleProp->setPropString(prop);

    domWidget->addProperty(styleProp);
}

void FormatProperty::createRadioBtnImageProp(RDomWidget *domWidget,Html::BaseData * baseData,QString widgetName)
{
    QString checkImageSrc = switchImageURL(baseData->m_checkedImage);
    QString unCheckImageSrc = switchImageURL(baseData->m_unCheckedImage);
    QString middleCheckImageSrc = switchImageURL(baseData->m_partiallyCheckedImage);

    if(!checkImageSrc.isEmpty()||!unCheckImageSrc.isEmpty()){
        MProperty * styleProp = new MProperty();

        QStringList normalNameList = baseData->m_unCheckedImage.split(".");
        QString mouseOverImageSrc = baseData->m_unCheckedImage;

        //hover状态在正常状态加入_mouseOver
        if(normalNameList.size() == 2){
            mouseOverImageSrc = switchImageURL(normalNameList.at(0) + "_mouseOver."+normalNameList.at(1));
        }

        styleProp->setAttributeName("styleSheet");

        QString prop = QString("%1::indicator:checked {image: url(:/%2);}" + G_NewLine +
                               "%3::indicator:unchecked {image: url(:/%4);}" + G_NewLine +
                               "%5::indicator:unchecked:hover {image: url(:/%6);}" + G_NewLine)
                       .arg(widgetName).arg(checkImageSrc)
                       .arg(widgetName).arg(unCheckImageSrc)
                       .arg(widgetName).arg(mouseOverImageSrc);

        //checkbox设置了半选中状态图片
        if(!baseData->m_partiallyCheckedImage.isEmpty()){
            prop += QString("%1::indicator:indeterminate {image: url(:/%2);}").arg(widgetName).arg(middleCheckImageSrc);
        }

        styleProp->setPropString(prop);

        domWidget->addProperty(styleProp);
    }
}

/*!
 * @brief 创建QTabWidget中tab相关的样式属性
 * @attention 包括选中、mouseover时以及未选中时状态
 * @param[in] tabData tab页面数据createTabWidgetStyleSheet
 */
void FormatProperty::createTabWidgetImageProp(RDomWidget *domWidget, Html::TabWidgetData * tabData)
{
    //提取’选项卡‘中tab的样式信息
    CSS::CssSegment tabStyleSegment = m_pageCss.value(tabData->m_tabBarId);

    QString tabStyle;
    for(CSS::CssRule rule : tabStyleSegment.rules){
        if(rule.name != "left" && rule.name != "top"){
            tabStyle += QString("%1:%2;").arg(rule.name).arg(rule.value);
        }
    }

    QString normalImageSrc = switchImageURL(tabData->m_tabNormalImage);
    QString selectedImageSrc = switchImageURL(tabData->m_tabSelectedImage);
    QString tabRightBorderImageSrc = switchImageURL(tabData->m_tabRightImage);

    if(!normalImageSrc.isEmpty() && !selectedImageSrc.isEmpty()){
        QStringList normalNameList = tabData->m_tabNormalImage.split(".");
        QString mouseOverImageSrc = normalImageSrc;
        //hover状态在正常状态加入_mouseOver
        if(normalNameList.size() == 2){
            mouseOverImageSrc = switchImageURL(normalNameList.at(0) + "_mouseOver."+normalNameList.at(1));
        }

        MProperty * styleProp = new MProperty();
        styleProp->setAttributeName("styleSheet");

        //QTabBar::tab无border-image属性
        styleProp->setPropString(QString("QTabWidget::pane{ border: none;}" + G_NewLine +
                                         "QTabBar::tab:selected,QTabBar::tab:selected:hover{border-image: url(':/%1');}" + G_NewLine +
                                         "QTabBar::tab:!selected{border-image: url(':/%2');}" + G_NewLine +
                                         "QTabBar::tab:!selected:hover{border-image: url(':/%3');}" + G_NewLine +
                                         "QTabBar::tab{%4;}")
                                 .arg(selectedImageSrc).arg(normalImageSrc).arg(mouseOverImageSrc).arg(tabStyle));
        domWidget->addProperty(styleProp);
    }
}

/*!
 * @brief 创建QTabWidget中tab相关的样式属性
 * @attention 包括选中、mouseover时以及未选中时状态
 * @param[in] tabData tab页面数据createTabWidgetStyleSheet
 */
void FormatProperty::createTabWidgetStyleSheet(RDomWidget *domWidget, Html::TabWidgetData * tabData)
{
	//提取’选项卡‘中tab的样式信息
	QStringList tabStyleIDList;
	auto iter = m_pageCss.begin();
	while (iter != m_pageCss.end()){
		QString name = iter.key();
		if (name.contains(tabData->m_tabBarId))
			tabStyleIDList << name;
		iter++;
	}

	CSS::CssSegment tabStyleSegment = m_pageCss.value(tabData->m_tabBackStyleID);
	CSS::CssSegment tabStyleSegment_over;
	CSS::CssSegment tabStyleSegment_select;
	CSS::CssSegment tabStyleSegment_pressed;


	for (QString tabStyleID : tabStyleIDList) {
		if (tabStyleID.contains(":hover"))
			tabStyleSegment_over = m_pageCss.value(tabStyleID);
		else if(tabStyleID.contains(":pressed"))
			tabStyleSegment_pressed = m_pageCss.value(tabStyleID);
		else if(tabStyleID.contains(":checked"))
			tabStyleSegment_select = m_pageCss.value(tabStyleID);
	}

	auto getTabStateStyle = [&](CSS::CssSegment styleSegment) {
		QString style;
		for (CSS::CssRule rule : styleSegment.rules) {
			if (rule.name != "left" && rule.name != "top" && !rule.name.contains("box-shadow")) {
				if (rule.value.contains("gradient"))
					rule.value = gradientSwitchToQss(rule.value);
				style += QString("%1:%2;").arg(rule.name).arg(rule.value);
			}
		}
		return style;
	};

	MProperty * styleProp = new MProperty();
	styleProp->setAttributeName("styleSheet");

	styleProp->setPropString(QString("QTabWidget::pane{ border: none;}" + G_NewLine +
		"QTabBar::tab:selected,QTabBar::tab:selected:hover{%1}" + G_NewLine +
		"QTabBar::tab:!selected{%2}" + G_NewLine +
		"QTabBar::tab:!selected:hover{%3}" + G_NewLine +
		"QTabBar::tab:pressed{%4}" + G_NewLine +
		"QTabBar::tab{%5}")
		.arg(getTabStateStyle(tabStyleSegment_select)).arg(getTabStateStyle(tabStyleSegment))
		.arg(getTabStateStyle(tabStyleSegment_over)).arg(getTabStateStyle(tabStyleSegment_pressed))
		.arg(getTabStateStyle(tabStyleSegment)));
	domWidget->addProperty(styleProp);
}
/**
* @brief 获取简单的渐变背景
*/
QString FormatProperty::gradientSwitchToQss(QString value)
{
    QStringList list = value.split(QRegExp("\\s+"));
	QStringList m_rgbList;
	QString t_rgb;

	QStringList rangeList;
	QString rangel;

	QString gradientDirection;
	std::for_each(list.begin(), list.end(), [&](QString graduatedInfo) {
		if (graduatedInfo.contains("gradient"))
		{
			if (graduatedInfo.contains(","))
			{
				graduatedInfo = graduatedInfo.remove(QChar(','), Qt::CaseInsensitive);
				QStringList t_filtration = graduatedInfo.split("(");
				std::for_each(t_filtration.begin(), t_filtration.end(), [&](QString filtrationDirection) {
					if (filtrationDirection.contains("deg"))
					{
						gradientDirection = filtrationDirection;
					}
				});
			}
		}
		else if (graduatedInfo.contains("%"))
		{
			if (graduatedInfo.contains(")"))
			{
				graduatedInfo.remove(QChar(')'), Qt::CaseInsensitive);
			}
			rangel = rangel + graduatedInfo;
		}
		else
		{
			if (graduatedInfo.contains(")"))
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
	std::for_each(rangeList.begin(), rangeList.end(), [&](QString graduatedRange) {
		if (graduatedRange.contains("%"))
		{
			float rangeFloat = (graduatedRange.left(graduatedRange.indexOf("%")).toInt()) / float(100);
			rangeFloats.append(QString::number(rangeFloat));
		}
	});
	QString graduatedInfo;
	if (rangeFloats.size() == m_rgbList.size())
	{
		for (int i = 0; i < rangeFloats.size(); i++)
		{
			QString t_stopFloat = rangeFloats.at(i);
			QString t_rgbInfo = m_rgbList.at(i);
			QString t_graduatedInfo = QString(" stop:%1 %2,").arg(t_stopFloat).arg(t_rgbInfo);
			graduatedInfo += t_graduatedInfo;
		}
	}

	int xDirection;
	int yDirection;
	if ((gradientDirection == "90deg") || (gradientDirection == "270deg"))
	{
		xDirection = 1;
		yDirection = 0;
	}
	else if ((gradientDirection == "180deg") || (gradientDirection == "0deg"))
	{
		xDirection = 0;
		yDirection = 1;
	}
	value = QString(" qlineargradient(spread:pad, x1:0, y1:0, x2:%1, y2:%2, %3)").arg(QString::number(xDirection))
		.arg(QString::number(yDirection)).arg(graduatedInfo);
	return value;
}
void FormatProperty::createComBoxImageProp(RDomWidget *domWidget, QString imageSrc, QString arrowImage, QString unArrowImage)
{
    imageSrc = switchImageURL(imageSrc);
    arrowImage = switchImageURL(arrowImage);
    unArrowImage = switchImageURL(unArrowImage);

    if(!imageSrc.isEmpty() || !arrowImage.isEmpty() || !unArrowImage.isEmpty()){
        MProperty * styleProp = new MProperty();
        styleProp->setAttributeName("styleSheet");
        styleProp->setPropString(QString("QComboBox{border-image:url(:/%1);}" + G_NewLine +
                                         "QComboBox QAbstractItemView {border-image: url(:/%2);}" + G_NewLine +
                                         "QComboBox::down-arrow {image: url(:/%3);}" + G_NewLine +
                                         "QComboBox::down-arrow:on {image: url(:/%4);}")
                                 .arg(imageSrc).arg(imageSrc).arg(arrowImage).arg(unArrowImage));

        domWidget->addProperty(styleProp);
    }
}

void FormatProperty::createSpinboxImageProp(RDomWidget *domWidget, Html::SpinboxData *data)
{
    QString upArrow = switchImageURL(data->m_upArrowImage);
    QString downArrow = switchImageURL(data->m_downArrowImage);
    QString upArrowOver = switchImageURL(data->m_upArrowMouseOverImage);
    QString downArrowOver = switchImageURL(data->m_downArrowMouseOverImage);

	CSS::CssSegment divCss = m_pageCss.value(data->m_spinBoxId + "_div");
	CSS::CssSegment inputCss = m_pageCss.value(data->m_sinBoxTextId + "_input");

	CSS::CssSegment divCssHover = m_pageCss.value(data->m_spinBoxId + ":hover");

	//NOTE 20201230 此处CSS样式表中包含多个border-width属性，取最后设置那个
	CSS::Rules rules = findRulesByName(divCss.rules, "border-width");
	QString borderWidth;
	if (rules.size() > 0)
		borderWidth = rules.last().value;

	QString prop;
	prop += QString("QSpinBox{background-color:%1; border-color:%2;border-width:%3;color:%4;border-style:%5}" + G_NewLine)
			.arg(switchCssRgbaToQt(findRuleByName(divCss.rules, "background-color").value))
			.arg(switchCssRgbaToQt(findRuleByName(divCss.rules, "border-color").value))
			.arg(borderWidth)
			.arg(findRuleByName(inputCss.rules,"color").value)
			.arg(findRuleByName(divCss.rules, "border-style").value);

	prop += QString("QSpinBox:hover{border-color:%1;border-width:%2;border-style:%3;}" + G_NewLine)
			.arg(switchCssRgbaToQt(findRuleByName(divCssHover.rules, "border-color").value))
			.arg(borderWidth)
			.arg(findRuleByName(divCssHover.rules, "border-style").value);

	//上下箭头
	MProperty * styleProp = new MProperty();
	styleProp->setAttributeName("styleSheet");
    if(!upArrow.isEmpty() && !downArrow.isEmpty() && !upArrowOver.isEmpty() && !downArrowOver.isEmpty()){
		prop += QString("QSpinBox::up-arrow {image:url(:/%1);}" + G_NewLine +
                                         "QSpinBox::up-arrow:hover {image: url(:/%2);}" + G_NewLine +
                                         "QSpinBox::down-arrow {image: url(:/%3);}" + G_NewLine +
                                         "QSpinBox::down-arrow:hover {image: url(:/%4);}")
                                 .arg(upArrow).arg(upArrowOver).arg(downArrow).arg(downArrowOver);
    }else{
		prop += QString("QSpinBox::up-arrow {width:0px;}" + G_NewLine +
                                         "QSpinBox::down-arrow {width:0px;}" + G_NewLine);
    }

	styleProp->setPropString(prop);

	domWidget->addProperty(styleProp);
}

void FormatProperty::createProgressStyleProp(RDomWidget *domWidget, Html::SliderData *data)
{
    MProperty * styleProp = new MProperty();
    styleProp->setAttributeName("styleSheet");

    QString prop;

    QString direction = "horizontal";

    //sub-line
    {
        QString barId = data->m_progressBarId + "_div";
        CSS::CssSegment seg = m_pageCss.value(barId);

        prop += QString("QSlider::sub-page:%1 {background: %2; height: %3;border-radius: 1px;padding-left:-1px;padding-right:-1px;}" + G_NewLine)
                .arg(direction)
                .arg(switchCssRgbaToQt(findRuleByName(seg.rules,"background-color").value))
                .arg(findRuleByName(seg.rules,"height").value);
    }

    //若grooove的高度小于handle高度，那么handle操作不方便
    QString handleHeight = 0;
    //handle
    {
        CSS::CssSegment seg = m_pageCss.value(data->m_handleId);

        handleHeight = findRuleByName(seg.rules,"height").value;

        prop += QString("QSlider::handle:%1 {image: url(:/%2); width:%3; height: %4; margin:%5; border-radius: 1px; padding:0px %6px;}" + G_NewLine)
                .arg(direction)
                .arg(switchImageURL(data->m_srcImage))
                .arg(findRuleByName(seg.rules,"width").value)
                .arg(handleHeight)
                .arg(QString("-%1px").arg(removePxUnit(handleHeight)/2))
                .arg(removePxUnit(handleHeight)/2 + 1);


        seg = m_pageCss.value(data->m_pressedHandleId);

        QString tmpHeight = findRuleByName(seg.rules,"height").value;

        prop += QString("QSlider::handle:%1:hover {image: url(:/%2); width:%3; height: %4; margin:%5; border-radius: 1px;}" + G_NewLine)
                .arg(direction)
                .arg(switchImageURL(data->m_checkedImage))
                .arg(findRuleByName(seg.rules,"width").value)
                .arg(tmpHeight)
                .arg(QString("-%1px").arg(removePxUnit(tmpHeight)/2));
    }

    //add-line和groove
    {
        QString slotId = data->m_progressSlotId + "_div";
        CSS::CssSegment seg = m_pageCss.value(slotId);

        prop += QString("QSlider::groove:%1 {background: %2; height: %3;border-radius: 1px;padding-left:-1px;padding-right:-1px;}" + G_NewLine)
                .arg(direction)
                .arg(switchCssRgbaToQt(findRuleByName(seg.rules,"background-color").value))
                .arg(findRuleByName(seg.rules,"height").value);
    }


    styleProp->setPropString(prop);

    domWidget->addProperty(styleProp);
}

void FormatProperty::createSrollBarStyleProp(RDomWidget *domWidget, Html::ScrollBarData *data,bool horizonal)
{
    MProperty * styleProp = new MProperty();
    styleProp->setAttributeName("styleSheet");

    QString prop;

    QString direction = "horizontal";
    if(!horizonal)
        direction = "vertical";

    QString subLineWidth;

    //向上按钮
    {
        QString barId = data->m_subLineId + "_div";
        CSS::CssSegment seg = m_pageCss.value(barId);

        subLineWidth = findRuleByName(seg.rules,"width").value;

        prop += QString("QScrollBar::sub-line:%1 {border: 0px;background: %2; width:%3; height:%4 ; subcontrol-origin: margin; image: url(:/%5);}" + G_NewLine)
                .arg(direction)
                .arg(switchCssGradientToQt(findRulesByName(seg.rules,"background")))
                .arg(findRuleByName(seg.rules,"width").value)
                .arg(findRuleByName(seg.rules,"height").value)
                .arg(switchImageURL(data->m_upArrowImage));

        barId = data->m_subLineId + ":hover";
        seg = m_pageCss.value(barId);

        prop += QString("QScrollBar::sub-line:%1:hover {border: 0px;background: %2; width:%3; height:%4 ; subcontrol-origin: margin; image: url(:/%5);}" + G_NewLine)
                .arg(direction)
                .arg(switchCssGradientToQt(findRulesByName(seg.rules,"background")))
                .arg(findRuleByName(seg.rules,"width").value)
                .arg(findRuleByName(seg.rules,"height").value)
                .arg(switchImageURL(data->m_upArrowImage));

        barId = data->m_subLineId + ":pressed";
        seg = m_pageCss.value(barId);

        prop += QString("QScrollBar::sub-line:%1:pressed {border: 0px;background: %2; width:%3; height:%4 ; subcontrol-origin: margin; image: url(:/%5);}" + G_NewLine)
                .arg(direction)
                .arg(switchCssGradientToQt(findRulesByName(seg.rules,"background")))
                .arg(findRuleByName(seg.rules,"width").value)
                .arg(findRuleByName(seg.rules,"height").value)
                .arg(switchImageURL(data->m_upArrowImage));

    }

    //向下按钮
    {
        QString barId = data->m_addLineId + "_div";
        CSS::CssSegment seg = m_pageCss.value(barId);

        subLineWidth = findRuleByName(seg.rules,"width").value;

        prop += QString("QScrollBar::add-line:%1 {border: 0px;background: %2; width:%3; height:%4;subcontrol-origin: margin; image: url(:/%5);}" + G_NewLine)
                .arg(direction)
                .arg(switchCssGradientToQt(findRulesByName(seg.rules,"background")))
                .arg(findRuleByName(seg.rules,"width").value)
                .arg(findRuleByName(seg.rules,"height").value)
                .arg(switchImageURL(data->m_downArrowImage));

        barId = data->m_addLineId + ":hover";
        seg = m_pageCss.value(barId);

        prop += QString("QScrollBar::add-line:%1:hover {border: 0px;background: %2; width:%3; height:%4;subcontrol-origin: margin; image: url(:/%5);}" + G_NewLine)
                .arg(direction)
                .arg(switchCssGradientToQt(findRulesByName(seg.rules,"background")))
                .arg(findRuleByName(seg.rules,"width").value)
                .arg(findRuleByName(seg.rules,"height").value)
                .arg(switchImageURL(data->m_downArrowImage));

        barId = data->m_addLineId + ":pressed";
        seg = m_pageCss.value(barId);

        prop += QString("QScrollBar::add-line:%1:pressed {border: 0px;background: %2; width:%3; height:%4;subcontrol-origin: margin; image: url(:/%5);}" + G_NewLine)
                .arg(direction)
                .arg(switchCssGradientToQt(findRulesByName(seg.rules,"background")))
                .arg(findRuleByName(seg.rules,"width").value)
                .arg(findRuleByName(seg.rules,"height").value)
                .arg(switchImageURL(data->m_downArrowImage));
    }

    //滑动槽
    {
        QString barId = data->m_scrollSlotId + "_div";
        CSS::CssSegment seg = m_pageCss.value(barId);

        QString margin;
        if(horizonal){
            margin = QString("0 %1 0 %2").arg(subLineWidth).arg(subLineWidth);
        }else{
            margin = QString("%1 0 %2 0").arg(subLineWidth).arg(subLineWidth);
        }

        prop += QString("QScrollBar:%1 {border: 0px;background: %2; width:%3;margin:%4}" + G_NewLine)
                .arg(direction)
                .arg(switchCssRgbaToQt(findRuleByName(seg.rules,"background-color").value))
                .arg(findRuleByName(seg.rules,"width").value)
                .arg(margin);
    }

    //滑块
    {
        QString barId = data->m_scrollBarId + "_div";
        CSS::CssSegment seg = m_pageCss.value(barId);

        prop += QString("QScrollBar::handle:%1 {border: 0px;background: %2; border-radius:%4 ;}" + G_NewLine)
                .arg(direction)
                .arg(switchCssGradientToQt(findRulesByName(seg.rules,"background")))
                .arg(findRuleByName(seg.rules,"border-radius").value);

        barId = data->m_scrollBarId + ":hover";
        seg = m_pageCss.value(barId);

        prop += QString("QScrollBar::handle:%1:hover {border: 0px;background: %2; width:%3; border-radius:%4 ;}" + G_NewLine)
                .arg(direction)
                .arg(switchCssGradientToQt(findRulesByName(seg.rules,"background")))
                .arg(findRuleByName(seg.rules,"width").value)
                .arg(findRuleByName(seg.rules,"border-radius").value);

        barId = data->m_scrollBarId + ":pressed";
        seg = m_pageCss.value(barId);

        prop += QString("QScrollBar::handle:%1:pressed {border: 0px;background: %2; width:%3; border-radius:%4 ;}" + G_NewLine)
                .arg(direction)
                .arg(switchCssGradientToQt(findRulesByName(seg.rules,"background")))
                .arg(findRuleByName(seg.rules,"width").value)
                .arg(findRuleByName(seg.rules,"border-radius").value);
    }

    //可控制滑槽不显示纹理
    prop += " QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical {background: none;}";


    styleProp->setPropString(prop);

    domWidget->addProperty(styleProp);
}

void FormatProperty::createTreeImageProp(RDomWidget *domWidget, Html::TreeData *data,QString textColorId)
{
    CSS::CssSegment simpleTextRules = m_pageCss.value(textColorId);

    QStringList selectedNameList = data->m_srcImage.split(".");
    if(selectedNameList.size() == 2){
        QString selectedName = selectedNameList.at(0);

        QString normalItemImage = switchImageURL(data->m_srcImage);
        QString mouseOverImageSrc = switchImageURL(selectedName + "_mouseOver." + selectedNameList.at(1));

        MProperty * styleProp = new MProperty();
        styleProp->setAttributeName("styleSheet");
        styleProp->setPropString(QString("QTreeView::item {border-image: url(:/%1);}" + G_NewLine +
                                         "QTreeView::item:hover {border-image: url(:/%2);}" + G_NewLine +
                                         "QTreeView::item:selected:hover {}" + G_NewLine +
                                         "QTreeView{background-color:rgba(0,0,0,0);border:none;color:%3;}"
                                         )
                                 .arg(normalItemImage).arg(mouseOverImageSrc).arg(findRuleByName(simpleTextRules.rules,"color").value));

        domWidget->addProperty(styleProp);
    }
}

/*!
  @details 因Qt4和Qt5在ui中对表格的样式支持不一致，为了保持统一，取消从ui中直接设置样式， @n
           转而使用代码控制
 */
void FormatProperty::createTableCodeData(Html::DomNode *node, Html::TableData *data, QString hSectionSize, QString vSectionSize,
                                         int columnCount, CXX::TableStyleCodeData *tableData)
{
    if(data->m_cells.size() < columnCount){
        return;
    }

    QString imageSrc = switchImageURL(data->m_srcImage);

    Html::CellData conrnerCell = data->m_cells.first();
    QString cornerSrc = switchImageURL(conrnerCell.imageSrc);

    if(!imageSrc.isEmpty()){

//        CXX::TableStyleCodeData * tableData = new CXX::TableStyleCodeData();

        tableData->m_tableId = node->m_id;

        {
            tableData->m_tableStyle = QString("QTableWidget{border-image:url(:/%1);} QTableView QTableCornerButton::section {border-image: url(:/%2);}")
                    .arg(imageSrc).arg(cornerSrc);
        }

        //水平表头
        {
            Html::CellData horizontalSectionCell = data->m_cells.at(2);
            QString sectionTextColor = getCssStyle(horizontalSectionCell.id,"color");
            QString horizonalSectionImageSrc = switchImageURL(horizontalSectionCell.imageSrc);

            QStringList normalNameList = horizontalSectionCell.imageSrc.split(".");
            QString mouseOverImageSrc;
            if(normalNameList.size() == 2){
                mouseOverImageSrc = switchImageURL(normalNameList.at(0) + "_mouseOver." + normalNameList.at(1));
            }

            tableData->m_horizontalStyle = QString("QHeaderView::section:horizonal{width:%1px;height:%2px;color:%3;border-image: url(:/%4);} QHeaderView::section:horizonal:checked {border-image: url(:/%5);}")
                    .arg(hSectionSize).arg(vSectionSize).arg(sectionTextColor).arg(horizonalSectionImageSrc)
                    .arg(mouseOverImageSrc);
        }

        //竖直表头
        {
            Html::CellData verticalSectionCell = data->m_cells.at(columnCount);
            QString sectionTextColor = getCssStyle(verticalSectionCell.id,"color");
            QString horizonalSectionImageSrc = switchImageURL(verticalSectionCell.imageSrc);

            QStringList normalNameList = verticalSectionCell.imageSrc.split(".");
            QString mouseOverImageSrc;
            if(normalNameList.size() == 2){
                mouseOverImageSrc = switchImageURL(normalNameList.at(0) + "_mouseOver." + normalNameList.at(1));
            }

            tableData->m_verticalStyle = QString("QHeaderView::section:vertical{width:%1px;height:%2px;color:%3;border-image: url(:/%4);} QHeaderView::section:vertical:checked {border-image: url(:/%5);}")
                    .arg(hSectionSize).arg(vSectionSize).arg(sectionTextColor).arg(horizonalSectionImageSrc)
                    .arg(mouseOverImageSrc);
        }

//        m_codeDatas.append(tableData);
    }
}

/*!
 * @brief 1.记录原始图片路径；2.去除图片中包含axure页面的名称，使其路径在images/xxx.png
 * @param[in] imageSrc 原始图片路径1
 */
QString FormatProperty::switchImageURL(QString imageSrc)
{
    if(imageSrc.isEmpty())
        return imageSrc;

    m_originalResources.append(imageSrc);

    int firstSplitPos = imageSrc.indexOf("/");
    int secondSplitPos = imageSrc.indexOf("/",firstSplitPos + 1);

    imageSrc = imageSrc.remove(firstSplitPos,secondSplitPos - firstSplitPos);

    if(!m_resources.contains(imageSrc))
        m_resources.append(imageSrc);

    return imageSrc;
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

void FormatProperty::createVisibleProp(RDomWidget *domWidget, bool visible)
{
    MProperty * visibleProp = new MProperty;
    visibleProp->setAttributeName("visible");
    visibleProp->setPropBool(visible?"true":"false");
    domWidget->addProperty(visibleProp);
}

void FormatProperty::createEnableProp(RDomWidget *domWidget, bool disable)
{
    MProperty * enableProp = new MProperty;
    enableProp->setAttributeName("enabled");
    enableProp->setPropBool(disable?"false":"true");
    domWidget->addProperty(enableProp);
}

void FormatProperty::createTristateProp(RDomWidget *domWidget, bool tristate)
{
    MProperty * triProp = new MProperty;
    triProp->setAttributeName("tristate");
    triProp->setPropBool(tristate?"true":"false");
    domWidget->addProperty(triProp);
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

void FormatProperty::createOrientationProp(RDomWidget *domWidget, bool horizonal)
{
    MProperty * layoutProp = new MProperty;
    layoutProp->setAttributeName("orientation");
    layoutProp->setPropEnum(horizonal?"Qt::Horizontal":"Qt::Vertical");
    domWidget->addProperty(layoutProp);
}

void FormatProperty::createToolTipProp(RDomWidget *domWidget, QString toolTip)
{
    if(toolTip.size() != 0){
        MProperty * tipProp = new MProperty;
        tipProp->setAttributeName("toolTip");
        tipProp->setPropString(toolTip);
        domWidget->addProperty(tipProp);
    }
}

void FormatProperty::createCurrentIndexProp(RDomWidget *domWidget,int currentIndex)
{
    MProperty * layoutProp = new MProperty;
    layoutProp->setAttributeName("currentIndex");
    layoutProp->setPropNumber(QString::number(currentIndex));
    domWidget->addProperty(layoutProp);
}

/*!
 * @brief 判断控件布局方向
 * @param[in] selectorId 控件选择器Id
 * @return true:从左至右布局，false:从右至左布局
 */
bool FormatProperty::judgeLayoutDirection(QString selectorId)
{
    //WARNING 不同Axure版本生成checkBox的格式存在一定的差异
    QString textId;
    QString tmpTextId = selectorId + "_text";
    QString selector = selectorId;
    if(m_pageCss.contains(tmpTextId)){
        textId = tmpTextId;
    }else{
        textId = "u" + QString::number(selectorId.remove("u").toInt() + 1);
    }

    //获取checkbox中id为X_text的样式属性，用其与当前控件的left属性比较，判断方向
    QString textLeftVal = getCssStyle(textId,"left");

    if(textLeftVal.toLower().contains("px")){
        textLeftVal = textLeftVal.toLower().remove("px");
    }

    //Axure9.0.0版本以选择框的背景id为准，而不是input
    QString inputId;
    QString tmpImgId = selector + "_img";
    if(m_pageCss.contains(tmpImgId)){
        inputId = tmpImgId;
    }else{
        inputId = selector + "_input";
    }
    QString inputLeftVal = getCssStyle(inputId,"left");
    if(inputLeftVal.toLower().contains("px")){
        inputLeftVal = inputLeftVal.toLower().remove("px");
    }

    if(textLeftVal.toInt() < inputLeftVal.toInt())
         return false;

    return true;
}

/*!
 * @brief 获取指定选择器的指定属性
 * @param[in] selectorName 选择器名
 * @param[in] propName 属性名
 * @return 若存在属性则返回，否则返回空
 */
QString FormatProperty::getCssStyle(QString selectorName, QString propName)
{
    CSS::Rules rs = m_pageCss.value(selectorName).rules;

    for(int i = 0;i < rs.size(); i++){
        if(rs.at(i).name == propName){
            return rs.at(i).value;
        }
    }

    return QString();
}

/*!
 * @brief 根据属性名在属性集合中查找对应的属性
 * @param[in] rules 待查找集合列表
 * @param[in] ruleName 待查找的属性名
 * @return 若查找则返回对应的属性记录，否则返回一个空的
 */
CSS::CssRule FormatProperty::findRuleByName(CSS::Rules &rules, QString ruleName)
{
    auto iresult = std::find_if(rules.begin(),rules.end(),[&ruleName](const CSS::CssRule & rule){
        return rule.name == ruleName;
    });

    if(iresult != rules.end()){
        return *iresult;
    }

    return CSS::CssRule();
}

CSS::Rules FormatProperty::findRulesByName(CSS::Rules &rules, QString ruleName)
{
    CSS::Rules result;

    std::for_each(rules.begin(),rules.end(),[&](const CSS::CssRule & rule){
        if(rule.name == ruleName){
            result.append(rule);
        }
    });

    return result;
}

/*!
 * @brief 根据属性名替换属性集合中的原始属性
 * @param[in] rules 待替换集合列表
 * @param[in] ruleName 待替换的属性名
 * @param[in] newRule 待替换的属性
 */
void FormatProperty::replaceRuleByName(CSS::Rules &rules, QString ruleName, CSS::CssRule newRule)
{
    std::replace_if(rules.begin(),rules.end(),[&](const CSS::CssRule & rule){
        return rule.name == ruleName;
    },newRule);
}

/**
 * @brief 设置自动生成代码。
 * @details 自定义控件需要代码辅助完成一定功能
 * @param node
 */
void FormatProperty::createCodeDatas(Html::DomNode *node)
{
    auto setLocation = [&](QString m_id){
        CXX::Location t_location;
        t_location.m_width = removePxUnit(getCssStyle(m_id,"width"));
        t_location.m_height = removePxUnit(getCssStyle(m_id,"height"));
        t_location.m_top = removePxUnit(getCssStyle(m_id,"top"));
        t_location.m_left = removePxUnit(getCssStyle(m_id,"left"));

        return t_location;
    };

    auto setBaseInfo = [&](CXX::BaseInfo &baseInfo,Html::NodeType controlType){

        baseInfo.m_location = setLocation(baseInfo.m_ID);

		QString styleID = baseInfo.m_ID + "_div";

        if(controlType != Html::RINVALID)
            m_selectorType.insert(baseInfo.m_ID,controlType);

        if(!baseInfo.m_srcImg.isEmpty()){
			styleID = baseInfo.m_ID;
            addCustomControlBgImg(baseInfo.m_srcImg);
            baseInfo.m_srcImg = switchImageURL(baseInfo.m_srcImg);
        }

        CSS::CssSegment t_segment;
        if(m_pageCss.contains(styleID)){
            t_segment = m_pageCss.value(styleID);
        }
        for(CSS::CssRule rule : t_segment.rules){
            if (rule.name == "color")
                baseInfo.m_textInfo.m_color = rule.value;
            if (rule.name.contains("background-color"))
                baseInfo.m_style = rule.value;
            if(rule.name.contains("font-family"))
                baseInfo.m_textInfo.m_font = rule.value;
        }

        return baseInfo;
    };

    auto setBaseInfos = [&](QList<CXX::BaseInfo> &baseInfos){
        for(int i = 0; i < baseInfos.size(); i++){
            CXX::BaseInfo baseInfo = baseInfos.at(i);
            baseInfos.replace(i,setBaseInfo(baseInfo,Html::RLABEL));
        }
    };

    if(node->m_data && node->m_data->m_codeData){
        //处理手动添加的背景图片路径问题
        if(node->m_type == Html::R_CUSTOM_RBUTTON){
            CXX::DropDownButtonData * dropDownBtnData = dynamic_cast<CXX::DropDownButtonData *>(node->m_data->m_codeData);

            setBaseInfo(dropDownBtnData->m_triggerBtnClose.m_iconInfo,Html::RINVALID);
            setBaseInfo(dropDownBtnData->m_triggerBtnClose.m_buttonInfo,Html::RINVALID);

            setBaseInfo(dropDownBtnData->m_triggerBtnOpen.m_iconInfo,Html::RINVALID);
            setBaseInfo(dropDownBtnData->m_triggerBtnOpen.m_buttonInfo,Html::RINVALID);

            setBaseInfo(dropDownBtnData->m_optionPopUp.m_border.m_baseInfo,Html::RLABEL);
            setBaseInfo(dropDownBtnData->m_optionPopUp.m_titleInfo.m_baseInfo,Html::RLABEL);

            auto dealWithBtnInfo = [&](CXX::ButtonGroup &curBtnGroup){

                for(int i = 0; i < curBtnGroup.m_buttons.size(); i++){
                    CXX::ButtonInfo buttonInfo = curBtnGroup.m_buttons.at(i);
                    buttonInfo.m_baseInfo = setBaseInfo(buttonInfo.m_baseInfo,Html::RBUTTON);

                    curBtnGroup.m_buttons.replace(i,buttonInfo);
                }
            };

            dealWithBtnInfo(dropDownBtnData->m_optionPopUp.m_optionBtnGroup);
            dealWithBtnInfo(dropDownBtnData->m_optionPopUp.m_exitBtnGroup);


        }else if(node->m_type == Html::R_CUSTOM_KEYBOARD_RFIELD){
            //将自定义控件用到的样式ID包保留，将背景图片路径转换到生成路径。
            CXX::KeyBoardInputBoxData * inputTextBoxData = dynamic_cast<CXX::KeyBoardInputBoxData *>(node->m_data->m_codeData);
            inputTextBoxData->m_triggerInputBox.m_baseAtrribute.m_srcImage = switchImageURL(inputTextBoxData->m_triggerInputBox.m_baseAtrribute.m_srcImage);
            m_selectorType.insert(inputTextBoxData->m_triggerInputBox.m_baseAtrribute.m_ID,Html::RTEXT_FIELD);

            inputTextBoxData->m_virtualKeyGroup.m_baseAtrribute.m_srcImage = switchImageURL(inputTextBoxData->m_virtualKeyGroup.m_baseAtrribute.m_srcImage);
            m_selectorType.insert(inputTextBoxData->m_virtualKeyGroup.m_baseAtrribute.m_ID,Html::RBOX);

            m_selectorType.insert(inputTextBoxData->m_virtualKeyGroup.m_ID,Html::RBOX);

            inputTextBoxData->m_virtualKeyGroup.m_title.m_baseAtrribute.m_srcImage = switchImageURL(inputTextBoxData->m_virtualKeyGroup.m_title.m_baseAtrribute.m_srcImage);
            m_selectorType.insert(inputTextBoxData->m_virtualKeyGroup.m_title.m_baseAtrribute.m_ID,Html::RTEXT_FIELD);

            inputTextBoxData->m_virtualKeyGroup.m_baseAtrribute.m_location = setLocation(inputTextBoxData->m_virtualKeyGroup.m_baseAtrribute.m_ID);

            inputTextBoxData->m_virtualKeyGroup.m_title.m_location = setLocation(inputTextBoxData->m_virtualKeyGroup.m_title.m_baseAtrribute.m_ID);

            auto changeImg = [&](QList<CXX::BUTTON> &buttonsInfos){
                for(int i = 0; i < buttonsInfos.size(); i++){
                    CXX::BUTTON t_buttnInfo = buttonsInfos.at(i);
                    //设置按钮位置坐标
                    CXX::Location t_location;
                    t_location.m_width = removePxUnit(getCssStyle(t_buttnInfo.m_baseAtrribute.m_ID,"width"));
                    t_location.m_height = removePxUnit(getCssStyle(t_buttnInfo.m_baseAtrribute.m_ID,"height"));
                    t_location.m_top = removePxUnit(getCssStyle(t_buttnInfo.m_baseAtrribute.m_ID,"top"));
                    t_location.m_left = removePxUnit(getCssStyle(t_buttnInfo.m_baseAtrribute.m_ID,"left"));

                    t_buttnInfo.m_location = t_location;

                    m_selectorType.insert(t_buttnInfo.m_baseAtrribute.m_ID,Html::RBUTTON);
                    addCustomControlBgImg(t_buttnInfo.m_baseAtrribute.m_srcImage);
                    t_buttnInfo.m_baseAtrribute.m_srcImage = switchImageURL(t_buttnInfo.m_baseAtrribute.m_srcImage);
                    if(t_buttnInfo.deleteBtn){
                        t_buttnInfo.m_baseAtrribute.m_iconInfo.m_location  = setLocation(t_buttnInfo.m_baseAtrribute.m_iconInfo.m_ID);
                        t_buttnInfo.m_baseAtrribute.m_iconInfo.m_srcIcon = switchImageURL(t_buttnInfo.m_baseAtrribute.m_iconInfo.m_srcIcon);
                    }

                    buttonsInfos.replace(i,t_buttnInfo);

                }
            };

            inputTextBoxData->m_virtualKeyGroup.m_keyGroup.m_baseAtrribute.m_srcImage = switchImageURL(inputTextBoxData->m_virtualKeyGroup.m_keyGroup.m_baseAtrribute.m_srcImage);
            m_selectorType.insert(inputTextBoxData->m_virtualKeyGroup.m_keyGroup.m_baseAtrribute.m_ID,Html::RBOX);
            changeImg(inputTextBoxData->m_virtualKeyGroup.m_keyGroup.m_keyBtns);

            inputTextBoxData->m_virtualKeyGroup.m_tailGroup.m_baseAtrribute.m_srcImage = switchImageURL(inputTextBoxData->m_virtualKeyGroup.m_tailGroup.m_baseAtrribute.m_srcImage);
            m_selectorType.insert(inputTextBoxData->m_virtualKeyGroup.m_tailGroup.m_baseAtrribute.m_ID,Html::RBOX);
            changeImg(inputTextBoxData->m_virtualKeyGroup.m_tailGroup.m_quitBtns);

        }else if(node->m_type == Html::R_CUSTOM_BIDIRECTIONAL_SLIDER){
            CXX::BidirectionalSlider * customScrollBarData = dynamic_cast<CXX::BidirectionalSlider *>(node->m_data->m_codeData);

            setBaseInfos(customScrollBarData->m_rectangularsInfo);

            setBaseInfos(customScrollBarData->m_leftScrollBar.m_srcImgs);

            setBaseInfos(customScrollBarData->m_rightScrollBar.m_srcImgs);

            if(customScrollBarData->m_leftScrollBar.m_location.m_width <= 0){
                customScrollBarData->m_leftScrollBar.m_location = setLocation(customScrollBarData->m_leftScrollBar.m_id);
            }

            customScrollBarData->m_leftToolTip.m_location = setLocation(customScrollBarData->m_leftToolTip.m_ID);
            setBaseInfo(customScrollBarData->m_leftToolTip.m_textBox,Html::RLABEL);

            customScrollBarData->m_rightToolTip.m_location = setLocation(customScrollBarData->m_rightToolTip.m_ID);
            setBaseInfo(customScrollBarData->m_rightToolTip.m_textBox,Html::RLABEL);

        }else if(node->m_type == Html::R_CUSTOM_FLOATING_WINDOW){
            CXX::FloatingWindow * floatingWindowData = dynamic_cast<CXX::FloatingWindow *>(node->m_data->m_codeData);

            floatingWindowData->m_location = setLocation(floatingWindowData->m_ID);
            setBaseInfos(floatingWindowData->m_floatButton.m_srcImgs);//悬浮按钮
            floatingWindowData->m_mainWidget.m_location = setLocation(floatingWindowData->m_mainWidget.m_id);
            floatingWindowData->m_mainWidget.m_srcImg = switchImageURL(floatingWindowData->m_mainWidget.m_srcImg);

            for(int i = 0; i < floatingWindowData->m_mainWidget.m_switchButtons.size();i++){
                CXX::SwitchButton t_switchButton = floatingWindowData->m_mainWidget.m_switchButtons.at(i);
                setBaseInfo(t_switchButton.m_closeState,Html::RINVALID);
                setBaseInfo(t_switchButton.m_openState,Html::RINVALID);

                t_switchButton.m_location = setLocation(t_switchButton.m_id);

                floatingWindowData->m_mainWidget.m_switchButtons.replace(i,t_switchButton);
            }

        }else if(node->m_type == Html::R_CUSTOM_FOLDINGCONTROLS || node->m_type == Html::R_CUSTOM_DRAWERD_CONTROL){
            CXX::FoldingControls * foldingControlData = dynamic_cast<CXX::FoldingControls *>(node->m_data->m_codeData);

            if(node->m_type == Html::R_CUSTOM_DRAWERD_CONTROL){

                QString backImgID = node->m_data->m_srcImageId + "_div";

                if(m_pageCss.contains(backImgID)){

                    QString newID = node->m_id;
                    m_pageCss.operator [](backImgID).selectorName = newID;

                    m_pageCss.insert(newID,m_pageCss.value(backImgID));

                }
            }

            if(foldingControlData->m_location.m_width <= 0)
                foldingControlData->m_location = setLocation(foldingControlData->m_ID);

            for(int i = 0; i < foldingControlData->m_informations.size();i++){
                CXX::Information t_information = foldingControlData->m_informations.at(i);
                setBaseInfos(t_information.m_foldingInfo.m_information);
                setBaseInfos(t_information.m_unFoldInfo.m_information);

                t_information.m_unFoldInfo.m_parentLocation = setLocation(t_information.m_unFoldInfo.m_parentID);

                foldingControlData->m_informations.replace(i,t_information);
            }

        }else if(node->m_type == Html::R_CUSTOM_SWITCH_BUTTON){
            CXX::customSwitchButton * customSwitchBtn = dynamic_cast<CXX::customSwitchButton *>(node->m_data->m_codeData);
            setBaseInfo(customSwitchBtn->m_closeState,Html::RBUTTON);
            setBaseInfo(customSwitchBtn->m_openState,Html::RBUTTON);


            auto setStyle = [&](QString oldID,QString newID){
                if(m_pageCss.contains(oldID)){
                    m_pageCss.operator [](oldID).selectorName = newID;

                    m_pageCss.insert(newID,m_pageCss.value(oldID));
                    m_pageCss.remove(oldID);
                }
            };

            QString newCheckedID = customSwitchBtn->m_closeState.m_ID + ":checked";
            setStyle(customSwitchBtn->m_openState.m_ID,newCheckedID);

            customSwitchBtn->m_location = setLocation(customSwitchBtn->m_ID);

            QString firstLabelID;

            if(customSwitchBtn->m_lineEdits.size() > 0)
                setBaseInfos(customSwitchBtn->m_lineEdits);
            if(customSwitchBtn->m_textParagraphs.size() > 0){
                firstLabelID = customSwitchBtn->m_textParagraphs.at(0).m_ID;
                setBaseInfos(customSwitchBtn->m_textParagraphs);
            }

            /*!< 默认使用第一个文本段落的样式作为输入框的样式 */
            for(CXX::BaseInfo t_baseInfo : customSwitchBtn->m_lineEdits){

                if(m_pageCss.contains(firstLabelID)){
                    m_pageCss.operator [](firstLabelID).selectorName = t_baseInfo.m_ID;

                    m_pageCss.insert(t_baseInfo.m_ID,m_pageCss.value(firstLabelID));
                }
            }

        }else if(node->m_type == Html::R_CUSTOM_SINGLE_SLIDING_BLOCK){

            CXX::SingleSlidingBlockData * customSingleScrollBar = dynamic_cast<CXX::SingleSlidingBlockData *>(node->m_data->m_codeData);
//            customSingleScrollBar->m_leftScrollBar.m_location = setLocation(customSingleScrollBar->m_leftScrollBar.m_id);
            setBaseInfos(customSingleScrollBar->m_leftScrollBar.m_srcImgs);

            customSingleScrollBar->m_letfToolTip.m_location = setLocation(customSingleScrollBar->m_letfToolTip.m_ID);
            setBaseInfo(customSingleScrollBar->m_letfToolTip.m_textBox,Html::RLABEL);

            setBaseInfos(customSingleScrollBar->m_rectangularsInfo);


            customSingleScrollBar->m_inputBox.m_location = setLocation(customSingleScrollBar->m_inputBox.m_ID);
            setBaseInfos(customSingleScrollBar->m_inputBox.m_inputBoxInfos);

        }else if(node->m_type == Html::R_CUSTOM_LABEL){

            CXX::CustomLabelData * customLabelData = dynamic_cast<CXX::CustomLabelData *>(node->m_data->m_codeData);
            setBaseInfo(customLabelData->m_defaultInfo,Html::RLABEL);
            setBaseInfo(customLabelData->m_mouseEnter,Html::RLABEL);

            m_selectorType.insert(customLabelData->m_labelPopupWindow.m_ID,Html::RBOX);

            setBaseInfos(customLabelData->m_labelPopupWindow.m_optionsInfo);

        }

        m_codeDatas.append(node->m_data->m_codeData);

    }
}

/**
 * @brief 获取自定义控件不同状态下的绘图样式(目前提取文字颜色，和背景颜色(背景渐变色暂不处理))
 * @param twSwitchData存放提取结果
 */
void FormatProperty::getDrawStyle(CXX::DropDownButtonData * twSwitchData)
{
	//QStringList t_closeStyleIDList;
	//QStringList t_openStyleIDList;

	//auto iter = m_pageCss.begin();
	//while (iter != m_pageCss.end()) {
	//	QString name = iter.key();
	//	if (name.contains(twSwitchData->m_closeStateID))
	//		t_closeStyleIDList << name;
	//	if (name.contains(twSwitchData->m_openStateID))
	//		t_openStyleIDList << name;
	//	iter++;
	//}

	////有的ID使用_dev设置样式，有的使用自身ID设置样式，所以没有dev的改为ID样式作为dev样式
	//auto changeDevStyle = [&](QStringList &currentList, QString currentID) {
	//	if (!currentList.contains(currentID + "_div")) {
	//		currentList << currentID + "_div";

	//		if (m_pageCss.contains(currentID)) {
	//			QString newSelectedId = currentID + "_div";

	//			m_pageCss.operator [](currentID).selectorName = newSelectedId;

	//			m_pageCss.insert(newSelectedId, m_pageCss.value(currentID));
	//			m_pageCss.remove(currentID);
	//		}
	//	}
	//};
	//changeDevStyle(t_closeStyleIDList, twSwitchData->m_closeStateID);
	//changeDevStyle(t_openStyleIDList, twSwitchData->m_openStateID);

	//auto filterStyle = [&](QStringList styleIDList, QMap<QString, QPair<QString, QString>> &curStytles) {
	//	for (QString styleID : styleIDList) {

	//		auto getColor = [&](CSS::CssSegment segment) {
	//			QString t_fontColor;
	//			QString t_bgColor;
	//			for (CSS::CssRule rule : segment.rules) {
	//				if (rule.name == "color")
	//					t_fontColor = rule.value;
	//				if (rule.name.contains("background"))
	//					t_bgColor = rule.value;
	//			}
	//			return qMakePair(t_bgColor, t_fontColor);
	//		};

	//		if (styleID.contains("_div")) {
	//			curStytles.insert("default", getColor(m_pageCss.value(styleID)));
	//		}
	//		else if (styleID.contains(":hover")) {
	//			curStytles.insert("hover", getColor(m_pageCss.value(styleID)));

	//		}
	//		else if (styleID.contains(":pressed")) {
	//			curStytles.insert("pressed", getColor(m_pageCss.value(styleID)));

	//		}
	//		else if (styleID.contains(":checked")) {
	//			curStytles.insert("checked", getColor(m_pageCss.value(styleID)));
	//		}
	//	}
	//};

	//filterStyle(t_closeStyleIDList, twSwitchData->m_closeStateStytles);
	//filterStyle(t_openStyleIDList, twSwitchData->m_openStateStytles);
}

void FormatProperty::addCustomControlBgImg(QString srcImg)
{
    QStringList t_list = srcImg.split(".");
    if(t_list.size() == 2){
        switchImageURL(t_list.at(0) + "_mouseOver." + t_list.at(1));
        switchImageURL(t_list.at(0) + "_mouseDown." + t_list.at(1));
    }
}

/*!
 * @brief 判断是否包含信号槽信息，若有则提取并保存至信号槽列表
 * @param[in] node 待解析的元素节点
 */
void FormatProperty::createConnections(Html::DomNode *node)
{
    if(node->m_data && !node->m_data->m_signals.isEmpty()){
        if(m_conns == nullptr){
            m_conns = new MConnections();
        }

        for(Html::SignalSlotInfo sinfo : node->m_data->m_signals){
            MConnection * con = new MConnection();

            con->setSender(sinfo.m_sender);
            con->setSignal(sinfo.m_signal);
            con->setReceiver(sinfo.m_receiver);
            con->setSlot(sinfo.m_slot);

            m_conns->addConn(con);
        }
    }
}

/*!
 * @brief 判断是否由有需要提升的控件
 * @param[in] node带解析控件
 */
void FormatProperty::createControlImprove(Html::DomNode *node)
{
    if(node->m_data && !node->m_data->m_controlImproveInfos.isEmpty())
    {
        if(m_controlImproves == nullptr)
        {
            m_controlImproves = new McontrolImproves();
        }

        for(Html::ControlImproveInfo control : node->m_data->m_controlImproveInfos){
            McontrolImprove * customControl = new McontrolImprove();

            if (!m_customClassList.contains(control.m_newClass)){

                customControl->setExtends(control.m_extends);
                customControl->setClass(control.m_newClass);
                customControl->setHeader(control.m_headFileName);

                m_customClassList.append(control.m_newClass);

                m_controlImproves->addCustomControl(customControl);
            }
        }
    }
}

/*!
 * @brief 将CSS中RGBA的颜色转换成Qt
 * @attention CSS中a的取值范围是0~1，而Qt中是0~255
 * @param[in] cssRgba css颜色属性：格式为(r,g,b,a)
 */
QString FormatProperty::switchCssRgbaToQt(QString cssRgba)
{
    QRegExp exp("(\\d+(\\.?\\d+)?)");

    QStringList list;
    int pos = 0;

    while ((pos = exp.indexIn(cssRgba, pos)) != -1) {
        list << exp.cap(1);
        pos += exp.matchedLength();
    }

    if(list.size() == 4){
        double alpha = list.last().toDouble();
        alpha *= 255;
        if(alpha >= 255)
            alpha = 255;

        list.removeLast();
        list.append(QString::number(alpha));
    }

    QString qtRgba = QString("rgba(%1)").arg(list.join(","));

    return qtRgba;
}

/*!
 * @brief 将CSS中涉及的渐变色修改为Qt格式的渐变色
 * @attention 目前只处理linear-gradient开头的渐变色:
 *      linear-gradient(90deg, rgba(64, 111, 181, 1) 0%, rgba(64, 111, 181, 1) 0%, rgba(31, 78, 141, 1) 100%, rgba(31, 78, 141, 1) 100%)
 * @param[in] linearGradient 线性渐变
 * @return
 */
QString FormatProperty::switchCssGradientToQt(CSS::Rules linearGradients)
{
    QString newGradient = "qlineargradient(";

    for(int i = 0; i < linearGradients.size(); i++){
        QString linearGradient = linearGradients.at(i).value;
        if(linearGradient.startsWith("linear-gradient")){
            int firstBracket = linearGradient.indexOf("(");
            int lastBracket = linearGradient.lastIndexOf(")");

            QString noBracket = linearGradient.mid(firstBracket + 1,lastBracket - firstBracket - 1);

            int firstPos = noBracket.indexOf(",");
            //第一个为渐变角度
            QString deg = noBracket.left(firstPos);
            noBracket.append(",");      //手动补一个,用来和%组合作为分隔符
            QStringList smallBlocks = noBracket.mid(firstPos + 1).split("%,");

            //将角度进行转换成Qt格式
            {
                int degree = deg.remove("deg").toInt();
                if(degree == 0){

                }else if(degree == 90){
                    newGradient += QString("x1: %1, y1: %2, x2: %3, y2: %4").arg(0).arg(0).arg(1).arg(0);
                }
            }

            int lastPercent = -1;
            for(int j = 0; j < smallBlocks.size(); j++){
                QString block = smallBlocks.at(j);
                if(block.trimmed().isEmpty())
                    continue;

                int endBracket = block.indexOf(")");
                QString rgba = block.left(endBracket + 1);
                QString percent = block.right(block.size() - endBracket - 1);

                double pcent = (double)percent.toInt() / 100;

                if(lastPercent != pcent){
                    newGradient += ",";
                    newGradient += QString("stop:%1 %2").arg(pcent).arg(switchCssRgbaToQt(rgba));

                    lastPercent = pcent;
                }
            }

            break;
        }
    }

    newGradient += " )";

    return newGradient;
}

} //namespace RQt
