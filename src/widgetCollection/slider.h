/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2024 Sergey Vlasov <sergey@vlasov.me>
**
**  This program can be distributed under the terms of the GNU
**  General Public License version 3.0 as published by the Free
**  Software Foundation and appearing in the file LICENSE.GPL3
**  included in the packaging of this file.  Please review the
**  following information to ensure the GNU General Public License
**  version 3.0 requirements will be met:
**
**  http://www.gnu.org/licenses/gpl-3.0.html
**
*********************************************************************/

#ifndef N_SLIDER_H
#define N_SLIDER_H

#include <QSlider>

class NSlider : public QSlider
{
    Q_OBJECT

public:
    NSlider(QWidget *parent);
    qreal valueAtPos(int pos);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;

private slots:
    void setValue(int){};

public slots:
    void setValue(qreal value);

signals:
    void sliderMoved(qreal value);
};

#endif
