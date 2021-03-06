/***************************************************************************
    File                 : ColumnPrivate.cpp
    Project              : AbstractColumn
    Description          : Private data class of Column
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2008 Tilman Benkert (thzs@gmx.net)
    Copyright            : (C) 2012-2017 Alexander Semke (alexander.semke@web.de)
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

#include "ColumnPrivate.h"
#include "ColumnStringIO.h"
#include "Column.h"
#include "backend/core/datatypes/SimpleCopyThroughFilter.h"
#include "backend/core/datatypes/String2DoubleFilter.h"
#include "backend/core/datatypes/Double2StringFilter.h"
#include "backend/core/datatypes/Double2DateTimeFilter.h"
#include "backend/core/datatypes/Double2MonthFilter.h"
#include "backend/core/datatypes/Double2DayOfWeekFilter.h"
#include "backend/core/datatypes/String2DateTimeFilter.h"
#include "backend/core/datatypes/DateTime2StringFilter.h"
#include "backend/core/datatypes/String2MonthFilter.h"
#include "backend/core/datatypes/String2DayOfWeekFilter.h"
#include "backend/core/datatypes/DateTime2DoubleFilter.h"
#include "backend/core/datatypes/DayOfWeek2DoubleFilter.h"
#include "backend/core/datatypes/Month2DoubleFilter.h"

ColumnPrivate::ColumnPrivate(Column* owner, AbstractColumn::ColumnMode mode)
	: statisticsAvailable(false),
	m_column_mode(mode),
	m_plot_designation(AbstractColumn::NoDesignation),
	m_width(0),
	m_owner(owner) {
	Q_ASSERT(owner != 0);

	switch(mode) {
	case AbstractColumn::Numeric:
		m_input_filter = new String2DoubleFilter();
		m_output_filter = new Double2StringFilter();
		m_data = new QVector<double>();
		break;
	case AbstractColumn::Text:
		m_input_filter = new SimpleCopyThroughFilter();
		m_output_filter = new SimpleCopyThroughFilter();
		m_data = new QStringList();
		break;
	case AbstractColumn::DateTime:
		m_input_filter = new String2DateTimeFilter();
		m_output_filter = new DateTime2StringFilter();
		m_data = new QVector<QDateTime>();
		break;
	case AbstractColumn::Month:
		m_input_filter = new String2MonthFilter();
		m_output_filter = new DateTime2StringFilter();
		static_cast<DateTime2StringFilter *>(m_output_filter)->setFormat("MMMM");
		m_data = new QVector<QDateTime>();
		break;
	case AbstractColumn::Day:
		m_input_filter = new String2DayOfWeekFilter();
		m_output_filter = new DateTime2StringFilter();
		static_cast<DateTime2StringFilter *>(m_output_filter)->setFormat("dddd");
		m_data = new QVector<QDateTime>();
		break;
	}

	connect(m_output_filter, SIGNAL(formatChanged()), m_owner, SLOT(handleFormatChange()));

	m_input_filter->setName("InputFilter");
	m_output_filter->setName("OutputFilter");
}

/**
 * \brief Special ctor (to be called from Column only!)
 */
ColumnPrivate::ColumnPrivate(Column* owner, AbstractColumn::ColumnMode mode, void* data)
	: statisticsAvailable(false),
	m_column_mode(mode),
	m_data(data),
	m_plot_designation(AbstractColumn::NoDesignation),
	m_width(0),
	m_owner(owner) {

	switch(mode) {
	case AbstractColumn::Numeric:
		m_input_filter = new String2DoubleFilter();
		m_output_filter = new Double2StringFilter();
		connect(static_cast<Double2StringFilter *>(m_output_filter), SIGNAL(formatChanged()),
		        m_owner, SLOT(handleFormatChange()));
		break;
	case AbstractColumn::Text:
		m_input_filter = new SimpleCopyThroughFilter();
		m_output_filter = new SimpleCopyThroughFilter();
		break;
	case AbstractColumn::DateTime:
		m_input_filter = new String2DateTimeFilter();
		m_output_filter = new DateTime2StringFilter();
		connect(static_cast<DateTime2StringFilter *>(m_output_filter), SIGNAL(formatChanged()),
		        m_owner, SLOT(handleFormatChange()));
		break;
	case AbstractColumn::Month:
		m_input_filter = new String2MonthFilter();
		m_output_filter = new DateTime2StringFilter();
		static_cast<DateTime2StringFilter *>(m_output_filter)->setFormat("MMMM");
		connect(static_cast<DateTime2StringFilter *>(m_output_filter), SIGNAL(formatChanged()),
		        m_owner, SLOT(handleFormatChange()));
		break;
	case AbstractColumn::Day:
		m_input_filter = new String2DayOfWeekFilter();
		m_output_filter = new DateTime2StringFilter();
		static_cast<DateTime2StringFilter *>(m_output_filter)->setFormat("dddd");
		connect(static_cast<DateTime2StringFilter *>(m_output_filter), SIGNAL(formatChanged()),
		        m_owner, SLOT(handleFormatChange()));
		break;
	}

	m_input_filter->setName("InputFilter");
	m_output_filter->setName("OutputFilter");
}

