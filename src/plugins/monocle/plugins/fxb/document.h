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
#include <QUrl>
#include <interfaces/monocle/ihavetoc.h>
#include <interfaces/monocle/isearchabledocument.h>
#include "documentadapter.h"

namespace LeechCraft
{
namespace Monocle
{
namespace FXB
{
	class Document : public QObject
				   , public DocumentAdapter
				   , public IHaveTOC
				   , public ISearchableDocument
	{
		Q_OBJECT
		Q_INTERFACES (LeechCraft::Monocle::IDocument
				LeechCraft::Monocle::IHaveTOC
				LeechCraft::Monocle::ISearchableDocument)

		DocumentInfo Info_;
		TOCEntryLevel_t TOC_;
		QUrl DocURL_;

		QObject *Plugin_;
	public:
		Document (const QString&, QObject*);

		QObject* GetBackendPlugin () const;
		QObject* GetQObject ();
		DocumentInfo GetDocumentInfo () const;
		QUrl GetDocURL () const;

		TOCEntryLevel_t GetTOC ();

		QMap<int, QList<QRectF>> GetTextPositions (const QString& text, Qt::CaseSensitivity cs);

		void RequestNavigation (int);
	signals:
		void navigateRequested (const QString&, int pageNum, double x, double y);
		void printRequested (const QList<int>&);
	};
}
}
}
