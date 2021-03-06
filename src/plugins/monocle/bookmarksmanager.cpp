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

#include "bookmarksmanager.h"
#include <QUrl>
#include <QFileInfo>
#include <QtDebug>
#include <util/util.h>
#include "bookmark.h"

namespace LeechCraft
{
namespace Monocle
{
	BookmarksManager::BookmarksManager (QObject *parent)
	: QObject (parent)
	{
		qRegisterMetaType<Bookmark> ("LeechCraft::Monocle::Bookmark");
		qRegisterMetaTypeStreamOperators<Bookmark> ("LeechCraft::Monocle::Bookmark");

		Load ();
	}

	namespace
	{
		QString GetDocID (IDocument_ptr doc)
		{
			const auto& info = doc->GetDocumentInfo ();
			if (!info.Title_.trimmed ().isEmpty ())
				return info.Title_;

			return QFileInfo (doc->GetDocURL ().path ()).fileName ();
		}
	}

	void BookmarksManager::AddBookmark (IDocument_ptr doc, const Bookmark& bm)
	{
		auto fileElem = GetDocElem (GetDocID (doc));

		auto elem = BookmarksDOM_.createElement ("bm");
		bm.ToXML (elem, BookmarksDOM_);
		fileElem.appendChild (elem);

		Save ();
	}

	void BookmarksManager::RemoveBookmark (IDocument_ptr doc, const Bookmark& bm)
	{
		auto fileElem = GetDocElem (GetDocID (doc));

		auto bmElem = fileElem.firstChildElement ("bm");
		while (!bmElem.isNull ())
		{
			auto next = bmElem.nextSiblingElement ("bm");
			if (Bookmark::FromXML (bmElem) == bm)
				fileElem.removeChild (bmElem);
			bmElem = next;
		}

		Save ();
	}

	QList<Bookmark> BookmarksManager::GetBookmarks (IDocument_ptr doc) const
	{
		QList<Bookmark> result;

		auto fileElem = GetDocElem (GetDocID (doc));
		auto bmElem = fileElem.firstChildElement ("bm");
		while (!bmElem.isNull ())
		{
			result << Bookmark::FromXML (bmElem);
			bmElem = bmElem.nextSiblingElement ("bm");
		}

		return result;
	}

	QDomElement BookmarksManager::GetDocElem (const QString& id)
	{
		auto fileElem = BookmarksDOM_.documentElement ().firstChildElement ("doc");
		while (!fileElem.isNull ())
		{
			if (fileElem.attribute ("id") == id)
				break;

			fileElem = fileElem.nextSiblingElement ("doc");
		}

		if (fileElem.isNull ())
		{
			fileElem = BookmarksDOM_.createElement ("doc");
			fileElem.setAttribute ("id", id);
			BookmarksDOM_.documentElement ().appendChild (fileElem);
		}
		return fileElem;
	}

	QDomElement BookmarksManager::GetDocElem (const QString& id) const
	{
		auto fileElem = BookmarksDOM_.documentElement ().firstChildElement ("doc");
		while (!fileElem.isNull ())
		{
			if (fileElem.attribute ("id") == id)
				return fileElem;

			fileElem = fileElem.nextSiblingElement ("doc");
		}
		return QDomElement ();
	}

	void BookmarksManager::Load ()
	{
		if (LoadSaved ())
			return;

		auto docElem = BookmarksDOM_.createElement ("bookmarks");
		docElem.setTagName ("bookmarks");
		docElem.setAttribute ("version", "1");
		BookmarksDOM_.appendChild (docElem);
	}

	bool BookmarksManager::LoadSaved ()
	{
		auto dir = Util::CreateIfNotExists ("monocle");
		if (!dir.exists ("bookmarks.xml"))
			return false;

		QFile file (dir.absoluteFilePath ("bookmarks.xml"));
		if (!file.open (QIODevice::ReadOnly))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< file.fileName ()
					<< file.errorString ();
			return false;
		}

		if (!BookmarksDOM_.setContent (&file))
		{
			qWarning () << Q_FUNC_INFO
					<< "error parsing file"
					<< file.fileName ();
			return false;
		}

		return true;
	}

	void BookmarksManager::Save () const
	{
		auto dir = Util::CreateIfNotExists ("monocle");
		QFile file (dir.absoluteFilePath ("bookmarks.xml"));
		if (!file.open (QIODevice::WriteOnly | QIODevice::Truncate))
		{
			qWarning () << Q_FUNC_INFO
					<< "unable to open file"
					<< file.fileName ()
					<< file.errorString ();
			return;
		}

		file.write (BookmarksDOM_.toByteArray (2));
	}
}
}