ColumnPrivate::~ColumnPrivate() {
	if (!m_data) return;

	switch(m_column_mode) {
	case AbstractColumn::Numeric:
		delete static_cast<QVector<double>*>(m_data);
		break;

	case AbstractColumn::Text:
		delete static_cast<QVector<QString>*>(m_data);
		break;

	case AbstractColumn::DateTime:
	case AbstractColumn::Month:
	case AbstractColumn::Day:
		delete static_cast<QVector<QDateTime>*>(m_data);
		break;
	}
}

AbstractColumn::ColumnMode ColumnPrivate::columnMode() const {
	return m_column_mode;
}

/**
 * \brief Set the column mode
 *
 * This sets the column mode and, if
 * necessary, converts it to another datatype.
 * Remark: setting the mode back to undefined (the
 * initial value) is not supported.
 */
void ColumnPrivate::setColumnMode(AbstractColumn::ColumnMode mode) {
	if (mode == m_column_mode) return;

	void * old_data = m_data;
	// remark: the deletion of the old data will be done in the dtor of a command

	AbstractSimpleFilter* filter = 0;
	AbstractSimpleFilter* new_in_filter = 0;
	AbstractSimpleFilter* new_out_filter = 0;
	bool filter_is_temporary = false; // it can also become outputFilter(), which we may not delete here
	Column* temp_col = 0;

	emit m_owner->modeAboutToChange(m_owner);

	// determine the conversion filter and allocate the new data vector
	switch(m_column_mode) {
	case AbstractColumn::Numeric:
		disconnect(static_cast<Double2StringFilter *>(m_output_filter), SIGNAL(formatChanged()),
		           m_owner, SLOT(handleFormatChange()));
		switch(mode) {
		case AbstractColumn::Numeric:
			break;
		case AbstractColumn::Text:
			filter = outputFilter();
			filter_is_temporary = false;
			temp_col = new Column("temp_col", *(static_cast< QVector<double>* >(old_data)));
			m_data = new QStringList();
			break;
		case AbstractColumn::DateTime:
			filter = new Double2DateTimeFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast< QVector<double>* >(old_data)));
			m_data = new QVector<QDateTime>();
			break;
		case AbstractColumn::Month:
			filter = new Double2MonthFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast< QVector<double>* >(old_data)));
			m_data = new QVector<QDateTime>();
			break;
		case AbstractColumn::Day:
			filter = new Double2DayOfWeekFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast< QVector<double>* >(old_data)));
			m_data = new QVector<QDateTime>();
			break;
		} // switch(mode)
		break;

	case AbstractColumn::Text:
		switch(mode) {
		case AbstractColumn::Text:
			break;
		case AbstractColumn::Numeric:
			filter = new String2DoubleFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast<QVector<QString>*>(old_data)));
			m_data = new QVector<double>();
			break;
		case AbstractColumn::DateTime:
			filter = new String2DateTimeFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast<QVector<QString>*>(old_data)));
			m_data = new QVector<QDateTime>();
			break;
		case AbstractColumn::Month:
			filter = new String2MonthFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast<QVector<QString>*>(old_data)));
			m_data = new QVector<QDateTime>();
			break;
		case AbstractColumn::Day:
			filter = new String2DayOfWeekFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast<QVector<QString>*>(old_data)));
			m_data = new QVector<QDateTime>();
			break;
		} // switch(mode)
		break;

	case AbstractColumn::DateTime:
	case AbstractColumn::Month:
	case AbstractColumn::Day:
		disconnect(static_cast<DateTime2StringFilter *>(m_output_filter), SIGNAL(formatChanged()),
		           m_owner, SLOT(handleFormatChange()));
		switch(mode) {
		case AbstractColumn::DateTime:
			break;
		case AbstractColumn::Text:
			filter = outputFilter();
			filter_is_temporary = false;
			temp_col = new Column("temp_col", *(static_cast< QVector<QDateTime>* >(old_data)));
			m_data = new QStringList();
			break;
		case AbstractColumn::Numeric:
			if (m_column_mode == AbstractColumn::Month)
				filter = new Month2DoubleFilter();
			else if (m_column_mode == AbstractColumn::Day)
				filter = new DayOfWeek2DoubleFilter();
			else
				filter = new DateTime2DoubleFilter();
			filter_is_temporary = true;
			temp_col = new Column("temp_col", *(static_cast< QVector<QDateTime>* >(old_data)));
			m_data = new QVector<double>();
			break;
		case AbstractColumn::Month:
		case AbstractColumn::Day:
			break;
		} // switch(mode)
		break;

	}

	// determine the new input and output filters
	switch(mode) {
	case AbstractColumn::Numeric:
		new_in_filter = new String2DoubleFilter();
		new_out_filter = new Double2StringFilter();
		connect(static_cast<Double2StringFilter *>(new_out_filter), SIGNAL(formatChanged()),
		        m_owner, SLOT(handleFormatChange()));
		break;
	case AbstractColumn::Text:
		new_in_filter = new SimpleCopyThroughFilter();
		new_out_filter = new SimpleCopyThroughFilter();
		break;
	case AbstractColumn::DateTime:
		new_in_filter = new String2DateTimeFilter();
		new_out_filter = new DateTime2StringFilter();
		connect(static_cast<DateTime2StringFilter *>(new_out_filter), SIGNAL(formatChanged()),
		        m_owner, SLOT(handleFormatChange()));
		break;
	case AbstractColumn::Month:
		new_in_filter = new String2MonthFilter();
		new_out_filter = new DateTime2StringFilter();
		static_cast<DateTime2StringFilter *>(new_out_filter)->setFormat("MMMM");
		connect(static_cast<DateTime2StringFilter *>(new_out_filter), SIGNAL(formatChanged()),
		        m_owner, SLOT(handleFormatChange()));
		break;
	case AbstractColumn::Day:
		new_in_filter = new String2DayOfWeekFilter();
		new_out_filter = new DateTime2StringFilter();
		static_cast<DateTime2StringFilter *>(new_out_filter)->setFormat("dddd");
		connect(static_cast<DateTime2StringFilter *>(new_out_filter), SIGNAL(formatChanged()),
		        m_owner, SLOT(handleFormatChange()));
		break;
	} // switch(mode)

	m_column_mode = mode;

	new_in_filter->setName("InputFilter");
	new_out_filter->setName("OutputFilter");
	m_input_filter = new_in_filter;
	m_output_filter = new_out_filter;
	m_input_filter->input(0, m_owner->m_string_io);
	m_output_filter->input(0, m_owner);
	m_input_filter->setHidden(true);
	m_output_filter->setHidden(true);

	if (temp_col) { // if temp_col == 0, only the input/output filters need to be changed
		// copy the filtered, i.e. converted, column
		filter->input(0, temp_col);
		copy(filter->output(0));
		delete temp_col;
	}

	if (filter_is_temporary) delete filter;

	emit m_owner->modeChanged(m_owner);
}

