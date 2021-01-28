#ifndef MCONTROLIMPROVES_H
#define MCONTROLIMPROVES_H

#include <QWidget>
#include <QList>
#include <QXmlStreamWriter>

namespace RQt{

class McontrolImprove;

class McontrolImproves
{
public:
    explicit McontrolImproves();
    ~McontrolImproves();

    void write(QXmlStreamWriter &writer, QString tagName);
    void addCustomControl(McontrolImprove *controlImprove);

private:
    QList<McontrolImprove*> m_controlImproves;
};
} //namespace RQt
#endif // MCONTROLIMPROVES_H
