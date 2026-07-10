#include "WindTrendChart.h"

#include <QDateTime>
#include <QPainter>

WindTrendChart::WindTrendChart(QWidget *parent) : QWidget(parent), m_timeWindow("1min"), m_refreshTimer(new QTimer(this)) { setupUI(); setMinimumSize(300,200); setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Expanding); }
WindTrendChart::~WindTrendChart() = default;
void WindTrendChart::setTimeWindow(const QString &window) { if (m_timeWindow != window) { m_timeWindow=window; emit timeWindowChanged(window); } }
void WindTrendChart::addDataPoint(double value) { m_data.append(QPointF(QDateTime::currentMSecsSinceEpoch(),value)); while(m_data.size()>1000)m_data.removeFirst(); update(); }
void WindTrendChart::clear() {m_data.clear();update();}
void WindTrendChart::setupUI(){setStyleSheet("background:#ffffff; border:0; border-radius:0;");}
void WindTrendChart::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event) QPainter p(this);p.setRenderHint(QPainter::Antialiasing);p.fillRect(rect(),QColor("#fff"));const QRect area=rect().adjusted(52,14,-16,-30);
    if(m_data.isEmpty()){p.setPen(QColor("#7b8794"));p.setFont(QFont("Microsoft YaHei",10));p.drawText(rect(),Qt::AlignCenter,QStringLiteral("暂无趋势数据"));return;}
    double min=m_data.first().y(),max=min;for(const auto &point:m_data){min=qMin(min,point.y());max=qMax(max,point.y());}double range=qMax(1.0,max-min);min-=range*.1;max+=range*.1;range=max-min;
    p.setFont(QFont("Microsoft YaHei",8));for(int i=0;i<=4;++i){int y=area.top()+i*area.height()/4;p.setPen(QPen(QColor("#e6eaee"),1,Qt::DashLine));p.drawLine(area.left(),y,area.right(),y);p.setPen(QColor("#7b8794"));p.drawText(QRect(0,y-8,area.left()-5,16),Qt::AlignRight|Qt::AlignVCenter,QString::number(max-i*range/4,'f',1));}
    if(m_data.size()<2)return;const qint64 first=m_data.first().x(),last=m_data.last().x(),span=qMax<qint64>(1,last-first);QPainterPath path;for(int i=0;i<m_data.size();++i){const auto &point=m_data[i];const int x=area.left()+int((point.x()-first)*area.width()/span);const int y=area.top()+int((max-point.y())*area.height()/range);if(i==0)path.moveTo(x,y);else path.lineTo(x,y);}QPainterPath fillPath(path);fillPath.lineTo(area.right(),area.bottom());fillPath.lineTo(area.left(),area.bottom());fillPath.closeSubpath();p.fillPath(fillPath,QColor(52, 152, 219, 24));p.setPen(QPen(QColor("#2274a5"),2));p.drawPath(path);
}
void WindTrendChart::updateChart(){update();}
