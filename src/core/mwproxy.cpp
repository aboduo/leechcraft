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

#include "mwproxy.h"
#include <QDockWidget>
#include "core.h"
#include "mainwindow.h"
#include "dockmanager.h"

namespace LeechCraft
{
	MWProxy::MWProxy (MainWindow *win, QObject *parent)
	: QObject (parent)
	, Win_ (win)
	{
	}

	void MWProxy::AddDockWidget (Qt::DockWidgetArea area, QDockWidget *w)
	{
		Core::Instance ().GetDockManager ()->AddDockWidget (w, area);
		ToggleViewActionVisiblity (w, true);
	}

	void MWProxy::AssociateDockWidget (QDockWidget *dock, QWidget *tab)
	{
		Core::Instance ().GetDockManager ()->AssociateDockWidget (dock, tab);
	}

	void MWProxy::ToggleViewActionVisiblity (QDockWidget *w, bool visible)
	{
		Core::Instance ().GetDockManager ()->ToggleViewActionVisiblity (w, visible);
	}

	void MWProxy::SetViewActionShortcut (QDockWidget *w, const QKeySequence& seq)
	{
		w->toggleViewAction ()->setShortcut (seq);
	}

	void MWProxy::AddToolbar (QToolBar *bar, Qt::ToolBarArea area)
	{
		bar->setParent (Win_);
		Win_->addToolBar (area, bar);
	}

	void MWProxy::AddSideWidget (QWidget *w, WidgetArea area)
	{
		auto splitter = Win_->GetMainSplitter ();

		switch (area)
		{
		case WALeft:
			splitter->insertWidget (0, w);
			break;
		case WARight:
			splitter->addWidget (w);
			break;
		case WABottom:
			qWarning () << Q_FUNC_INFO
					<< "not implemented yet";
			break;
		}
	}

	void MWProxy::ToggleVisibility ()
	{
		Win_->showHideMain ();
	}

	QMenu* MWProxy::GetMainMenu ()
	{
		return Win_->GetMainMenu ();
	}

	void MWProxy::HideMainMenu ()
	{
		Win_->HideMainMenu ();
	}
}
