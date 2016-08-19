#ifndef CPUPIEMARKER_H
#define CPUPIEMARKER_H
//这个类显示如何扩展QwtPlotItem。
//它显示出一个饼图。内容为 user/total/idle cpu使用百分比。

#include <Qwt/qwt_plot_item.h>
#include <Qwt/qwt_scale_map.h>

#include "cpuPlot.h"

class CpuPieMarker : public QwtPlotItem
{
public:
    CpuPieMarker();

    virtual int rtti() const;

    virtual void draw(QPainter *painter,
                      const QwtScaleMap &,
                      const QwtScaleMap &,
                      const QRectF &canvasRect) const;
};

#endif // CPUPIEMARKER_H
