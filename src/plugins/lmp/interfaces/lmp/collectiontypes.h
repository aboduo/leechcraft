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

#include <memory>
#include <QStringList>
#include <QDateTime>

namespace LeechCraft
{
namespace LMP
{
namespace Collection
{
	struct Track
	{
		int ID_;

		int Number_;
		QString Name_;
		int Length_;
		QStringList Genres_;

		QString FilePath_;
	};

	struct Album
	{
		int ID_;

		QString Name_;
		int Year_;
		QString CoverPath_;

		QList<Track> Tracks_;
	};
	typedef std::shared_ptr<Album> Album_ptr;

	struct Artist
	{
		int ID_;

		QString Name_;
		QList<Album_ptr> Albums_;
	};
	typedef QList<Artist> Artists_t;

	struct TrackStats
	{
		int TrackID_;

		int Playcount_;
		QDateTime Added_;
		QDateTime LastPlay_;
		int Score_;
		int Rating_;
	};
}
}
}
