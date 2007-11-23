/*
 *   Copyright 2007 by Alexander Wiedenbruch <mail@wiedenbruch.de>
 *                      and Matias Valdenegro <mvaldenegro@informatica.utem.cl>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2, or
 *   (at your option) any later version.

 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */


#include "widget.h"

#include <cmath>
#include <limits>

#include <QApplication>
#include <QList>
#include <QPainter>
#include <QPixmapCache>
#include <QStyleOptionGraphicsItem>

#include <KDebug>

#include "layouts/freelayout.h"
#include "plasma/plasma.h"

namespace Plasma
{

class Widget::Private
{
    public:
        Private()
            : minimumSize(0,0),
              maximumSize(std::numeric_limits<qreal>::infinity(),
                          std::numeric_limits<qreal>::infinity()),
              opacity(1.0),
              cachePaintMode(Widget::NoCacheMode),
              wasMovable(false)
        { }
        ~Private() { }

        QSizeF size;
        QSizeF minimumSize;
        QSizeF maximumSize;

        QList<Widget *> childList;

        qreal opacity;

        // Replace with CacheMode in 4.4
#if QT_VERSION >= 0x040400
#warning Replace Plasma::Widget::CachePaintMode with QGraphicsItem::CacheMode
#endif
        Widget::CachePaintMode cachePaintMode;
        QSize cacheSize;
        QString cacheKey;
        QRectF cacheInvalidated;

        bool wasMovable;

