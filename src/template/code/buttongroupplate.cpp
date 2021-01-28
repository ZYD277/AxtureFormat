#include "buttongroupplate.h"

#include "../cppgenerate.h"

namespace CXX{

ButtonGroupPlate::ButtonGroupPlate() : AbstractCodeTemplate(CUSTOM_DROPDOWN_BOX)
{

}

void ButtonGroupPlate::prepareOutput(CppGenerate *generate)
{

    QString t_buttonGroupName = QString("buttonGroup_%1").arg(m_sameTypeIndex);

    generate->addConstructInitCode(NEW_EMPTY);

    generate->addConstructInitCode(QString("QButtonGroup * %1 = new QButtonGroup;").arg(t_buttonGroupName));
    generate->addConstructInitCode(QString("%1->setExclusive(true);").arg(t_buttonGroupName));

    generate->addConstructInitCode(NEW_EMPTY);

    for(QString buttonID:m_buttonIds){
        generate->addConstructInitCode(QString("%1->addButton(ui->%2);")
                                       .arg(t_buttonGroupName).arg(buttonID));
    }
}

void ButtonGroupPlate::setButtonGroupIds(QStringList buttonIds)
{
    m_buttonIds = buttonIds;
}

}


