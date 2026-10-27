#include <basewidgets/Acoustics/DemonAnalysis.h>
#include <complex>
#include <cmath>
namespace basewidgets {
namespace {
constexpr double pi=3.14159265358979323846;
bool validSettings(int count,double rate,const DemonSettings& settings)
{
    if(count<256||count>65536||(count&(count-1))||!std::isfinite(rate)||rate<=0
        ||settings.decimation<1||settings.decimation>256||(settings.decimation&(settings.decimation-1))
        ||count/settings.decimation<16||!std::isfinite(settings.carrierLowerHz)||!std::isfinite(settings.carrierUpperHz)
        ||settings.carrierLowerHz<=0||settings.carrierUpperHz<=settings.carrierLowerHz||settings.carrierUpperHz>=rate/2
        ||!std::isfinite(settings.maximumModulationHz)||settings.maximumModulationHz<=0
        ||settings.maximumModulationHz<rate/count||settings.maximumModulationHz>.4*rate/settings.decimation||!std::isfinite(settings.floorDb)
        ||settings.floorDb< -600||settings.floorDb>300) return false;
    return settings.spectrumWindow==AcousticWindow::Hann||settings.spectrumWindow==AcousticWindow::Rectangular;
}
void fft(QVector<std::complex<double>>& values,bool inverse)
{
    const int n=values.size();
    for(int i=1,j=0;i<n;++i) { int bit=n>>1; for(;j&bit;bit>>=1) j^=bit; j^=bit; if(i<j) std::swap(values[i],values[j]); }
    for(int length=2;length<=n;length<<=1) {
        const auto step=std::polar(1.,(inverse?2:-2)*pi/length);
        for(int start=0;start<n;start+=length) {
            std::complex<double> rotation(1,0);
            for(int j=0;j<length/2;++j) {
                const auto a=values[start+j],b=values[start+j+length/2]*rotation;
                values[start+j]=a+b; values[start+j+length/2]=a-b; rotation*=step;
            }
        }
    }
    if(inverse) for(auto& value:values) value/=n;
}
}
bool DemonSettings::operator==(const DemonSettings& other) const
{
    return carrierLowerHz==other.carrierLowerHz&&carrierUpperHz==other.carrierUpperHz
        &&maximumModulationHz==other.maximumModulationHz&&decimation==other.decimation
        &&spectrumWindow==other.spectrumWindow&&floorDb==other.floorDb;
}
bool DemonResult::isValid() const
{
    if(!validSettings(inputSampleCount,inputSampleRate,settings)||(!std::isnan(meanEnvelope)&&(!std::isfinite(meanEnvelope)||meanEnvelope<=0))
        ||!spectrum.isValid()||spectrum.unit!=AcousticUnit::RelativeEnvelope
        ||spectrum.scaling!=SpectrumScaling::PowerSpectralDensity||!spectrum.meanRemoved
        ||spectrum.window!=settings.spectrumWindow||spectrum.floorDb!=settings.floorDb
        ||spectrum.fftSize!=inputSampleCount/settings.decimation||spectrum.sampleRate!=inputSampleRate/settings.decimation
        ||(!normalizedEnvelope.isEmpty()&&normalizedEnvelope.size()!=spectrum.fftSize)) return false;
    for(double value:normalizedEnvelope) if(!std::isfinite(value)) return false;
    return true;
}
bool DemonAnalysis::analyze(const QVector<double>& samples,double rate,const DemonSettings& settings,DemonResult* result,QString* error)
{
    auto fail=[&](const QString& message) { if(error)*error=message; return false; };
    const int n=samples.size();
    if(!result||!validSettings(n,rate,settings)) return fail("无效 DEMON 参数：检查载频带、调制带宽、2 次幂抽取率及样点数");
    QVector<std::complex<double>> analytic(n);
    for(int i=0;i<n;++i) { if(!std::isfinite(samples[i])) return fail("DEMON 输入含缺样/非有限值"); analytic[i]=samples[i]; }
    fft(analytic,false); int selectedBins=0;
    for(int k=0;k<n;++k) {
        const double frequency=k*rate/n;
        if(k>0&&k<n/2&&frequency>=settings.carrierLowerHz&&frequency<=settings.carrierUpperHz) {
            analytic[k]*=2; ++selectedBins;
        } else analytic[k]=0;
    }
    if(selectedBins<3) return fail("载频带内频点不足，请增加块长或扩大载频带");
    fft(analytic,true); long double sum=0;
    for(const auto& value:analytic) { const double magnitude=std::abs(value); if(!std::isfinite(magnitude)) return fail("包络计算溢出"); sum+=magnitude; }
    const double mean=double(sum/n);
    if(!std::isfinite(mean)||mean<=0||mean<=AcousticAnalysis::statistics(samples,0).rms*1e-12) return fail("载频带中没有足够数值精度的可归一化能量");
    for(auto& value:analytic) value=std::abs(value)/mean-1;
    // Anti-alias before decimation. The chosen band is below 80% of output Nyquist.
    fft(analytic,false);
    for(int k=0;k<n;++k) {
        const double frequency=std::min(k,n-k)*rate/n;
        if(k==0||frequency>settings.maximumModulationHz) analytic[k]=0;
    }
    fft(analytic,true);
    DemonResult output; output.settings=settings; output.inputSampleRate=rate; output.inputSampleCount=n; output.meanEnvelope=mean;
    for(int i=0;i<n;i+=settings.decimation) {
        const double value=analytic[i].real(); if(!std::isfinite(value)) return fail("归一化包络溢出"); output.normalizedEnvelope.append(value);
    }
    QString message;
    if(!AcousticAnalysis::spectrum(output.normalizedEnvelope,rate/settings.decimation,&output.spectrum,
        AcousticUnit::RelativeEnvelope,SpectrumScaling::PowerSpectralDensity,settings.spectrumWindow,true,settings.floorDb,&message)) return fail(message);
    if(!output.isValid()) return fail("DEMON 结果无效");
    *result=output; if(error) error->clear(); return true;
}
}
