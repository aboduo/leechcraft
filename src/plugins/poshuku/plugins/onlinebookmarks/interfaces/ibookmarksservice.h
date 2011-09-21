/**********************************************************************
 * LeechCraft - modular cross-platform feature rich internet client.
 * Copyright (C) 2010-2011  Oleg Linkin
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

#ifndef PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_IBOOKMARKSSERVICE_H
#define PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_IBOOKMARKSSERVICE_H

namespace LeechCraft
{
namespace Poshuku
{
namespace OnlineBookmarks
{
	class IBookmarksService
	{
	public:
		virtual ~IBookmarksService () {};
	};
}
}
}

Q_DECLARE_INTERFACE (LeechCraft::Poshuku::OnlineBookmarks::IBookmarksService,
					 "org.Deviant.LeechCraft.Poshuku.OnlineBookmarks.IBookmarksService/1.0");

#endif // PLUGINS_POSHUKU_PLUGINS_ONLINEBOOKMARKS_IBOOKMARKSSERVICE_H
