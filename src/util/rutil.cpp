#include "rutil.h"

#include <cmath>
#include <QDir>
#include <QFileInfo>
#include <QDateTime>
#include <QCryptographicHash>
#include <QDesktopWidget>
#include <QApplication>
#include <QUuid>
#include <QRegExp>
#include <QDir>
#include <QDomDocument>
#include <QDomElement>
#include <QProcess>

#include <QDebug>

QSettings * RUtil::gSettings = NULL;

RUtil::RUtil()
{

}

bool RUtil::createDir(QString dirPath)
{
    QFileInfo fileInfo(dirPath);
    if(!fileInfo.exists())
    {
        QDir tmpDir;
        return tmpDir.mkpath(dirPath);
    }

    return true;
}

bool RUtil::isFileExist(QString filePath)
{
    QFileInfo fileInfo(filePath);
    return fileInfo.exists();
}

bool RUtil::isDir(QString dirPath)
{
    QFileInfo fileInfo(dirPath);
    return fileInfo.isDir();
}

/*!
 * @brief 对字符串进行MD5加密
 * @param[in] text 待加密的字符串
 * @return 加密后的字符串
 */
QString RUtil::MD5(QString text)
{
    QByteArray bb;
    bb = QCryptographicHash::hash (text.toLocal8Bit(), QCryptographicHash::Md5 );
    return QString().append(bb.toHex());
}

/*!
 * @brief 获取文件的MD5信息
 * @details 对文件进行MD5计算，用于网络传输后对接收的文件数据进行正确性校验，防止传输过程中错误或者被篡改；
 * @param[in] fileName 文件名信息
 * @return 计算后的MD5值信息
 */
QString RUtil::MD5File(QString fileName)
{
    QFile theFile(fileName);
    theFile.open(QIODevice::ReadOnly);
    QByteArray ba = QCryptographicHash::hash(theFile.readAll(), QCryptographicHash::Md5).toHex();
    theFile.close();
    return QString(ba);
}

QString RUtil::getTimeStamp(QString format)
{
    QDateTime dtime = QDateTime::currentDateTime();
    return dtime.toString(format);
}

QString RUtil::getDate(QString format)
{
    QDate date = QDate::currentDate();
    return date.toString(format);
}

qint64 RUtil::timeStamp()
{
    return QDateTime::currentDateTime().toMSecsSinceEpoch();
}

/*!
 * @brief 将4字节整数转换成IP地址
 * @param[in] value 待转换字符串
 * @param[in] reverse true:低字节在ip头，正向
 *                    false:低字节在ip尾，反向
 */
QString RUtil::swithIntToIp(int value, bool reverse)
{
    if(reverse)
        return QString("%1.%2.%3.%4").arg(value & 0xFF).arg((value>>8) & 0xFF).arg((value>>16) & 0xFF).arg((value>>24) & 0xFF);
    else
        return QString("%1.%2.%3.%4").arg((value>>24) & 0xFF).arg((value>>16) & 0xFF).arg((value>>8) & 0xFF).arg(value & 0xFF);
}

/*!
 * @brief 将6字节整数转换成MAC地址
 * @param[in] value 待转换字符串
 * @param[in] reverse true:低字节在MAC头，正向
 *                    false:低字节在MAC尾，反向
 */
QString RUtil::swithLongToMac(qulonglong value,bool reverse)
{
    QStringList list;
    for(int i = 0;i < 6;i++){
        QString t_value = QString::number(value>>(8*i) & 0xFF,16);
        if(reverse)
            list.insert(0,t_value);
        else
            list.append(t_value);
    }
    return list.join("-");
}

void RUtil::setGlobalSettings(QSettings *settings)
{
    if(gSettings)
    {
        delete gSettings;
    }

    gSettings = settings;
}

QSettings *RUtil::globalSettings()
{
    return gSettings;
}

/*!
 * @brief 查询全局ini配置文件中字段内容
 * @param[in] group 分组名称
 * @param[in] key 待查询值key
 * @param[in] defaultValue 默认值，若group下不存在对应的key，使用默认值代替
 * @return 查询结果
 */
QVariant RUtil::getGlobalValue(const QString & group,const QString &key, const QVariant & defaultValue)
{
    gSettings->beginGroup(group);
    if(!gSettings->contains(key))
    {
        gSettings->setValue(key,defaultValue);
    }

    QVariant val = gSettings->value(key,defaultValue);

    gSettings->endGroup();

    return val;
}

