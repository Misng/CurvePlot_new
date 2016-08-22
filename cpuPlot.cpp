#include <QApplication>
#include <QtGui>
#include "cpuPlot.h"


class TimeScaleDraw : public QwtScaleDraw
{
public:
    TimeScaleDraw(const QTime &base) :
        baseTime(base)
    {
    }

    virtual QwtText label(double v) const//plot类会一次次的调用这个函数为坐标轴上的刻度标签赋值
    {
        QTime upTime = baseTime.addSecs(static_cast<int> (v));
        return upTime.toString("hh:mm:ss");
    }
private:
    QTime baseTime;
};

class Background: public QwtPlotItem
{
public:
    Background()
    {
        setZ(0.0);
    }

    virtual int rtti() const
    {
        return QwtPlotItem::Rtti_PlotUserItem;
    }

    virtual void draw(QPainter *painter,
                      const QwtScaleMap &xMap,
                      const QwtScaleMap &yMap,
                      const QRectF &canvasRect) const//函数只被调用了一次
    {
//        static int i = 0;
//        qDebug() << i++;
        QColor c(Qt::white);
        QRectF r = canvasRect;//获得坐标轴内画布的大小

        int step = 10;
        for (int i=100;i > 0;i -= step){  //100 约数，可以修改。
            r.setBottom(yMap.transform( i - step )); //transform 将标尺坐标系中的一个值转换到绘图设备坐标系中的一个值。
            r.setTop(yMap.transform(i));
            painter->fillRect( r,c );

            c = c.dark( 110 );
        }//作用，绘制背景颜色。疑惑：难道这个item是一个和画布一样大小的部件吗？
    }
};

class CpuCurve : public QwtPlotCurve
{
public:
    CpuCurve(const QString &title):QwtPlotCurve(title)
    {
        setRenderHint(QwtPlotItem::RenderAntialiased);//反锯齿样式
    }

    void setColor( const QColor &color)
    {
        QColor c = color;
        c.setAlpha(150);

        setPen(c);
//        setPen(QPen(Qt::NoPen));//绘制折线图
//        setBrush(c);//用于绘制矩形区域
    }


};

CputPlot::CputPlot(QWidget *parent) :
    QwtPlot(parent),
    dataCount(0)
{
    setAutoReplot(false);

    QwtPlotCanvas * canvas = new QwtPlotCanvas();
    canvas->setBorderRadius(10);//画布的周边角

    setCanvas(canvas);

    plotLayout()->setAlignCanvasToScales(true);//画布与坐标轴水平对齐

    QwtLegend * legend = new QwtLegend();
    legend->setDefaultItemMode(QwtLegendData::Checkable);//设置图例的样式
    insertLegend(legend,QwtPlot::RightLegend);//添加图例并设置图例的位置

    setAxisTitle(QwtPlot::xBottom," SYstem Uptime [h:m:s]");
    setAxisScaleDraw(QwtPlot::xBottom,
                     new TimeScaleDraw(QDateTime::currentDateTime().time()));
    setAxisScale(QwtPlot::xBottom,0,HISTORY);//一屏显示60个小间隔
    setAxisLabelRotation(QwtPlot::xBottom,-50.0);
    setAxisLabelAlignment(QwtPlot::xBottom,Qt::AlignLeft | Qt::AlignBottom);//设置坐标轴的位置和旋转角度

    /*
     在一些情况，当一个标签在scale更多右边的位置，额外的空间需要被显示在Lable重复的部分
     这样的label会通过减少scale和画布的宽度。为避免“跳跃画布”的影响，我们添加一个永恒的边缘。
     我们不需要去从左边的边框，这样会有左刻度下的重复标签有足够的空间
*/
    /*
     In situations, when there is a label at the most right position of the
     scale, additional space is needed to display the overlapping part
     of the label would be taken by reducing the width of scale and canvas.
     To avoid this "jumping canvas" effect, we add a permanent margin.
     We don't need to do the same for the left border, because there
     is enough space for the overlapping label below the left scale.
     */

    QwtScaleWidget * scaleWidget = axisWidget(QwtPlot::xBottom);//QwtScaleWidget：A Widget which contains a scale
    const int fmh = QFontMetrics( scaleWidget->font()).height();
    scaleWidget->setMinBorderDist(0,fmh / 2);//设置一个距离的最小值，这个scale的端点到部件的边框

    setAxisTitle(QwtPlot::yLeft,"Cpu Usage [%]");
    setAxisScale(QwtPlot::yLeft,0.0,100.0);

    Background * bg = new Background();
    bg->attach(this);

    CpuCurve * curve;

    curve = new CpuCurve("System");
    curve->setColor(Qt::red);
    curve->attach(this);
    data[System].curve = curve;

    curve = new CpuCurve("User");
    curve->setColor(Qt::blue);
    curve->setZ(curve->z()-1);//Z值越大，越在表层上实现。
    curve->attach(this);
    data[User].curve = curve;

    curve = new CpuCurve("Total");
    curve->setColor(Qt::black);
    curve->setZ(curve->z()-2);
    curve->attach(this);
    data[Total].curve = curve;


    curve = new CpuCurve("Idle");
    curve->setColor(Qt::yellow);
    curve->setZ(curve->z()-3);
    curve->attach(this);
    data[Idle].curve = curve;

    showCurve(data[System].curve,true);
    showCurve(data[User].curve,true);
    showCurve(data[Total].curve,false);
    showCurve(data[Idle].curve,false);

    CpuPieMarker *pie = new CpuPieMarker();//扇形图
    pie->attach(this);

    for (int var = 0; var < HISTORY; ++var)
        timeData[HISTORY - 1 -var] = var;
     startTimer(1000);//不知道为什么前面要加一个void，已去掉

     connect( legend, SIGNAL( checked( const QVariant &, bool, int ) ),SLOT( legendChecked( const QVariant &, bool ) ) );

}

