/*
    Scan Tailor - Interactive post-processing tool for scanned pages.
    Copyright (C) 2020 Alexander Trufanov <trufanovan@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef PUBLISH_FILTER_H_
#define PUBLISH_FILTER_H_

#include "NonCopyable.h"
#include "AbstractFilter.h"
#include "PageView.h"
#include "IntrusivePtr.h"
#include "FilterResult.h"
#include "SafeDeletingQObjectPtr.h"
#include "ProjectPages.h"
#include <QCoreApplication>
#include <QImage>

class PageId;
class PageSelectionAccessor;
class ThumbnailPixmapCache;
class OutputFileNameGenerator;
class QString;

namespace publish
{

class OptionsWidget;
class Task;
class CacheDrivenTask;
class Settings;

/**
 * \note All methods of this class except the destructor
 *       must be called from the GUI thread only.
 */
class Filter : public AbstractFilter
{
    DECLARE_NON_COPYABLE(Filter)
    Q_DECLARE_TR_FUNCTIONS(publishing::Filter)
public:
    Filter(IntrusivePtr<ProjectPages> const& pages, PageSelectionAccessor const& page_selection_accessor);

    virtual ~Filter();

    virtual QString getName() const;

    virtual PageView getView() const;

    virtual void performRelinking(AbstractRelinker const& relinker);

    virtual void preUpdateUI(FilterUiInterface* ui, PageId const&);

    virtual QDomElement saveSettings(
        ProjectWriter const& writer, QDomDocument& doc) const;

    virtual void loadSettings(
        ProjectReader const& reader, QDomElement const& filters_el);

    virtual void invalidateSetting(PageId const& page);

    IntrusivePtr<Task> createTask(
        PageId const& page_id,
        bool batch_processing);

    IntrusivePtr<CacheDrivenTask> createCacheDrivenTask();

    OptionsWidget* optionsWidget()
    {
        return m_ptrOptionsWidget.get();
    }

    Settings* getSettings()
    {
        return m_ptrSettings.get();
    }
private:
    void writeImageSettings(
        QDomDocument& doc, QDomElement& filter_el,
        ImageId const& image_id, int numeric_id) const;

    IntrusivePtr<ProjectPages> m_ptrPages;
    IntrusivePtr<Settings> m_ptrSettings;
    SafeDeletingQObjectPtr<OptionsWidget> m_ptrOptionsWidget;
};

} // publish

#endif