/**
 * \brief Replace all mode related members
 *
 * Replace column mode, data type, data pointer and filters directly
 */
void ColumnPrivate::replaceModeData(AbstractColumn::ColumnMode mode, void * data,
                                    AbstractSimpleFilter * in_filter, AbstractSimpleFilter * out_filter) {
	emit m_owner->modeAboutToChange(m_owner);
	// disconnect formatChanged()
	switch(m_column_mode) {
	case AbstractColumn::Numeric:
		disconnect(static_cast<Double2StringFilter *>(m_output_filter), SIGNAL(formatChanged()),
		           m_owner, SLOT(handleFormatChange()));
		break;
	case AbstractColumn::Text:
		break;
	case AbstractColumn::DateTime:
	case AbstractColumn::Month:
	case AbstractColumn::Day:
		disconnect(static_cast<DateTime2StringFilter *>(m_output_filter), SIGNAL(formatChanged()),
		           m_owner, SLOT(handleFormatChange()));
		break;
	}

	m_column_mode = mode;
	m_data = data;

	in_filter->setName("InputFilter");
	out_filter->setName("OutputFilter");
	m_input_filter = in_filter;
	m_output_filter = out_filter;
	m_input_filter->input(0, m_owner->m_string_io);
	m_output_filter->input(0, m_owner);

	// connect formatChanged()
	switch(m_column_mode) {
	case AbstractColumn::Numeric:
		connect(static_cast<Double2StringFilter *>(m_output_filter), SIGNAL(formatChanged()),
		        m_owner, SLOT(handleFormatChange()));
		break;
	case AbstractColumn::Text:
		break;
	case AbstractColumn::DateTime:
	case AbstractColumn::Month:
	case AbstractColumn::Day:
		connect(static_cast<DateTime2StringFilter *>(m_output_filter), SIGNAL(formatChanged()),
		        m_owner, SLOT(handleFormatChange()));
		break;
	}

	emit m_owner->modeChanged(m_owner);
}

