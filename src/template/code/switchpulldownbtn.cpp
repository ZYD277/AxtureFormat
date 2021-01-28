#include "switchpulldownbtn.h"

#include "../cppgenerate.h"

namespace CXX{

SwitchPullDownBtn::SwitchPullDownBtn():AbstractCodeTemplate(CUSTOM_SWITCH_PULLDOWN_BUTTON)
{

}

void SwitchPullDownBtn::prepareOutput(CppGenerate *generate)
{

    /*!< 默认关闭状态 */
    if(!m_switchPullDownBtnData->m_stackedWidgetID.isEmpty()
            && !m_switchPullDownBtnData->m_closePageID.isEmpty()
            && !m_switchPullDownBtnData->m_openPageID.isEmpty()){

        /*!< "开"按钮槽函数 */
        QString t_openSlotName = QString("openBtnSlot_%1()").arg(m_sameTypeIndex);
        generate->addPrivateSlot(QString("void %1;").arg(t_openSlotName));

        /*!< "关"按钮槽函数 */
        QString t_closeSlotName = QString("closeBtnSlot_%1()").arg(m_sameTypeIndex);
        generate->addPrivateSlot(QString("void %1;").arg(t_closeSlotName));


        generate->addConstructInitCode(NEW_EMPTY);

        if(!m_switchPullDownBtnData->m_datalabel.contains(QStringLiteral("选择器")))
            generate->addConstructInitCode(QString("ui->%1->setCurrentWidget(ui->%2);").arg(m_switchPullDownBtnData->m_stackedWidgetID).arg(m_switchPullDownBtnData->m_closePageID));

        if(!m_switchPullDownBtnData->m_openBtnID.isEmpty() && !m_switchPullDownBtnData->m_closeBtnID.isEmpty()){
        /*!< 实现"开关"按钮信号槽 */
        generate->addConstructInitCode(QString("connect(ui->%1,SIGNAL(pressed()),this,SLOT(%2));")
                                       .arg(m_switchPullDownBtnData->m_openBtnID)
                                       .arg(t_openSlotName));

        generate->addConstructInitCode(QString("connect(ui->%1,SIGNAL(pressed()),this,SLOT(%2));")
                                       .arg(m_switchPullDownBtnData->m_closeBtnID)
                                       .arg(t_closeSlotName));
        }

        /*!< 实现"开关"状态槽函数 */
        generate->addConstructInitCode(NEW_EMPTY);

        QString openBtnCode = ConcatNewLine(QString("void %1::%2").arg(generate->getClassName()).arg(t_openSlotName));
        openBtnCode += ConcatNewLine("{");
        openBtnCode += ConcatNewLine("    ui->%1->setCurrentWidget(ui->%2);").arg(m_switchPullDownBtnData->m_stackedWidgetID).arg(m_switchPullDownBtnData->m_closePageID);
        openBtnCode += ConcatNewLine("}");

        generate->addFunImplement(openBtnCode);


        generate->addConstructInitCode(NEW_EMPTY);

        QString closeBtnCode = ConcatNewLine(QString("void %1::%2").arg(generate->getClassName()).arg(t_closeSlotName));
        closeBtnCode += ConcatNewLine("{");
        closeBtnCode += ConcatNewLine("    ui->%1->setCurrentWidget(ui->%2);").arg(m_switchPullDownBtnData->m_stackedWidgetID).arg(m_switchPullDownBtnData->m_openPageID);
        closeBtnCode += ConcatNewLine("}");

        generate->addFunImplement(closeBtnCode);
    }

    /*!< 处理选项信号槽 */
    if(!m_switchPullDownBtnData->m_triggerPopupBtnID.isEmpty()
            && !m_switchPullDownBtnData->m_popupWidgetID.isEmpty()){

        /*!< "触发弹窗"槽函数名 */
        QString t_slotName = QString("triggerBox_%1()").arg(m_sameTypeIndex);
        generate->addPrivateSlot(QString("void %1;").arg(t_slotName));


        generate->addConstructInitCode(NEW_EMPTY);
        generate->addConstructInitCode(QString("connect(ui->%1,SIGNAL(pressed()),this,SLOT(%2));")
                                       .arg(m_switchPullDownBtnData->m_triggerPopupBtnID)
                                       .arg(t_slotName));

        /*!< 实现"弹窗"槽函数 */
        generate->addConstructInitCode(NEW_EMPTY);

        QString optionsCode = ConcatNewLine(QString("void %1::%2").arg(generate->getClassName()).arg(t_slotName));
        optionsCode += ConcatNewLine("{");
        optionsCode += ConcatNewLine("    if(ui->%1->isHidden())").arg(m_switchPullDownBtnData->m_popupWidgetID);
        optionsCode += ConcatNewLine("        ui->%1->setHidden(false);").arg(m_switchPullDownBtnData->m_popupWidgetID);
        optionsCode += ConcatNewLine("    else");
        optionsCode += ConcatNewLine("        ui->%1->setHidden(true);").arg(m_switchPullDownBtnData->m_popupWidgetID);
        optionsCode += ConcatNewLine("}");

        generate->addFunImplement(optionsCode);

    }

    /*!< "触发弹窗"槽函数名 */
    QString t_optionSwitchName = QString("optionSwitch_%1()").arg(m_sameTypeIndex);
    generate->addPrivateSlot(QString("void %1;").arg(t_optionSwitchName));


    /*!< 链接"选项"点击信号槽,以及实现槽函数 */
    for(QString optionId:m_switchPullDownBtnData->m_optionIdList){
        generate->addConstructInitCode(QString("connect(ui->%1,SIGNAL(pressed()),this,SLOT(%2));")
                                       .arg(optionId)
                                       .arg(t_optionSwitchName));
    }

    generate->addConstructInitCode(NEW_EMPTY);

    for(QString optionId:m_switchPullDownBtnData->m_optionIdList){
        generate->addConstructInitCode(QString("ui->%1->setProperty(\"Options\",ui->%2->text());")
                                       .arg(optionId).arg(optionId));
    }

    generate->addConstructInitCode(NEW_EMPTY);


    if(m_switchPullDownBtnData->m_datalabel.contains(QStringLiteral("选择器"))){
        QString code = ConcatNewLine(QString("void %1::%2").arg(generate->getClassName()).arg(t_optionSwitchName));
        code += ConcatNewLine("{");
        code += ConcatNewLine("    QString OptionsIndex = QObject::sender()->property(\"Options\").toString();");
        code += ConcatNewLine("    ui->%1->setText(OptionsIndex);").arg(m_switchPullDownBtnData->m_openBtnID);
        code += ConcatNewLine("    ui->%1->setCurrentWidget(ui->%2);").arg(m_switchPullDownBtnData->m_stackedWidgetID)
                .arg(m_switchPullDownBtnData->m_openPageID);
        code += ConcatNewLine("}");

        generate->addFunImplement(code);
    }else{
        QString code = ConcatNewLine(QString("void %1::%2").arg(generate->getClassName()).arg(t_optionSwitchName));
        code += ConcatNewLine("{");
        code += ConcatNewLine("    QString OptionsIndex = QObject::sender()->property(\"Options\").toString();");
        code += ConcatNewLine("    ui->%1->setText(OptionsIndex);").arg(m_switchPullDownBtnData->m_closeBtnID);
        code += ConcatNewLine("    ui->%1->setVisible(false);").arg(m_switchPullDownBtnData->m_popupWidgetID);
        code += ConcatNewLine("}");

        generate->addFunImplement(code);

    }
}

void SwitchPullDownBtn::setIds(SwitchPullDownButtonData *switchPullDownBtnData)
{
     m_switchPullDownBtnData = switchPullDownBtnData;
}

} //namespace CXX

