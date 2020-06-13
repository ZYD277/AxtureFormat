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

namespace RQt{

QString G_NewLine = "\r\n";

FormatProperty::FormatProperty():m_conns(nullptr)
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
            QString m_widthTable;
            QString m_heightTable;
            if(!node->m_data->m_srcImageId.isEmpty()){
                CSS::CssSegment cellSegment = m_pageCss.value(node->m_data->m_srcImageId);
                for(int i = 0;i < cellSegment.rules.size(); i++){
                    if(cellSegment.rules.at(i).name == "width")
                        m_widthTable = cellSegment.rules.at(i).value;
                    else if(cellSegment.rules.at(i).name == "height")
                        m_heightTable = cellSegment.rules.at(i).value;
                }
                m_widthTable = QString::number(removePxUnit(m_widthTable));
                m_heightTable = QString::number(removePxUnit(m_heightTable));
            }
            MAttribute * hvisible = new MAttribute();
            hvisible->setAttributeName("horizontalHeaderVisible");
            hvisible->setAttributeBool("false");
            domWidget->addAttrinute(hvisible);

            MAttribute * vvisible = new MAttribute();
            vvisible->setAttributeName("verticalHeaderVisible");
            vvisible->setAttributeBool("false");
            domWidget->addAttrinute(vvisible);

            MAttribute * hSectionSize = new MAttribute();
            hSectionSize->setAttributeName("horizontalHeaderDefaultSectionSize");
            hSectionSize->setPropNumber(m_widthTable);
            domWidget->addAttrinute(hSectionSize);

            MAttribute * vhSectionSize = new MAttribute();
            vhSectionSize->setAttributeName("verticalHeaderDefaultSectionSize");
            vhSectionSize->setPropNumber(m_heightTable);
            domWidget->addAttrinute(vhSectionSize);


            QString imageSrc = node->m_data->m_srcImage;
            if(!imageSrc.isEmpty())
                createImageProp(domWidget,imageSrc);

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

                MAttribute * hprop = new MAttribute();
                hprop->setAttributeName("horizontalHeaderVisible");
                hprop->setAttributeBool("false");

                MAttribute * vprop = new MAttribute();
                vprop->setAttributeName("verticalHeaderVisible");
                vprop->setAttributeBool("false");

                for(int i = 0; i < columnCount; i++){
                    MRow  * row = new MRow();

                    MProperty * rowProp = new MProperty();
                    rowProp->setPropString(QString("row%1").arg(i));
                    row->addProperty(rowProp);

                    domWidget->addRow(row);
                }

                for(int i = 0; i < rowCount; i++){
                    MColumn  * column = new MColumn();

                    MProperty * rowProp = new MProperty();
                    rowProp->setPropString(QString("column%1").arg(i));
                    column->addProperty(rowProp);

                    domWidget->addColumn(column);
                }

                Html::TableData * tdata = dynamic_cast<Html::TableData *>(node->m_data);

                if(!tdata->m_itemId.isEmpty()){
                    tdata->m_itemId = tdata->m_itemId + "_div_" + node->m_id;
                    m_selectorType.insert(tdata->m_itemId,Html::RTABLE);
                }
                for(int i = 0; i < columnCount; i++){
                    for(int j = 0; j < rowCount; j++){
                        MItem * item = new MItem();

                        item->setAttributeRow(QString::number(i));
                        item->setAttributeColumn(QString::number(j));
                        MProperty * prop = new MProperty();
                        prop->setAttributeName("text");
                        prop->setPropString(QString(tdata->m_items.at(i*rowCount + j)));
                        item->setProperty(prop);

                        domWidget->addItem(item);
                    }
                }
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

            for(int i = treeData->m_treeDatas.size() - 1; i >= 0; i--){
                Html::TreeNodeData nodeData = treeData->m_treeDatas.at(i);

                //概要信息
                MItem * item = new MItem();

                MProperty * prop = new MProperty();
                prop->setAttributeName("text");
                prop->setPropString(QString("%1 %2").arg(nodeData.timestamp).arg(nodeData.simpleInfo));
                item->setProperty(prop);

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

            createTreeImageProp(domWidget,treeData);

            break;
        }
        case Html::RTEXT_FIELD:
        case Html::R_CUSTOM_TEXT_FIELD:{
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

            createReadonlyProp(domWidget,node->m_data->m_bReadOnly);
            createEnableProp(domWidget,node->m_data->m_bDisabled);

            MProperty * currentRow = new MProperty();
            currentRow->setAttributeName("plainText");
            currentRow->setPropString(node->m_data->m_text);
            domWidget->addProperty(currentRow);

            if(node->m_data->m_toolTip.size() > 0)
                createToolTipProp(domWidget,node->m_data->m_toolTip);

            break;
        }

