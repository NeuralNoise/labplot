/***************************************************************************
File                 : NetCDFOptionsWidget.cpp
Project              : LabPlot
Description          : widget providing options for the import of NetCDF data
--------------------------------------------------------------------
Copyright            : (C) 2015-2017 Stefan Gerlach (stefan.gerlach@uni.kn)
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#include "NetCDFOptionsWidget.h"
#include "ImportFileWidget.h"
#include "backend/datasources/filters/NetCDFFilter.h"
#include "backend/lib/macros.h"

 /*!
	\class NetCDFOptionsWidget
	\brief Widget providing options for the import of NetCDF data

	\ingroup kdefrontend
 */
NetCDFOptionsWidget::NetCDFOptionsWidget(QWidget* parent, ImportFileWidget* fileWidget)
		: QWidget(parent), m_fileWidget(fileWidget) {
	ui.setupUi(parent);

	QStringList headers;
	headers << i18n("Name") << i18n("Type") << i18n("Properties") << i18n("Values");
	ui.twContent->setHeaderLabels(headers);
	// type column is hidden
	ui.twContent->hideColumn(1);
	ui.twContent->setSelectionMode(QAbstractItemView::ExtendedSelection);
	ui.twContent->setAlternatingRowColors(true);
	ui.twPreview->setEditTriggers(QAbstractItemView::NoEditTriggers);

	ui.bRefreshPreview->setIcon( QIcon::fromTheme("view-refresh") );

	connect( ui.twContent, SIGNAL(itemSelectionChanged()), SLOT(netcdfTreeWidgetSelectionChanged()) );
	connect( ui.bRefreshPreview, SIGNAL(clicked()), fileWidget, SLOT(refreshPreview()) );
}

void NetCDFOptionsWidget::clear() {
	ui.twContent->clear();
	ui.twPreview->clear();
}

void NetCDFOptionsWidget::updateContent(NetCDFFilter *filter, const QString& fileName) {
	ui.twContent->clear();

	QTreeWidgetItem *rootItem = ui.twContent->invisibleRootItem();
	filter->parse(fileName, rootItem);
	ui.twContent->insertTopLevelItem(0, rootItem);
	ui.twContent->expandAll();
	ui.twContent->resizeColumnToContents(0);
	ui.twContent->resizeColumnToContents(2);
}

/*!
	updates the selected var name of a NetCDF file when the tree widget item is selected
*/
void NetCDFOptionsWidget::netcdfTreeWidgetSelectionChanged() {
	DEBUG("netcdfTreeWidgetSelectionChanged()");
	QDEBUG("SELECTED ITEMS =" << ui.twContent->selectedItems());

	if (ui.twContent->selectedItems().isEmpty())
		return;

	QTreeWidgetItem* item = ui.twContent->selectedItems().first();
	if (item->data(1, Qt::DisplayRole).toString() == "variable")
		m_fileWidget->refreshPreview();
	else if (item->data(1, Qt::DisplayRole).toString().contains("attribute")) {
		// reads attributes (only for preview)
		NetCDFFilter *filter = (NetCDFFilter *)m_fileWidget->currentFileFilter();
		QString fileName = m_fileWidget->ui.kleFileName->text();
		QString name = item->data(0, Qt::DisplayRole).toString();
		QString varName = item->data(1, Qt::DisplayRole).toString().split(' ')[0];
		QDEBUG("name =" << name << "varName =" << varName);

		QString importedText = filter->readAttribute(fileName, name, varName);
		DEBUG("importedText =" << importedText.toStdString());

		QStringList lineStrings = importedText.split('\n');
		int rows = lineStrings.size();
		ui.twPreview->setRowCount(rows);
		ui.twPreview->setColumnCount(0);
		for (int i = 0; i < rows; i++) {
			QStringList lineString = lineStrings[i].split(' ');
			int cols = lineString.size();
			if (ui.twPreview->columnCount() < cols)
				ui.twPreview->setColumnCount(cols);

			for (int j = 0; j < cols; j++) {
				QTableWidgetItem* item = new QTableWidgetItem();
				item->setText(lineString[j]);
				ui.twPreview->setItem(i, j, item);
			}
		}
	} else
		DEBUG("non showable object selected in NetCDF tree widget");
}

/*!
	return list of selected NetCDF item names
*/
const QStringList NetCDFOptionsWidget::selectedNetCDFNames() const {
	QStringList names;
	for (auto* item: ui.twContent->selectedItems())
		names << item->text(0);

	return names;
}