/**
 * \brief Replace data pointer
 */
void ColumnPrivate::replaceData(void * data) {
	emit m_owner->dataAboutToChange(m_owner);
	m_data = data;
	if (!m_owner->m_suppressDataChangedSignal)
		emit m_owner->dataChanged(m_owner);
}

/**
 * \brief Copy another column of the same type
 *
 * This function will return false if the data type
 * of 'other' is not the same as the type of 'this'.
 * Use a filter to convert a column to another type.
 */
bool ColumnPrivate::copy(const AbstractColumn * other) {
	if (other->columnMode() != columnMode()) return false;
	int num_rows = other->rowCount();

	emit m_owner->dataAboutToChange(m_owner);
	resizeTo(num_rows);

	// copy the data
	switch(m_column_mode) {
	case AbstractColumn::Numeric: {
			double * ptr = static_cast<QVector<double>*>(m_data)->data();
			for(int i=0; i<num_rows; i++)
				ptr[i] = other->valueAt(i);
			break;
		}
	case AbstractColumn::Text: {
			for(int i=0; i<num_rows; i++)
				static_cast<QVector<QString>*>(m_data)->replace(i, other->textAt(i));
			break;
		}
	case AbstractColumn::DateTime:
	case AbstractColumn::Month:
	case AbstractColumn::Day: {
			for(int i=0; i<num_rows; i++)
				static_cast<QVector<QDateTime>*>(m_data)->replace(i, other->dateTimeAt(i));
			break;
		}
	}

	if (!m_owner->m_suppressDataChangedSignal)
		emit m_owner->dataChanged(m_owner);

	return true;
}

/**
 * \brief Copies a part of another column of the same type
 *
 * This function will return false if the data type
 * of 'other' is not the same as the type of 'this'.
 * \param other pointer to the column to copy
 * \param src_start first row to copy in the column to copy
 * \param dest_start first row to copy in
 * \param num_rows the number of rows to copy
 */
bool ColumnPrivate::copy(const AbstractColumn * source, int source_start, int dest_start, int num_rows) {
	if (source->columnMode() != m_column_mode) return false;
	if (num_rows == 0) return true;

	emit m_owner->dataAboutToChange(m_owner);
	if (dest_start + num_rows > rowCount())
		resizeTo(dest_start + num_rows);

	// copy the data
	switch(m_column_mode) {
	case AbstractColumn::Numeric: {
			double * ptr = static_cast<QVector<double>*>(m_data)->data();
			for(int i=0; i<num_rows; i++)
				ptr[dest_start+i] = source->valueAt(source_start + i);
			break;
		}
	case AbstractColumn::Text:
		for(int i=0; i<num_rows; i++)
			static_cast<QVector<QString>*>(m_data)->replace(dest_start+i, source->textAt(source_start + i));
		break;
	case AbstractColumn::DateTime:
	case AbstractColumn::Month:
	case AbstractColumn::Day:
		for(int i=0; i<num_rows; i++)
			static_cast<QVector<QDateTime>*>(m_data)->replace(dest_start+i, source->dateTimeAt(source_start + i));
		break;
	}

	if (!m_owner->m_suppressDataChangedSignal)
		emit m_owner->dataChanged(m_owner);

	return true;
}

