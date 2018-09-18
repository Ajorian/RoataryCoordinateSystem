#ifndef __ROTARYCORDINATEWIDGET__
#define __ROTARYCORDINATEWIDGET__

#include <QWidget>
#include <QVector>
#include <QTimer>
#include <QGraphicsScene>
#include <QGraphicsView>
#include <QToolButton>
#include <QtMath>

struct RotaryItem 
{
	int		posIdx;
	double	posRad;
	double  scale;
	QWidget* widget;
	QGraphicsProxyWidget* proxy;
};

enum Direction
{
	DirectionCW = -1,
	DirectionCCW = 1
};

class RotaryCoordinateWidget : public QWidget
{
	Q_OBJECT
	
	QTimer timer;
	QWidget* topItem;
	QGraphicsScene* scene;
	QGraphicsView*  view;
	QGraphicsProxyWidget *nextBtn, *prvsBtn;
	QVector<RotaryItem> m_items;
	struct RotationInfo { double angle = 0;} m_rotationInfo;

	static const int  frameRate = 30;
	double velocity_RadPerSec = M_PI / 2;
	double targetPosition_Rad = 0, currentPosition_rad = 0;

public:
	RotaryCoordinateWidget(QWidget* parent);	
	void insertWidget(QWidget* itm);
	void sitItems();
	void pause() { timer.stop(); }
	void setVelocity(const double degPerSec);

private slots:
	void goPrevious();
	void goNext();
	void rotateOneStep();

private:
	int	 getTopItem()const;
	void resizeEvent(QResizeEvent *e);
	double getScale(double posRad);

signals:
	void topItemChanged(QWidget*);
};

#endif // ROTARYWIDGET_H
