#ifndef CUSTOMSCROLLBAR_H
#define CUSTOMSCROLLBAR_H

#include <QWidget>
#include "abstractcodetemplate.h"

namespace CXX{

class CustomScrollBar : public AbstractCodeTemplate
{
public:
    CustomScrollBar();

    void prepareOutput(CppGenerate *generate);

    void setScrollBarParameter(BidirectionalSlider * srollBarData);

private:
    BidirectionalSlider *m_scrollBar;
};

} //namespace CXX
#endif // CUSTOMSCROLLBAR_H