/**
 * \brief Copy another column of the same type
 *
 * This function will return false if the data type
 * of 'other' is not the same as the type of 'this'.
 * Use a filter to convert a column to another type.
 */
bool ColumnPrivate::copy(const ColumnPrivate * other) {
	if (other->columnMode() != m_column_mode) return false;
	int num_rows = other->rowCount();

	emit m_owner->dataAboutToChange(m_owner);
	resizeTo(num_rows);

	// copy the data
	switch(m_column_mode) {
	case AbstractColumn::Numeric: {
			double * ptr = static_cast<QVector<double>*>(m_data)->data();
			for(int i=0; i<num_rows; i++)
				ptr[i] = other->valueAt(i);
			break;
		}
	case AbstractColumn::Text: {
			for(int i=0; i<num_rows; i++)
				static_cast<QVector<QString>*>(m_data)->replace(i, other->textAt(i));
			break;
		}
	case AbstractColumn::DateTime:
	case AbstractColumn::Month:
	case AbstractColumn::Day: {
			for(int i=0; i<num_rows; i++)
				static_cast<QVector<QDateTime>*>(m_data)->replace(i, other->dateTimeAt(i));
			break;
		}
	}

	if (!m_owner->m_suppressDataChangedSignal)
		emit m_owner->dataChanged(m_owner);

	return true;
}

/**
 * \brief Copies a part of another column of the same type
 *
 * This function will return false if the data type
 * of 'other' is not the same as the type of 'this'.
 * \param other pointer to the column to copy
 * \param src_start first row to copy in the column to copy
 * \param dest_start first row to copy in
 * \param num_rows the number of rows to copy
 */
bool ColumnPrivate::copy(const ColumnPrivate * source, int source_start, int dest_start, int num_rows) {
	if (source->columnMode() != m_column_mode) return false;
	if (num_rows == 0) return true;

	emit m_owner->dataAboutToChange(m_owner);
	if (dest_start + num_rows > rowCount())
		resizeTo(dest_start + num_rows);

	// copy the data
	switch(m_column_mode) {
	case AbstractColumn::Numeric: {
			double * ptr = static_cast<QVector<double>*>(m_data)->data();
			for(int i=0; i<num_rows; i++)
				ptr[dest_start+i] = source->valueAt(source_start + i);
			break;
		}
	case AbstractColumn::Text:
		for(int i=0; i<num_rows; i++)
			static_cast<QVector<QString>*>(m_data)->replace(dest_start+i, source->textAt(source_start + i));
		break;
	case AbstractColumn::DateTime:
	case AbstractColumn::Month:
	case AbstractColumn::Day:
		for(int i=0; i<num_rows; i++)
			static_cast<QVector<QDateTime>*>(m_data)->replace(dest_start+i, source->dateTimeAt(source_start + i));
		break;
	}

	if (!m_owner->m_suppressDataChangedSignal)
		emit m_owner->dataChanged(m_owner);

	return true;
}

/**
 * \brief Return the data vector size
 *
 * This returns the number of rows that actually contain data.
 * Rows beyond this can be masked etc. but should be ignored by filters,
 * plots etc.
 */
int ColumnPrivate::rowCount() const {
	switch(m_column_mode) {
	case AbstractColumn::Numeric:
		return static_cast<QVector<double>*>(m_data)->size();
	case AbstractColumn::DateTime:
	case AbstractColumn::Month:
	case AbstractColumn::Day:
		return static_cast<QVector<QDateTime>*>(m_data)->size();
	case AbstractColumn::Text:
		return static_cast<QVector<QString>*>(m_data)->size();
	}

	return 0;
}

