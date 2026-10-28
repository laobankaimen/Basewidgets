#include <basewidgets/Charts/SampledGraph.h>
#include <array>
#include <algorithm>
#include <cmath>
#include <limits>
namespace basewidgets {
SampledGraph::SampledGraph(QCPAxis* key,QCPAxis* value) : QCPGraph(key,value) {}
QVector<QCPGraphData> SampledGraph::displaySamples() const
{
    QVector<QCPGraphData> result;
    if(!keyAxis()||data()->isEmpty())return result;
    QCPGraphDataContainer::const_iterator begin,end;
    getVisibleDataBounds(begin,end,QCPDataRange(0,dataCount()));
    getOptimizedLineData(&result,begin,end); return result;
}
void SampledGraph::getOptimizedLineData(QVector<QCPGraphData>* output,const QCPGraphDataContainer::const_iterator& begin,
                                      const QCPGraphDataContainer::const_iterator& end) const
{
    output->clear(); if(begin==end||!keyAxis())return;
    const auto* axis=keyAxis(); const auto range=axis->range();
    const int pixels=axis->orientation()==Qt::Horizontal?axis->axisRect()->width():axis->axisRect()->height();
    const int buckets=qBound(1,pixels,4096);
    output->reserve(qMin(int(end-begin),9*(buckets+2)));
    const bool log=axis->scaleType()==QCPAxis::stLogarithmic;
    const double lower=log?std::log(std::abs(range.lower)):range.lower;
    const double upper=log?std::log(std::abs(range.upper)):range.upper;
    const double span=upper-lower;
    if(!std::isfinite(span)||span==0)return;
    auto bucketFor=[&](double key) {
        const double coordinate=log?std::log(std::abs(key)):key;
        const double fraction=(coordinate-lower)/span;
        // Adjacent samples outside the viewport are kept, without converting
        // unbounded pixel coordinates to int. Reversed axes use the same buckets.
        if(!(fraction>=0))return -1;
        if(fraction>=1)return buckets;
        return int(fraction*buckets);
    };
    struct Point { QCPGraphData data; int index=-1,gaps=0; };
    Point first,last,minimum,maximum;
    bool active=false,haveBucket=false;
    int current=0,index=0,gaps=0,lastOutputGaps=0;
    double gapKey=begin->key;
    auto gap=[&](double key) {
        if(output->isEmpty()||std::isfinite(output->last().value))output->append(QCPGraphData(key,std::numeric_limits<double>::quiet_NaN()));
    };
    auto flush=[&] {
        if(active) {
            std::array<Point,4> points{{first,minimum,maximum,last}};
            std::sort(points.begin(),points.end(),[](const Point& a,const Point& b) { return a.index<b.index; });
            int previous=-1;
            for(const auto& point:points) {
                if(point.index==previous)continue;
                if(point.gaps!=lastOutputGaps)gap(point.data.key);
                output->append(point.data); previous=point.index; lastOutputGaps=point.gaps;
            }
        }
        if(gaps!=lastOutputGaps) { gap(gapKey); lastOutputGaps=gaps; }
        active=false;
    };
    for(auto it=begin;it!=end;++it,++index) {
        const int bucket=bucketFor(it->key);
        if(haveBucket&&bucket!=current)flush();
        current=bucket; haveBucket=true;
        if(!std::isfinite(it->value)) { ++gaps; gapKey=it->key; continue; }
        Point point{*it,index,gaps};
        if(!active) { first=minimum=maximum=point; active=true; }
        if(it->value<minimum.data.value)minimum=point;
        if(it->value>maximum.data.value)maximum=point;
        last=point;
    }
    flush();
}
void SampledGraph::getOptimizedScatterData(QVector<QCPGraphData>* output,QCPGraphDataContainer::const_iterator begin,
                                         QCPGraphDataContainer::const_iterator end) const
{
    getOptimizedLineData(output,begin,end);
    output->erase(std::remove_if(output->begin(),output->end(),[](const QCPGraphData& point) { return !std::isfinite(point.value); }),output->end());
    // Honor caller scatterSkip as an additional display reduction, after sampling.
    if(scatterSkip()>0) {
        int kept=0; const qint64 stride=qint64(scatterSkip())+1;
        for(qint64 i=0;i<output->size();i+=stride)(*output)[kept++]=output->at(int(i));
        output->resize(kept);
    }
}
}