/*!
 * @brief 设置全局ini配置文件中字段内容
 * @param[in] group 分组名称
 * @param[in] key 待查询值key
 * @param[in] defaultValue 字段值
 */
void RUtil::setGlobalValue(const QString &group, const QString &key, const QVariant &defaultValue)
{
    gSettings->beginGroup(group);
    gSettings->setValue(key,defaultValue);
    gSettings->endGroup();
}

/*!
 * @brief 将超长的字符串转换以...结尾的字符
 * @param[in] font 带显示区字体
 * @param[in] origin 原始显示内容
 * @param[in] maxLen 最大显示宽度
 * @return 转换后的字符
 */
QString RUtil::replaceLongTextWidthElide(const QFont &font, const QString &origin, const int maxLen)
{
    QString str;
    QFontMetrics fontWidth(font);
    if(fontWidth.width(origin) >= maxLen)
        str = fontWidth.elidedText(origin,Qt::ElideRight,maxLen);
    else
        str = origin;
    return str;
}

QString RUtil::UUID()
{
    QString uuid = QUuid::createUuid().toString();
    uuid.replace(QRegExp("\\W+"),"");
    return uuid;
}

/*!
 * @brief 在windows exploer中打开并选中文件
 * @param[in]   filepath 待打开文件全路径
 */
void RUtil::openFileInExplorer(QString filepath)
{
    QProcess process;
    filepath.replace("/","\\");
    QString cmd = QString("explorer.exe /select,\"%1\"").arg(filepath);
    process.startDetached(cmd);
}

/*!
* @brief 对字节数进行格式转换
* @details 为了便于阅读，将接受的字节转换成bytes、Mb、Gb等单位
* @param[in] bytes 网络数据字节数
* @return 转换后的数据信息
*/
QString RUtil::switchBytesUnit(quint64 bytes)
{
    if(bytes > 0){
        static const char * t_unit[] = {"B","KB","MB","GB","TB","PB","EB","ZB","YB"};
        int t_integer = 0;
        int t_iLoopTime = 0;
        do{
            t_integer = bytes / pow(1024,++t_iLoopTime);
        }while(t_integer >= 1);

        int t_realLoop = t_iLoopTime - 1;
        return QString("%1%2").arg(QString::number((double)bytes/pow(1024,t_realLoop),'f',2)).arg(QString(t_unit[t_realLoop]));
    }
    return QString("-");
}

QSize RUtil::screenSize(int num)
{
	return QSize(1920,1080);
    if(num >= 0 && num < qApp->desktop()->screenCount())
    {
        return qApp->desktop()->screen(num)->size();
    }

    return qApp->desktop()->screen()->size();
}

/*!
 * @brief 获取屏幕坐标尺寸范围
 * @details 在多窗口下，主屏和副屏有顺序关系。顺序不同，其坐标的范围存在差异 @n
 *          若A为主屏、B为辅屏，那么AB排列时，坐标范围为[0,2W][0,H] @n
 *          若BA排列时，坐标范围是[-W,W][0,H] @n
 * @warning AB、BA排列时在获取控件的位置时有较大的影响。
 * @param[in] 无
 * @return 屏幕坐标
 */
QRect RUtil::screenGeometry()
{
    int screenSize = qApp->desktop()->screenCount();

    int minX = 0,maxX = 0 ,minY = 0,maxY = 0;
    for(int i = 0; i < screenSize; i++)
    {
        QRect tmpRect = qApp->desktop()->screenGeometry(i);
        minX = tmpRect.x() < minX ? tmpRect.x() : minX;
        maxX = tmpRect.x() + tmpRect.width() > maxX ? tmpRect.x() + tmpRect.width() : maxX;
        minY = tmpRect.y() < minY ? tmpRect.y() : minY;
        maxY = tmpRect.y() + tmpRect.height() > maxY ? tmpRect.y() + tmpRect.height() : maxY;
    }

    QRect rect;
    rect.setLeft(minX);
    rect.setRight(maxX);
    rect.setTop(minY);
    rect.setBottom(maxY);

    return rect;
}

QRect RUtil::centerScreen(QSize &widgetSize)
{
    QSize screen = screenSize();
    return QRect(QPoint((screen.width() - widgetSize.width()) /2,
                        (screen.height() - widgetSize.height()) /2),widgetSize);
}

qint64 RUtil::currentMSecsSinceEpoch()
{
    return QDateTime::currentDateTime().currentMSecsSinceEpoch();
}

