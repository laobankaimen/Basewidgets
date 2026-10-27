#include <basewidgets/Acoustics/AcousticAnalysis.h>
#include <cmath>
#include <complex>
#include <algorithm>
namespace basewidgets {
namespace {
constexpr double pi=3.14159265358979323846;
double nan() { return std::numeric_limits<double>::quiet_NaN(); }
bool validUnit(AcousticUnit unit) { return unit==AcousticUnit::FullScale || unit==AcousticUnit::Volt || unit==AcousticUnit::MicroPascal || unit==AcousticUnit::RelativeEnvelope; }
bool validScaling(SpectrumScaling scaling) { return scaling==SpectrumScaling::Power || scaling==SpectrumScaling::PowerSpectralDensity; }
bool validWindow(AcousticWindow window) { return window==AcousticWindow::Hann || window==AcousticWindow::Rectangular; }
}
QString AcousticAnalysis::amplitudeLabel(AcousticUnit unit)
{
    switch(unit) { case AcousticUnit::FullScale:return "归一化幅值 (FS)"; case AcousticUnit::Volt:return "电压 (V)"; case AcousticUnit::MicroPascal:return "声压 (μPa)"; case AcousticUnit::RelativeEnvelope:return "归一化包络波动"; }
    return QString();
}
QString AcousticAnalysis::levelLabel(AcousticUnit unit,bool density)
{
    switch(unit) {
    case AcousticUnit::FullScale:return density ? "PSD (dBFS/Hz)" : "电平 (dBFS)";
    case AcousticUnit::Volt:return density ? "PSD (dB re 1 V²/Hz)" : "电平 (dB re 1 V)";
    case AcousticUnit::MicroPascal:return density ? "PSD (dB re 1 μPa²/Hz)" : "声压级 (dB re 1 μPa)";
    case AcousticUnit::RelativeEnvelope:return density ? "包络 PSD (dB re 1/Hz)" : "包络功率 (dB re 1)";
    }
    return QString();
}
QString AcousticSpectrumFrame::levelLabel() const { return AcousticAnalysis::levelLabel(unit,scaling==SpectrumScaling::PowerSpectralDensity); }
bool AcousticSpectrumFrame::isValid() const
{
    if (!std::isfinite(sampleRate) || sampleRate<=0 || fftSize<16 || fftSize>65536 || (fftSize&(fftSize-1))
        || !validUnit(unit) || !validScaling(scaling) || !validWindow(window) || !std::isfinite(floorDb)) return false;
    if(!std::isfinite(binWidth()) || binWidth()<=0) return false;
    const int bins=fftSize/2+1;
    if (frequencies.size()!=bins || levelsDb.size()!=bins || linearPower.size()!=bins) return false;
    for (int i=0;i<bins;++i) {
        if (!std::isfinite(frequencies[i]) || std::abs(frequencies[i]-i*binWidth())>std::max(1e-9,binWidth()*1e-7)
            || !std::isfinite(levelsDb[i]) || !std::isfinite(linearPower[i]) || linearPower[i]<0) return false;
        const double expected=linearPower[i]>0 ? std::max(floorDb,10*std::log10(linearPower[i])) : floorDb;
        if(std::abs(levelsDb[i]-expected)>1e-6) return false;
    }
    return true;
}
SignalStatistics AcousticAnalysis::statistics(const QVector<double>& samples,double clippingLevel)
{
    SignalStatistics result; result.sampleCount=samples.size();
    // Normalize by peak before accumulating squares. This is stable even where
    // long double equals double (MSVC) and raw samples approach the floating-point limit.
    for (double value:samples) {
        if (!std::isfinite(value)) { ++result.invalidCount; continue; }
        ++result.validCount; result.peak=std::max(result.peak,std::abs(value));
        if (std::isfinite(clippingLevel) && clippingLevel>0 && std::abs(value)>=clippingLevel) ++result.clippedCount;
    }
    if (!result.valid()) return result;
    long double sum=0,squares=0;
    if(result.peak>0) for(double value:samples) if(std::isfinite(value)) {
        const long double scaled=value/result.peak; sum+=scaled; squares+=scaled*scaled;
    }
    result.mean=double(sum/result.validCount)*result.peak;
    result.rms=std::min(1.,double(std::sqrt(squares/result.validCount)))*result.peak;
    result.rmsDb=result.rms>0 ? 20*std::log10(result.rms) : -std::numeric_limits<double>::infinity();
    result.peakDb=result.peak>0 ? 20*std::log10(result.peak) : -std::numeric_limits<double>::infinity();
    result.crestFactorDb=result.rms>0 ? result.peakDb-result.rmsDb : nan();
    return result;
}
bool AcousticAnalysis::spectrum(const QVector<double>& samples,double sampleRate,AcousticSpectrumFrame* output,
    AcousticUnit unit,SpectrumScaling scaling,AcousticWindow window,bool removeMean,double floorDb,QString* error)
{
    auto fail=[&](const QString& message) { if(error)*error=message; return false; };
    const int n=samples.size();
    if(!output || n<16 || n>65536 || (n&(n-1))) return fail("FFT 长度必须是 16–65536 内的 2 次幂");
    if(!std::isfinite(sampleRate) || sampleRate<=0 || !std::isfinite(sampleRate/n) || sampleRate/n<=0
        || !validUnit(unit) || !validScaling(scaling) || !validWindow(window) || !std::isfinite(floorDb)
        || floorDb< -600 || floorDb>300) return fail("采样率、单位、窗函数或显示下限无效");
    long double mean=0;
    for(double value:samples) { if(!std::isfinite(value)) return fail("FFT 输入含缺样/非有限值，请先处理数据间隙"); mean+=value; }
    mean=removeMean ? mean/n : 0;
    QVector<std::complex<double>> data(n); double sumWindow=0,sumWindowSquared=0;
    for(int i=0;i<n;++i) {
        const double weight=window==AcousticWindow::Hann ? .5-.5*std::cos(2*pi*i/n) : 1.;
        const double value=double((static_cast<long double>(samples[i])-mean)*weight);
        if(!std::isfinite(value)) return fail("输入幅值超出计算范围");
        data[i]=value; sumWindow+=weight; sumWindowSquared+=weight*weight;
    }
    for(int i=1,j=0;i<n;++i) {
        int bit=n>>1; for(;j&bit;bit>>=1) j^=bit; j^=bit;
        if(i<j) std::swap(data[i],data[j]);
    }
    for(int length=2;length<=n;length<<=1) {
        const auto step=std::polar(1.,-2*pi/length);
        for(int i=0;i<n;i+=length) {
            std::complex<double> rotation(1,0);
            for(int j=0;j<length/2;++j) {
                const auto even=data[i+j], odd=data[i+j+length/2]*rotation;
                data[i+j]=even+odd; data[i+j+length/2]=even-odd; rotation*=step;
            }
        }
    }
    AcousticSpectrumFrame frame; frame.fftSize=n; frame.sampleRate=sampleRate; frame.unit=unit; frame.scaling=scaling; frame.window=window; frame.meanRemoved=removeMean; frame.floorDb=floorDb;
    const double denominator=scaling==SpectrumScaling::PowerSpectralDensity ? sampleRate*sumWindowSquared : sumWindow*sumWindow;
    if(!std::isfinite(denominator) || denominator<=0) return fail("谱归一化系数超出范围");
    for(int k=0;k<=n/2;++k) {
        const double power=std::norm(data[k])/denominator*((k==0 || k==n/2)?1.:2.);
        if(!std::isfinite(power)) return fail("谱功率溢出");
        frame.frequencies.append(k*frame.binWidth()); frame.linearPower.append(power);
        frame.levelsDb.append(power>0 ? std::max(floorDb,10*std::log10(power)) : floorDb);
    }
    if(!frame.isValid()) return fail("生成频谱无效");
    *output=frame; if(error)error->clear(); return true;
}
double AcousticAnalysis::bandPowerDb(const AcousticSpectrumFrame& frame,double lower,double upper)
{
    if(!frame.isValid() || frame.scaling!=SpectrumScaling::PowerSpectralDensity || !std::isfinite(lower)
        || !std::isfinite(upper) || lower<0 || upper<lower || upper>frame.sampleRate/2) return nan();
    long double sum=0; int count=0;
    for(int i=0;i<frame.frequencies.size();++i) if(frame.frequencies[i]>=lower && frame.frequencies[i]<=upper) { sum+=frame.linearPower[i]*frame.binWidth(); ++count; }
    if(!count) return nan();
    return sum>0 ? double(10*std::log10(sum)) : -std::numeric_limits<double>::infinity();
}
bool AcousticAnalysis::scaleToMicroPascals(const QVector<double>& samples,double scale,QVector<double>* output)
{
    if(!output || !std::isfinite(scale) || scale<=0) return false;
    QVector<double> converted; converted.reserve(samples.size());
    for(double value:samples) {
        if(std::isinf(value)) return false;
        const double scaled=value*scale;
        if(std::isfinite(value) && !std::isfinite(scaled)) return false;
        converted.append(scaled);
    }
    *output=converted; return true;
}
}