        case Html::RSPINBOX:{
            Html::SpinboxData *spinBox = static_cast<Html::SpinboxData *>(node->m_data);

            if(!spinBox->m_sinBoxTextId.isEmpty()){
                QString m_sinBoxTextId = spinBox->m_sinBoxTextId + "_div_text" + node->m_id;
                m_selectorType.insert(m_sinBoxTextId,Html::RSPINBOX);
            }

            if(!spinBox->m_spinBoxId.isEmpty()){
                QString m_spinbox = spinBox->m_spinBoxId + "_div_spinbox" + node->m_id;
                m_selectorType.insert(m_spinbox,Html::RSPINBOX);
            }

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
    //修改RIMAGE和RLABEL类型的位置尺寸为背景图片的
    else if(((node->m_type == Html::RIMAGE)||(node->m_type == Html::RLABEL)) && (!node->m_data->m_srcImageId.isEmpty()))
    {
        QString twidth = getCssStyle(node->m_data->m_srcImageId,"width");
        QString theight = getCssStyle(node->m_data->m_srcImageId,"height");
        QString tleft = getCssStyle(node->m_data->m_srcImageId,"left");
        QString ttop = getCssStyle(node->m_data->m_srcImageId,"top");

        rect.setLeft(rect.left() + removePxUnit(tleft));
        rect.setTop(rect.top() + removePxUnit(ttop));
        rect.setWidth(removePxUnit(twidth));
        rect.setHeight(removePxUnit(theight));
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
    }

    if(rect.left() < 0)
        rect.setLeft(0);

    return rect;
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

    if(baseData->m_needMouseChecked && baseData->m_bChecked){
        prop += QString("QPushButton:checked {border-image: url(:/%1);}").arg(mouseSelectedImageSrc);
    }

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
 * @param[in] tabData tab页面数据
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
                                         "QTabBar::tab{%4;border-right-image:url(':/%5');}")
                                 .arg(selectedImageSrc).arg(normalImageSrc).arg(mouseOverImageSrc).arg(tabStyle)
                                 .arg(tabRightBorderImageSrc));
        domWidget->addProperty(styleProp);
    }
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

    if(!upArrow.isEmpty() && !downArrow.isEmpty() && !upArrowOver.isEmpty() && !downArrowOver.isEmpty()){
        MProperty * styleProp = new MProperty();
        styleProp->setAttributeName("styleSheet");
        styleProp->setPropString(QString("QSpinBox::up-arrow {image:url(:/%1);}" + G_NewLine +
                                         "QSpinBox::up-arrow:hover {image: url(:/%2);}" + G_NewLine +
                                         "QSpinBox::down-arrow {image: url(:/%3);}" + G_NewLine +
                                         "QSpinBox::down-arrow:hover {image: url(:/%4);}")
                                 .arg(upArrow).arg(upArrowOver).arg(downArrow).arg(downArrowOver));

        domWidget->addProperty(styleProp);
    }else{
        MProperty * styleProp = new MProperty();
        styleProp->setAttributeName("styleSheet");
        styleProp->setPropString(QString("QSpinBox::up-arrow {width:0px;}" + G_NewLine +
                                         "QSpinBox::down-arrow {width:0px;}" + G_NewLine));

        domWidget->addProperty(styleProp);
    }
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

        prop += QString("QSlider::handle:%1 {image: url(:/%2); width:%3; height: %4;border-radius: 1px;}" + G_NewLine)
                .arg(direction)
                .arg(switchImageURL(data->m_srcImage))
                .arg(findRuleByName(seg.rules,"width").value)
                .arg(handleHeight);


        seg = m_pageCss.value(data->m_pressedHandleId);

        prop += QString("QSlider::handle:%1:hover {image: url(:/%2); width:%3; height: %4;border-radius: 1px;}" + G_NewLine)
                .arg(direction)
                .arg(switchImageURL(data->m_checkedImage))
                .arg(findRuleByName(seg.rules,"width").value)
                .arg(findRuleByName(seg.rules,"height").value);
    }

    //add-line和groove
    {
        QString slotId = data->m_progressSlotId + "_div";
        CSS::CssSegment seg = m_pageCss.value(slotId);

        prop += QString("QSlider::groove:%1 {background: %2; height: %3;border-radius: 1px;padding-left:-1px;padding-right:-1px;}" + G_NewLine)
                .arg(direction)
                .arg(switchCssRgbaToQt(findRuleByName(seg.rules,"background-color").value))
                .arg(handleHeight);
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

void FormatProperty::createTreeImageProp(RDomWidget *domWidget, Html::TreeData *data)
{

    QStringList selectedNameList = data->m_srcImage.split(".");
    if(selectedNameList.size() == 2){
        QString selectedName = selectedNameList.at(0);

        QString normalItemImage = switchImageURL(data->m_srcImage);
        QString mouseOverImageSrc = switchImageURL(selectedName + "_mouseOver." + selectedNameList.at(1));

        MProperty * styleProp = new MProperty();
        styleProp->setAttributeName("styleSheet");
        styleProp->setPropString(QString("QTreeView::item {border-image: url(:/%1);}" + G_NewLine +
                                         "QTreeView::item:hover {border-image: url(:/%2);}" + G_NewLine +
                                         "QTreeView::item:selected:hover {}"
                                         )
                                 .arg(normalItemImage).arg(mouseOverImageSrc));

        domWidget->addProperty(styleProp);
    }
}

/*!
 * @brief 1.记录原始图片路径；2.去除图片中包含axure页面的名称，使其路径在images/xxx.png
 * @param[in] imageSrc 原始图片路径1
 */
QString FormatProperty::switchImageURL(QString imageSrc)
{
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
