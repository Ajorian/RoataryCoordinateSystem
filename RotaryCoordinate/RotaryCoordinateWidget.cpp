#include "RotaryCoordinateWidget.h"
#include <QPropertyAnimation>
#include <QGraphicsProxyWidget>
#include <QVBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QGraphicsBlurEffect>

#define SCALING_COEF_COUNT 5
#define NEXT_PERV_BTN_WIDTH 31
#define NEXT_PERV_BTN_HEIGTH 50
#define BIAS_PHASE_RAD	((M_PI/2))
#define TO_RADIAN(x) (M_PI*(x)/180.0)
#define TO_DEGREE(x) (180.0*(x)/M_PI)
#define myABS(x) ( ((x)>=0) ? x : -x)
#define MAX_BLUR_RADIOS 5

struct _Pair { double angle; double coef; };

_Pair scalingCoefs[SCALING_COEF_COUNT];

void initScalingCoefs()
{
	scalingCoefs[0].angle = 0;
	scalingCoefs[0].coef = 1.0 / 2.0;

	scalingCoefs[1].angle = 90;
	scalingCoefs[1].coef = 1.0;

	scalingCoefs[2].angle = 180;
	scalingCoefs[2].coef = 1.0 / 2.0;

	scalingCoefs[3].angle = 270;
	scalingCoefs[3].coef = 1.0 / 3.0;

	scalingCoefs[4].angle = 360;
	scalingCoefs[4].coef = 1.0 / 2.0;
}

RotaryCoordinateWidget::RotaryCoordinateWidget(QWidget *parent) : QWidget(parent)
 {
 	view = new QGraphicsView(this);
 	view->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
 	view->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
 	view->setFrameStyle(QFrame::NoFrame);
 	view->setRenderHints(QPainter::HighQualityAntialiasing | QPainter::TextAntialiasing);
 	scene = new QGraphicsScene(view);
 	view->setScene(scene);
 	QVBoxLayout* lay = new QVBoxLayout(this);
	lay->setMargin(0);
	lay->setContentsMargins(0, 0, 0, 0);
 	lay->addWidget(view);

	connect(&timer, SIGNAL(timeout()), this, SLOT(rotateOneStep()));
	setStyleSheet(QStringLiteral("background:transparent"));
	initScalingCoefs();

	QToolButton* nxt = new QToolButton();
	QToolButton* prv = new QToolButton();
    nxt->setIcon(QIcon(":/RotaryCoordinate/next.png"));
    prv->setIcon(QIcon(":/RotaryCoordinate/pervious.png"));
	nxt->setStyleSheet("border:none;");
	prv->setStyleSheet("border:none;");
	nxt->setFixedSize(NEXT_PERV_BTN_WIDTH, NEXT_PERV_BTN_HEIGTH);
	nxt->setIconSize(QSize(NEXT_PERV_BTN_WIDTH, NEXT_PERV_BTN_HEIGTH));
	prv->setFixedSize(NEXT_PERV_BTN_WIDTH, NEXT_PERV_BTN_HEIGTH);
	prv->setIconSize(QSize(NEXT_PERV_BTN_WIDTH, NEXT_PERV_BTN_HEIGTH));
	nextBtn = scene->addWidget(nxt);
	prvsBtn = scene->addWidget(prv);

	connect(nxt, SIGNAL(clicked()), this, SLOT(goNext()),Qt::DirectConnection);
	connect(prv, SIGNAL(clicked()), this, SLOT(goPrevious()), Qt::DirectConnection);
}

int RotaryCoordinateWidget::getTopItem() const
{
	double m = -1;
	int idx = -1;

	for (int i=0; i<m_items.size(); i++)
	{
		if (m_items[i].scale > m)
		{
			m = m_items[i].scale;
			idx = i;
		}
	}
	return idx;
}

void RotaryCoordinateWidget::resizeEvent(QResizeEvent *e)
{
	scene->setSceneRect(0, 0,e->size().width(), e->size().height());
 	const QSize scineSize = size();
	const QSizeF nxtSize = nextBtn->size();
	const QSizeF prvSize = prvsBtn->size();

	nextBtn->setPos(scineSize.width() - 1.3*nxtSize.width(), 0.5*scineSize.height() - 0.5*nxtSize.height());
	prvsBtn->setPos(0.3*prvSize.width() , 0.5*scineSize.height() - 0.5*prvSize.height());
	sitItems();
	QWidget::resizeEvent(e);
}

