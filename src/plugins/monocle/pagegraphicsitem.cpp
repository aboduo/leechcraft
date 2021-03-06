/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2006-2013  Georg Rudoy
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **********************************************************************/

#include "pagegraphicsitem.h"
#include <QtDebug>
#include <QtConcurrentRun>
#include <QFutureWatcher>
#include <QGraphicsSceneMouseEvent>
#include <QCursor>
#include <QApplication>
#include "core.h"
#include "pixmapcachemanager.h"

namespace LeechCraft
{
namespace Monocle
{
	PageGraphicsItem::PageGraphicsItem (IDocument_ptr doc, int page, QGraphicsItem *parent)
	: QGraphicsPixmapItem (parent)
	, Doc_ (doc)
	, PageNum_ (page)
	, IsHoverLink_ (false)
	, Links_ (Doc_->GetPageLinks (PageNum_))
	, XScale_ (1)
	, YScale_ (1)
	, Invalid_ (true)
	{
		setPixmap (QPixmap (Doc_->GetPageSize (page)));

		if (!Links_.isEmpty ())
			setAcceptHoverEvents (true);
	}

	PageGraphicsItem::~PageGraphicsItem ()
	{
		Core::Instance ().GetPixmapCacheManager ()->PixmapDeleted (this);
	}

	void PageGraphicsItem::SetReleaseHandler (std::function<void (int, QPointF)> handler)
	{
		ReleaseHandler_ = handler;
	}

	void PageGraphicsItem::SetScale (double xs, double ys)
	{
		XScale_ = xs;
		YScale_ = ys;

		auto size = Doc_->GetPageSize (PageNum_);
		size.rwidth () *= xs;
		size.rheight () *= ys;
		setPixmap (QPixmap (size));

		Invalid_ = true;

		update ();

		for (auto i = Item2RectInfo_.begin (); i != Item2RectInfo_.end (); ++i)
		{
			const auto& info = *i;
			info.Setter_ (MapFromDoc (info.DocRect_));
		}
	}

	int PageGraphicsItem::GetPageNum () const
	{
		return PageNum_;
	}

	QRectF PageGraphicsItem::MapFromDoc (const QRectF& rect) const
	{
		return
		{
			rect.x () * XScale_,
			rect.y () * YScale_,
			rect.width () * XScale_,
			rect.height () * YScale_
		};
	}

	QRectF PageGraphicsItem::MapToDoc (const QRectF& rect) const
	{
		return
		{
			rect.x () / XScale_,
			rect.y () / YScale_,
			rect.width () / XScale_,
			rect.height () / YScale_
		};
	}

	void PageGraphicsItem::RegisterChildRect (QGraphicsItem *item,
			const QRectF& docRect, RectSetter_f setter)
	{
		Item2RectInfo_ [item] = { docRect, setter };
		setter (MapFromDoc (docRect));
	}

	void PageGraphicsItem::UnregisterChildRect (QGraphicsItem *item)
	{
		Item2RectInfo_.remove (item);
	}

	void PageGraphicsItem::ClearPixmap ()
	{
		auto size = Doc_->GetPageSize (PageNum_);
		size.rwidth () *= XScale_;
		size.rheight () *= YScale_;
		setPixmap (QPixmap (size));

		Invalid_ = true;
	}

	void PageGraphicsItem::UpdatePixmap ()
	{
		Invalid_ = true;
		if (isVisible ())
			update ();
	}

	void PageGraphicsItem::paint (QPainter *painter,
			const QStyleOptionGraphicsItem *option, QWidget *w)
	{
		if (Invalid_)
		{
			auto backendObj = Doc_->GetBackendPlugin ();
			if (qobject_cast<IBackendPlugin*> (backendObj)->IsThreaded ())
			{
				auto watcher = new QFutureWatcher<QImage> ();
				connect (watcher,
						SIGNAL (finished ()),
						this,
						SLOT (handlePixmapRendered ()));

				std::function<QImage ()> worker ([this] ()
						{
							return Doc_->RenderPage (PageNum_, XScale_, YScale_);
						});
				watcher->setFuture (QtConcurrent::run (worker));

				auto size = Doc_->GetPageSize (PageNum_);
				size.rwidth () *= XScale_;
				size.rheight () *= YScale_;
				QPixmap px (size);
				px.fill ();
				setPixmap (px);
			}
			else
			{
				const auto& img = Doc_->RenderPage (PageNum_, XScale_, YScale_);
				setPixmap (QPixmap::fromImage (img));
			}
			LayoutLinks ();
			Invalid_ = false;

			Core::Instance ().GetPixmapCacheManager ()->PixmapChanged (this);
		}

		QGraphicsPixmapItem::paint (painter, option, w);
		Core::Instance ().GetPixmapCacheManager ()->PixmapPainted (this);
	}

	void PageGraphicsItem::hoverMoveEvent (QGraphicsSceneHoverEvent *event)
	{
		if (!IsHoverLink_ && FindLink (event->pos ()))
		{
			QApplication::setOverrideCursor (QCursor (Qt::PointingHandCursor));
			IsHoverLink_ = true;
		}
		else if (IsHoverLink_ && !FindLink (event->pos ()))
		{
			QApplication::restoreOverrideCursor ();
			IsHoverLink_ = false;
		}

		QGraphicsItem::hoverMoveEvent (event);
	}

	void PageGraphicsItem::hoverLeaveEvent (QGraphicsSceneHoverEvent *event)
	{
		if (IsHoverLink_)
			QApplication::restoreOverrideCursor ();

		QGraphicsItem::hoverLeaveEvent (event);
	}

	void PageGraphicsItem::mousePressEvent (QGraphicsSceneMouseEvent *event)
	{
		PressedLink_ = FindLink (event->pos ());
		if (PressedLink_ || ReleaseHandler_)
			return;

		QGraphicsItem::mousePressEvent (event);
	}

	void PageGraphicsItem::mouseReleaseEvent (QGraphicsSceneMouseEvent *event)
	{
		auto relLink = FindLink (event->pos ());
		const bool handle = relLink && relLink == PressedLink_;
		PressedLink_ = ILink_ptr ();
		if (!handle)
		{
			QGraphicsItem::mouseReleaseEvent (event);
			if (ReleaseHandler_)
				ReleaseHandler_ (PageNum_, event->pos ());
			return;
		}

		relLink->Execute ();
	}

	void PageGraphicsItem::LayoutLinks ()
	{
		Rect2Link_.clear ();

		const auto& rect = boundingRect ();
		const auto width = rect.width ();
		const auto height = rect.height ();
		for (auto link : Links_)
		{
			const auto& area = link->GetArea ();
			const QRect linkRect (width * area.left (), height * area.top (),
					width * area.width (), height * area.height ());
			Rect2Link_ << qMakePair (linkRect, link);
		}
	}

	ILink_ptr PageGraphicsItem::FindLink (const QPointF& point)
	{
		for (const auto& pair : Rect2Link_)
			if (pair.first.contains (point.toPoint ()))
				return pair.second;
		return ILink_ptr ();
	}

	void PageGraphicsItem::handlePixmapRendered ()
	{
		auto watcher = dynamic_cast<QFutureWatcher<QImage>*> (sender ());
		watcher->deleteLater ();

		const auto& img = watcher->result ();
		setPixmap (QPixmap::fromImage (img));
	}
}
}
