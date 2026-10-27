#include <basewidgets/Charts/RecordingTimelinePlot.h>
#include <basewidgets/Core/ThemeTokens.h>
#include <QSet>
#include <cmath>
namespace basewidgets {
RecordingTimelinePlot::RecordingTimelinePlot(QWidget* parent) : StyledPlot(parent)
{
    setPlotTitle("录音时间线 · 点击记录定位"); xAxis->setLabel("时间 (s)"); yAxis->setLabel("通道");
    yAxis->setRangeReversed(true); yAxis->grid()->setVisible(false); xAxis->setRange(0,60); yAxis->setRange(.5,1.5);
    connect(this,&QCustomPlot::beforeReplot,this,&RecordingTimelinePlot::layoutLabels);
    connect(ThemeTokens::instance(),&ThemeTokens::tokensChanged,this,&RecordingTimelinePlot::colorItems);
    captureResetView();
    connect(this,&StyledPlot::coordinateClicked,this,[this](double x,double y,Qt::KeyboardModifiers) {
        for(int i=m_records.size()-1;i>=0;--i) {
            const int row=m_channels.indexOf(m_records[i].channel)+1;
            if(x>=m_records[i].startSeconds && x<=m_records[i].endSeconds && std::abs(y-row)<=.32) {
                selectRecord(m_records[i].id); emit recordSelected(m_records[i].id); return;
            }
        }
    });
}
bool RecordingTimelinePlot::setRecords(const QVector<RecordingInterval>& records)
{
    if(records.size()>10000) return false;
    QSet<QString> ids; QStringList channels;
    double first=std::numeric_limits<double>::infinity(),last=-first;
    for(const auto& record:records) {
        if(record.id.isEmpty() || record.channel.isEmpty() || ids.contains(record.id) || !std::isfinite(record.startSeconds)
            || !std::isfinite(record.endSeconds) || record.endSeconds<=record.startSeconds || std::abs(record.startSeconds)>1e12
            || std::abs(record.endSeconds)>1e12 || (record.state!=RecordingState::Ready && record.state!=RecordingState::Processing && record.state!=RecordingState::Flagged)) return false;
        ids.insert(record.id); if(!channels.contains(record.channel)) channels.append(record.channel);
        if(channels.size()>64) return false;
        first=std::min(first,record.startSeconds); last=std::max(last,record.endSeconds);
    }
    for(auto* item:m_rectangles) removeItem(item);
    for(auto* item:m_labels) removeItem(item);
    m_labels.clear(); m_rectangles.clear(); m_records=records; m_channels=channels; if(!ids.contains(m_selected)) { m_selected.clear(); setToolTip(QString()); }
    auto ticker=QSharedPointer<QCPAxisTickerText>(new QCPAxisTickerText);
    for(int i=0;i<channels.size();++i) ticker->addTick(i+1,channels[i]);
    yAxis->setTicker(ticker); yAxis->setRange(.5,std::max(1,channels.size())+.5);
    if(!records.isEmpty()) { const double pad=std::max((last-first)*.03,.01); xAxis->setRange(first-pad,last+pad); }
    for(const auto& record:records) {
        auto* item=new QCPItemRect(this); const int row=channels.indexOf(record.channel)+1;
        item->topLeft->setCoords(record.startSeconds,row-.32); item->bottomRight->setCoords(record.endSeconds,row+.32);
        item->setClipToAxisRect(true); m_rectangles.append(item);
        auto* label=new QCPItemText(this); label->position->setCoords((record.startSeconds+record.endSeconds)/2,row);
        label->setClipToAxisRect(true); label->setPadding(QMargins(3,0,3,0)); label->setFont(font()); m_labels.append(label);
    }
    captureResetView(); colorItems(); replot(); return true;
}
void RecordingTimelinePlot::setUtcTimeAxis(bool utc)
{
    if(utc) {
        auto ticker=QSharedPointer<QCPAxisTickerDateTime>(new QCPAxisTickerDateTime);
        ticker->setDateTimeSpec(Qt::UTC); ticker->setDateTimeFormat("MM-dd\nhh:mm:ss"); xAxis->setTicker(ticker); xAxis->setLabel("录音时间 (UTC)");
    } else { xAxis->setTicker(QSharedPointer<QCPAxisTicker>(new QCPAxisTicker)); xAxis->setLabel("相对时间 (s)"); }
    replot();
}
bool RecordingTimelinePlot::selectRecord(const QString& id)
{
    bool found=id.isEmpty(); for(const auto& record:m_records) if(record.id==id) found=true;
    if(!found) return false;
    m_selected=id; if(id.isEmpty()) setToolTip(QString()); colorItems();
    for(const auto& record:m_records) if(record.id==id) setToolTip(QString("%1\n%2\n%3 s").arg(record.label,record.channel).arg(record.endSeconds-record.startSeconds,0,'f',3));
    replot(); return true;
}
void RecordingTimelinePlot::colorItems()
{
    auto* tokens=ThemeTokens::instance();
    for(int i=0;i<m_rectangles.size();++i) {
        const auto& record=m_records[i];
        QColor fill=record.state==RecordingState::Ready ? tokens->primary() : record.state==RecordingState::Processing ? tokens->warning() : tokens->danger();
        fill.setAlpha(160); m_rectangles[i]->setBrush(fill);
        m_rectangles[i]->setPen(QPen(record.id==m_selected ? tokens->textPrimary() : fill,record.id==m_selected ? 2 : 1));
    }
    replot();
}
void RecordingTimelinePlot::layoutLabels()
{
    for(int i=0;i<m_labels.size();++i) {
        const auto& record=m_records[i]; const double row=m_channels.indexOf(record.channel)+1;
        const double left=std::max(record.startSeconds,xAxis->range().lower),right=std::min(record.endSeconds,xAxis->range().upper);
        const double pixels=right>left ? std::abs(xAxis->coordToPixel(right)-xAxis->coordToPixel(left)) : 0;
        const double rowHeight=std::abs(yAxis->coordToPixel(row+.32)-yAxis->coordToPixel(row-.32));
        const QFontMetrics metrics(m_labels[i]->font());
        m_labels[i]->setVisible(pixels>40 && rowHeight>metrics.height()+3);
        m_labels[i]->position->setCoords((left+right)/2,row);
        const QString state=record.state==RecordingState::Ready ? tr("就绪") : record.state==RecordingState::Processing ? tr("处理中") : tr("待复核");
        const QString description=(record.label.isEmpty()?record.id:record.label)+" · "+state;
        m_labels[i]->setText(metrics.elidedText(description,Qt::ElideRight,std::max(0,int(pixels)-10)));
        m_labels[i]->setColor(ThemeTokens::instance()->textPrimary());
    }
}
}
