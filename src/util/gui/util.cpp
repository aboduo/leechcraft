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

#include "util.h"
#include <QSize>
#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QTimer>
#include <QDesktopWidget>
#include <QLabel>
#include <QtDebug>

namespace LeechCraft
{
namespace Util
{
	QPoint FitRectScreen (QPoint pos, const QSize& size, FitFlags flags, const QPoint& shiftAdd)
	{
		return FitRect (pos, size, QApplication::desktop ()->screenGeometry (pos), flags, shiftAdd);
	}

	QPoint FitRect (QPoint pos, const QSize& size, const QRect& geometry,
			FitFlags flags, const QPoint& shiftAdd)
	{
		int xDiff = std::max (0, pos.x () + size.width () - (geometry.width () + geometry.x ()));
		if (!xDiff)
			xDiff = std::min (0, pos.x () - geometry.x ());
		int yDiff = std::max (0, pos.y () + size.height () - (geometry.height () + geometry.y ()));
		if (!yDiff)
			yDiff = std::min (0, pos.y () - geometry.y ());

		if (flags & FitFlag::NoOverlap)
		{
			auto overlapFixer = [] (int& diff, int dim)
			{
				if (diff > 0)
					diff = dim > diff ? dim : diff;
			};

			const QRect newRect (pos - QPoint (xDiff, yDiff), size);
			if (newRect.contains (pos))
			{
				overlapFixer (xDiff, size.width ());
				overlapFixer (yDiff, size.height ());
			}
		}

		if (xDiff)
			pos.rx () -= xDiff + shiftAdd.x ();
		if (yDiff)
			pos.ry () -= yDiff + shiftAdd.y ();

		return pos;
	}

	namespace
	{
		class AADisplayEventFilter : public QObject
		{
			QWidget *Display_;
		public:
			AADisplayEventFilter (QWidget *display)
			: QObject (display)
			, Display_ (display)
			{
			}
		protected:
			bool eventFilter (QObject*, QEvent *event)
			{
				bool shouldClose = false;
				switch (event->type ())
				{
				case QEvent::KeyRelease:
					shouldClose = static_cast<QKeyEvent*> (event)->key () == Qt::Key_Escape;
					break;
				case QEvent::MouseButtonRelease:
					shouldClose = true;
					break;
				default:
					break;
				}

				if (!shouldClose)
					return false;

				QTimer::singleShot (0,
						Display_,
						SLOT (close ()));
				return true;
			}
		};
	}

	QLabel* ShowPixmapLabel (const QPixmap& srcPx, const QPoint& pos)
	{
		const auto& availGeom = QApplication::desktop ()->availableGeometry (pos).size () * 0.9;

		auto px = srcPx;
		if (px.size ().width () > availGeom.width () ||
			px.size ().height () > availGeom.height ())
			px = px.scaled (availGeom, Qt::KeepAspectRatio, Qt::SmoothTransformation);

		auto label = new QLabel;
		label->setWindowFlags (Qt::Tool);
		label->setAttribute (Qt::WA_DeleteOnClose);
		label->setFixedSize (px.size ());
		label->setPixmap (px);
		label->show ();
		label->activateWindow ();
		label->installEventFilter (new AADisplayEventFilter (label));
		label->move (pos);
		return label;
	}
}
}
