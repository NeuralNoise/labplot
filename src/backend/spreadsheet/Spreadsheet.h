/***************************************************************************
    File                 : Spreadsheet.h
    Project              : LabPlot
    Description          : Aspect providing a spreadsheet table with column logic
    --------------------------------------------------------------------
    Copyright            : (C) 2010-2016 Alexander Semke(alexander.semke@web.de)
    Copyright            : (C) 2006-2008 Tilman Benkert (thzs@gmx.net)

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
#ifndef SPREADSHEET_H
#define SPREADSHEET_H

#include "backend/datasources/AbstractDataSource.h"
#include "backend/core/column/Column.h"
#include "backend/core/column/ColumnStringIO.h"

class AbstractFileFilter;
template <class t> class QList;

class Spreadsheet : public AbstractDataSource {
	Q_OBJECT

public:
	Spreadsheet(AbstractScriptingEngine* engine, const QString& name, bool loading = false);

	virtual QIcon icon() const override;
	virtual QMenu* createContextMenu() override;
	virtual QWidget* view() const override;

	virtual bool exportView() const override;
	virtual bool printView() override;
	virtual bool printPreview() const override;

	int columnCount() const;
	int columnCount(AbstractColumn::PlotDesignation) const;
	Column* column(int index) const;
	Column* column(const QString&) const;
	int rowCount() const;

	void removeRows(int first, int count);
	void insertRows(int before, int count);
	void removeColumns(int first, int count);
	void insertColumns(int before, int count);

	int colX(int col);
	int colY(int col);
	QString text(int row, int col) const;

	void copy(Spreadsheet* other);

	virtual void save(QXmlStreamWriter*) const override;
	virtual bool load(XmlStreamReader*) override;

	void setColumnSelectedInView(int index, bool selected);

	// used from model to inform dock
	void emitRowCountChanged() { emit rowCountChanged(rowCount()); }
	void emitColumnCountChanged() { emit columnCountChanged(columnCount()); }

	//data import
	virtual int prepareImport(QVector<void*>& dataContainer, AbstractFileFilter::ImportMode,
		int rows, int cols, QStringList colNameList, QVector<AbstractColumn::ColumnMode>) override;
	virtual void finalizeImport(int columnOffset, int startColumn , int endColumn,
		const QString& dateTimeFormat, AbstractFileFilter::ImportMode) override;
	int resize(AbstractFileFilter::ImportMode, QStringList colNameList, int cols);

public slots:
	void appendRows(int count);
	void appendRow();
	void appendColumns(int count);
	void appendColumn();
	void prependColumns(int count);

	void setColumnCount(int);
	void setRowCount(int);

	void clear();
	void clearMasks();

	void moveColumn(int from, int to);
	void sortColumns(Column* leading, QList<Column*>, bool ascending);

private:
	void init();

private slots:
	virtual void childSelected(const AbstractAspect*) override;
	virtual void childDeselected(const AbstractAspect*) override;

signals:
	void requestProjectContextMenu(QMenu*);
	void columnSelected(int);
	void columnDeselected(int);

	// for spreadsheet dock
	void rowCountChanged(int);
	void columnCountChanged(int);
};

#endif
