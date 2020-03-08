/*!
 *  @brief     单例模板类
 *  @details   快速提供单例子模板
 *  @file      rsingleton.h
 *  @author    wey
 *  @version   1.0
 *  @date      2017.12.11
 *  @warning
 *  @copyright NanJing RenGu.
 */
#ifndef RSINGLETON_H
#define RSINGLETON_H

#include <QObject>

/*!
    模板类和函数只是可复用的源代码，是不需要导出的，
    只需在调用时#include头文件就行。因为模板要在实例化之后才会生成obj，进而链接成DLL。
*/

template<class T>
class RSingleton
{
public:
    static T * instance()
    {
        static T * t = NULL;
        if(!t)
        {
            t = new T();
        }
        return t;
    }

};

#endif // RSINGLETON_H
