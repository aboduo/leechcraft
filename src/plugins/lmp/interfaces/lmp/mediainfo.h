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
#include <QMetaType>
#include <interfaces/media/audiostructs.h>
#include "lmpconfig.h"

namespace LeechCraft
{
namespace LMP
{
	struct LMP_API MediaInfo
	{
		QString LocalPath_;

		QString Artist_;
		QString Album_;
		QString Title_;

		QStringList Genres_;

		qint32 Length_;
		qint32 Year_;
		qint32 TrackNumber_;

		MediaInfo& operator= (const Media::AudioInfo&);

		bool IsUseless () const;

		operator Media::AudioInfo () const;

		static MediaInfo FromAudioInfo (const Media::AudioInfo&);
	};

	inline bool operator== (const MediaInfo& l, const MediaInfo& r)
	{
		return l.LocalPath_ == r.LocalPath_ &&
			l.Artist_ == r.Artist_ &&
			l.Album_ == r.Album_ &&
			l.Title_ == r.Title_ &&
			l.Genres_ == r.Genres_ &&
			l.Length_ == r.Length_ &&
			l.Year_ == r.Year_ &&
			l.TrackNumber_ == r.TrackNumber_;
	}

	inline bool operator!= (const MediaInfo& l, const MediaInfo& r)
	{
		return !(l == r);
	}
}
}

Q_DECLARE_METATYPE (LeechCraft::LMP::MediaInfo);
