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

#pragma once

#include <QObject>
#include <QDir>
#include <interfaces/media/ialbumartprovider.h>
#include "localcollection.h"

namespace LeechCraft
{
namespace LMP
{
	class AlbumArtManager : public QObject
	{
		Q_OBJECT

		QDir AADir_;

		struct TaskQueue
		{
			Media::AlbumInfo Info_;
			bool PreviewMode_;
		};
		QList<TaskQueue> Queue_;
		QHash<Media::AlbumInfo, int> NumRequests_;

		QHash<Media::AlbumInfo, QSize> BestSizes_;
	public:
		AlbumArtManager (QObject* = 0);

		void CheckAlbumArt (const Collection::Artist&, Collection::Album_ptr);
		void CheckAlbumArt (const QString& artist, const QString& album, bool preview);
	public slots:
		void handleGotAlbumArt (const Media::AlbumInfo&, const QList<QImage>&);
	private slots:
		void rotateQueue ();
		void handleSaved ();
	signals:
		void gotImages (const Media::AlbumInfo&, const QList<QImage>&);
	};
}
}
