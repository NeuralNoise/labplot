/***************************************************************************
File                 : HDFFilterPrivate.h
Project              : LabPlot
Description          : Private implementation class for HDFFilter.
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
#ifndef HDFFILTERPRIVATE_H
#define HDFFILTERPRIVATE_H

#include <QList>
#ifdef HAVE_HDF5
#include <hdf5.h>
#endif

class AbstractDataSource;

class HDFFilterPrivate {

public:
	explicit HDFFilterPrivate(HDFFilter*);

	void parse(const QString & fileName, QTreeWidgetItem* rootItem);
	QVector<QStringList> readDataFromFile(const QString& fileName, AbstractDataSource* = nullptr,
					AbstractFileFilter::ImportMode = AbstractFileFilter::Replace, int lines = -1);
	QVector<QStringList> readCurrentDataSet(const QString& fileName, AbstractDataSource*, bool &ok,
						AbstractFileFilter::ImportMode = AbstractFileFilter::Replace, int lines = -1);
	void write(const QString& fileName, AbstractDataSource*);

	const HDFFilter* q;

	QString currentDataSetName;
	int startRow;
	int endRow;
	int startColumn;
	int endColumn;

private:
	const static int MAXNAMELENGTH = 1024;
	const static int MAXSTRINGLENGTH = 1024*1024;
	int m_status;
	QList<unsigned long> m_multiLinkList;	// used to find hard links

#ifdef HAVE_HDF5
	void handleError(int err, QString function, QString arg=QString());
	QString translateHDFOrder(H5T_order_t);
	QString translateHDFType(hid_t);
	QString translateHDFClass(H5T_class_t);
	QStringList readHDFCompound(hid_t tid);
	template <typename T> QStringList readHDFData1D(hid_t dataset, hid_t type, int rows, int lines,
							void* dataPointer = nullptr);
	QStringList readHDFCompoundData1D(hid_t dataset, hid_t tid, int rows, int lines, QVector<void*>& dataPointer);
	template <typename T> QVector<QStringList> readHDFData2D(hid_t dataset, hid_t ctype, int rows, int cols, int lines,
								 QVector<void*>& dataPointer);
	QVector<QStringList> readHDFCompoundData2D(hid_t dataset, hid_t tid, int rows, int cols, int lines);
	QStringList readHDFAttr(hid_t aid);
	QStringList scanHDFAttrs(hid_t oid);
	QStringList readHDFDataType(hid_t tid);
	QStringList readHDFPropertyList(hid_t pid);
	void scanHDFDataType(hid_t tid, char* dataTypeName,  QTreeWidgetItem* parentItem);
	void scanHDFLink(hid_t gid, char* linkName,  QTreeWidgetItem* parentItem);
	void scanHDFDataSet(hid_t dsid, char* dataSetName,  QTreeWidgetItem* parentItem);
	void scanHDFGroup(hid_t gid, char* groupName, QTreeWidgetItem* parentItem);
#endif
};

#endif