        bool shouldPaint(QPainter *painter, const QTransform &transform);
};

QGraphicsItem* Widget::graphicsItem()
{
    return this;
}

bool Widget::Private::shouldPaint(QPainter *painter, const QTransform &transform)
{
    Q_UNUSED(painter)
    Q_UNUSED(transform)
    //qreal zoomLevel = painter->transform().m11() / transform.m11();
    //return (fabs(zoomLevel - scalingFactor(Plasma::DesktopZoom))) < std::numeric_limits<double>::epsilon();
    return true;
}

Widget::Widget(QGraphicsItem *parent, QObject* parentObject)
  : QObject(parentObject),
    QGraphicsItem(parent),
    d(new Private)
{
    setFlag(QGraphicsItem::ItemClipsToShape, true);
    setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    setCachePaintMode(DeviceCoordinateCacheMode);

    Widget *w = dynamic_cast<Widget *>(parent);
    if (w) {
        w->addChild(this);
    }
}

Widget::~Widget()
{
    delete d;
}

void Widget::setOpacity(qreal opacity)
{
    d->opacity = opacity;
}
qreal Widget::opacity() const
{
    return d->opacity;
}

void Widget::setCachePaintMode(CachePaintMode mode, const QSize &size)
{
    d->cachePaintMode = mode;
    if (mode == NoCacheMode) {
        QPixmapCache::remove(d->cacheKey);
        d->cacheKey.clear();
    } else {
        d->cacheKey = QString("%1").arg(long(this));
        if (mode == ItemCoordinateCacheMode) {
            d->cacheSize = size.isNull() ? boundingRect().size().toSize() : size;
        }
    }
}

Widget::CachePaintMode Widget::cachePaintMode() const
{
    return d->cachePaintMode;
}

void Widget::update(const QRectF &rect)
{
    if (d->cachePaintMode != NoCacheMode) {
        d->cacheInvalidated |= rect.isNull() ? boundingRect() : rect;
    }
    QGraphicsItem::update(rect);
}

Qt::Orientations Widget::expandingDirections() const
{
    return Qt::Horizontal | Qt::Vertical;
}

void Widget::setMinimumSize(const QSizeF& size)
{
    d->minimumSize = size;
}

QSizeF Widget::minimumSize() const
{
    return d->minimumSize;
}

void Widget::setMaximumSize(const QSizeF& size)
{
    d->maximumSize = size;
}

QSizeF Widget::maximumSize() const
{
    return d->maximumSize;
}

bool Widget::hasHeightForWidth() const
{
    return false;
}

qreal Widget::heightForWidth(qreal w) const
{
    Q_UNUSED(w);

    return -1.0;
}

bool Widget::hasWidthForHeight() const
{
    return false;
}

qreal Widget::widthForHeight(qreal h) const
{
    Q_UNUSED(h);

    return -1.0;
}

QRectF Widget::geometry() const
{
    return QRectF(pos(), d->size);
}

#if 0
QRectF Widget::localGeometry() const
{
    return QRectF(QPointF(0.0f, 0.0f), boundingRect().size);
}
#endif

void Widget::setGeometry(const QRectF& geometry)
{
    if (geometry.size().width() > 0 && geometry.size().height() > 0 && d->size != geometry.size()) {
        prepareGeometryChange();
        qreal width = qBound(d->minimumSize.width(), geometry.size().width(), d->maximumSize.width());
        qreal height = qBound(d->minimumSize.height(), geometry.size().height(), d->maximumSize.height());

        d->size = QSizeF(width, height);

        if (layout()) {
            layout()->setGeometry(QRectF(QPointF(0, 0), d->size));
        }

        if (managingLayout()) {
            managingLayout()->invalidate();
        }
    }

    setPos(geometry.topLeft());
    update();
}

void Widget::setSize(qreal width, qreal height)
{
    setSize(QSizeF(width, height));
}

void Widget::setSize(const QSizeF& size)
{
    d->size = size;
}

void Widget::updateGeometry()
{
    if (managingLayout()) {
        managingLayout()->invalidate();
    } else {
        setGeometry(QRectF(pos(), sizeHint()));
    }
}

QSizeF Widget::sizeHint() const
{
    if (layout()) {
        return layout()->sizeHint();
    } else {
        return d->size;
    }
}

QSizeF Widget::size() const
{
    return d->size;
}

QFont Widget::font() const
{
    return QApplication::font();
}

/*
void Widget::setFont(const QFront& font)
{
}
*/

QRectF Widget::boundingRect() const
{
    return QRectF(QPointF(0,0), d->size);
}

void Widget::resize(const QSizeF& size)
{
    //FIXME: we need to respect minimum size here i think
    setGeometry(QRectF(pos(), size));
}

void Widget::resize(qreal w, qreal h)
{
    resize(QSizeF(w, h));
}

Widget *Widget::parent() const
{
    return dynamic_cast<Widget *>(parentItem());
}

void Widget::addChild(Widget *w)
{
    if (!w || d->childList.contains(w)) {
        return;
    }

    d->childList.append(w);
    w->setParentItem(this);

    //kDebug() << "Added Child Widget" <<  (QObject*)w << "our geom is" << geometry();

    if (layout()) {
        layout()->addItem(w);
    }

    updateGeometry();
    //kDebug() << "after the item is added our geom is now" << geometry();
}

void Widget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    painter->setOpacity(d->opacity);

    /*
NOTE: put this back if we end up needing to control when things paint due to, e.g. zooming.
    if (!d->shouldPaint(painter, transform())) {
        return;
    }
    */

    if (d->cachePaintMode == NoCacheMode) {
        paintWidget(painter, option, widget);
        return;
    }

    // Cached painting
    QRectF brect = boundingRect();

    // Fetch the off-screen transparent buffer and exposed area info.
    QPixmap pix;
    QPixmapCache::find(d->cacheKey, pix);
    QRectF exposed = d->cacheInvalidated;

