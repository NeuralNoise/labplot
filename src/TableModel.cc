//LabPlot: TableModel.cc

#include <KDebug>
#include "TableModel.h"

TableModel::TableModel(QObject *parent)
	: QAbstractTableModel(parent)
{
	insertColumns(0,2);
	insertRows(0,100);
}

int TableModel::rowCount(const QModelIndex &) const {
	if(columnCount()>0)
		return table[0].size();
	return 0;
}

int TableModel::columnCount(const QModelIndex &) const {
	return table.size();
}

void TableModel::setRowCount(int rows) {
	kdDebug()<<"TableModel::setRowCount("<<rows<<") rowCount()="<<rowCount()<<endl;
	if(rows==rowCount())
		return;
	else if(rows>rowCount())
		insertRows(rowCount(),rows-rowCount());
	else
		removeRows(rows,rowCount()-rows);
}

void TableModel::setColumnCount(int cols) {
	kdDebug()<<"TableModel::setColumnCount("<<cols<<")"<<endl;
	if(cols==columnCount())
		return;
	else if(cols>columnCount())
		insertColumns(columnCount(),cols-columnCount());
	else
		removeColumns(cols,columnCount()-cols);
}

bool TableModel::insertRows(int row, int count, const QModelIndex &parent) {
	kdDebug()<<"TableModel::insertRows("<<row<<count<<")"<<endl;
	beginInsertRows(parent,row,row+count-1);
	for(int i=0;i<columnCount();i++) {
		for(int j=row;j<row+count;j++)
//			table[i].append(QString::number(j)+'/'+QString::number(i));
			table[i].append(QString(""));
	}
	endInsertRows();
	return true;
}

bool TableModel::removeRows(int row, int count, const QModelIndex &parent) {
	kdDebug()<<"TableModel::removeRows("<<row<<count<<")"<<endl;
	beginRemoveRows(parent,row,row+count-1);
	for(int i=0;i<columnCount();i++) {
		for(int j=0;j<count;j++)
			table[i].removeLast();
	}
	endRemoveRows();
	return true;
}

bool TableModel::insertColumns(int col, int count, const QModelIndex &parent) {
	kdDebug()<<"TableModel::insertColumns("<<col<<count<<")"<<endl;
	beginInsertColumns(parent,col,col+count-1);
	for(int i=0;i<count;i++) {
		QList<QString> list;
		table.append(list);
		for(int j=0;j<rowCount();j++)
//			table[col+i].append(QString::number(j)+'/'+QString::number(col+i));
			table[col+i].append(QString(""));
		header.append(QString(""));
	}
	endInsertColumns();
	return true;
}

bool TableModel::removeColumns(int col, int count, const QModelIndex &parent) {
	kdDebug()<<"TableModel::removeColumns("<<col<<count<<")"<<endl;
	beginRemoveColumns(parent,col,col+count-1);
	for(int j=0;j<count;j++) {
		table.removeLast();
		header.removeLast();
	}
	endRemoveColumns();
	return true;
}

QVariant TableModel::data(const QModelIndex &index, int role) const {
	if (!index.isValid())
		return QVariant();

	int row = index.row(), col = index.column();

	switch(role) {
		case Qt::ToolTipRole:
		case Qt::EditRole:
		case Qt::DisplayRole:
     			return QVariant(table[col][row]);
	}
	return QVariant();
}

bool TableModel::setData(const QModelIndex &index, const QVariant &value, int role) {
	if (index.isValid() && role == Qt::EditRole) {
		table[index.column()].replace(index.row(),value.toString());
		emit dataChanged(index, index);
		return true;
	}
	return false;
}

Qt::ItemFlags TableModel::flags(const QModelIndex &index) const {
     if (!index.isValid())
         return Qt::ItemIsEnabled;

     return QAbstractItemModel::flags(index) | Qt::ItemIsEditable;
}

QVariant TableModel::headerData(int section, Qt::Orientation orientation, int role) const {
	switch(orientation) {
		case Qt::Horizontal:
			switch(role) {
				case Qt::DisplayRole:
				case Qt::ToolTipRole:
					return QVariant(header[section]);
			}
		case Qt::Vertical:
			switch(role) {
				case Qt::DisplayRole:
				case Qt::ToolTipRole:
					return QVariant(QString::number(section+1));
			}
	}
	return QVariant();
}

bool TableModel::setHeaderData(int section, Qt::Orientation orientation, const QVariant &value, int) {
	if(orientation != Qt::Horizontal || section < 0 || section > columnCount() ) return false;

	header[section]=value.toString();

	emit headerDataChanged(orientation,section,section);
	return true;
}
