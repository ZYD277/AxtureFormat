#include "selectorpulldownmenu.h"

#include "../cppgenerate.h"

namespace CXX{

SelectorPullDownMenu::SelectorPullDownMenu() : AbstractCodeTemplate(CUSTOM_SELECTOR_MENU)
{

}

void SelectorPullDownMenu::prepareOutput(CppGenerate *generate)
{

    /*!< 设置默认状态 */
    if(!m_selectorMenuData->m_stackedWidgetID.isEmpty()
            && !m_selectorMenuData->m_defaultPageID.isEmpty()
            && !m_selectorMenuData->m_pressedPageID.isEmpty()){

        /*!< "默认"按钮槽函数 */
        QString t_defaultSlotName = QString("SelectorDefaultBtnSlot_%1()").arg(m_sameTypeIndex);
        generate->addPrivateSlot(QString("void %1;").arg(t_defaultSlotName));

        /*!< "按下"按钮槽函数 */
        QString t_pressedSlotName = QString("SelectorPressedBtnSlot_%1()").arg(m_sameTypeIndex);
        generate->addPrivateSlot(QString("void %1;").arg(t_pressedSlotName));


        generate->addConstructInitCode(NEW_EMPTY);

        if(!m_selectorMenuData->m_defaultBtnID.isEmpty() && !m_selectorMenuData->m_pressedBtnID.isEmpty()){
        /*!< 实现"开关"按钮信号槽 */
        generate->addConstructInitCode(QString("connect(ui->%1,SIGNAL(pressed()),this,SLOT(%2));")
                                       .arg(m_selectorMenuData->m_defaultBtnID)
                                       .arg(t_defaultSlotName));

        generate->addConstructInitCode(QString("connect(ui->%1,SIGNAL(pressed()),this,SLOT(%2));")
                                       .arg(m_selectorMenuData->m_pressedBtnID)
                                       .arg(t_pressedSlotName));
        }

        /*!< 实现"开关"状态槽函数 */
        generate->addConstructInitCode(NEW_EMPTY);

        QString defaultBtnCode = ConcatNewLine(QString("void %1::%2").arg(generate->getClassName()).arg(t_defaultSlotName));
        defaultBtnCode += ConcatNewLine("{");
        defaultBtnCode += ConcatNewLine("    ui->%1->setCurrentWidget(ui->%2);").arg(m_selectorMenuData->m_stackedWidgetID).arg(m_selectorMenuData->m_pressedPageID);
        defaultBtnCode += ConcatNewLine("}");

        generate->addFunImplement(defaultBtnCode);


        generate->addConstructInitCode(NEW_EMPTY);

        QString pressedBtnCode = ConcatNewLine(QString("void %1::%2").arg(generate->getClassName()).arg(t_pressedSlotName));
        pressedBtnCode += ConcatNewLine("{");
        pressedBtnCode += ConcatNewLine("    ui->%1->setCurrentWidget(ui->%2);").arg(m_selectorMenuData->m_stackedWidgetID).arg(m_selectorMenuData->m_defaultPageID);
        pressedBtnCode += ConcatNewLine("}");

        generate->addFunImplement(pressedBtnCode);
    }

    /*!< "触发弹窗"槽函数名 */
    QString t_optionSwitchName = QString("selectorOptionSwitch_%1()").arg(m_sameTypeIndex);
    generate->addPrivateSlot(QString("void %1;").arg(t_optionSwitchName));


    /*!< 链接"选项"点击信号槽,以及实现槽函数 */
    for(QString optionId:m_selectorMenuData->m_optionIdList){
        generate->addConstructInitCode(QString("connect(ui->%1,SIGNAL(pressed()),this,SLOT(%2));")
                                       .arg(optionId)
                                       .arg(t_optionSwitchName));
    }

    generate->addConstructInitCode(NEW_EMPTY);

    for(QString optionId:m_selectorMenuData->m_optionIdList){
        generate->addConstructInitCode(QString("ui->%1->setProperty(\"Options\",ui->%2->text());")
                                       .arg(optionId).arg(optionId));
    }

    generate->addConstructInitCode(NEW_EMPTY);

    QString code = ConcatNewLine(QString("void %1::%2").arg(generate->getClassName()).arg(t_optionSwitchName));
    code += ConcatNewLine("{");
    code += ConcatNewLine("    QString OptionsIndex = QObject::sender()->property(\"Options\").toString();");
    code += ConcatNewLine("    ui->%1->setText(OptionsIndex);").arg(m_selectorMenuData->m_defaultBtnID);
    code += ConcatNewLine("    ui->%1->setCurrentWidget(ui->%2);").arg(m_selectorMenuData->m_stackedWidgetID)
            .arg(m_selectorMenuData->m_defaultPageID);
    code += ConcatNewLine("}");

    generate->addFunImplement(code);

}

void SelectorPullDownMenu::setIds(TheSelectorMenu *selectorMenuData)
{
     m_selectorMenuData = selectorMenuData;
}

}
