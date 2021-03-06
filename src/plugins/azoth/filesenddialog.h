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

#include <QDialog>
#include "ui_filesenddialog.h"

namespace LeechCraft
{
namespace Azoth
{
	class ICLEntry;

	class FileSendDialog : public QDialog
	{
		Q_OBJECT

		Ui::FileSendDialog Ui_;
		ICLEntry *Entry_;
		const QString EntryVariant_;
		bool AccSupportsFT_;

		struct SharerInfo
		{
			QObject *Sharer_;
			QString Service_;
		};
		QMap<int, SharerInfo> Pos2Sharer_;
	public:
		FileSendDialog (ICLEntry*, const QString& = QString (), QWidget* = 0);
	private:
		void FillSharers ();
		void SendSharer (const SharerInfo&);
		void SendProto ();
	private slots:
		void send ();
		void on_FileBrowse__released ();
	};
}
}