//CputPlot::~CputPlot(){}

void CputPlot::timerEvent(QTimerEvent *)
{
    for ( int i = dataCount; i  > 0; --i ) {//dataCount 默认为0
        for ( int c = 0; c < NCputData; ++c ) {
            if (i < HISTORY)
                data[c].data[i] = data[c].data[i-1];
        }
    }

    cpuStart.statistic(data[User].data[0],data[System].data[0]);//统计数据

    data[Total].data[0] = data[User].data[0] + data[System].data[0];
    data[Idle].data[0] = 100.0 - data[Total].data[0];
    //至此，完成四个曲线数据的获取

    if ( dataCount < HISTORY )//dataCount 决定了一屏要显示多少个数据点
        dataCount++;

    for (int j = 0; j < HISTORY; ++j)//循环来绘制刻度标签
        timeData[j]++;
    setAxisScale(QwtPlot::xBottom,
                 timeData[HISTORY - 1], timeData[0]);

    for (int c = 0; c < NCputData; ++c) {
        data[c].curve->setRawSamples(
                    timeData,data[c].data,dataCount);
    }
    replot();
}

void CputPlot::showCurve(QwtPlotItem *item, bool b)
{
    item->setVisible(b);//显示/隐藏这个item

    QwtLegend * lgd = qobject_cast<QwtLegend*> ( legend() );
    /*!
     QwtLegend 部件是一个图层项的表格安排。
    egend项可能是任意的窗口部件，但通常是一个QwtLegendLabel。
*/
    QList<QWidget*> legendWidgets =
            lgd->legendWidgets(itemToInfo(item));

    if( legendWidgets.size() == 1){
        QwtLegendLabel * legendLabel =
                qobject_cast<QwtLegendLabel *>( legendWidgets[0]);
        if(legendLabel){
            legendLabel->setChecked(b);
        }
    }//费心费力，难道就为了得到一个label,然后去改变他的状态

    replot();
}

void CputPlot::legendChecked(const QVariant &itemInfo, bool b)
{
    QwtPlotItem *plotItem = infoToItem(itemInfo);
    if ( plotItem)
        showCurve(plotItem,b);

}



int main(int argc, char *argv[])
{
    QApplication app(argc,argv);

    QWidget w;
    w.setWindowTitle("A Cpu Plot Time");

    CputPlot * plot = new CputPlot(&w);
    plot->setTitle("History");
    QVBoxLayout *vLayout = new QVBoxLayout(&w);
    vLayout->addWidget(plot);

    w.resize(600,400);
    w.show();

//    qDebug() << 5760/2;
    return app.exec();
}














