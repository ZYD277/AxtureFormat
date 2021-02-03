#include "doublefoldingbox.h"

#include "../cppgenerate.h"

namespace CXX{

DoubleFoldingBox::DoubleFoldingBox():AbstractCodeTemplate(CUSTOM_DOUBLEFOLDING)
{

}

void DoubleFoldingBox::prepareOutput(CppGenerate *generate)
{

    /*!< 默认关闭状态 */
    if(!m_doubleFoldingData->m_stackedWidgetID.isEmpty()
            && !m_doubleFoldingData->foldingBtnID.isEmpty()
            && !m_doubleFoldingData->unFlodBtnID.isEmpty()){

        /*!< "开"按钮槽函数 */
        QString t_defaultSlotName = QString("DfoldingDefaultSlot_%1()").arg(m_sameTypeIndex);
        generate->addPrivateSlot(QString("void %1;").arg(t_defaultSlotName));

        /*!< "关"按钮槽函数 */
        QString t_unFoldSlotName = QString("DfoldingUnfoldSlot_%1()").arg(m_sameTypeIndex);
        generate->addPrivateSlot(QString("void %1;").arg(t_unFoldSlotName));


        generate->addConstructInitCode(NEW_EMPTY);

        if(!m_doubleFoldingData->foldingBtnID.isEmpty() && !m_doubleFoldingData->unFlodBtnID.isEmpty()){
        /*!< 实现"开关"按钮信号槽 */
        generate->addConstructInitCode(QString("connect(ui->%1,SIGNAL(pressed()),this,SLOT(%2));")
                                       .arg(m_doubleFoldingData->foldingBtnID)
                                       .arg(t_defaultSlotName));

        generate->addConstructInitCode(QString("connect(ui->%1,SIGNAL(pressed()),this,SLOT(%2));")
                                       .arg(m_doubleFoldingData->unFlodBtnID)
                                       .arg(t_unFoldSlotName));
        }

        /*!< 实现"开关"状态槽函数 */
        generate->addConstructInitCode(NEW_EMPTY);

        QString openBtnCode = ConcatNewLine(QString("void %1::%2").arg(generate->getClassName()).arg(t_defaultSlotName));
        openBtnCode += ConcatNewLine("{");
        openBtnCode += ConcatNewLine("    ui->%1->setCurrentWidget(ui->%2);").arg(m_doubleFoldingData->m_stackedWidgetID).arg(m_doubleFoldingData->unFoldPageID);
        openBtnCode += ConcatNewLine("}");

        generate->addFunImplement(openBtnCode);


        generate->addConstructInitCode(NEW_EMPTY);

        QString closeBtnCode = ConcatNewLine(QString("void %1::%2").arg(generate->getClassName()).arg(t_unFoldSlotName));
        closeBtnCode += ConcatNewLine("{");
        closeBtnCode += ConcatNewLine("    ui->%1->setCurrentWidget(ui->%2);").arg(m_doubleFoldingData->m_stackedWidgetID).arg(m_doubleFoldingData->foldingPageID);
        closeBtnCode += ConcatNewLine("}");

        generate->addFunImplement(closeBtnCode);
    }
}

void DoubleFoldingBox::setIds(DoubleFoldingPanel *doubleFoldingData)
{
     m_doubleFoldingData = doubleFoldingData;
}

}
