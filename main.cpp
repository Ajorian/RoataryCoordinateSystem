#include <QApplication>
#include "RotaryCoordinate/RotaryCoordinateWidget.h"
#include <QHBoxLayout>

#define NUM_OF_WIDGETS 6
#define ITEMS_WIDTH    384
#define ITEMS_HEIGHT   285

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QWidget* w = new QWidget;
    QHBoxLayout * ly = new  QHBoxLayout;
    RotaryCoordinateWidget* rCoor = new RotaryCoordinateWidget(0);
    ly->addWidget(rCoor);
    w->setLayout(ly);

    for (int i=0; i<NUM_OF_WIDGETS; i++)
    {
        QToolButton* btn = new QToolButton; // QToolButton or any other derrived QWidget classes
        btn->setIcon(QIcon(QString(":/images/%1.jpg").arg(QString::number(i)))); // you have to set your Icon.
        btn->setIconSize(QSize(ITEMS_WIDTH, ITEMS_HEIGHT));
        btn->setFixedSize(ITEMS_WIDTH, ITEMS_HEIGHT);
        rCoor->insertWidget(btn);
    }
    w->show();
    return a.exec();
}
