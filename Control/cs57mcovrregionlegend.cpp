#include "cs57mcovrregionlegend.h"
#include <QPainter>
#define X 40
#define Y 10
#define H 20
#define W 100
#define Space 5
#define TextY Y+15
#define TextSpace 20

#pragma execution_character_set("utf-8")
CS57McovrRegionLegend::CS57McovrRegionLegend(QWidget *parent)
	: QDialog(parent)
	, _size(150, 140)
{
	ui.setupUi(this);
	setWindowFlags(Qt::FramelessWindowHint);
	setFixedSize(_size);
}

CS57McovrRegionLegend::~CS57McovrRegionLegend()
{}

void CS57McovrRegionLegend::paintEvent(QPaintEvent *event)
{
	QPainter painter(this);

	QRect rectI;
	rectI.setTop(Y);
	rectI.setLeft(X);
	rectI.setRight(X + W);
	rectI.setBottom(Y + H);
	painter.fillRect(rectI, QColor(0, 0, 255, 50));

	QRect rectII;
	rectII.setTop(Y + H + Space);
	rectII.setLeft(X);
	rectII.setRight(X + W);
	rectII.setBottom(Y + H + Space + H);
	painter.fillRect(rectII, QColor(0, 255, 255, 50));

	QRect rectIII;
	rectIII.setTop(Y + 2 * (H + Space));
	rectIII.setLeft(X);
	rectIII.setRight(X + W);
	rectIII.setBottom(Y + 2 * (H + Space) + H);
	painter.fillRect(rectIII, QColor(0, 255, 0, 50));

	QRect rectIV;
	rectIV.setTop(Y + 3 * (H + Space));
	rectIV.setLeft(X);
	rectIV.setRight(X + W);
	rectIV.setBottom(Y + 3 * (H + Space) + H);
	painter.fillRect(rectIV, QColor(255, 255, 0, 50));

	QRect rectV;
	rectV.setTop(Y + 4 * (H + Space));
	rectV.setLeft(X);
	rectV.setRight(X + W);
	rectV.setBottom(Y + 4 * (H + Space) + H);
	painter.fillRect(rectV, QColor(255, 0, 0, 50));

	QPen pen(QColor(50, 50, 50), 3);
	QFont font;
	font.setBold(true);
	painter.setPen(pen);
	painter.setFont(font);
	painter.drawText(QPoint(X - TextSpace - 10, TextY), "I");
	painter.drawText(QPoint(X - TextSpace - 10, TextY + 25), "II");
	painter.drawText(QPoint(X - TextSpace - 10, TextY + 25 * 2), "III");
	painter.drawText(QPoint(X - TextSpace - 10, TextY + 25 * 3), "IV");
	painter.drawText(QPoint(X - TextSpace - 10, TextY + 25 * 4), "V");

	painter.drawText(QPoint(X + TextSpace - 20, TextY), "＜10万");
	painter.drawText(QPoint(X + TextSpace - 20, TextY + 25), "10万-20万");
	painter.drawText(QPoint(X + TextSpace - 20, TextY + 25 * 2), "20万-100万");
	painter.drawText(QPoint(X + TextSpace - 20, TextY + 25 * 3), "100万-300万");
	painter.drawText(QPoint(X + TextSpace - 20, TextY + 25 * 4), "≥300万");
}
