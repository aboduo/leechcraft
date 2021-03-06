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

#include <QStringList>
#include <QFileInfo>
#include <interfaces/media/idiscographyprovider.h>

class QPixmap;
class QPoint;

namespace Phonon
{
	class MediaSource;
}

namespace LeechCraft
{
namespace LMP
{
	struct MediaInfo;

	QList<QFileInfo> RecIterateInfo (const QString& dirPath, bool followSymlinks = false);
	QStringList RecIterate (const QString& dirPath, bool followSymlinks = false);

	QString FindAlbumArtPath (const QString& near, bool ignoreCollection = false);
	QPixmap FindAlbumArt (const QString& near, bool ignoreCollection = false);

	void ShowAlbumArt (const QString& near, const QPoint& pos);

	QMap<QString, std::function<QString (MediaInfo)>> GetSubstGetters ();
	QMap<QString, std::function<void (MediaInfo&, QString)>> GetSubstSetters ();
	QString PerformSubstitutions (QString mask, const MediaInfo& info);

	bool ShouldRememberProvs ();

	QString MakeTrackListTooltip (const QList<QList<Media::ReleaseTrackInfo>>&);

	bool operator!= (const Phonon::MediaSource&, const Phonon::MediaSource&);
}
}
