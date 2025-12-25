/*
 * Author: zhoutao@zkhx.com
 * Date: 2025-12-25
 */
// Ported to match test/source/qcgaugewidget.* behavior exactly
#ifndef QTADVANCEDUI_QCGAUGEWIDGET_H
#define QTADVANCEDUI_QCGAUGEWIDGET_H

#include <QtAdvancedUI/QtAdvancedUIExport.h>
#include <QtAdvancedUI/Core/StyledWidget.h>
#include <QWidget>
#include <QPainter>
#include <QObject>
#include <QRectF>
#include <QPainterPath>
#include <QtMath>

namespace QtAdvancedUI {

class QcGaugeWidget;
class QcItem;
class QcBackgroundItem;
class QcDegreesItem;
class QcValuesItem;
class QcArcItem;
class QcColorBand;
class QcNeedleItem;
class QcLabelItem;
class QcGlassItem;
class QcAttitudeMeter;

class ACSS_UI_EXPORT QcGaugeWidget : public StyledWidget
{
    Q_OBJECT
public:
    explicit QcGaugeWidget(QWidget *parent = nullptr);

    QcBackgroundItem* addBackground(float position);
    QcDegreesItem* addDegrees(float position);
    QcValuesItem* addValues(float position);
    QcArcItem* addArc(float position);
    QcColorBand* addColorBand(float position);
    QcNeedleItem* addNeedle(float position);
    QcLabelItem* addLabel(float position);
    QcGlassItem* addGlass(float position);
    QcAttitudeMeter* addAttitudeMeter(float position);

    void addItem(QcItem* item, float position);
    int removeItem(QcItem* item);
    QList<QcItem*> items();
    QList<QcItem*> mItems;

protected:
    void paintEvent(QPaintEvent *) override;
};

class ACSS_UI_EXPORT QcItem : public QObject
{
    Q_OBJECT
public:
    explicit QcItem(QObject *parent = nullptr);
    virtual void draw(QPainter *) = 0;
    virtual int type();

    void setPosition(float percentage);
    float position();
    QRectF rect();
    enum Error{InvalidValueRange,InvalidDegreeRange,InvalidStep};

protected:
    QRectF adjustRect(float percentage);
    float getRadius(const QRectF &);
    float getAngle(const QPointF&, const QRectF &tmpRect);
    QPointF getPoint(float deg, const QRectF &tmpRect);
    QRectF resetRect();
    void update();

private:
    QRectF mRect;
    QWidget *parentWidget;
    float mPosition;
};

class ACSS_UI_EXPORT QcScaleItem : public QcItem
{
    Q_OBJECT
public:
    explicit QcScaleItem(QObject *parent = nullptr);

    void setValueRange(float minValue,float maxValue);
    void setDgereeRange(float minDegree,float maxDegree);
    void setMinValue(float minValue);
    void setMaxValue(float maxValue);
    void setMinDegree(float minDegree);
    void setMaxDegree(float maxDegree);

protected:
    float getDegFromValue(float);

    float mMinValue;
    float mMaxValue;
    float mMinDegree;
    float mMaxDegree;
};

class ACSS_UI_EXPORT QcBackgroundItem : public QcItem
{
    Q_OBJECT
public:
    explicit QcBackgroundItem(QObject *parent = nullptr);
    void draw(QPainter*);
    void addColor(float position, const QColor& color);
    void clearrColors();

private:
    QPen mPen;
    QList<QPair<float,QColor> > mColors;
};

class ACSS_UI_EXPORT QcGlassItem : public QcItem
{
    Q_OBJECT
public:
    explicit QcGlassItem(QObject *parent = nullptr);
    void draw(QPainter*);
};

class ACSS_UI_EXPORT QcLabelItem : public QcItem
{
    Q_OBJECT
public:
    explicit QcLabelItem(QObject *parent = nullptr);
    virtual void draw(QPainter *);
    void setAngle(float);
    float angle();
    void setText(const QString &text, bool repaint = true);
    QString text();
    void setColor(const QColor& color);
    QColor color();

private:
    float mAngle;
    QString mText;
    QColor mColor;
};

class ACSS_UI_EXPORT QcArcItem : public QcScaleItem
{
    Q_OBJECT
public:
    explicit QcArcItem(QObject *parent = nullptr);
    void draw(QPainter*);
    void setColor(const QColor& color);

private:
    QColor mColor;
};

class ACSS_UI_EXPORT QcColorBand : public QcScaleItem
{
    Q_OBJECT
public:
    explicit QcColorBand(QObject *parent = nullptr);
    void draw(QPainter*);
    void setColors(const QList<QPair<QColor,float> >& colors);

private:
   QPainterPath createSubBand(float from,float sweep);

   QList<QPair<QColor,float> > mBandColors;
   float mBandStartValue = 0;
};

class ACSS_UI_EXPORT QcDegreesItem : public QcScaleItem
{
    Q_OBJECT
public:
    explicit QcDegreesItem(QObject *parent = nullptr);
    void draw(QPainter *painter);
    void setStep(float step);
    void setColor(const QColor& color);
    void setSubDegree(bool );
private:
    float mStep;
    QColor mColor;
    bool mSubDegree;
};

class ACSS_UI_EXPORT QcNeedleItem : public QcScaleItem
{
    Q_OBJECT
public:
    explicit QcNeedleItem(QObject *parent = nullptr);
    void draw(QPainter*);
    void setCurrentValue(float value);
    float currentValue();
    void setValueFormat(QString format);
    QString currentValueFormat();
    void setColor(const QColor & color);
    QColor color();

    void setLabel(QcLabelItem*);
    QcLabelItem * label();

    enum NeedleType{DiamonNeedle,TriangleNeedle,FeatherNeedle,AttitudeMeterNeedle,CompassNeedle};

    void setNeedle(QcNeedleItem::NeedleType needleType);
private:
    QPolygonF mNeedlePoly;
    float mCurrentValue;
    QColor mColor;
    void createDiamonNeedle(float r);
    void createTriangleNeedle(float r);
    void createFeatherNeedle(float r);
    void createAttitudeNeedle(float r);
    void createCompassNeedle(float r);
    NeedleType mNeedleType;
    QcLabelItem *mLabel;
    QString mFormat;
};

class ACSS_UI_EXPORT QcValuesItem : public QcScaleItem
{
    Q_OBJECT
public:
    explicit QcValuesItem(QObject *parent = nullptr);
    void draw(QPainter*);
    void setStep(float step);
    void setColor(const QColor& color);
private:
    float mStep;
    QColor mColor;
};

class ACSS_UI_EXPORT QcAttitudeMeter : public QcItem
{
    Q_OBJECT
public:
    explicit QcAttitudeMeter(QObject *parent = nullptr);

    void draw(QPainter *);
    void setCurrentPitch(float pitch);
    void setCurrentRoll(float roll);
private:
    float mRoll;
    float mPitch;
    float mPitchOffset;

    QPolygonF mHandlePoly;
    QPainterPath mStepsPath;

    QPointF getIntersection(float r,const QPointF& pitchPoint,const QPointF& pt);
    float getStartAngle(const QRectF& tmpRect);

    void drawDegrees(QPainter *);
    void drawDegree(QPainter * painter, const QRectF& tmpRect,float deg);
    void drawUpperEllipse(QPainter *,const QRectF&);
    void drawLowerEllipse(QPainter *,const QRectF&);
    void drawPitchSteps(QPainter *,const QRectF&);
    void drawHandle(QPainter *);
    void drawSteps(QPainter *,float);
};

} // namespace QtAdvancedUI

#endif // QTADVANCEDUI_QCGAUGEWIDGET_H

