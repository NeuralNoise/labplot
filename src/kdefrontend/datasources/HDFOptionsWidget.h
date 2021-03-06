/***************************************************************************
File                 : HDFOptionsWidget.h
Project              : LabPlot
Description          : widget providing options for the import of HDF data
--------------------------------------------------------------------
Copyright            : (C) 2015-2017 Stefan Gerlach (stefan.gerlach@uni.kn)

**************************************************************************/

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
#ifndef HDFOPTIONSWIDGET_H
#define HDFOPTIONSWIDGET_H

#include "ui_hdfoptionswidget.h"

class HDFFilter;
class ImportFileWidget;

class HDFOptionsWidget : public QWidget {
    Q_OBJECT

public:
	explicit HDFOptionsWidget(QWidget*, ImportFileWidget*);
	void clear();
	void updateContent(HDFFilter*, QString fileName);
	const QStringList selectedHDFNames() const;
	int lines() const { return ui.sbPreviewLines->value(); }
	QTableWidget* previewWidget() const { return ui.twPreview; }

private:
	Ui::HDFOptionsWidget ui;
	ImportFileWidget* m_fileWidget;

private slots:
	void hdfTreeWidgetSelectionChanged();
};

#endif
