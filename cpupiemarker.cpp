#include "cpupiemarker.h"

CpuPieMarker::CpuPieMarker()
{
    setZ(1000);//为什么是1000，难道是一个相对大的数？为了保持饼图在图表的最表层吗？ Z值越大，则该item就越接近图表的表层，越不容易被其他的item遮挡
    setRenderHint(QwtPlotItem::RenderAntialiased,true);//反锯齿绘图
}

int CpuPieMarker::rtti() const
{
    return QwtPlotItem::Rtti_PlotUserItem;
}

void CpuPieMarker::draw(QPainter *painter,
                        const QwtScaleMap &,
                        const QwtScaleMap &,
                        const QRectF &canvasRect) const
{
    const CputPlot * cpuPlot = static_cast<CputPlot *>(plot());//当两个类为继承关系是，只要你确定两个类一致是，可以使用static_cast来将转换对象。此处的plot()的返回值应当是QwtPlot,但是其本来就是CpuPlot，只是转换了一下。

    const QwtScaleMap yMap = cpuPlot->canvasMap(QwtPlot::yLeft);

    const int margin = 5;

    QRectF pieRect;
    pieRect.setX( canvasRect.x() + margin );
    pieRect.setY( canvasRect.y() + margin );
    pieRect.setHeight(yMap.transform( 80.0));
    pieRect.setWidth( pieRect.height());//设置饼图的矩形块

    const int dataType[] = { CputPlot::User, CputPlot::System,CputPlot::Idle};

    int angle = static_cast<int>( 5760 * 0.75);//数据是什么意思？ 5760，搞不明白，这个数的含义  变量名：angle
    for (unsigned int i = 0;
          i < sizeof(dataType) / sizeof(dataType[0]) ; ++i) {
        const QwtPlotCurve * curve = cpuPlot->cpuCurve(dataType[i]);

        if ( curve->dataSize() > 0){
            const int value = static_cast<int>( 5760 * curve->sample(0).y() / 100);//0，又是什么意思？

            painter->save();//保存一个画家的状态，修改，然后在恢复。
            painter->setBrush(QBrush( curve->brush().color(),Qt::SolidPattern));//获取和折线相同的颜色。
            if ( value != 0){
                painter->drawPie(pieRect,-angle,-value);
                // a full circle equals 5760 (16 * 360) 角度a和alen是1/16度，也就是说一个完整的圆等于5760（16*360）。
                //正数的angle和value意味着逆时针方向而负值意味着顺时针方向。0度在3点的时钟位置。
            }
            painter->restore();

            angle += value;//添加偏转的角度

        }

    }


}