/**
 * \brief Resize the vector to the specified number of rows
 *
 * Since selecting and masking rows higher than the
 * real internal number of rows is supported, this
 * does not change the interval attributes. Also
 * no signal is emitted. If the new rows are filled
 * with values AbstractColumn::dataChanged()
 * must be emitted.
 */
void ColumnPrivate::resizeTo(int new_size) {
	int old_size = rowCount();
	if (new_size == old_size) return;

	switch(m_column_mode) {
	case AbstractColumn::Numeric: {
			QVector<double>* numeric_data = static_cast<QVector<double>*>(m_data);
			numeric_data->insert(numeric_data->end(), new_size-old_size, NAN);
			break;
		}
	case AbstractColumn::DateTime:
	case AbstractColumn::Month:
	case AbstractColumn::Day: {
			int new_rows = new_size - old_size;
			if (new_rows > 0) {
				for (int i = 0; i < new_rows; i++)
					static_cast<QVector<QDateTime>*>(m_data)->append(QDateTime());
			} else {
				for(int i = 0; i < -new_rows; i++)
					static_cast<QVector<QDateTime>*>(m_data)->removeLast();
			}
			break;
		}
	case AbstractColumn::Text: {
			int new_rows = new_size - old_size;
			if (new_rows > 0) {
				for(int i = 0; i < new_rows; i++)
					static_cast<QVector<QString>*>(m_data)->append(QString());
			} else {
				for(int i = 0; i < -new_rows; i++)
					static_cast<QVector<QString>*>(m_data)->removeLast();
			}
			break;
		}
	}
}

/**
 * \brief Insert some empty (or initialized with zero) rows
 */
void ColumnPrivate::insertRows(int before, int count) {
	if (count == 0) return;

	m_formulas.insertRows(before, count);

	if (before <= rowCount()) {
		switch(m_column_mode) {
		case AbstractColumn::Numeric:
			static_cast<QVector<double>*>(m_data)->insert(before, count, NAN);
			break;
		case AbstractColumn::DateTime:
		case AbstractColumn::Month:
		case AbstractColumn::Day:
			for (int i = 0; i < count; i++)
				static_cast<QVector<QDateTime>*>(m_data)->insert(before, QDateTime());
			break;
		case AbstractColumn::Text:
			for (int i = 0; i < count; i++)
				static_cast<QVector<QString>*>(m_data)->insert(before, QString());
			break;
		}
	}
}

/**
 * \brief Remove 'count' rows starting from row 'first'
 */
void ColumnPrivate::removeRows(int first, int count) {
	if (count == 0) return;

	m_formulas.removeRows(first, count);

	if (first < rowCount()) {
		int corrected_count = count;
		if (first + count > rowCount())
			corrected_count = rowCount() - first;

		switch(m_column_mode) {
		case AbstractColumn::Numeric:
			static_cast<QVector<double>*>(m_data)->remove(first, corrected_count);
			break;
		case AbstractColumn::DateTime:
		case AbstractColumn::Month:
		case AbstractColumn::Day:
			for (int i = 0; i < corrected_count; i++)
				static_cast<QVector<QDateTime>*>(m_data)->removeAt(first);
			break;
		case AbstractColumn::Text:
			for (int i = 0; i < corrected_count; i++)
				static_cast<QVector<QString>*>(m_data)->removeAt(first);
			break;
		}
	}
}

//! Return the column name
QString ColumnPrivate::name() const {
	return m_owner->name();
}

/**
 * \brief Return the column plot designation
 */
AbstractColumn::PlotDesignation ColumnPrivate::plotDesignation() const {
	return m_plot_designation;
}

/**
 * \brief Set the column plot designation
 */
void ColumnPrivate::setPlotDesignation(AbstractColumn::PlotDesignation pd) {
	emit m_owner->plotDesignationAboutToChange(m_owner);
	m_plot_designation = pd;
	emit m_owner->plotDesignationChanged(m_owner);
}

/**
 * \brief Get width
 */
int ColumnPrivate::width() const {
	return m_width;
}

/**
 * \brief Set width
 */
void ColumnPrivate::setWidth(int value) {
	m_width = value;
}

/**
 * \brief Return the data pointer
 */
void* ColumnPrivate::data() const {
	return m_data;
}

/**
 * \brief Return the input filter (for string -> data type conversion)
 */