double RotaryCoordinateWidget::getScale(double posRad)
{
	double posDeg = (posRad / M_PI)*180.0;
	posDeg = posDeg - ((int)(posDeg / 360.0))*360.0;
	if (posDeg < 0) posDeg += 360.0;
	
	for (int i=0 ; i < SCALING_COEF_COUNT-1; i++)
	{
		if (scalingCoefs[i].angle <= posDeg && posDeg <= scalingCoefs[i + 1].angle)
			return ((scalingCoefs[i+1].coef - scalingCoefs[i].coef) / (scalingCoefs[i+1].angle - scalingCoefs[i].angle))*(posDeg - scalingCoefs[i].angle) + scalingCoefs[i].coef;
	}
	return 0;
}

void RotaryCoordinateWidget::insertWidget(QWidget* itm)
{
	QGraphicsBlurEffect* effect = new QGraphicsBlurEffect;
	effect->setBlurRadius(0);
	itm->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QGraphicsProxyWidget* proxy = scene->addWidget(itm);
	proxy->setFlag(QGraphicsItem::ItemIsSelectable, true);
	proxy->setFlag(QGraphicsItem::ItemIsMovable, true);
	proxy->setGraphicsEffect(effect);
	RotaryItem ritm;
	ritm.posIdx = m_items.size();
	ritm.proxy = proxy;
	ritm.widget = itm;
	m_items.append(ritm);

	const double dtheta = (2 * M_PI) / m_items.size();
	for (int i = 0; i < m_items.size(); i++)
		m_items[i].posRad = i*dtheta + BIAS_PHASE_RAD;

	topItem = m_items[0].widget;
	sitItems();
}

void RotaryCoordinateWidget::rotateOneStep()
{
	const double dTheta_rad = (m_rotationInfo.angle > 0) ? (velocity_RadPerSec / frameRate) : -(velocity_RadPerSec / frameRate);
	QWidget * w = m_items[getTopItem()].widget;
	if(w!= topItem)
		emit topItemChanged(topItem=w);
	if (abs(m_rotationInfo.angle) < abs(dTheta_rad))
	{
		timer.stop();
		m_rotationInfo.angle = 0;
		return;
	}

	m_rotationInfo.angle -= dTheta_rad;

	for (int i=0; i<m_items.size(); i++)
		m_items[i].posRad += dTheta_rad;

	sitItems();
}

void RotaryCoordinateWidget::sitItems()
{
	QSize sceneSize = size();
	const double bRadius = 0.2*(sceneSize.height() / 2);
	const double aRadius = 0.7*(sceneSize.width() / 2);

	for (int i=0; i<m_items.size(); i++)
	{
		QGraphicsProxyWidget * const proxy = m_items[i].proxy;
		const double posRad = m_items[i].posRad;
		const double scale = getScale(posRad);
		const QTransform trans = QTransform().scale(scale, scale);
		m_items[i].scale = scale;
		proxy->setTransform(trans);

		const QPointF center = QPointF(sceneSize.width() / 2, sceneSize.height() / 2) - QPointF(proxy->size().width() / 2, proxy->size().height() / 2)*trans;
		const QPointF p = QPointF(aRadius*cos(posRad), bRadius*sin(posRad));
		proxy->setPos(p+center);
		proxy->setZValue(p.y());
		((QGraphicsBlurEffect*)proxy->graphicsEffect())->setBlurRadius( (-MAX_BLUR_RADIOS/(2* bRadius))*(p.y()- bRadius));
	}
}

void RotaryCoordinateWidget::setVelocity(const double degPerSec)
{
	velocity_RadPerSec = TO_RADIAN(degPerSec);
}

void RotaryCoordinateWidget::goPrevious()
{
	m_rotationInfo.angle += ((2 * M_PI) / m_items.size());
	
	if (!timer.isActive())
		timer.start(1000.0 / frameRate);
}

void RotaryCoordinateWidget::goNext()
{
	m_rotationInfo.angle -= ((2 * M_PI) / m_items.size());

	if (!timer.isActive())
		timer.start(1000.0 / frameRate);
}
