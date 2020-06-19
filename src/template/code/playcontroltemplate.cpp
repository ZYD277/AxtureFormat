#include "playcontroltemplate.h"

#include "../cppgenerate.h"

namespace CXX{

PlayControlTemplate::PlayControlTemplate():AbstractCodeTemplate(PLAY_CONTROL)
{

}

void PlayControlTemplate::prepareOutput(CppGenerate *generate)
{
    QString t_slotName = QString("switchPlayState_%1()").arg(m_sameTypeIndex);
    generate->addPrivateSlot(QString("void %1;").arg(t_slotName));

    generate->addConstructInitCode(NEW_EMPTY);

    for(QString id:m_modelIds){
        generate->addConstructInitCode(QString("connect(ui->%1,SIGNAL(pressed()),this,SLOT(%2));")
                                       .arg(id)
                                       .arg(QString("switchPlayState_%1()").arg(m_sameTypeIndex)));
    }

    generate->addConstructInitCode(NEW_EMPTY);

    QString code = ConcatNewLine(QString("void %1::%2").arg(generate->getClassName()).arg(t_slotName));
    code += ConcatNewLine("{");
    code += ConcatNewLine("    int index = ui->%1->currentIndex();").arg(m_stackedContainerId);
    code += ConcatNewLine("    ui->%1->setCurrentIndex((index + 1 == ui->%2->count()) ? 0: index + 1);")
            .arg(m_stackedContainerId).arg(m_stackedContainerId);
    code += ConcatNewLine("}");

    generate->addFunImplement(code);

}

} //namespace CXX
