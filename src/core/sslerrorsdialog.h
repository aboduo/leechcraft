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

#ifndef SSLERRORSDIALOG_H
#define SSLERRORSDIALOG_H
#include <QDialog>
#include <QList>
#include <QSslError>
#include "ui_sslerrorsdialog.h"

namespace LeechCraft
{
	class SslErrorsDialog : public QDialog
	{
		Q_OBJECT

		Ui::SslErrorsDialog Ui_;
	public:
		enum RememberChoice
		{
			RCNot
			, RCFile
			, RCHost
		};

		SslErrorsDialog (QWidget* = 0);
		SslErrorsDialog (const QString&, const QList<QSslError>&, QWidget* = 0);
		virtual ~SslErrorsDialog ();

		void Update (const QString&, const QList<QSslError>&);

		RememberChoice GetRememberChoice () const;
	private:
		void PopulateTree (const QSslError&);
	};
};

#endif

