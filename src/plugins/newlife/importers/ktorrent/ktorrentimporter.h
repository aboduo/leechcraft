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

#include "abstractimporter.h"

namespace LeechCraft
{
namespace NewLife
{
namespace Importers
{
	class KTorrentImportPage;

	class KTorrentImporter : public AbstractImporter
	{
		Q_OBJECT

		KTorrentImportPage *ImportPage_;
	public:
		KTorrentImporter (QWidget* = 0);

		QStringList GetNames () const;
		QList<QIcon> GetIcons () const;
		QList<QWizardPage*> GetWizardPages () const;
	};
}
}
}