/*!
 * @brief 获取当前时间自格林尼治时间1970-1-1 8:0:0以来的秒数
 * @details QDateTime会先将两个时间都转换为gmt时间，再做比较。
 */
qint64 RUtil::currentSecsSinceEpoch()
{
    QDateTime epochTime(QDate(1970,1,1),QTime(8,0,0));
    return epochTime.secsTo(QDateTime::currentDateTime());
}

/*!
 * @brief RUtil::addMSecsToEpoch 将毫秒数加上格林尼治时间为实际时间
 * @param[in] mSeonds 格林尼治时间以来的毫秒数
 * @return 实际时间
 */
QDateTime RUtil::addMSecsToEpoch(qint64 mSeonds)
{
    QDateTime epochTime(QDate(1970,1,1),QTime(8,0,0));
    return epochTime.addMSecs(mSeonds);
}

/*!
 * @brief RUtil::addMSecsToEpoch 将秒数加上格林尼治时间为实际时间
 * @param[in] mSeonds 格林尼治时间以来的秒数
 * @return 实际时间
 */
QDateTime RUtil::addSecsToEpoch(qint64 seonds)
{
    QDateTime epochTime(QDate(1970,1,1),QTime(8,0,0));
    return epochTime.addSecs(seonds);
}

bool RUtil::validateIpFormat(QString dest)
{
    QString matchIp = "(\\d|([1-9]\\d)|(1\\d{2})|(2[0-4]\\d)|(25[0-5]))";
    QString matchWholeIp = QString("(%1\\.){3}%2").arg(matchIp).arg(matchIp);
    QRegExp rx(matchWholeIp);

    return rx.exactMatch(dest);
}

/*!
 * @brief 将图片转换成灰度图
 * @param[in] image：const QImage &，转换前的图片
 * @return 无 t_grayImage：QImage，转换后的灰度图
 */
QImage RUtil::convertToGray(const QImage & t_image)
{
    int t_height = t_image.height();
    int t_width = t_image.width();
    QImage t_grayImage(t_width, t_height, QImage::Format_Indexed8);
    t_grayImage.setColorCount(256);
    for(int i = 0; i < 256; i++)
    {
        t_grayImage.setColor(i, qRgb(i, i, i));
    }
    switch(t_image.format())
    {
    case QImage::Format_Indexed8:
        for(int i = 0; i < t_height; i ++)
        {
            const unsigned char *pSrc = (unsigned char *)t_image.constScanLine(i);
            unsigned char *pDest = (unsigned char *)t_grayImage.scanLine(i);
            memcpy(pDest, pSrc, t_width);
        }
        break;
    case QImage::Format_RGB32:
    case QImage::Format_ARGB32:
    case QImage::Format_ARGB32_Premultiplied:
        for(int i = 0; i < t_height; i ++)
        {
            const QRgb *pSrc = (QRgb *)t_image.constScanLine(i);
            unsigned char *pDest = (unsigned char *)t_grayImage.scanLine(i);

            for( int j = 0; j < t_width; j ++)
            {
                 pDest[j] = qGray(pSrc[j]);
            }
        }
        break;
    default:
        break;
    }
    return t_grayImage;
}

/*!
* @brief 将经纬度转换成时分秒形式
* @param[in] lonlatVal 经纬度值，需在[-180,180]之间
*/
QString RUtil::switchLonLatToDegree(double lonlatVal)
{
    if(lonlatVal > 180 || lonlatVal < -180)
        return QString("-");

    int t_result[3] = {0};
    int t_loopTime = 0;
    double t_lonlatVal = fabs(lonlatVal);
    while(t_loopTime <= 2){
        int t_v = (int)(t_lonlatVal);
        t_result[t_loopTime] = t_v;
        t_lonlatVal = (t_lonlatVal - t_v)* 60;
        t_loopTime++;
    }

    QString t_ret = QString("%1"+QStringLiteral("°")+"%2"+QStringLiteral("′")+"%3%4").arg(t_result[0]).arg(t_result[1]).arg(t_result[2]).arg(QStringLiteral("″"));
    if(lonlatVal < 0)
        t_ret.prepend("-");

    return t_ret;
}

/*!
 * @brief 将字符串转换为可显示的HTML源码
 * @param targetHtml 需要转义处理的字符串
 */
void RUtil::StringToHtml(QString &targetHtml)
{
    escapeSingleQuote(targetHtml);
    escapeBracketsQuote(targetHtml);
    escapeLFQuote(targetHtml);
}

