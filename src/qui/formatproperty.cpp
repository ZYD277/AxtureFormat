#include "formatproperty.h"

#include <QDebug>
#include "../head.h"
#include "../util/rutil.h"
#include "props/mdomwidget.h"
#include "props/mproperty.h"
#include "props/mrow.h"
#include "props/mcolumn.h"
#include "props/mitem.h"
#include "props/mattribute.h"

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
        case Html::RMENUBUTTON:
        case Html::RUNMENUBUTTON:
        case Html::RBUTTON:return QString("QPushButton");break;
        case Html::RDYNAMIC_PANEL:return QString("QStackedWidget");break;
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

            break;
        }
        case Html::RGROUP:{
            Html::GroupData * gdata = dynamic_cast<Html::GroupData*>(node->m_data);
            rect = QRect(gdata->m_left,gdata->m_top,gdata->m_width,gdata->m_height);
            break;
        }
        case Html::RDYNAMIC_PANEL:{
            if(node->m_data->m_toolTip.size() > 0)
                createToolTipProp(domWidget,node->m_data->m_toolTip);
            QString imageSrc = node->m_data->m_srcImage;
            if(!imageSrc.isEmpty() && rect.width() < 1)
                createImageProp(domWidget,imageSrc);

            for(int i = 0; i < node->m_childs.size();i++){
                createDomWidget(domWidget,node->m_childs.at(i),rect);
            }

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
            Html::TreeItemData * virtualRoot = dynamic_cast<Html::TreeItemData *>(node->m_data);
            //创建列(axure树只有一列，并且在qt ui中需要将水平表头隐藏)
            MColumn  * column = new MColumn();
            MProperty * rowProp = new MProperty();
            rowProp->setPropString(virtualRoot->m_text);
            column->addProperty(rowProp);
            domWidget->addColumn(column);

            MAttribute * hVisibleAttribute = new MAttribute();
            hVisibleAttribute->setAttributeName("headerVisible");
            hVisibleAttribute->setAttributeBool("false");
            domWidget->addAttrinute(hVisibleAttribute);

            QList<Html::TreeItemData *> childs = virtualRoot->m_childItems;
            for(int i = 0; i < childs.size(); i++){
                Html::TreeItemData * itemData = childs.at(i);
                MItem * item = new MItem();

                MProperty * prop = new MProperty();
                prop->setAttributeName("text");
                prop->setPropString(itemData->m_text);
                item->setProperty(prop);

                for(int j = 0 ; j < itemData->m_childItems.size(); j++){
                    createTreeNode(item,itemData->m_childItems.at(j));
                }

                domWidget->addItem(item);
            }
            if(!virtualRoot->m_childItemId.isEmpty()){
                virtualRoot->m_childItemId = virtualRoot->m_childItemId + "_div_" + node->m_id;
                m_selectorType.insert(virtualRoot->m_childItemId,Html::RTREE);
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

        case Html::RCHECKBOX:{
            node->m_data->m_bLeftToRight = judgeLayoutDirection(node->m_id);

            createCheckedProp(domWidget,node->m_data->m_bChecked);
            createEnableProp(domWidget,node->m_data->m_bDisabled);
            createTextProp(domWidget,node->m_data->m_text);
            createLayoutDirectionProp(domWidget,node->m_data->m_bLeftToRight);

            if(!node->m_data->m_checkedImage.isEmpty()&&!node->m_data->m_unCheckedImage.isEmpty())
            {
                createRadioBtnImageProp(domWidget,node->m_data->m_checkedImage,node->m_data->m_unCheckedImage,"QCheckBox");
            }
            if(!node->m_data->m_textId.contains("text")){
                QString m_textId = node->m_data->m_textId + "_div_" + node->m_id;
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
            if(!node->m_data->m_checkedImage.isEmpty()&&!node->m_data->m_unCheckedImage.isEmpty())
            {
                createRadioBtnImageProp(domWidget,node->m_data->m_checkedImage,node->m_data->m_unCheckedImage,"QRadioButton");
            }

            if(!node->m_data->m_textId.contains("text")){
                QString m_textId = node->m_data->m_textId + "_div_" + node->m_id;
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

        case Html::RMENUBUTTON:{
            createTextProp(domWidget,node->m_data->m_text);

            if(node->m_data->m_toolTip.size() > 0)
                createToolTipProp(domWidget,node->m_data->m_toolTip);
            for(int i = node->m_childs.size()-1; i >=0 ; i--)
            {
                createDomWidget(domWidget,node->m_childs.at(i),rect);
            }
            break;
        }
        case Html::RUNMENUBUTTON:
        case Html::RBUTTON:{
            createCheckedProp(domWidget,node->m_data->m_bChecked);
            createEnableProp(domWidget,node->m_data->m_bDisabled);

            QString imageSrc = node->m_data->m_srcImage;
            if(!imageSrc.isEmpty())
                createImageProp(domWidget,imageSrc);
            createTextProp(domWidget,node->m_data->m_text);

            if(node->m_data->m_toolTip.size() > 0)
                createToolTipProp(domWidget,node->m_data->m_toolTip);
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

            if(!listData->m_srcImage.isEmpty()||!listData->m_arrowImageOn.isEmpty()||!listData->m_arrowImageSrc.isEmpty())
            {
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
            Html::GroupData *spinBox = static_cast<Html::GroupData *>(node->m_data);
            if(!spinBox->m_sinBoxTextId.isEmpty())
            {
                QString m_sinBoxTextId = spinBox->m_sinBoxTextId + "_div_text" + node->m_id;
                m_selectorType.insert(m_sinBoxTextId,Html::RSPINBOX);
            }
            if(!spinBox->m_spinBoxId.isEmpty())
            {
                QString m_spinbox = spinBox->m_spinBoxId + "_div_spinbox" + node->m_id;
                m_selectorType.insert(m_spinbox,Html::RSPINBOX);
            }
            MProperty * miniMum = new MProperty();
            miniMum->setAttributeName("minimum");
            miniMum->setPropNumber("-99");
            domWidget->addProperty(miniMum);
             break;
        }
        case Html::RSCROLLBAR:{
            Html::ScrollBarData * scrollbarData = static_cast<Html::ScrollBarData *>(node->m_data);
            if(!scrollbarData->m_addLine.isEmpty())
            {
                QString m_addLine = scrollbarData->m_addLine + "_div_addline" + node->m_id;
                m_selectorType.insert(m_addLine,Html::RSCROLLBAR);
            }
            if(!scrollbarData->m_downArrow.isEmpty())
            {
                QString m_downArrow = scrollbarData->m_downArrow + "_div_downarrow" + node->m_id;
                m_selectorType.insert(m_downArrow,Html::RSCROLLBAR);
            }
            if(!scrollbarData->m_upArrow.isEmpty())
            {
                QString m_upArrow = scrollbarData->m_upArrow + "_div_uparrow" + node->m_id;
                m_selectorType.insert(m_upArrow,Html::RSCROLLBAR);
            }
            if(!scrollbarData->m_subLine.isEmpty())
            {
                QString m_subLine= scrollbarData->m_subLine + "_div_subline" + node->m_id;
                m_selectorType.insert(m_subLine,Html::RSCROLLBAR);
            }
            if(!scrollbarData->m_scrollBarId.isEmpty())
            {
                QString m_scrollBar = scrollbarData->m_scrollBarId + "_div_scrollbar" + node->m_id;
                m_selectorType.insert(m_scrollBar,Html::RSCROLLBAR);
            }
            QString scrollBarWidth = getCssStyle(node->m_id,"width");
            QString scrollBarHight = getCssStyle(node->m_id,"height");
            bool orientation = (removePxUnit(scrollBarWidth) > removePxUnit(scrollBarHight) ? true : false);
            createOrientationProp(domWidget,orientation);
            break;
        }
       case Html::RPROGRESSBAR:{
            Html::ProgressBarData *progressBarData = static_cast<Html::ProgressBarData *>(node->m_data);
            if(!progressBarData->m_progressBarId.isEmpty())
            {
                QString m_textId = progressBarData->m_progressBarId + "_div_bar" + node->m_id;
                m_selectorType.insert(m_textId,Html::RPROGRESSBAR);
            }
            if(!progressBarData->m_ProgressSlotId.isEmpty())
            {
                QString m_textId = progressBarData->m_ProgressSlotId + "_div_slot" + node->m_id;
                m_selectorType.insert(m_textId,Html::RPROGRESSBAR);
            }
            break;
       }
       case Html::RTABWIDGET:{
        Html::TabWidgetData *tabData = static_cast<Html::TabWidgetData *>(node->m_data);
            for(int i = node->m_childs.size()-1; i >=0 ; i--)
            {
                createDomWidget(domWidget,node->m_childs.at(i),rect);
            }
            createTabWidgetImageProp(domWidget,tabData->m_srcImage,tabData->m_selectedImage,"QTabBar");
            qDebug()<<__FILE__<<__FUNCTION__<<__LINE__<<"\n"
                   <<tabData->m_srcImage<<tabData->m_selectedImage
                   <<"\n";
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

    if(node->m_childs.size() > 0 && node->m_type != Html::RTABLE && node->m_type != Html::RDYNAMIC_PANEL
            &&node->m_type != Html::RTABWIDGET){
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
    //NOTE dynamic panel中每个子页面容器已经是相对位置，不必计算相对位置。
    if(node->m_type != Html::RDYNAMIC_PANEL_PAGE){
        rect.moveLeft(rect.x() - parentRect.x());
        rect.moveTop(rect.y() - parentRect.y());
    }

    //自制动态面板只有位置没有大小
    if(node->m_type == Html::RDYNAMIC_PANEL && rect.width() < 1)
    {
        Html::PanelData * panelData = dynamic_cast<Html::PanelData *>(node->m_data);
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
            //获取二级子菜单背景尺寸位置，结合一级子菜单确定动态动态面板位置尺寸
            if(!panelData->m_secondSrcImageId.isEmpty())
            {
                QString tsecondImageWidth = getCssStyle(panelData->m_secondSrcImageId,"width");

                rect.setWidth(removePxUnit(twidth)+removePxUnit(tsecondImageWidth));
                rect.setHeight(removePxUnit(theight));
            }

            //获取两种控件组合的情况下，第二个控件位置尺寸
            else if(!panelData->m_panelTextId.isEmpty())
            {
                QString textWidth = getCssStyle(panelData->m_panelTextId,"width");
                QString textheight = getCssStyle(panelData->m_panelTextId,"height");
                QString textX = getCssStyle(panelData->m_panelTextId,"left");

                //获取自定义复选框，单选框，半选中复选框等特殊自定义控件位置尺寸
                if((panelData->m_panelDataLab.contains(QStringLiteral("复选"))) ||(panelData->m_panelDataLab.contains(QStringLiteral("单选")))){
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
            if(node->m_data->m_widths != 0 && node->m_data->m_heights != 0){
                rect.setWidth(node->m_data->m_widths);
                rect.setHeight(node->m_data->m_heights);
            }
            else
            {
                QString textId = node->m_data->m_textId;
                rect.setWidth(removePxUnit(getCssStyle(textId,"width")) + 20);
                rect.setHeight(removePxUnit(getCssStyle(textId,"height")));
            }
        }
    }
    else if(node->m_type == Html::RDROPLIST)
    {
        if(rect.width() == 0 || rect.height() == 0){
            QString textId = node->m_data->m_widgetSizeId;
            rect.setWidth(removePxUnit(getCssStyle(textId,"width")));
            rect.setHeight(removePxUnit(getCssStyle(textId,"height")));
        }
    }
    else if(node->m_type == Html::RSPINBOX)
    {
        Html::GroupData * gdata = dynamic_cast<Html::GroupData*>(node->m_data);
        rect = QRect(gdata->m_left,gdata->m_top,gdata->m_width,gdata->m_height);
    }
    else if(node->m_type == Html::RTABWIDGET)
    {
        int width = 0;
        int height = 0;
        for(int i = 0; i < node->m_childs.size(); i++)
        {
            rect.setTop(removePxUnit(getCssStyle(node->m_childs.at(i)->m_id,"top")));
            rect.setLeft(removePxUnit(getCssStyle(node->m_childs.at(i)->m_id,"left")));
            height = removePxUnit(getCssStyle(node->m_childs.at(i)->m_id,"height"));
            width = width + removePxUnit(getCssStyle(node->m_childs.at(i)->m_id,"width"));
        }
        height = height + removePxUnit(getCssStyle(node->m_data->m_srcImageId,"height"));
        rect.setHeight(height);
        rect.setWidth(width);
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
    QString imagePath = imageSrc;
    int firstSplitPos = imageSrc.indexOf("/");
    int secondSplitPos = imageSrc.indexOf("/",firstSplitPos + 1);

    imageSrc = imageSrc.remove(firstSplitPos,secondSplitPos - firstSplitPos);

    m_originalResources.append(imagePath);
    if(!m_resources.contains(imageSrc))
        m_resources.append(imageSrc);
    if(!imageSrc.isEmpty()){
        MProperty * styleProp = new MProperty();
        styleProp->setAttributeName("styleSheet");
        styleProp->setPropString(QString("border-image:url(:/%1);").arg(imageSrc));
        domWidget->addProperty(styleProp);
    }
}

void FormatProperty::createRadioBtnImageProp(RDomWidget *domWidget,QString checkImageSrc,QString unCheckImageSrc,QString widgetName)
{
    checkImageSrc = handleImage(checkImageSrc);
    unCheckImageSrc = handleImage(unCheckImageSrc);

    if(!checkImageSrc.isEmpty()||!unCheckImageSrc.isEmpty()){
        MProperty * styleProp = new MProperty();
        styleProp->setAttributeName("styleSheet");
        styleProp->setPropString(QString("%1::indicator:checked {image: url(:/%2);}"
                                         "%3::indicator:unchecked {image: url(:/%4);}")
                                 .arg(widgetName).arg(checkImageSrc).arg(widgetName).arg(unCheckImageSrc));
        domWidget->addProperty(styleProp);
    }
}

void FormatProperty::createTabWidgetImageProp(RDomWidget *domWidget,QString srcImage,QString selectImageSrc,QString widgetName)
{
    srcImage = handleImage(srcImage);
    selectImageSrc = handleImage(selectImageSrc);

    if(!srcImage.isEmpty()||!selectImageSrc.isEmpty()){
        MProperty * styleProp = new MProperty();
        styleProp->setAttributeName("styleSheet");
        styleProp->setPropString(QString("%1::tab:selected, %2::tab:hover { border-image: url(:/%3);}"
                                         "%4::tab:!selected, %5::tab:hover {  border-image: url(:/%6);}")
                                 .arg(widgetName).arg(widgetName).arg(selectImageSrc).arg(widgetName).arg(widgetName)
                                 .arg(srcImage));
        domWidget->addProperty(styleProp);
    }
}

void FormatProperty::createComBoxImageProp(RDomWidget *domWidget, QString imageSrc, QString arrowImage, QString unArrowImage)
{
    imageSrc = handleImage(imageSrc);
    arrowImage = handleImage(arrowImage);
    unArrowImage = handleImage(unArrowImage);

    if(!imageSrc.isEmpty()||!arrowImage.isEmpty()||!unArrowImage.isEmpty()){
        MProperty * styleProp = new MProperty();
        styleProp->setAttributeName("styleSheet");
        styleProp->setPropString(QString("QComboBox{border-image:url(:/%1);}"
                                         "QComboBox QAbstractItemView {border-image: url(:/%2);}"
                                         "QComboBox::down-arrow {image: url(:/%3);}"
                                         "QComboBox::down-arrow:focus {image: url(:/%4);}")
                                 .arg(imageSrc).arg(imageSrc).arg(arrowImage).arg(unArrowImage));
        domWidget->addProperty(styleProp);
    }
}

QString FormatProperty::handleImage(QString imageSrc)
{
    QString imagePath = imageSrc;
    int firstSplitPos = imageSrc.indexOf("/");
    int secondSplitPos = imageSrc.indexOf("/",firstSplitPos + 1);

    imageSrc = imageSrc.remove(firstSplitPos,secondSplitPos - firstSplitPos);

    m_originalResources.append(imagePath);
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

void FormatProperty::createEnableProp(RDomWidget *domWidget, bool disable)
{
    MProperty * enableProp = new MProperty;
    enableProp->setAttributeName("enabled");
    enableProp->setPropBool(disable?"false":"true");
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

void FormatProperty::createOrientationProp(RDomWidget *domWidget, bool leftToRight)
{
    MProperty * layoutProp = new MProperty;
    layoutProp->setAttributeName("orientation");
    layoutProp->setPropEnum(leftToRight?"Qt::Horizontal":"Qt::Vertical");
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

void FormatProperty::createTreeNode(MItem * parentItem,Html::TreeItemData * textData)
{
    MItem * item = new MItem();

    MProperty * prop = new MProperty();
    prop->setAttributeName("text");
    prop->setPropString(textData->m_text);
    item->setProperty(prop);

    parentItem->addItem(item);

    for(int j = 0 ; j < textData->m_childItems.size(); j++){
        createTreeNode(item,textData->m_childItems.at(j));
    }
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
    QString tmpImgId = selectorId + "_img";
    if(m_pageCss.contains(tmpImgId)){
        inputId = tmpImgId;
    }else{
        inputId = selectorId + "_input";
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

} //namespace RQt