AbstractSimpleFilter *ColumnPrivate::inputFilter() const {
	return m_input_filter;
}

/**
 * \brief Return the output filter (for data type -> string  conversion)
 */
AbstractSimpleFilter *ColumnPrivate::outputFilter() const {
	return m_output_filter;
}

////////////////////////////////////////////////////////////////////////////////
//! \name Formula related functions
//@{
////////////////////////////////////////////////////////////////////////////////
/**
 * \brief Return the formula last used to generate data for the column
 */
QString ColumnPrivate::formula() const {
	return m_formula;
}

/**
 * \brief Sets the formula used to generate column values
 */
void ColumnPrivate::setFormula(const QString& formula, const QStringList& variableNames, const QStringList& variableColumnPathes) {
	m_formula = formula;
	m_formulaVariableNames = variableNames;
	m_formulaVariableColumnPathes = variableColumnPathes;
}

/**
 * \brief Return the formula associated with row 'row'
 */
QString ColumnPrivate::formula(int row) const {
	return m_formulas.value(row);
}

const QStringList& ColumnPrivate::formulaVariableNames() const {
	return m_formulaVariableNames;
}

const QStringList& ColumnPrivate::formulaVariableColumnPathes() const {
	return m_formulaVariableColumnPathes;
}

/**
 * \brief Return the intervals that have associated formulas
 *
 * This can be used to make a list of formulas with their intervals.
 * Here is some example code:
 *
 * \code
 * QStringList list;
 * QList< Interval<int> > intervals = my_column.formulaIntervals();
 * foreach(Interval<int> interval, intervals)
 * 	list << QString(interval.toString() + ": " + my_column.formula(interval.start()));
 * \endcode
 */
QList< Interval<int> > ColumnPrivate::formulaIntervals() const {
	return m_formulas.intervals();
}

/**
 * \brief Set a formula string for an interval of rows
 */
void ColumnPrivate::setFormula(Interval<int> i, QString formula) {
	m_formulas.setValue(i, formula);
}

/**
 * \brief Overloaded function for convenience
 */
void ColumnPrivate::setFormula(int row, QString formula) {
	setFormula(Interval<int>(row,row), formula);
}

/**
 * \brief Clear all formulas
 */
void ColumnPrivate::clearFormulas() {
	m_formulas.clear();
}

////////////////////////////////////////////////////////////////////////////////
//@}
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
//! \name type specific functions
//@{
////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Return the content of row 'row'.
 *
 * Use this only when columnMode() is Text
 */
QString ColumnPrivate::textAt(int row) const {
	if (m_column_mode != AbstractColumn::Text) return QString();
	return static_cast<QVector<QString>*>(m_data)->value(row);
}

/**
 * \brief Return the date part of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QDate ColumnPrivate::dateAt(int row) const {
	return dateTimeAt(row).date();
}

/**
 * \brief Return the time part of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QTime ColumnPrivate::timeAt(int row) const {
	return dateTimeAt(row).time();
}

/**
 * \brief Return the QDateTime in row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
QDateTime ColumnPrivate::dateTimeAt(int row) const {
	if (m_column_mode != AbstractColumn::DateTime &&
	        m_column_mode != AbstractColumn::Month &&
	        m_column_mode != AbstractColumn::Day)
		return QDateTime();
	return static_cast<QVector<QDateTime>*>(m_data)->value(row);
}

/**
 * \brief Return the double value in row 'row'
 */
