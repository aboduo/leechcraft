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
#include <interfaces/iinfo.h>
#include <interfaces/itexteditor.h>
#include <interfaces/ihavesettings.h>

namespace LeechCraft
{
namespace LHTR
{
	class Plugin : public QObject
				 , public IInfo
				 , public ITextEditor
				 , public IHaveSettings
	{
		Q_OBJECT
		Q_INTERFACES (IInfo ITextEditor IHaveSettings)

		ICoreProxy_ptr Proxy_;
		Util::XmlSettingsDialog_ptr XSD_;
	public:
		void Init (ICoreProxy_ptr);
		void SecondInit ();
		QByteArray GetUniqueID () const;
		void Release ();
		QString GetName () const;
		QString GetInfo () const;
		QIcon GetIcon () const;

		bool SupportsEditor (ContentType) const;
		QWidget* GetTextEditor (ContentType);

		Util::XmlSettingsDialog_ptr GetSettingsDialog () const;
	};
}
}