    // Render using item coodinate cache mode.
    if (d->cachePaintMode == ItemCoordinateCacheMode) {
        // Recreate the pixmap if it's gone.
        if (pix.isNull()) {
            pix = QPixmap(d->cacheSize);
            pix.fill(Qt::transparent);
            exposed = brect;
        }

        // Check for newly invalidated areas.
        if (!exposed.isNull()) {
            d->cacheInvalidated = QRectF();

            QStyleOptionGraphicsItem cacheOption = *option;
            cacheOption.exposedRect = exposed.toRect(); // <- truncation

            QPainter pixmapPainter(&pix);

            // Fit the item's bounding rect into the pixmap's coordinates.
            pixmapPainter.scale(pix.width() / brect.width(),
                                pix.height() / brect.height());
            pixmapPainter.translate(-brect.topLeft());

            // erase the exposed area so we don't paint over and over
            pixmapPainter.setCompositionMode(QPainter::CompositionMode_Source);
            pixmapPainter.fillRect(exposed, Qt::transparent);
            pixmapPainter.setCompositionMode(QPainter::CompositionMode_SourceOver);

            // Re-render the invalidated areas of the pixmap. Important: don't
            // fool the item into using the widget - pass 0 instead of \a
            // widget.
            paintWidget(&pixmapPainter, &cacheOption, 0);
            pixmapPainter.end();

            // Reinsert this pixmap into the cache
            QPixmapCache::insert(d->cacheKey, pix);
        }

        // Redraw the exposed area using the transformed painter. Depending on
        // the hardware, this may be a server-side operation, or an expensive
        // qpixmap-image-transform-pixmap roundtrip.
        painter->drawPixmap(brect, pix, QRectF(QPointF(), pix.size()));
        return;
    }

    // Render using device coordinate cache mode.
    if (d->cachePaintMode == DeviceCoordinateCacheMode) {
        QTransform transform = painter->worldTransform();
        QRect deviceRect = transform.mapRect(brect).toRect();

        if (deviceRect.width() < 1 ||
            deviceRect.height() < 1) {
            // we have nothing to paint!
            return;
        }

        // Auto-adjust the pixmap size.
        if (deviceRect.size() != pix.size()) {
            pix = QPixmap(deviceRect.size());
            pix.fill(Qt::transparent);
            exposed = brect;
        }

        // Check for newly invalidated areas.
        if (!exposed.isNull()) {
            d->cacheInvalidated = QRectF();

            // Construct the new styleoption, reset the exposed rect.
            QStyleOptionGraphicsItem cacheOption = *option;
            cacheOption.exposedRect = exposed.toRect(); // <- truncation

            QPointF viewOrigo = transform.map(QPointF(0,  0));
            QPointF offset = viewOrigo - deviceRect.topLeft();

            // Transform the painter, and render the item in device coordinates.
            QPainter pixmapPainter(&pix);

            pixmapPainter.translate(offset);
            pixmapPainter.setWorldTransform(transform, true);
            pixmapPainter.translate(transform.inverted().map(QPointF(0, 0)));

            // erase the exposed area so we don't paint over and over
            pixmapPainter.setCompositionMode(QPainter::CompositionMode_Source);
            pixmapPainter.fillRect(exposed, Qt::transparent);
            pixmapPainter.setCompositionMode(QPainter::CompositionMode_SourceOver);

            paintWidget(&pixmapPainter, &cacheOption, 0);
            pixmapPainter.end();

            // Reinsert this pixmap into the cache
            QPixmapCache::insert(d->cacheKey, pix);
        }

        // Redraw the exposed area using an untransformed painter. This
        // effectively becomes a bitblit that does not transform the cache.
        painter->setWorldTransform(QTransform());
        painter->drawPixmap(deviceRect.topLeft(), pix);
        return;
    }
}

void Widget::paintWidget(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(painter);
    Q_UNUSED(option);
    Q_UNUSED(widget);

    // Replaced by widget's own function
}

QVariant Widget::itemChange(GraphicsItemChange change, const QVariant &value)
{
    if (change == QGraphicsItem::ItemChildRemovedChange) {
        Widget *child = dynamic_cast<Plasma::Widget*>(value.value<QGraphicsItem*>());
        if (child) {
            kDebug() << "removing" << (QObject*)child;
            d->childList.removeAll(child);
            if (layout()) {
                layout()->removeItem(child);
            }
            updateGeometry();
        }
    }

    return QGraphicsItem::itemChange(change, value);
}

void Widget::managingLayoutChanged()
{
    if (managingLayout()) {
        d->wasMovable = flags() & ItemIsMovable;
        if (!dynamic_cast<FreeLayout*>(managingLayout())) {
            setFlag(ItemIsMovable, false);
        }
    } else {
        setFlag(ItemIsMovable, d->wasMovable);
    }
}

} // Plasma namespace
