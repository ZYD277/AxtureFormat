#ifndef DROPDOWNBUTTON_H
#define DROPDOWNBUTTON_H

#include "abstractcodetemplate.h"

namespace CXX{

class DropdownButton : public AbstractCodeTemplate
{
public:
    DropdownButton();

    void prepareOutput(CppGenerate * generate);

    void setCustomButtonStyle(DropDownButtonData *customButton);

private:
    DropDownButtonData *m_customButton;
};

} //namespace CXX

#endif // DROPDOWNBUTTON_H
