#ifndef CPUSTART_H
#define CPUSTART_H

#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>

class CpuStart
{
public:
    CpuStart();
    void statistic(double &user,double &system);//statistic ：统计

    QTime upTime()const;

    enum Value
    {
        User,
        Nice,
        System,
        Idle,

        NValues
    };
private:
    void lookUp( double [NValues]) const;
    double procValues[NValues];
};

#endif // CPUSTART_H
