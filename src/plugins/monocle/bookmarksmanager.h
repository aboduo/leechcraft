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
#include <QtXml/QDomDocument>
#include "interfaces/monocle/idocument.h"

namespace LeechCraft
{
namespace Monocle
{
	class Bookmark;

	class BookmarksManager : public QObject
	{
		QDomDocument BookmarksDOM_;
	public:
		BookmarksManager (QObject* = 0);

		void AddBookmark (IDocument_ptr, const Bookmark&);
		void RemoveBookmark (IDocument_ptr, const Bookmark&);

		QList<Bookmark> GetBookmarks (IDocument_ptr) const;
	private:
		QDomElement GetDocElem (const QString&) const;
		QDomElement GetDocElem (const QString&);

		void Load ();
		bool LoadSaved ();
		void Save () const;
	};
}
}
