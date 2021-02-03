#ifndef DOUBLEFOLDINGBOX_H
#define DOUBLEFOLDINGBOX_H

#include <QWidget>

#include "abstractcodetemplate.h"

namespace CXX{

class DoubleFoldingBox : public AbstractCodeTemplate
{
public:
    DoubleFoldingBox();

    void prepareOutput(CppGenerate * generate);
    void setIds(DoubleFoldingPanel *doubleFoldingData);

private:
    DoubleFoldingPanel *m_doubleFoldingData;

};

} //namespace CXX

#endif // DOUBLEFOLDINGBOX_H
