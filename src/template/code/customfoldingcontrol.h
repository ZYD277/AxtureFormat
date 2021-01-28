#ifndef CUSTOMFOLDINGCONTROL_H
#define CUSTOMFOLDINGCONTROL_H

#include <QWidget>
#include "abstractcodetemplate.h"

namespace CXX{

class CustomFoldingControl : public AbstractCodeTemplate
{

public:
    CustomFoldingControl();

    void prepareOutput(CppGenerate *generate);

    void setFoldingControlParameter(FoldingControls * foldingControl);

signals:

public slots:

private:
    FoldingControls *m_foldingControl;

};

} //namespace CXX

#endif // CUSTOMFOLDINGCONTROL_H
