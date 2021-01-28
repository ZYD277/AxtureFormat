#ifndef MCONTROLIMPROVE_H
#define MCONTROLIMPROVE_H

#include <QWidget>
#include <QXmlStreamWriter>
#include <QList>

namespace RQt{

class McontrolImprove
{
public:
	McontrolImprove();
    ~McontrolImprove();

    void write(QXmlStreamWriter &writer, QString tagName);
    void setClass(QString className);
    void setExtends(QString extendsName);
    void setHeader(QString headerName);

signals:

public slots:

private:
    QString m_class;
    QString m_extends;
    QString m_header;
};

} //namespace RQt

#endif // MCONTROLIMPROVE_H
