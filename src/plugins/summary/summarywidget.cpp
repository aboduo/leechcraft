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

#include "summarywidget.h"
#include <QTimer>
#include <QComboBox>
#include <QMenu>
#include <QToolBar>
#include <QMainWindow>
#include <QWidgetAction>
#include <QCloseEvent>
#include <QSortFilterProxyModel>
#include <QLineEdit>
#include <QtDebug>
#include <interfaces/structures.h>
#include <interfaces/ijobholder.h>
#include <interfaces/core/icoreproxy.h>
#include <interfaces/core/ipluginsmanager.h>
#include <interfaces/core/irootwindowsmanager.h>
#include <interfaces/imwproxy.h>
#include <util/gui/clearlineeditaddon.h>
#include "core.h"
#include "summary.h"
#include "modeldelegate.h"

namespace LeechCraft
{
namespace Summary
{
	QObject *SummaryWidget::S_ParentMultiTabs_ = 0;

	class SearchWidget : public QWidget
	{
		QLineEdit *Edit_;
	public:
		SearchWidget (SummaryWidget *summary)
		: Edit_ (new QLineEdit)
		{
			auto lay = new QHBoxLayout;
			setLayout (lay);

			Edit_->setPlaceholderText (SummaryWidget::tr ("Search..."));
			Edit_->setMaximumWidth (400);
			Edit_->setSizePolicy (QSizePolicy::Expanding, QSizePolicy::Fixed);
			lay->addStretch ();
			lay->addWidget (Edit_, 0, Qt::AlignRight);
			new Util::ClearLineEditAddon (Core::Instance ().GetProxy (), Edit_);

			connect (Edit_,
					SIGNAL (textChanged (QString)),
					summary,
					SLOT (filterParametersChanged ()));
			connect (Edit_,
					SIGNAL (returnPressed ()),
					summary,
					SLOT (feedFilterParameters ()));
		}

		QString GetText () const
		{
			return Edit_->text ();
		}

		void SetText (const QString& text)
		{
			Edit_->setText (text);
		}
	};

