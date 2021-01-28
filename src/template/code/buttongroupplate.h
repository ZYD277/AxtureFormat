#ifndef BUTTONGROUPPLATE_H
#define BUTTONGROUPPLATE_H

#include <QWidget>

#include "abstractcodetemplate.h"

namespace CXX{

class ButtonGroupPlate : public AbstractCodeTemplate
{
public:
    ButtonGroupPlate();

    void prepareOutput(CppGenerate * generate);

    void setButtonGroupIds(QStringList buttonIds);

private:

    QStringList m_buttonIds;
};
} //namespace CXX

#endif // BUTTONGROUPPLATE_H