double ColumnPrivate::valueAt(int row) const {
	if (m_column_mode != AbstractColumn::Numeric) return NAN;
	return static_cast<QVector<double>*>(m_data)->value(row, NAN);
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is Text
 */
void ColumnPrivate::setTextAt(int row, const QString& new_value) {
	if (m_column_mode != AbstractColumn::Text) return;

	emit m_owner->dataAboutToChange(m_owner);
	if (row >= rowCount())
		resizeTo(row+1);

	static_cast<QVector<QString>*>(m_data)->replace(row, new_value);
	if (!m_owner->m_suppressDataChangedSignal)
		emit m_owner->dataChanged(m_owner);
}

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is Text
 */
void ColumnPrivate::replaceTexts(int first, const QVector<QString>& new_values) {
	if (m_column_mode != AbstractColumn::Text) return;

	emit m_owner->dataAboutToChange(m_owner);
	int num_rows = new_values.size();
	if (first + num_rows > rowCount())
		resizeTo(first + num_rows);

	for (int i = 0; i < num_rows; i++)
		static_cast<QVector<QString>*>(m_data)->replace(first+i, new_values.at(i));

	if (!m_owner->m_suppressDataChangedSignal)
		emit m_owner->dataChanged(m_owner);
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void ColumnPrivate::setDateAt(int row, const QDate& new_value) {
	if (m_column_mode != AbstractColumn::DateTime &&
	        m_column_mode != AbstractColumn::Month &&
	        m_column_mode != AbstractColumn::Day)
		return;

	setDateTimeAt(row, QDateTime(new_value, timeAt(row)));
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void ColumnPrivate::setTimeAt(int row, const QTime& new_value) {
	if (m_column_mode != AbstractColumn::DateTime &&
	        m_column_mode != AbstractColumn::Month &&
	        m_column_mode != AbstractColumn::Day)
		return;

	setDateTimeAt(row, QDateTime(dateAt(row), new_value));
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void ColumnPrivate::setDateTimeAt(int row, const QDateTime& new_value) {
	if (m_column_mode != AbstractColumn::DateTime &&
	        m_column_mode != AbstractColumn::Month &&
	        m_column_mode != AbstractColumn::Day)
		return;

	emit m_owner->dataAboutToChange(m_owner);
	if (row >= rowCount())
		resizeTo(row+1);

	static_cast< QVector<QDateTime>* >(m_data)->replace(row, new_value);
	if (!m_owner->m_suppressDataChangedSignal)
		emit m_owner->dataChanged(m_owner);
}

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is DateTime, Month or Day
 */
void ColumnPrivate::replaceDateTimes(int first, const QVector<QDateTime>& new_values) {
	if (m_column_mode != AbstractColumn::DateTime &&
	        m_column_mode != AbstractColumn::Month &&
	        m_column_mode != AbstractColumn::Day)
		return;

	emit m_owner->dataAboutToChange(m_owner);
	int num_rows = new_values.size();
	if (first + num_rows > rowCount())
		resizeTo(first + num_rows);

	for (int i = 0; i < num_rows; i++)
		static_cast<QVector<QDateTime>*>(m_data)->replace(first+i, new_values.at(i));

	if (!m_owner->m_suppressDataChangedSignal)
		emit m_owner->dataChanged(m_owner);
}

/**
 * \brief Set the content of row 'row'
 *
 * Use this only when columnMode() is Numeric
 */
void ColumnPrivate::setValueAt(int row, double new_value) {
	if (m_column_mode != AbstractColumn::Numeric) return;

	emit m_owner->dataAboutToChange(m_owner);
	if (row >= rowCount())
		resizeTo(row+1);

	static_cast<QVector<double>*>(m_data)->replace(row, new_value);
	if (!m_owner->m_suppressDataChangedSignal)
		emit m_owner->dataChanged(m_owner);
}

/**
 * \brief Replace a range of values
 *
 * Use this only when columnMode() is Numeric
 */
void ColumnPrivate::replaceValues(int first, const QVector<double>& new_values) {
	if (m_column_mode != AbstractColumn::Numeric) return;

	emit m_owner->dataAboutToChange(m_owner);
	int num_rows = new_values.size();
	if (first + num_rows > rowCount())
		resizeTo(first + num_rows);

	double* ptr = static_cast<QVector<double>*>(m_data)->data();
	for(int i=0; i<num_rows; i++)
		ptr[first+i] = new_values.at(i);

	if (!m_owner->m_suppressDataChangedSignal)
		emit m_owner->dataChanged(m_owner);
}

////////////////////////////////////////////////////////////////////////////////
//@}
////////////////////////////////////////////////////////////////////////////////

/**
 * \brief Return the interval attribute representing the formula strings
 */
IntervalAttribute<QString> ColumnPrivate::formulaAttribute() const {
	return m_formulas;
}

/**
 * \brief Replace the interval attribute for the formula strings
 */
void ColumnPrivate::replaceFormulas(IntervalAttribute<QString> formulas) {
	m_formulas = formulas;
}