	SummaryWidget::SummaryWidget (QWidget *parent)
	: QWidget (parent)
	, FilterTimer_ (new QTimer)
	, SearchWidget_ (CreateSearchWidget ())
	, Toolbar_ (new QToolBar)
	, Sorter_ (Core::Instance ().GetTasksModel ())
	{
		Toolbar_->setWindowTitle ("Summary");
		connect (Toolbar_.get (),
				SIGNAL (actionTriggered (QAction*)),
				this,
				SLOT (handleActionTriggered (QAction*)));

		Toolbar_->addWidget (SearchWidget_);

		Ui_.setupUi (this);
		Ui_.PluginsTasksTree_->setItemDelegate (new ModelDelegate (this));

		FilterTimer_->setSingleShot (true);
		FilterTimer_->setInterval (800);
		connect (FilterTimer_,
				SIGNAL (timeout ()),
				this,
				SLOT (feedFilterParameters ()));

		Ui_.ControlsDockWidget_->hide ();

		auto pm = Core::Instance ().GetProxy ()->GetPluginsManager ();
		Q_FOREACH (QObject *plugin, pm->GetAllCastableRoots<IJobHolder*> ())
			ConnectObject (plugin);

		Ui_.PluginsTasksTree_->setModel (Sorter_);

		connect (Sorter_,
				SIGNAL (dataChanged (const QModelIndex&, const QModelIndex&)),
				this,
				SLOT (checkDataChanged (const QModelIndex&, const QModelIndex&)));
		connect (Sorter_,
				SIGNAL (modelAboutToBeReset ()),
				this,
				SLOT (handleReset ()));
		connect (Sorter_,
				SIGNAL (rowsAboutToBeRemoved (const QModelIndex&, int, int)),
				this,
				SLOT (checkRowsToBeRemoved (const QModelIndex&, int, int)));
		connect (Ui_.PluginsTasksTree_->selectionModel (),
				SIGNAL (currentRowChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (updatePanes (QModelIndex, QModelIndex)));
		connect (Ui_.PluginsTasksTree_->selectionModel (),
				SIGNAL (currentRowChanged (QModelIndex, QModelIndex)),
				this,
				SLOT (syncSelection (QModelIndex)),
				Qt::QueuedConnection);

		QHeaderView *itemsHeader = Ui_.PluginsTasksTree_->header ();
		QFontMetrics fm = fontMetrics ();
		itemsHeader->resizeSection (0,
				fm.width ("Average download job or torrent name is just like this."));
		itemsHeader->resizeSection (1,
				fm.width ("Of the download."));
		itemsHeader->resizeSection (2,
				fm.width ("99.99% (1024.0 kb from 1024.0 kb at 1024.0 kb/s)"));

		ReconnectModelSpecific ();
	}

	void SummaryWidget::ReconnectModelSpecific ()
	{
		QItemSelectionModel *sel = Ui_.PluginsTasksTree_->selectionModel ();

#define C2(sig,sl,arg1,arg2) \
		if (mo->indexOfMethod (QMetaObject::normalizedSignature ("handleTasksTreeSelection" #sl "(" #arg1 ", " #arg2 ")")) != -1) \
			connect (sel, \
					SIGNAL (sig (arg1, arg2)), \
					object, \
					SLOT (handleTasksTreeSelection##sl (arg1, arg2)));

		auto pm = Core::Instance ().GetProxy ()->GetPluginsManager ();
		Q_FOREACH (QObject *object, pm->GetAllCastableRoots<IJobHolder*> ())
		{
			const QMetaObject *mo = object->metaObject ();

			C2 (currentChanged, CurrentChanged, const QModelIndex&, const QModelIndex&);
			C2 (currentColumnChanged, CurrentColumnChanged, const QModelIndex&, const QModelIndex&);
			C2 (currentRowChanged, CurrentRowChanged, const QModelIndex&, const QModelIndex&);
		}
#undef C2
	}

	void SummaryWidget::ConnectObject (QObject *object)
	{
		const QMetaObject *mo = object->metaObject ();

#define C1(sig,sl,arg) \
		if (mo->indexOfMethod (QMetaObject::normalizedSignature ("handleTasksTree" #sl "(" #arg ")")) != -1) \
			connect (Ui_.PluginsTasksTree_, \
					SIGNAL (sig (arg)), \
					object, \
					SLOT (handleTasksTree##sl (arg)));

		C1 (activated, Activated, const QModelIndex&);
		C1 (clicked, Clicked, const QModelIndex&);
		C1 (doubleClicked, DoubleClicked, const QModelIndex&);
		C1 (entered, Entered, const QModelIndex&);
		C1 (pressed, Pressed, const QModelIndex&);
		C1 (viewportEntered, ViewportEntered, );
#undef C1
	}

	SummaryWidget::~SummaryWidget ()
	{
		Toolbar_->clear ();

		QWidget *widget = Ui_.ControlsDockWidget_->widget ();
		Ui_.ControlsDockWidget_->setWidget (0);
		if (widget)
			widget->setParent (0);

		delete Sorter_;
	}

	void SummaryWidget::SetParentMultiTabs (QObject *parent)
	{
		S_ParentMultiTabs_ = parent;
	}

	void SummaryWidget::Remove ()
	{
		emit needToClose ();
	}

	QToolBar* SummaryWidget::GetToolBar () const
	{
		return Toolbar_.get ();
	}

	QList<QAction*> SummaryWidget::GetTabBarContextMenuActions () const
	{
		return QList<QAction*> ();
	}

	QObject* SummaryWidget::ParentMultiTabs ()
	{
		return S_ParentMultiTabs_;
	}

	TabClassInfo SummaryWidget::GetTabClassInfo () const
	{
		return qobject_cast<Summary*> (S_ParentMultiTabs_)->GetTabClasses ().first ();
	}

	SearchWidget* SummaryWidget::CreateSearchWidget ()
	{
		return new SearchWidget (this);
	}

	void SummaryWidget::ReinitToolbar ()
	{
		Q_FOREACH (QAction *action, Toolbar_->actions ())
		{
			auto wa = qobject_cast<QWidgetAction*> (action);
			if (!wa)
			{
				Toolbar_->removeAction (action);
				delete action;
			}
			else if (wa->defaultWidget () != SearchWidget_)
				Toolbar_->removeAction (action);
		}
	}

	QList<QAction*> SummaryWidget::CreateProxyActions (const QList<QAction*>& actions) const
	{
		QList<QAction*> proxies;

		Q_FOREACH (QAction *action, actions)
		{
			QAction *pa = new QAction (action->icon (),
					action->text (), Toolbar_.get ());
			if (action->isSeparator ())
				pa->setSeparator (true);
			else if (qobject_cast<QWidgetAction*> (action))
			{
				proxies << action;
				continue;
			}
			else
			{
				pa->setCheckable (action->isCheckable ());
				pa->setChecked (action->isChecked ());
				pa->setShortcuts (action->shortcuts ());
				pa->setStatusTip (action->statusTip ());
				pa->setToolTip (action->toolTip ());
				pa->setWhatsThis (action->whatsThis ());
				pa->setData (QVariant::fromValue<QObject*> (action));

				connect (pa,
						SIGNAL (hovered ()),
						action,
						SIGNAL (hovered ()));
				connect (pa,
						SIGNAL (toggled (bool)),
						action,
						SIGNAL (toggled (bool)));
			}
			proxies << pa;
		}

		return proxies;
	}

	QByteArray SummaryWidget::GetTabRecoverData () const
	{
		QByteArray result;
		QDataStream out (&result, QIODevice::WriteOnly);
		out << static_cast<quint8> (1);
		return result;
	}

	QString SummaryWidget::GetTabRecoverName () const
	{
		return GetTabClassInfo ().VisibleName_;
	}

	QIcon SummaryWidget::GetTabRecoverIcon () const
	{
		return GetTabClassInfo ().Icon_;
	}

	void SummaryWidget::RestoreState (const QByteArray& data)
	{
		QDataStream in (data);
		quint8 version = 0;
		in >> version;
		if (version != 1)
		{
			qWarning () << Q_FUNC_INFO
					<< "unknown version";
			return;
		}
	}

	void SummaryWidget::SetUpdatesEnabled (bool)
	{
		// TODO implement this
	}

	Ui::SummaryWidget SummaryWidget::GetUi () const
	{
		return Ui_;
	}

	void SummaryWidget::handleActionTriggered (QAction *proxyAction)
	{
		QAction *action = qobject_cast<QAction*> (proxyAction->
				data ().value<QObject*> ());
		QItemSelectionModel *selModel =
				Ui_.PluginsTasksTree_->selectionModel ();
		QModelIndexList indexes = selModel->selectedRows ();
		action->setProperty ("SelectedRows",
				QVariant::fromValue<QList<QModelIndex>> (indexes));
		action->setProperty ("ItemSelectionModel",
				QVariant::fromValue<QObject*> (selModel));

		action->activate (QAction::Trigger);
	}

	void SummaryWidget::checkDataChanged (const QModelIndex& topLeft, const QModelIndex& bottomRight)
	{
		const QModelIndex& cur = Ui_.PluginsTasksTree_->
				selectionModel ()->currentIndex ();
		if (topLeft.row () <= cur.row () && bottomRight.row () >= cur.row ())
			updatePanes (cur, cur);
	}

	void SummaryWidget::handleReset ()
	{
		Ui_.PluginsTasksTree_->selectionModel ()->clear ();
	}

	void SummaryWidget::checkRowsToBeRemoved (const QModelIndex&, int begin, int end)
	{
		const QModelIndex& cur = Ui_.PluginsTasksTree_->
				selectionModel ()->currentIndex ();
		if (begin <= cur.row () && end >= cur.row ())
			Ui_.PluginsTasksTree_->selectionModel ()->clear ();
	}

	void SummaryWidget::updatePanes (const QModelIndex& newIndex, const QModelIndex& oldIndex)
	{
		QToolBar *controls = Core::Instance ().GetControls (newIndex);
		QWidget *addiInfo = Core::Instance ().GetAdditionalInfo (newIndex);

		if (oldIndex.isValid () &&
				addiInfo != Ui_.ControlsDockWidget_->widget ())
			Ui_.ControlsDockWidget_->hide ();

		if (Core::Instance ().SameModel (newIndex, oldIndex))
			return;

		ReinitToolbar ();
		if (newIndex.isValid ())
		{
			if (controls)
			{
				Q_FOREACH (QAction *action, controls->actions ())
				{
					QString ai = action->property ("ActionIcon").toString ();
					if (!ai.isEmpty () &&
							action->icon ().isNull ())
						action->setIcon (Core::Instance ().GetProxy ()->GetIcon (ai));
				}

				const auto& proxies = CreateProxyActions (controls->actions ());
				Toolbar_->insertActions (Toolbar_->actions ().first (), proxies);
			}
			if (addiInfo != Ui_.ControlsDockWidget_->widget ())
				Ui_.ControlsDockWidget_->setWidget (addiInfo);

			if (addiInfo)
			{
				Ui_.ControlsDockWidget_->show ();
				Core::Instance ().GetProxy()->UpdateIconset (addiInfo->findChildren<QAction*> ());
			}
		}
	}

	void SummaryWidget::filterParametersChanged ()
	{
		FilterTimer_->stop ();
		FilterTimer_->start ();
	}

	void SummaryWidget::filterReturnPressed ()
	{
		FilterTimer_->stop ();
		feedFilterParameters ();
	}

	void SummaryWidget::feedFilterParameters ()
	{
		Sorter_->setFilterFixedString (SearchWidget_->GetText ());
	}

	void SummaryWidget::on_PluginsTasksTree__customContextMenuRequested (const QPoint& pos)
	{
		QModelIndex current = Ui_.PluginsTasksTree_->currentIndex ();
		QMenu *sourceMenu = current.data (RoleContextMenu).value<QMenu*> ();
		if (!sourceMenu)
			return;

		QMenu *menu = new QMenu ();
		connect (menu,
				SIGNAL (triggered (QAction*)),
				this,
				SLOT (handleActionTriggered (QAction*)));
		menu->setAttribute (Qt::WA_DeleteOnClose, true);
		menu->addActions (CreateProxyActions (sourceMenu->actions ()));
		menu->setTitle (sourceMenu->title ());
		menu->popup (Ui_.PluginsTasksTree_->viewport ()->mapToGlobal (pos));
	}

	void SummaryWidget::syncSelection (const QModelIndex& current)
	{
		QItemSelectionModel *selm = Ui_.PluginsTasksTree_->selectionModel ();
		const QModelIndex& now = selm->currentIndex ();
#ifdef QT_DEBUG
		qDebug () << Q_FUNC_INFO << this << current << now;
#endif
		if (current != now ||
				(now.isValid () &&
					!selm->rowIntersectsSelection (now.row (), QModelIndex ())))
		{
			selm->select (now, QItemSelectionModel::ClearAndSelect |
					QItemSelectionModel::Rows);
			updatePanes (now, current);
		}
	}
}
}
