#ifndef SELECTORPULLDOWNMENU_H
#define SELECTORPULLDOWNMENU_H

#include <QWidget>

#include "abstractcodetemplate.h"

namespace CXX{

class SelectorPullDownMenu : public AbstractCodeTemplate
{
public:
    SelectorPullDownMenu();

    void prepareOutput(CppGenerate * generate);
    void setIds(TheSelectorMenu *selectorMenuData);

private:
    TheSelectorMenu *m_selectorMenuData;
};

} //namespace CXX

#endif // SELECTORPULLDOWNMENU_H