/*!
 * @brief RUtil::escapeQuote 将Html内容中的双引号、单引号进行转义
 * @param targetHtml 需要转义处理的Html内容
 */
void RUtil::escapeSingleQuote(QString &targetHtml)
{
    targetHtml = targetHtml.replace("\\","\\\\");
    targetHtml = targetHtml.replace("\'","&qpos");
}

/*!
 * @brief 转换尖括号为HTML可显示字符
 * @param targetHtml 源字符串
 */
void RUtil::escapeBracketsQuote(QString &targetHtml)
{
    targetHtml = targetHtml.replace("<","&lt;");
    targetHtml = targetHtml.replace("<=","&le;");
    targetHtml = targetHtml.replace(">","&gt;");
    targetHtml = targetHtml.replace(">=","&ge;");
}

/*!
 * @brief 转换回车符号为HTML可显示字符
 * @param targetHtml 源字符串
 */
void RUtil::escapeLFQuote(QString &targetHtml)
{
    targetHtml = targetHtml.replace("\n","</br>");
    targetHtml = targetHtml.replace(" ","&nbsp;");
}

/*!
 * @brief RUtil::removeEccapeQuote 移除Html内容中双引号的转义符号
 * @param targetHtml 需要移除转义处理的Html内容
 */
void RUtil::removeEccapeSingleQuote(QString &targetHtml)
{
    targetHtml = targetHtml.replace("\\\'","\'");
    targetHtml = targetHtml.replace("\n","");
}

/*!
 * @brief RUtil::escapeDoubleQuote
 * @param targetHtml
 */
void RUtil::escapeDoubleQuote(QString &targetHtml)
{
    targetHtml = targetHtml.replace("\"","\\\"");
    targetHtml = targetHtml.replace("\n","");
}

/*!
 * @brief RUtil::removeEccapeDoubleQuote
 * @param targetHtml
 */
void RUtil::removeEccapeDoubleQuote(QString &targetHtml)
{
    targetHtml = targetHtml.replace("\\\"","\"");
    targetHtml = targetHtml.replace("\n","");
}

/*!
 * @brief 在Explorer中显示
 * @param path 待显示路径
 */
void RUtil::showInExplorer(QString &pathIn)
{
    QString explorer = QLatin1String("explorer.exe");
    QMap<QString, QString> m_values;
    QString explorerPath;

    QStringList env = QProcessEnvironment::systemEnvironment().toStringList();
    foreach (const QString &s, env) {
        int i = s.indexOf(QLatin1Char('='));
        if (i >= 0)
        {
            m_values.insert(s.left(i).toUpper(), s.mid(i+1));
        }
    }

    QString exec = QDir::cleanPath(explorer);

    QStringList pathEnv = m_values.value(QLatin1String("PATH")).split(QLatin1Char(';'),
                                                                      QString::SkipEmptyParts);
    foreach (const QString &p, pathEnv) {
        QString directory = QDir::fromNativeSeparators(p);
        const QChar slash = QLatin1Char('/');
        if (!directory.endsWith(slash))
            directory += slash;
        QFileInfo fi(directory + exec);
        if (fi.exists() && fi.isFile() && fi.isExecutable())
        {
            explorerPath = fi.absoluteFilePath();
            break;
        }
    }
    if(explorerPath.isEmpty())
        return;
    QStringList param;
    if (!QFileInfo(pathIn).isDir())
        param += QLatin1String("/select,");
    param += QDir::toNativeSeparators(pathIn);
    QProcess::startDetached(explorerPath, param);

}

/*!
 * @brief 格式化显示文件大小
 * @param byteSize 文件大小，单位字节
 * @return 显示的文件大小
 */
QString RUtil::formatFileSize(int byteSize)
{
    QString t_fileSize = QString();
    if(byteSize < 1024)
    {
        QString t_showSize = QString::number(byteSize,'f',2);
        t_fileSize = QString("(%1B)").arg(t_showSize);
    }
    else if(byteSize >= 1024 && byteSize < 1024*1024)
    {
        QString t_showSize = QString::number(byteSize/1024,'f',2);
        t_fileSize = QString("(%1KB)").arg(t_showSize);
    }
    else if(byteSize >= 1024*1024)
    {
        QString t_showSize = QString::number(byteSize/(1024*1024),'f',2);
        t_fileSize = QString("(%1MB)").arg(t_showSize);
    }
    return t_fileSize;
}


