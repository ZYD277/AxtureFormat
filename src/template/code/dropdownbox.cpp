#include "dropdownbox.h"

#include "../cppgenerate.h"

namespace CXX{

DropDownBox::DropDownBox():AbstractCodeTemplate(CUSTOM_DROPDOWN_BOX)
{

}

void DropDownBox::prepareOutput(CppGenerate *generate)
{

    QString t_pullDownBoxName = QString("pullDownBoxSlot_%1()").arg(m_sameTypeIndex);
    generate->addPrivateSlot(QString("void %1;").arg(t_pullDownBoxName));

    QString t_slotName = QString("switchOptions_%1()").arg(m_sameTypeIndex);
    generate->addPrivateSlot(QString("void %1;").arg(t_slotName));


    /*!< 链接下拉框信号槽 */
    if(!m_buttonID.isEmpty() && !m_widgetID.isEmpty()){

        generate->addConstructInitCode(NEW_EMPTY);

        generate->addConstructInitCode(QString("connect(ui->%1,SIGNAL(pressed()),this,SLOT(%2));")
                                       .arg(m_buttonID)
                                       .arg(t_pullDownBoxName));

        /*!< 实现"弹窗"槽函数 */
        generate->addConstructInitCode(NEW_EMPTY);

        QString optionsCode = ConcatNewLine(QString("void %1::%2").arg(generate->getClassName()).arg(t_pullDownBoxName));
        optionsCode += ConcatNewLine("{");
        optionsCode += ConcatNewLine("    if(ui->%1->isHidden())").arg(m_widgetID);
        optionsCode += ConcatNewLine("        ui->%1->setHidden(false);").arg(m_widgetID);
        optionsCode += ConcatNewLine("    else");
        optionsCode += ConcatNewLine("        ui->%1->setHidden(true);").arg(m_widgetID);
        optionsCode += ConcatNewLine("}");

        generate->addFunImplement(optionsCode);

    }
    /*!< 链接"选项"点击信号槽,以及实现槽函数 */
    for(QString optionId:m_optionIdList){
        generate->addConstructInitCode(QString("connect(ui->%1,SIGNAL(pressed()),this,SLOT(%2));")
                                       .arg(optionId)
                                       .arg(t_slotName));
    }

    generate->addConstructInitCode(NEW_EMPTY);

    for(QString optionId:m_optionIdList){
        generate->addConstructInitCode(QString("ui->%1->setProperty(\"Options\",ui->%2->text());")
                                       .arg(optionId).arg(optionId));
    }

    generate->addConstructInitCode(NEW_EMPTY);


    QString code = ConcatNewLine(QString("void %1::%2").arg(generate->getClassName()).arg(t_slotName));
    code += ConcatNewLine("{");
    code += ConcatNewLine("    QString OptionsIndex = QObject::sender()->property(\"Options\").toString();");
    code += ConcatNewLine("    ui->%1->setText(OptionsIndex);").arg(m_buttonID);
    code += ConcatNewLine("    ui->%1->setVisible(false);").arg(m_widgetID);
    code += ConcatNewLine("}");

    generate->addFunImplement(code);

}

void DropDownBox::setIds(QString widgetID, QString buttonID, QStringList optionIdList)
{
     m_widgetID = widgetID;
     m_buttonID = buttonID;
     m_optionIdList = optionIdList;
}

} //namespace CXX
