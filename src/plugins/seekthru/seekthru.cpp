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

#include "seekthru.h"
#include <interfaces/entitytesthandleresult.h>
#include <util/util.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "core.h"
#include "xmlsettingsmanager.h"
#include "searcherslist.h"
#include "wizardgenerator.h"

namespace LeechCraft
{
	namespace Plugins
	{
		namespace SeekThru
		{
			void SeekThru::Init (ICoreProxy_ptr proxy)
			{
				Translator_.reset (Util::InstallTranslator ("seekthru"));

				Core::Instance ().SetProxy (proxy);

				connect (&Core::Instance (),
						SIGNAL (delegateEntity (const LeechCraft::Entity&,
								int*, QObject**)),
						this,
						SIGNAL (delegateEntity (const LeechCraft::Entity&,
								int*, QObject**)));
				connect (&Core::Instance (),
						SIGNAL (gotEntity (const LeechCraft::Entity&)),
						this,
						SIGNAL (gotEntity (const LeechCraft::Entity&)));
				connect (&Core::Instance (),
						SIGNAL (error (const QString&)),
						this,
						SLOT (handleError (const QString&)),
						Qt::QueuedConnection);
				connect (&Core::Instance (),
						SIGNAL (warning (const QString&)),
						this,
						SLOT (handleWarning (const QString&)),
						Qt::QueuedConnection);
				connect (&Core::Instance (),
						SIGNAL (categoriesChanged (const QStringList&, const QStringList&)),
						this,
						SIGNAL (categoriesChanged (const QStringList&, const QStringList&)));
				connect (&Core::Instance (),
						SIGNAL (newDeltasAvailable (const Sync::ChainID_t&)),
						this,
						SIGNAL (newDeltasAvailable (const Sync::ChainID_t&)));

				Core::Instance ().DoDelayedInit ();

				XmlSettingsDialog_.reset (new Util::XmlSettingsDialog ());
				XmlSettingsDialog_->RegisterObject (&XmlSettingsManager::Instance (),
						"seekthrusettings.xml");

				auto searchersList = new SearchersList;
				connect (searchersList,
						SIGNAL (gotEntity (LeechCraft::Entity)),
						this,
						SIGNAL (gotEntity (LeechCraft::Entity)));
				XmlSettingsDialog_->SetCustomWidget ("SearchersList", searchersList);

				Chains_ << Sync::ChainID_t ("osengines");
			}

			void SeekThru::SecondInit ()
			{
			}

			void SeekThru::Release ()
			{
				XmlSettingsDialog_.reset ();
			}

			QByteArray SeekThru::GetUniqueID () const
			{
				return "org.LeechCraft.SeekThru";
			}

			QString SeekThru::GetName () const
			{
				return "SeekThru";
			}

			QString SeekThru::GetInfo () const
			{
				return tr ("Search via OpenSearch-aware search providers.");
			}

			QIcon SeekThru::GetIcon () const
			{
				static QIcon icon (":/resources/images/seekthru.svg");
				return icon;
			}

			QStringList SeekThru::Provides () const
			{
				return QStringList ("search");
			}

			QStringList SeekThru::Needs () const
			{
				return QStringList ("http");
			}

			QStringList SeekThru::Uses () const
			{
				return QStringList ("webbrowser");
			}

			void SeekThru::SetProvider (QObject *object, const QString& feature)
			{
				Core::Instance ().SetProvider (object, feature);
			}

			QStringList SeekThru::GetCategories () const
			{
				return Core::Instance ().GetCategories ();
			}

			QList<IFindProxy_ptr> SeekThru::GetProxy (const LeechCraft::Request& r)
			{
				QList<IFindProxy_ptr> result;
				result << Core::Instance ().GetProxy (r);
				return result;
			}

			std::shared_ptr<LeechCraft::Util::XmlSettingsDialog> SeekThru::GetSettingsDialog () const
			{
				return XmlSettingsDialog_;
			}

			EntityTestHandleResult SeekThru::CouldHandle (const Entity& e) const
			{
				return Core::Instance ().CouldHandle (e) ?
						EntityTestHandleResult (EntityTestHandleResult::PIdeal) :
						EntityTestHandleResult ();
			}

			void SeekThru::Handle (Entity e)
			{
				Core::Instance ().Handle (e);
			}

			QString SeekThru::GetFilterVerb () const
			{
				return tr ("Search in OpenSearch engines");
			}

			QList<SeekThru::FilterVariant> SeekThru::GetFilterVariants () const
			{
				QList<FilterVariant> result;
				for (const auto& cat : Core::Instance ().GetCategories ())
					result << FilterVariant
						{
							cat.toUtf8 (),
							cat,
							tr ("Search this term in OpenSearch engines in category %1.").arg (cat)
						};
				return result;
			}

			QList<QWizardPage*> SeekThru::GetWizardPages () const
			{
				std::auto_ptr<WizardGenerator> wg (new WizardGenerator);
				return wg->GetPages ();
			}

			Sync::ChainIDs_t SeekThru::AvailableChains () const
			{
				return Chains_;
			}

			Sync::Payloads_t SeekThru::GetAllDeltas (const Sync::ChainID_t& chainId) const
			{
				return Core::Instance ().GetAllDeltas (chainId);
			}

			Sync::Payloads_t SeekThru::GetNewDeltas (const Sync::ChainID_t& chainId) const
			{
				return Core::Instance ().GetNewDeltas (chainId);
			}

			void SeekThru::PurgeNewDeltas (const Sync::ChainID_t& chainId, quint32 num)
			{
				Core::Instance ().PurgeNewDeltas (chainId, num);
			}

			void SeekThru::ApplyDeltas (const Sync::Payloads_t& payloads,
					const Sync::ChainID_t& chainId)
			{
				Core::Instance ().ApplyDeltas (payloads, chainId);
			}

			void SeekThru::handleError (const QString& error)
			{
				emit gotEntity (Util::MakeNotification ("SeekThru", error, PCritical_));
			}

			void SeekThru::handleWarning (const QString& error)
			{
				emit gotEntity (Util::MakeNotification ("SeekThru", error, PWarning_));
			}
		};
	};
};

LC_EXPORT_PLUGIN (leechcraft_seekthru, LeechCraft::Plugins::SeekThru::SeekThru);
