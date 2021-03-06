/***************************************************************************
    File             : XYFitCurveDock.cpp
    Project          : LabPlot
    --------------------------------------------------------------------
    Copyright        : (C) 2014-2017 Alexander Semke (alexander.semke@web.de)
    Copyright        : (C) 2016-2017 Stefan Gerlach (stefan.gerlach@uni.kn)
    Description      : widget for editing properties of fit curves

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

#include "XYFitCurveDock.h"
#include "backend/core/AspectTreeModel.h"
#include "backend/core/Project.h"
#include "backend/lib/macros.h"
#include "backend/gsl/ExpressionParser.h"
#include "commonfrontend/widgets/TreeViewComboBox.h"
#include "kdefrontend/widgets/ConstantsWidget.h"
#include "kdefrontend/widgets/FunctionsWidget.h"
#include "kdefrontend/widgets/FitOptionsWidget.h"
#include "kdefrontend/widgets/FitParametersWidget.h"

#include <QMenu>
#include <QWidgetAction>
#include <QStandardItemModel>

#include <cfloat>	// DBL_MAX
#include <cmath>	// fabs()

extern "C" {
#include "backend/nsl/nsl_sf_stats.h"
#include "backend/nsl/nsl_stats.h"
}

/*!
  \class XYFitCurveDock
  \brief  Provides a widget for editing the properties of the XYFitCurves
		(2D-curves defined by a fit model) currently selected in
		the project explorer.

  If more then one curves are set, the properties of the first column are shown.
  The changes of the properties are applied to all curves.
  The exclusions are the name, the comment and the datasets (columns) of
  the curves  - these properties can only be changed if there is only one single curve.

  \ingroup kdefrontend
*/

XYFitCurveDock::XYFitCurveDock(QWidget* parent) : XYCurveDock(parent),
	cbDataSourceCurve(0), cbXDataColumn(0), cbYDataColumn(0), cbXErrorColumn(0),
	cbYErrorColumn(0), m_fitCurve(0) {

	//remove the tab "Error bars"
	ui.tabWidget->removeTab(5);
}

/*!
 * 	// Tab "General"
 */
void XYFitCurveDock::setupGeneral() {
	QWidget* generalTab = new QWidget(ui.tabGeneral);
	uiGeneralTab.setupUi(generalTab);
	QGridLayout* gridLayout = qobject_cast<QGridLayout*>(generalTab->layout());
	if (gridLayout) {
		gridLayout->setContentsMargins(2, 2, 2, 2);
		gridLayout->setHorizontalSpacing(2);
		gridLayout->setVerticalSpacing(2);
	}

	uiGeneralTab.cbDataSourceType->addItem(i18n("Spreadsheet"));
	uiGeneralTab.cbDataSourceType->addItem(i18n("XY-Curve"));

	cbDataSourceCurve = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbDataSourceCurve, 6, 4, 1, 4);

	cbXDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbXDataColumn, 7, 4, 1, 1);

	cbXErrorColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbXErrorColumn, 7, 5, 1, 4);

	cbYDataColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbYDataColumn, 8, 4, 1, 1);

	cbYErrorColumn = new TreeViewComboBox(generalTab);
	gridLayout->addWidget(cbYErrorColumn, 8, 5, 1, 4);

	//Weight
	for(int i = 0; i < NSL_FIT_WEIGHT_TYPE_COUNT; i++)
		uiGeneralTab.cbWeight->addItem(nsl_fit_weight_type_name[i]);
	uiGeneralTab.cbWeight->setCurrentIndex(nsl_fit_weight_instrumental);
	uiGeneralTab.lWeight->setEnabled(false);
	uiGeneralTab.cbWeight->setEnabled(false);

	for(int i = 0; i < NSL_FIT_MODEL_CATEGORY_COUNT; i++)
		uiGeneralTab.cbCategory->addItem(nsl_fit_model_category_name[i]);

	//show the fit-model category for the currently selected default (first) fit-model category
	categoryChanged(uiGeneralTab.cbCategory->currentIndex());

	uiGeneralTab.teEquation->setMaximumHeight(uiGeneralTab.leName->sizeHint().height() * 2);

	//use white background in the preview label
	QPalette p;
	p.setColor(QPalette::Window, Qt::white);
	uiGeneralTab.lFuncPic->setAutoFillBackground(true);
	uiGeneralTab.lFuncPic->setPalette(p);

	uiGeneralTab.tbConstants->setIcon(QIcon::fromTheme("labplot-format-text-symbol"));
	uiGeneralTab.tbFunctions->setIcon(QIcon::fromTheme("preferences-desktop-font"));
	uiGeneralTab.pbRecalculate->setIcon(QIcon::fromTheme("run-build"));

	uiGeneralTab.twGeneral->setEditTriggers(QAbstractItemView::NoEditTriggers);
	uiGeneralTab.twParameters->setEditTriggers(QAbstractItemView::NoEditTriggers);
	uiGeneralTab.twGoodness->setEditTriggers(QAbstractItemView::NoEditTriggers);

	uiGeneralTab.twGeneral->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
	uiGeneralTab.twGoodness->horizontalHeader()->resizeSections(QHeaderView::ResizeToContents);
	uiGeneralTab.twGoodness->item(0, 1)->setText(QString::fromUtf8("\u03c7") + QString::fromUtf8("\u00b2"));
	uiGeneralTab.twGoodness->item(1, 1)->setText(i18n("reduced") + " " + QString::fromUtf8("\u03c7") + QString::fromUtf8("\u00b2")
		+ " (" + QString::fromUtf8("\u03c7") + QString::fromUtf8("\u00b2") + "/dof)");
	uiGeneralTab.twGoodness->item(3, 1)->setText("R" + QString::fromUtf8("\u00b2"));
	uiGeneralTab.twGoodness->item(4, 1)->setText("R" + QString::fromUtf8("\u0304") + QString::fromUtf8("\u00b2"));
	uiGeneralTab.twGoodness->item(5, 0)->setText(QString::fromUtf8("\u03c7") + QString::fromUtf8("\u00b2") + ' ' + i18n("test"));
	uiGeneralTab.twGoodness->item(5, 1)->setText("P > " + QString::fromUtf8("\u03c7") + QString::fromUtf8("\u00b2"));

	QHBoxLayout* layout = new QHBoxLayout(ui.tabGeneral);
	layout->setMargin(0);
	layout->addWidget(generalTab);

	//Slots
	connect(uiGeneralTab.leName, SIGNAL(returnPressed()), this, SLOT(nameChanged()));
	connect(uiGeneralTab.leComment, SIGNAL(returnPressed()), this, SLOT(commentChanged()));
	connect(uiGeneralTab.chkVisible, SIGNAL(clicked(bool)), this, SLOT(visibilityChanged(bool)));
	connect(uiGeneralTab.cbDataSourceType, SIGNAL(currentIndexChanged(int)), this, SLOT(dataSourceTypeChanged(int)));
	connect(uiGeneralTab.cbAutoRange, SIGNAL(clicked(bool)), this, SLOT(autoRangeChanged()));
	connect(uiGeneralTab.sbMin, SIGNAL(valueChanged(double)), this, SLOT(xRangeMinChanged()));
	connect(uiGeneralTab.sbMax, SIGNAL(valueChanged(double)), this, SLOT(xRangeMaxChanged()));

	connect(uiGeneralTab.cbWeight, SIGNAL(currentIndexChanged(int)), this, SLOT(weightChanged(int)));
	connect(uiGeneralTab.cbCategory, SIGNAL(currentIndexChanged(int)), this, SLOT(categoryChanged(int)));
	connect(uiGeneralTab.cbModel, SIGNAL(currentIndexChanged(int)), this, SLOT(modelChanged(int)));
	connect(uiGeneralTab.sbDegree, SIGNAL(valueChanged(int)), this, SLOT(updateModelEquation()));
	connect(uiGeneralTab.teEquation, SIGNAL(expressionChanged()), this, SLOT(enableRecalculate()));
	connect(uiGeneralTab.tbConstants, SIGNAL(clicked()), this, SLOT(showConstants()));
	connect(uiGeneralTab.tbFunctions, SIGNAL(clicked()), this, SLOT(showFunctions()));
	connect(uiGeneralTab.pbParameters, SIGNAL(clicked()), this, SLOT(showParameters()));
	connect(uiGeneralTab.pbOptions, SIGNAL(clicked()), this, SLOT(showOptions()));
	connect(uiGeneralTab.pbRecalculate, SIGNAL(clicked()), this, SLOT(recalculateClicked()));

	connect(cbDataSourceCurve, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(dataSourceCurveChanged(QModelIndex)));
	connect(cbXDataColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(xDataColumnChanged(QModelIndex)));
	connect(cbYDataColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(yDataColumnChanged(QModelIndex)));
	connect(cbXErrorColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(xErrorColumnChanged(QModelIndex)));
	connect(cbYErrorColumn, SIGNAL(currentModelIndexChanged(QModelIndex)), this, SLOT(yErrorColumnChanged(QModelIndex)));
}

void XYFitCurveDock::initGeneralTab() {
	//if there are more then one curve in the list, disable the tab "general"
	if (m_curvesList.size() == 1) {
		uiGeneralTab.lName->setEnabled(true);
		uiGeneralTab.leName->setEnabled(true);
		uiGeneralTab.lComment->setEnabled(true);
		uiGeneralTab.leComment->setEnabled(true);

		uiGeneralTab.leName->setText(m_curve->name());
		uiGeneralTab.leComment->setText(m_curve->comment());
	} else {
		uiGeneralTab.lName->setEnabled(false);
		uiGeneralTab.leName->setEnabled(false);
		uiGeneralTab.lComment->setEnabled(false);
		uiGeneralTab.leComment->setEnabled(false);

		uiGeneralTab.leName->setText("");
		uiGeneralTab.leComment->setText("");
	}

	//show the properties of the first curve
	m_fitCurve = dynamic_cast<XYFitCurve*>(m_curve);
	Q_ASSERT(m_fitCurve);

	uiGeneralTab.cbDataSourceType->setCurrentIndex(m_fitCurve->dataSourceType());
	this->dataSourceTypeChanged(uiGeneralTab.cbDataSourceType->currentIndex());
	XYCurveDock::setModelIndexFromAspect(cbDataSourceCurve, m_fitCurve->dataSourceCurve());
	XYCurveDock::setModelIndexFromAspect(cbXDataColumn, m_fitCurve->xDataColumn());
	XYCurveDock::setModelIndexFromAspect(cbYDataColumn, m_fitCurve->yDataColumn());
	XYCurveDock::setModelIndexFromAspect(cbXErrorColumn, m_fitCurve->xErrorColumn());
	XYCurveDock::setModelIndexFromAspect(cbYErrorColumn, m_fitCurve->yErrorColumn());
	uiGeneralTab.cbAutoRange->setChecked(m_fitData.autoRange);
	uiGeneralTab.sbMin->setValue(m_fitData.xRange.first());
	uiGeneralTab.sbMax->setValue(m_fitData.xRange.last());
	this->autoRangeChanged();

	unsigned int tmpModelType = m_fitData.modelType;	// save type because it's reset when category changes
	if (m_fitData.modelCategory == nsl_fit_model_custom)
		uiGeneralTab.cbCategory->setCurrentIndex(uiGeneralTab.cbCategory->count() - 1);
	else
		uiGeneralTab.cbCategory->setCurrentIndex(m_fitData.modelCategory);
	m_fitData.modelType = tmpModelType;
	if (m_fitData.modelCategory != nsl_fit_model_custom)
		uiGeneralTab.cbModel->setCurrentIndex(m_fitData.modelType);

	uiGeneralTab.cbWeight->setCurrentIndex(m_fitData.weightsType);
	uiGeneralTab.sbDegree->setValue(m_fitData.degree);
	updateModelEquation();
	this->showFitResult();

	uiGeneralTab.chkVisible->setChecked(m_curve->isVisible());

	//Slots
	connect(m_fitCurve, SIGNAL(aspectDescriptionChanged(const AbstractAspect*)), this, SLOT(curveDescriptionChanged(const AbstractAspect*)));
	connect(m_fitCurve, SIGNAL(dataSourceTypeChanged(XYCurve::DataSourceType)), this, SLOT(curveDataSourceTypeChanged(XYCurve::DataSourceType)));
	connect(m_fitCurve, SIGNAL(dataSourceCurveChanged(const XYCurve*)), this, SLOT(curveDataSourceCurveChanged(const XYCurve*)));
	connect(m_fitCurve, SIGNAL(xDataColumnChanged(const AbstractColumn*)), this, SLOT(curveXDataColumnChanged(const AbstractColumn*)));
	connect(m_fitCurve, SIGNAL(yDataColumnChanged(const AbstractColumn*)), this, SLOT(curveYDataColumnChanged(const AbstractColumn*)));
	connect(m_fitCurve, SIGNAL(xErrorColumnChanged(const AbstractColumn*)), this, SLOT(curveXErrorColumnChanged(const AbstractColumn*)));
	connect(m_fitCurve, SIGNAL(yErrorColumnChanged(const AbstractColumn*)), this, SLOT(curveYErrorColumnChanged(const AbstractColumn*)));
	connect(m_fitCurve, SIGNAL(fitDataChanged(XYFitCurve::FitData)), this, SLOT(curveFitDataChanged(XYFitCurve::FitData)));
	connect(m_fitCurve, SIGNAL(sourceDataChanged()), this, SLOT(enableRecalculate()));
}

void XYFitCurveDock::setModel() {
	QList<const char*>  list;
	list << "Folder" << "Datapicker" << "Worksheet" << "CartesianPlot" << "XYCurve";
	cbDataSourceCurve->setTopLevelClasses(list);

	QList<const AbstractAspect*> hiddenAspects;
	for (auto* curve: m_curvesList)
		hiddenAspects << curve;
	cbDataSourceCurve->setHiddenAspects(hiddenAspects);

	list.clear();
	list << "Folder" << "Workbook" << "Spreadsheet" << "FileDataSource" << "Column" << "CantorWorksheet" << "Datapicker";
	cbXDataColumn->setTopLevelClasses(list);
	cbYDataColumn->setTopLevelClasses(list);
	cbXErrorColumn->setTopLevelClasses(list);
	cbYErrorColumn->setTopLevelClasses(list);

	cbDataSourceCurve->setModel(m_aspectTreeModel);
	cbXDataColumn->setModel(m_aspectTreeModel);
	cbYDataColumn->setModel(m_aspectTreeModel);
	cbXErrorColumn->setModel(m_aspectTreeModel);
	cbYErrorColumn->setModel(m_aspectTreeModel);

	XYCurveDock::setModel();
}

/*!
  sets the curves. The properties of the curves in the list \c list can be edited in this widget.
*/
void XYFitCurveDock::setCurves(QList<XYCurve*> list) {
	m_initializing = true;
	m_curvesList = list;
	m_curve = list.first();

	m_fitCurve = dynamic_cast<XYFitCurve*>(m_curve);
	Q_ASSERT(m_fitCurve);
	m_aspectTreeModel = new AspectTreeModel(m_curve->project());
	this->setModel();
	m_fitData = m_fitCurve->fitData();

	initGeneralTab();
	initTabs();

	m_initializing = false;
}

//*************************************************************
//**** SLOTs for changes triggered in XYFitCurveDock *****
//*************************************************************
void XYFitCurveDock::nameChanged() {
	if (m_initializing)
		return;

	m_curve->setName(uiGeneralTab.leName->text());
}

void XYFitCurveDock::commentChanged() {
	if (m_initializing)
		return;

	m_curve->setComment(uiGeneralTab.leComment->text());
}

void XYFitCurveDock::dataSourceTypeChanged(int index) {
	const XYCurve::DataSourceType type = (XYCurve::DataSourceType)index;
	if (type == XYCurve::DataSourceSpreadsheet) {
		uiGeneralTab.lDataSourceCurve->hide();
		cbDataSourceCurve->hide();
		uiGeneralTab.lXColumn->show();
		cbXDataColumn->show();
		uiGeneralTab.lYColumn->show();
		cbYDataColumn->show();
		cbXErrorColumn->show();
		cbYErrorColumn->show();
	} else {
		uiGeneralTab.lDataSourceCurve->show();
		cbDataSourceCurve->show();
		uiGeneralTab.lXColumn->hide();
		cbXDataColumn->hide();
		uiGeneralTab.lYColumn->hide();
		cbYDataColumn->hide();
		cbXErrorColumn->hide();
		cbYErrorColumn->hide();
	}

	if (m_initializing)
		return;

	for (auto* curve: m_curvesList)
		dynamic_cast<XYFitCurve*>(curve)->setDataSourceType(type);
}

void XYFitCurveDock::dataSourceCurveChanged(const QModelIndex& index) {
	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	XYCurve* dataSourceCurve = 0;
	if (aspect) {
		dataSourceCurve = dynamic_cast<XYCurve*>(aspect);
		Q_ASSERT(dataSourceCurve);
	}

	this->updateSettings(dataSourceCurve->xColumn());

	if (m_initializing)
		return;

	for (auto* curve: m_curvesList)
		dynamic_cast<XYFitCurve*>(curve)->setDataSourceCurve(dataSourceCurve);
}

void XYFitCurveDock::xDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = 0;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	this->updateSettings(column);

	for (auto* curve: m_curvesList)
		dynamic_cast<XYFitCurve*>(curve)->setXDataColumn(column);
}

void XYFitCurveDock::updateSettings(const AbstractColumn* column) {
	if (!column)
		return;

	if (uiGeneralTab.cbAutoRange->isChecked()) {
		uiGeneralTab.sbMin->setValue(column->minimum());
		uiGeneralTab.sbMax->setValue(column->maximum());
	}
}

void XYFitCurveDock::yDataColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = 0;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* curve: m_curvesList)
		dynamic_cast<XYFitCurve*>(curve)->setYDataColumn(column);
}

void XYFitCurveDock::autoRangeChanged() {
	const bool autoRange = uiGeneralTab.cbAutoRange->isChecked();
	m_fitData.autoRange = autoRange;

	if (autoRange) {
		uiGeneralTab.sbMin->setEnabled(false);
		uiGeneralTab.lXRange2->setEnabled(false);
		uiGeneralTab.sbMax->setEnabled(false);

		const AbstractColumn* xDataColumn = 0;
		if (m_fitCurve->dataSourceType() == XYCurve::DataSourceSpreadsheet)
			xDataColumn = m_fitCurve->xDataColumn();
		else {
			if (m_fitCurve->dataSourceCurve())
				xDataColumn = m_fitCurve->dataSourceCurve()->xColumn();
		}

		if (xDataColumn) {
			uiGeneralTab.sbMin->setValue(xDataColumn->minimum());
			uiGeneralTab.sbMax->setValue(xDataColumn->maximum());
		}
	} else {
		uiGeneralTab.sbMin->setEnabled(true);
		uiGeneralTab.lXRange2->setEnabled(true);
		uiGeneralTab.sbMax->setEnabled(true);
	}

}
void XYFitCurveDock::xRangeMinChanged() {
	const double xMin = uiGeneralTab.sbMin->value();

	m_fitData.xRange.first() = xMin;
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYFitCurveDock::xRangeMaxChanged() {
	const double xMax = uiGeneralTab.sbMax->value();

	m_fitData.xRange.last() = xMax;
	uiGeneralTab.pbRecalculate->setEnabled(true);
}

void XYFitCurveDock::xErrorColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = 0;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* curve: m_curvesList)
		dynamic_cast<XYFitCurve*>(curve)->setXErrorColumn(column);
}

void XYFitCurveDock::yErrorColumnChanged(const QModelIndex& index) {
	if (m_initializing)
		return;

	AbstractAspect* aspect = static_cast<AbstractAspect*>(index.internalPointer());
	AbstractColumn* column = 0;
	if (aspect) {
		column = dynamic_cast<AbstractColumn*>(aspect);
		Q_ASSERT(column);
	}

	for (auto* curve: m_curvesList)
		dynamic_cast<XYFitCurve*>(curve)->setYErrorColumn(column);
}

void XYFitCurveDock::weightChanged(int index) {
	QDEBUG("weightChanged() weight =" << nsl_fit_weight_type_name[index]);

	m_fitData.weightsType = (nsl_fit_weight_type)index;
	enableRecalculate();
}

void XYFitCurveDock::categoryChanged(int index) {
	QDEBUG("categoryChanged() category =" << nsl_fit_model_category_name[index] << ", type =" << m_fitData.modelType);
	if (uiGeneralTab.cbCategory->currentIndex() == uiGeneralTab.cbCategory->count() - 1)
		m_fitData.modelCategory = nsl_fit_model_custom;
	else
		m_fitData.modelCategory = (nsl_fit_model_category)index;
	m_initializing = true;
	uiGeneralTab.cbModel->clear();
	uiGeneralTab.cbModel->show();
	uiGeneralTab.lModel->show();

	switch (m_fitData.modelCategory) {
	case nsl_fit_model_basic:
		for(int i = 0; i < NSL_FIT_MODEL_BASIC_COUNT; i++)
			uiGeneralTab.cbModel->addItem(nsl_fit_model_basic_name[i]);
		break;
	case nsl_fit_model_peak:
		for(int i = 0; i < NSL_FIT_MODEL_PEAK_COUNT; i++)
			uiGeneralTab.cbModel->addItem(nsl_fit_model_peak_name[i]);
		break;
	case nsl_fit_model_growth:
		for(int i = 0; i < NSL_FIT_MODEL_GROWTH_COUNT; i++)
			uiGeneralTab.cbModel->addItem(nsl_fit_model_growth_name[i]);
		break;
	case nsl_fit_model_distribution: {
		for(int i = 0; i < NSL_SF_STATS_DISTRIBUTION_COUNT; i++)
			uiGeneralTab.cbModel->addItem(nsl_sf_stats_distribution_name[i]);

		// not-used items are disabled here
        	const QStandardItemModel* model = qobject_cast<const QStandardItemModel*>(uiGeneralTab.cbModel->model());

		for(int i = 1; i < NSL_SF_STATS_DISTRIBUTION_COUNT; i++) {
			// unused distributions
			if (i == nsl_sf_stats_levy_alpha_stable || i == nsl_sf_stats_levy_skew_alpha_stable || i == nsl_sf_stats_bernoulli) {
					QStandardItem* item = model->item(i);
					item->setFlags(item->flags() & ~(Qt::ItemIsSelectable|Qt::ItemIsEnabled));
			}
		}
		break;
	}
	case nsl_fit_model_custom:
		uiGeneralTab.cbModel->addItem(i18n("Custom"));
		uiGeneralTab.cbModel->hide();
		uiGeneralTab.lModel->hide();
	}

	m_fitData.modelType = 0;
	uiGeneralTab.cbModel->setCurrentIndex(m_fitData.modelType);

	m_initializing = false;

	//show the fit-model for the currently selected default (first) fit-model
	modelChanged(m_fitData.modelType);
}

void XYFitCurveDock::modelChanged(int index) {
	DEBUG("modelChanged() type =" << index << ", initializing =" << m_initializing);
	// leave if there is no selection
	if(index == -1)
		return;

	unsigned int type = 0;
	bool custom = false;
	if (m_fitData.modelCategory == nsl_fit_model_custom)
		custom = true;
	else
		type = (unsigned int)index;
	m_fitData.modelType = type;
	uiGeneralTab.teEquation->setReadOnly(!custom);
	uiGeneralTab.tbFunctions->setVisible(custom);
	uiGeneralTab.tbConstants->setVisible(custom);

	// default settings
	uiGeneralTab.lDegree->setText(i18n("Degree"));

	switch (m_fitData.modelCategory) {
	case nsl_fit_model_basic:
		switch (type) {
		case nsl_fit_model_polynomial:
		case nsl_fit_model_fourier:
			uiGeneralTab.lDegree->setVisible(true);
			uiGeneralTab.sbDegree->setVisible(true);
			uiGeneralTab.sbDegree->setMaximum(9);
			uiGeneralTab.sbDegree->setValue(1);
			break;
		case nsl_fit_model_power:
			uiGeneralTab.lDegree->setVisible(true);
			uiGeneralTab.sbDegree->setVisible(true);
			uiGeneralTab.sbDegree->setMaximum(2);
			uiGeneralTab.sbDegree->setValue(1);
			break;
		case nsl_fit_model_exponential:
			uiGeneralTab.lDegree->setVisible(true);
			uiGeneralTab.sbDegree->setVisible(true);
			uiGeneralTab.sbDegree->setMaximum(3);
			uiGeneralTab.sbDegree->setValue(1);
			break;
		default:
			uiGeneralTab.lDegree->setVisible(false);
			uiGeneralTab.sbDegree->setVisible(false);
		}
		break;
	case nsl_fit_model_peak:	// all models support multiple peaks
		uiGeneralTab.lDegree->setText(i18n("Number of peaks"));
		uiGeneralTab.lDegree->setVisible(true);
		uiGeneralTab.sbDegree->setVisible(true);
		uiGeneralTab.sbDegree->setMaximum(9);
		uiGeneralTab.sbDegree->setValue(1);
		break;
	case nsl_fit_model_growth:
	case nsl_fit_model_distribution:
	case nsl_fit_model_custom:
		uiGeneralTab.lDegree->setVisible(false);
		uiGeneralTab.sbDegree->setVisible(false);
	}

	this->updateModelEquation();
}

void XYFitCurveDock::updateModelEquation() {
	DEBUG("updateModelEquation() type =" << m_fitData.modelType);

	int num = uiGeneralTab.sbDegree->value();
	QStringList vars; // variables/parameter that are known
	vars << "x";
	// indices used in multi peak parameter models
	QStringList indices;
	indices << QString::fromUtf8("\u2081") << QString::fromUtf8("\u2082") << QString::fromUtf8("\u2083") << QString::fromUtf8("\u2084") << QString::fromUtf8("\u2085")
		<< QString::fromUtf8("\u2086") << QString::fromUtf8("\u2087") << QString::fromUtf8("\u2088") << QString::fromUtf8("\u2089");

	switch (m_fitData.modelCategory) {
        case nsl_fit_model_basic:
		m_fitData.model = nsl_fit_model_basic_equation[m_fitData.modelType];
		break;
        case nsl_fit_model_peak:
		m_fitData.model = nsl_fit_model_peak_equation[m_fitData.modelType];
		break;
        case nsl_fit_model_growth:
		m_fitData.model = nsl_fit_model_growth_equation[m_fitData.modelType];
		break;
        case nsl_fit_model_distribution:
		m_fitData.model = nsl_sf_stats_distribution_equation[m_fitData.modelType];
		break;
        case nsl_fit_model_custom:
		// use the equation of the last selected predefined model
		uiGeneralTab.teEquation->setText(m_fitData.model);
		break;
	}
	// custom keeps the parameter from previous selected model
	if (m_fitData.modelCategory != nsl_fit_model_custom) {
		m_fitData.paramNames.clear();
		m_fitData.paramNamesUtf8.clear();
	}

	switch (m_fitData.modelCategory) {
	case nsl_fit_model_basic:
		switch (m_fitData.modelType) {
		case nsl_fit_model_polynomial:
			m_fitData.paramNames << "c0" << "c1";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("c\u2080") << QString::fromUtf8("c\u2081");
			if (num == 2) {
				m_fitData.model += " + c2*x^2";
				m_fitData.paramNames << "c2";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("c\u2082");
			} else if (num > 2) {
				QString numStr = QString::number(num);
				for (int i = 2; i <= num; ++i) {
					numStr = QString::number(i);
					m_fitData.model += "+c" + numStr + "*x^" + numStr;
					m_fitData.paramNames << "c" + numStr;
					m_fitData.paramNamesUtf8 << "c" + indices[i-1];
				}
			}
			break;
		case nsl_fit_model_power:
			if (num == 1)
				m_fitData.paramNames << "a" << "b";
			else {
				m_fitData.paramNames << "a" << "b" << "c";
				m_fitData.model = "a + b*x^c";
			}
			break;
		case nsl_fit_model_exponential:
			switch (num) {
			case 1:
				m_fitData.paramNames << "a" << "b";
				break;
			case 2:
				m_fitData.model = "a1*exp(b1*x) + a2*exp(b2*x)";
				m_fitData.paramNames << "a1" << "b1" << "a2" << "b2";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("a\u2081") << QString::fromUtf8("b\u2081")
						<< QString::fromUtf8("a\u2082") << QString::fromUtf8("b\u2082");
				break;
			case 3:
				m_fitData.model = "a1*exp(b1*x) + a2*exp(b2*x) + a3*exp(b3*x)";
				m_fitData.paramNames << "a1" << "b1" << "a2" << "b2" << "a3" << "b3";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("a\u2081") << QString::fromUtf8("b\u2081")
						<< QString::fromUtf8("a\u2082") << QString::fromUtf8("b\u2082")
						<< QString::fromUtf8("a\u2083") << QString::fromUtf8("b\u2083");
				break;
			//TODO: up to 9 exponentials
			}
			break;
		case nsl_fit_model_inverse_exponential:
			num = 1;
			m_fitData.paramNames << "a" << "b" << "c";
			break;
		case nsl_fit_model_fourier:
			m_fitData.paramNames << "w" << "a0" << "a1" << "b1";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c9") << QString::fromUtf8("a\u2080")
				<< QString::fromUtf8("a\u2081") << QString::fromUtf8("b\u2081");
			if (num > 1) {
				for (int i = 1; i <= num; ++i) {
					QString numStr = QString::number(i);
					m_fitData.model += "+ (a" + numStr + "*cos(" + numStr + "*w*x) + b" + numStr + "*sin(" + numStr + "*w*x))";
					m_fitData.paramNames << "a"+numStr << "b"+numStr;
					m_fitData.paramNamesUtf8 << "a" + indices[i-1] << "b" + indices[i-1];
				}
			}
			break;
		}
		break;
	case nsl_fit_model_peak:
		switch ((nsl_fit_model_type_peak)m_fitData.modelType) {
		case nsl_fit_model_gaussian:
			switch (num) {
			case 1:
				m_fitData.paramNames << "s" << "mu" << "a";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3") << QString::fromUtf8("\u03bc") << "A";
				break;
			case 2:
				m_fitData.model = "1./sqrt(2*pi) * (a1/s1 * exp(-((x-mu1)/s1)^2/2) + a2/s2 * exp(-((x-mu2)/s2)^2/2))";
				m_fitData.paramNames << "s1" << "mu1" << "a1" << "s2" << "mu2" << "a2";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3\u2081") << QString::fromUtf8("\u03bc\u2081") << QString::fromUtf8("A\u2081")
					<< QString::fromUtf8("\u03c3\u2082") << QString::fromUtf8("\u03bc\u2082") << QString::fromUtf8("A\u2082");
				break;
			case 3:
				m_fitData.model = "1./sqrt(2*pi) * (a1/s1 * exp(-((x-mu1)/s1)^2/2) + a2/s2 * exp(-((x-mu2)/s2)^2/2) + a3/s3 * exp(-((x-mu3)/s3)^2/2))";
				m_fitData.paramNames << "s1" << "mu1" << "a1" << "s2" << "mu2" << "a2" << "s3" << "mu3" << "a3";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3\u2081") << QString::fromUtf8("\u03bc\u2081") << QString::fromUtf8("A\u2081")
					<< QString::fromUtf8("\u03c3\u2082") << QString::fromUtf8("\u03bc\u2082") << QString::fromUtf8("A\u2082")
					<< QString::fromUtf8("\u03c3\u2083") << QString::fromUtf8("\u03bc\u2083") << QString::fromUtf8("A\u2083");
				break;
			default:
				m_fitData.model = "1./sqrt(2*pi) * (";
				for (int i = 1; i <= num; ++i) {
					QString numStr = QString::number(i);
					if (i > 1)
						m_fitData.model += " + ";
					m_fitData.model += "a" + numStr + "/s" + numStr + "* exp(-((x-mu" + numStr + ")/s" + numStr + ")^2/2)";
					m_fitData.paramNames << "s" + numStr << "mu" + numStr << "a" + numStr;
					m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3") + indices[i-1] << QString::fromUtf8("\u03bc") + indices[i-1]
						<< QString::fromUtf8("A") + indices[i-1];
				}
				m_fitData.model += ")";
			}
			break;
		case nsl_fit_model_lorentz:
			switch (num) {
			case 1:
				m_fitData.paramNames << "g" << "mu" << "a";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03b3") << QString::fromUtf8("\u03bc") << "A";
				break;
			case 2:
				m_fitData.model = "1./pi * (a1 * g1/(g1^2+(x-mu1)^2) + a2 * g2/(g2^2+(x-mu2)^2))";
				m_fitData.paramNames << "g1" << "mu1" << "a1" << "g2" << "mu2" << "a2";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03b3\u2081") << QString::fromUtf8("\u03bc\u2081") << QString::fromUtf8("A\u2081")
					<< QString::fromUtf8("\u03b3\u2082") << QString::fromUtf8("\u03bc\u2082") << QString::fromUtf8("A\u2082");
				break;
			case 3:
				m_fitData.model = "1./pi * (a1 * g1/(g1^2+(x-mu1)^2) + a2 * g2/(g2^2+(x-mu2)^2) + a3 * g3/(g3^2+(x-mu3)^2))";
				m_fitData.paramNames << "g1" << "mu1" << "a1" << "g2" << "mu2" << "a2" << "g3" << "mu3" << "a3";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03b3\u2081") << QString::fromUtf8("\u03bc\u2081") << QString::fromUtf8("A\u2081")
					<< QString::fromUtf8("\u03b3\u2082") << QString::fromUtf8("\u03bc\u2082") << QString::fromUtf8("A\u2082")
					<< QString::fromUtf8("\u03b3\u2083") << QString::fromUtf8("\u03bc\u2083") << QString::fromUtf8("A\u2083");
				break;
			default:
				QString numStr = QString::number(num);
				m_fitData.model = "1./pi * (";
				for (int i = 1; i <= num; ++i) {
					numStr = QString::number(i);
					if (i > 1)
						m_fitData.model += " + ";
					m_fitData.model += "a" + numStr + " * g" + numStr + "/(g" + numStr + "^2+(x-mu" + numStr + ")^2)";
					m_fitData.paramNames << "g" + numStr << "mu" + numStr << "a" + numStr;
					m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03b3") + indices[i-1] << QString::fromUtf8("\u03bc") + indices[i-1]
						<< QString::fromUtf8("A") + indices[i-1];
				}
				m_fitData.model += ")";
			}
			break;
		case nsl_fit_model_sech:
			switch (num) {
			case 1:
				m_fitData.paramNames << "s" << "mu" << "a";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3") << QString::fromUtf8("\u03bc") << "A";
				break;
			case 2:
				m_fitData.model = "1/pi * (a1/s1 * sech((x-mu1)/s1) + a2/s2 * sech((x-mu2)/s2))";
				m_fitData.paramNames << "s1" << "mu1" << "a1" << "s2" << "mu2" << "a2";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3\u2081") << QString::fromUtf8("\u03bc\u2081") << QString::fromUtf8("A\u2081")
					<< QString::fromUtf8("\u03c3\u2082") << QString::fromUtf8("\u03bc\u2082") << QString::fromUtf8("A\u2082");
				break;
			case 3:
				m_fitData.model = "1/pi * (a1/s1 * sech((x-mu1)/s1) + a2/s2 * sech((x-mu2)/s2) + a3/s3 * sech((x-mu3)/s3))";
				m_fitData.paramNames << "s1" << "mu1" << "a1" << "s2" << "mu2" << "a2" << "s3" << "mu3" << "a3";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3\u2081") << QString::fromUtf8("\u03bc\u2081") << QString::fromUtf8("A\u2081")
					<< QString::fromUtf8("\u03c3\u2082") << QString::fromUtf8("\u03bc\u2082") << QString::fromUtf8("A\u2082")
					<< QString::fromUtf8("\u03c3\u2083") << QString::fromUtf8("\u03bc\u2083") << QString::fromUtf8("A\u2083");
				break;
			default:
				QString numStr = QString::number(num);
				m_fitData.model = "1/pi * (";
				for (int i = 1; i <= num; ++i) {
					numStr = QString::number(i);
					if (i > 1)
						m_fitData.model += " + ";
					m_fitData.model += "a" + numStr + "/s" + numStr + "* sech((x-mu" + numStr + ")/s" + numStr + ")";
					m_fitData.paramNames << "s" + numStr << "mu" + numStr << "a" + numStr;
					m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3") + indices[i-1] << QString::fromUtf8("\u03bc") + indices[i-1]
						<< QString::fromUtf8("A") + indices[i-1];
				}
				m_fitData.model += ")";
			}
			break;
		case nsl_fit_model_logistic:
			switch (num) {
			case 1:
				m_fitData.paramNames << "s" << "mu" << "a";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3") << QString::fromUtf8("\u03bc") << "A";
				break;
			case 2:
				m_fitData.model = "1/4 * (a1/s1 * sech((x-mu1)/2/s1)**2 + a2/s2 * sech((x-mu2)/2/s2)**2)";
				m_fitData.paramNames << "s1" << "mu1" << "a1" << "s2" << "mu2" << "a2";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3\u2081") << QString::fromUtf8("\u03bc\u2081") << QString::fromUtf8("A\u2081")
					<< QString::fromUtf8("\u03c3\u2082") << QString::fromUtf8("\u03bc\u2082") << QString::fromUtf8("A\u2082");
				break;
			case 3:
				m_fitData.model = "1/4 * (a1/s1 * sech((x-mu1)/2/s1)**2 + a2/s2 * sech((x-mu2)/2/s2)**2 + a3/s3 * sech((x-mu3)/2/s3)**2)";
				m_fitData.paramNames << "s1" << "mu1" << "a1" << "s2" << "mu2" << "a2" << "s3" << "mu3" << "a3";
				m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3\u2081") << QString::fromUtf8("\u03bc\u2081") << QString::fromUtf8("A\u2081")
					<< QString::fromUtf8("\u03c3\u2082") << QString::fromUtf8("\u03bc\u2082") << QString::fromUtf8("A\u2082")
					<< QString::fromUtf8("\u03c3\u2083") << QString::fromUtf8("\u03bc\u2083") << QString::fromUtf8("A\u2083");
				break;
			default:
				QString numStr = QString::number(num);
				m_fitData.model = "1/4 * (";
				for (int i = 1; i <= num; ++i) {
					numStr = QString::number(i);
					if (i > 1)
						m_fitData.model += " + ";
					m_fitData.model += "a" + numStr + "/s" + numStr + "* sech((x-mu" + numStr + ")/2/s" + numStr + ")**2";
					m_fitData.paramNames << "s" + numStr << "mu" + numStr << "a" + numStr;
					m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3") + indices[i-1] << QString::fromUtf8("\u03bc") + indices[i-1]
						<< QString::fromUtf8("A") + indices[i-1];
				}
				m_fitData.model += ")";
			}
			break;
		}
		break;
	case nsl_fit_model_growth:
		switch ((nsl_fit_model_type_growth)m_fitData.modelType) {
		case nsl_fit_model_atan:
		case nsl_fit_model_tanh:
		case nsl_fit_model_algebraic_sigmoid:
		case nsl_fit_model_erf:
		case nsl_fit_model_gudermann:
			m_fitData.paramNames << "s" << "mu" << "a";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3") << QString::fromUtf8("\u03bc") << "A";
			break;
		case nsl_fit_model_sigmoid:
			m_fitData.paramNames << "k" << "mu" << "a";
			m_fitData.paramNamesUtf8 << "k" << QString::fromUtf8("\u03bc") << "A";
			break;
		case nsl_fit_model_hill:
			m_fitData.paramNames << "s" << "n" << "a";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3") << "n" << "A";
			break;
		case nsl_fit_model_gompertz:
			m_fitData.paramNames << "a" << "b" << "c";
			break;
		}
		break;
	case nsl_fit_model_distribution:
		switch ((nsl_sf_stats_distribution)m_fitData.modelType) {
		case nsl_sf_stats_gaussian:
		case nsl_sf_stats_laplace:
		case nsl_sf_stats_rayleigh_tail:
		case nsl_sf_stats_lognormal:
		case nsl_sf_stats_logistic:
		case nsl_sf_stats_sech:
			m_fitData.paramNames << "s" << "mu" << "a";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3") << QString::fromUtf8("\u03bc") << "A";
			break;
		case nsl_sf_stats_gaussian_tail:
			m_fitData.paramNames << "s" << "mu" << "A" << "a";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3") << QString::fromUtf8("\u03bc") << "A" << "a";
			break;
		case nsl_sf_stats_exponential:
			m_fitData.paramNames << "l" << "mu" << "a";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03bb") << QString::fromUtf8("\u03bc") << "A";
			break;
		case nsl_sf_stats_exponential_power:
			m_fitData.paramNames << "s" << "mu" << "b" << "a";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3") << QString::fromUtf8("\u03bc") << "b" << "A";
			break;
		case nsl_sf_stats_cauchy_lorentz:
		case nsl_sf_stats_levy:
			m_fitData.paramNames << "g" << "mu" << "a";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03b3") << QString::fromUtf8("\u03bc") << "A";
			break;
		case nsl_sf_stats_rayleigh:
			m_fitData.paramNames << "s" << "a";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3") << "A";
			break;
		case nsl_sf_stats_landau:
			m_fitData.paramNames << "a";
			m_fitData.paramNamesUtf8 << "A";
			break;
		case nsl_sf_stats_levy_alpha_stable:	// unused distributions
		case nsl_sf_stats_levy_skew_alpha_stable:
		case nsl_sf_stats_bernoulli:
			break;
		case nsl_sf_stats_gamma:
			m_fitData.paramNames << "t" << "k" << "a";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03b8") << "k" << "A";
			break;
		case nsl_sf_stats_flat:
			m_fitData.paramNames << "a" << "b" << "A";
			break;
		case nsl_sf_stats_chi_squared:
			m_fitData.paramNames << "n" << "a";
			m_fitData.paramNamesUtf8 << "n" << "A";
			break;
		case nsl_sf_stats_fdist:
			m_fitData.paramNames << "n1" << "n2" << "a";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03bd") + QString::fromUtf8("\u2081")
				<< QString::fromUtf8("\u03bd") + QString::fromUtf8("\u2082") << "A";
			break;
		case nsl_sf_stats_tdist:
			m_fitData.paramNames << "n" << "a";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03bd") << "A";
			break;
		case nsl_sf_stats_beta:
		case nsl_sf_stats_pareto:
			m_fitData.paramNames << "a" << "b" << "A";
			break;
		case nsl_sf_stats_weibull:
			m_fitData.paramNames << "k" << "l" << "mu" << "a";
			m_fitData.paramNamesUtf8 << "k" << QString::fromUtf8("\u03bb") << QString::fromUtf8("\u03bc") << "A";
			break;
		case nsl_sf_stats_gumbel1:
			m_fitData.paramNames << "s" << "b" << "mu" << "a";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3") << QString::fromUtf8("\u03b2") << QString::fromUtf8("\u03bc") << "A";
			break;
		case nsl_sf_stats_gumbel2:
			m_fitData.paramNames << "a" << "b" << "mu" << "A";
			m_fitData.paramNamesUtf8 << "a" << "b" << QString::fromUtf8("\u03bc") << "A";
			break;
		case nsl_sf_stats_poisson:
			m_fitData.paramNames << "l" << "a";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03bb") << "A";
			break;
		case nsl_sf_stats_binomial:
		case nsl_sf_stats_negative_binomial:
		case nsl_sf_stats_pascal:
			m_fitData.paramNames << "p" << "n" << "a";
			m_fitData.paramNamesUtf8 << "p" << "n" << "A";
			break;
		case nsl_sf_stats_geometric:
		case nsl_sf_stats_logarithmic:
			m_fitData.paramNames << "p" << "a";
			m_fitData.paramNamesUtf8 << "p" << "A";
			break;
		case nsl_sf_stats_hypergeometric:
			m_fitData.paramNames << "n1" << "n2" << "t" << "a";
			m_fitData.paramNamesUtf8 << "n" + QString::fromUtf8("\u2081") << "n" + QString::fromUtf8("\u2082") << "t" << "A";
			break;
		case nsl_sf_stats_maxwell_boltzmann:
			m_fitData.paramNames << "s" << "a";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03c3") << "A";
			break;
		case nsl_sf_stats_frechet:
			m_fitData.paramNames << "g" << "mu" << "s" << "a";
			m_fitData.paramNamesUtf8 << QString::fromUtf8("\u03b3") << QString::fromUtf8("\u03bc") << QString::fromUtf8("\u03c3") << "A";
			break;
		}
		break;
	case nsl_fit_model_custom:
		break;
	}
	vars << m_fitData.paramNames;

	// use normal param names if no utf8 param names are defined
	if (m_fitData.paramNamesUtf8.isEmpty())
		m_fitData.paramNamesUtf8 << m_fitData.paramNames;

	//resize the vector for the start values and set the elements to 1.0
	//in case a custom model is used, do nothing, we take over the previous values
	//when initializing, don't do anything - we use start values already
	//available - unless there're no values available
	if (m_fitData.modelCategory != nsl_fit_model_custom ||
	        !(m_initializing && m_fitData.paramNames.size() == m_fitData.paramStartValues.size())) {
		DEBUG(" number of start values" << m_fitData.paramNames.size() << ' ' << m_fitData.paramStartValues.size());
		m_fitData.paramStartValues.resize(m_fitData.paramNames.size());
		m_fitData.paramFixed.resize(m_fitData.paramNames.size());
		m_fitData.paramLowerLimits.resize(m_fitData.paramNames.size());
		m_fitData.paramUpperLimits.resize(m_fitData.paramNames.size());

		for (int i = 0; i < m_fitData.paramNames.size(); ++i) {
			m_fitData.paramStartValues[i] = 1.0;
			m_fitData.paramFixed[i] = false;
			m_fitData.paramLowerLimits[i] = -DBL_MAX;
			m_fitData.paramUpperLimits[i] = DBL_MAX;
		}

		// model-dependent start values
		if (m_fitData.modelCategory == nsl_fit_model_distribution) {
			nsl_sf_stats_distribution type = (nsl_sf_stats_distribution)m_fitData.modelType;
			if (type == nsl_sf_stats_flat)
				m_fitData.paramStartValues[0] = -1.0;
			else if (type == nsl_sf_stats_frechet || type == nsl_sf_stats_levy || type == nsl_sf_stats_exponential_power)
				m_fitData.paramStartValues[1] = 0.0;
			else if (type == nsl_sf_stats_weibull || type == nsl_sf_stats_gumbel2)
				m_fitData.paramStartValues[2] = 0.0;
			else if (type == nsl_sf_stats_binomial || type == nsl_sf_stats_negative_binomial || type == nsl_sf_stats_pascal
				|| type == nsl_sf_stats_geometric || type == nsl_sf_stats_logarithmic)
				m_fitData.paramStartValues[0] = 0.5;
		}
	}

	uiGeneralTab.teEquation->setVariables(vars);

	// set formula picture
	uiGeneralTab.lEquation->setText(("f(x) ="));
	QString file;
	switch (m_fitData.modelCategory) {
	case nsl_fit_model_basic: {
		// formula pic depends on degree
		QString numSuffix = QString::number(num);
		if (num > 4)
			numSuffix = "4";
		if ((nsl_fit_model_type_basic)m_fitData.modelType == nsl_fit_model_power && num > 2)
			numSuffix = "2";
		file = QStandardPaths::locate(QStandardPaths::AppDataLocation, "pics/fit_models/"
			+ QString(nsl_fit_model_basic_pic_name[m_fitData.modelType]) + numSuffix + ".jpg");
		break;
	}
	case nsl_fit_model_peak: {
		// formula pic depends on number of peaks
		QString numSuffix = QString::number(num);
		if (num > 4)
			numSuffix = "4";
		file = QStandardPaths::locate(QStandardPaths::AppDataLocation, "pics/fit_models/"
			+ QString(nsl_fit_model_peak_pic_name[m_fitData.modelType]) + numSuffix + ".jpg");
		break;
	}
	case nsl_fit_model_growth:
		file = QStandardPaths::locate(QStandardPaths::AppDataLocation, "pics/fit_models/"
			+ QString(nsl_fit_model_growth_pic_name[m_fitData.modelType]) + ".jpg");
		break;
	case nsl_fit_model_distribution:
		file = QStandardPaths::locate(QStandardPaths::AppDataLocation, "pics/gsl_distributions/"
			+ QString(nsl_sf_stats_distribution_pic_name[m_fitData.modelType]) + ".jpg");
		// change label
		if (m_fitData.modelType == nsl_sf_stats_poisson)
			uiGeneralTab.lEquation->setText(("f(k)/A ="));
		else
			uiGeneralTab.lEquation->setText(("f(x)/A ="));
		break;
	case nsl_fit_model_custom:
		uiGeneralTab.teEquation->show();
		uiGeneralTab.lFuncPic->hide();
	}

	if (m_fitData.modelCategory != nsl_fit_model_custom) {
		uiGeneralTab.lFuncPic->setPixmap(file);
		uiGeneralTab.lFuncPic->show();
		uiGeneralTab.teEquation->hide();
	}
}

void XYFitCurveDock::showConstants() {
	QMenu menu;
	ConstantsWidget constants(&menu);

	connect(&constants, SIGNAL(constantSelected(QString)), this, SLOT(insertConstant(QString)));
	connect(&constants, SIGNAL(constantSelected(QString)), &menu, SLOT(close()));
	connect(&constants, SIGNAL(canceled()), &menu, SLOT(close()));

	QWidgetAction* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&constants);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width() + uiGeneralTab.tbConstants->width(), -menu.sizeHint().height());
	menu.exec(uiGeneralTab.tbConstants->mapToGlobal(pos));
}

void XYFitCurveDock::showFunctions() {
	QMenu menu;
	FunctionsWidget functions(&menu);
	connect(&functions, SIGNAL(functionSelected(QString)), this, SLOT(insertFunction(QString)));
	connect(&functions, SIGNAL(functionSelected(QString)), &menu, SLOT(close()));
	connect(&functions, SIGNAL(canceled()), &menu, SLOT(close()));

	QWidgetAction* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&functions);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width() + uiGeneralTab.tbFunctions->width(), -menu.sizeHint().height());
	menu.exec(uiGeneralTab.tbFunctions->mapToGlobal(pos));
}

void XYFitCurveDock::updateParameterList() {
	// use current model function
	m_fitData.model = uiGeneralTab.teEquation->toPlainText();

	ExpressionParser* parser = ExpressionParser::getInstance();
	QStringList vars; // variables that are known
	vars << "x";	//TODO: others?
	m_fitData.paramNames = m_fitData.paramNamesUtf8 = parser->getParameter(m_fitData.model, vars);

	// if number of parameter changed
	bool moreParameter = false;
	if (m_fitData.paramNames.size() > m_fitData.paramStartValues.size())
		moreParameter = true;
	if (m_fitData.paramNames.size() != m_fitData.paramStartValues.size()) {
		m_fitData.paramStartValues.resize(m_fitData.paramNames.size());
		m_fitData.paramFixed.resize(m_fitData.paramNames.size());
		m_fitData.paramLowerLimits.resize(m_fitData.paramNames.size());
		m_fitData.paramUpperLimits.resize(m_fitData.paramNames.size());
	}
	if (moreParameter) {
		for (int i = m_fitData.paramStartValues.size() - 1; i < m_fitData.paramNames.size(); ++i) {
			m_fitData.paramStartValues[i] = 1.0;
			m_fitData.paramFixed[i] = false;
			m_fitData.paramLowerLimits[i] = -DBL_MAX;
			m_fitData.paramUpperLimits[i] = DBL_MAX;
		}
	}
	parametersChanged();
}

void XYFitCurveDock::showParameters() {
	if (m_fitData.modelCategory == nsl_fit_model_custom)
		updateParameterList();

	QMenu menu;
	FitParametersWidget w(&menu, &m_fitData);
	connect(&w, SIGNAL(finished()), &menu, SLOT(close()));
	connect(&w, SIGNAL(parametersChanged()), this, SLOT(parametersChanged()));

	QWidgetAction* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&w);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width() + uiGeneralTab.pbParameters->width(), -menu.sizeHint().height());
	menu.exec(uiGeneralTab.pbParameters->mapToGlobal(pos));
}

/*!
 * called when parameter names and/or start values for the custom model were changed
 */
void XYFitCurveDock::parametersChanged() {
	//parameter names were (probably) changed -> set the new names in EquationTextEdit
	uiGeneralTab.teEquation->setVariables(m_fitData.paramNames);
	enableRecalculate();
}

void XYFitCurveDock::showOptions() {
	QMenu menu;
	FitOptionsWidget w(&menu, &m_fitData);
	connect(&w, SIGNAL(finished()), &menu, SLOT(close()));
	connect(&w, SIGNAL(optionsChanged()), this, SLOT(enableRecalculate()));

	QWidgetAction* widgetAction = new QWidgetAction(this);
	widgetAction->setDefaultWidget(&w);
	menu.addAction(widgetAction);

	QPoint pos(-menu.sizeHint().width() + uiGeneralTab.pbParameters->width(), -menu.sizeHint().height());
	menu.exec(uiGeneralTab.pbOptions->mapToGlobal(pos));
}

void XYFitCurveDock::insertFunction(const QString& str) const {
	uiGeneralTab.teEquation->insertPlainText(str + "(x)");
}

void XYFitCurveDock::insertConstant(const QString& str) const {
	uiGeneralTab.teEquation->insertPlainText(str);
}

void XYFitCurveDock::recalculateClicked() {
	QApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
	m_fitData.degree = uiGeneralTab.sbDegree->value();
	if (m_fitData.modelCategory == nsl_fit_model_custom)
		updateParameterList();

	for (XYCurve* curve: m_curvesList)
		dynamic_cast<XYFitCurve*>(curve)->setFitData(m_fitData);

	this->showFitResult();
	uiGeneralTab.pbRecalculate->setEnabled(false);
	QApplication::restoreOverrideCursor();
}

void XYFitCurveDock::enableRecalculate() const {
	if (m_initializing)
		return;

	//no fitting possible without the x- and y-data
	bool hasSourceData = false;
	if (m_fitCurve->dataSourceType() == XYCurve::DataSourceSpreadsheet) {
		AbstractAspect* aspectX = static_cast<AbstractAspect*>(cbXDataColumn->currentModelIndex().internalPointer());
		AbstractAspect* aspectY = static_cast<AbstractAspect*>(cbYDataColumn->currentModelIndex().internalPointer());
		hasSourceData = (aspectX != 0 && aspectY != 0);
	} else {
		 hasSourceData = (m_fitCurve->dataSourceCurve() != NULL);
	}

	uiGeneralTab.pbRecalculate->setEnabled(hasSourceData);
}

/*!
 * show the fit result summary (with HTML tables)
 */
void XYFitCurveDock::showFitResultSummary(const XYFitCurve::FitResult& fitResult) {
	QString str = "<table border=1>";
	str += "<tr> <th>" + i18n("status:") + "</th> <th>" + fitResult.status + "</th> </tr>";
	str += "<tr> <th>" + i18n("degrees of freedom:") + "</th> <th>" + QString::number(fitResult.dof) + "</th> </tr>";
	//str += i18n("iterations:") + ' ' + QString::number(fitResult.iterations) + "<br>";
	//if (fitResult.elapsedTime > 1000)
	//	str += i18n("calculation time: %1 s", fitResult.elapsedTime/1000) + "<br>";
	// else
		//str += i18n("calculation time: %1 ms", fitResult.elapsedTime) + "<br>";
	str +=  "</table>";
	if (!fitResult.valid) {
		uiGeneralTab.teResult->setText(str);
		return; //result is not valid, there was an error which is shown in the status-string, nothing to show more.
	}

	const int np = fitResult.paramValues.size();
	const double rsquare = nsl_stats_rsquare(fitResult.sse,fitResult.sst);
	const double rsquareAdj = nsl_stats_rsquareAdj(rsquare, np, fitResult.dof);

	str += "<br><br><b>" + i18n("Parameters:") + "</b>";
	str += "<table border=1>";
	str += "<tr> <th>" + i18n("Name") + "</th> <th>" + i18n("Value") +  "</th> <th>" + i18n("Error") +  "</th> <th>" + i18n("Error, %") +  "</th> </tr>";
	for (int i = 0; i < np; i++) {
		if (m_fitData.paramFixed.at(i))
			str += "<tr> <th>" + m_fitData.paramNamesUtf8.at(i) + "</th> <th>" + QString::number(fitResult.paramValues.at(i)) + "</th> </tr>";
		else
			str += "<tr> <th>" + m_fitData.paramNamesUtf8.at(i) + "</th> <th>" + QString::number(fitResult.paramValues.at(i))
				+ "</th> <th>" + QString::fromUtf8("\u00b1") + QString::number(fitResult.errorValues.at(i))
				+ "</th> <th>" + QString::number(100.*fitResult.errorValues.at(i)/fabs(fitResult.paramValues.at(i)), 'g', 3) + " %" + "</th> </tr>";
	}
	str +=  "</table>";

	str += "<br><br><b>" + i18n("Goodness of fit:") + "</b>";
	str += "<table border=1>";
	if (fitResult.dof != 0) {
		str += "<tr> <th>" + i18n("reduced") + ' ' + QString::fromUtf8("\u03c7") + QString::fromUtf8("\u00b2")
			+ "</th> <th>" + QString::number(fitResult.rms) + "</th> </tr>";
		str += "<tr> <th>" + i18n("adj. coefficient of determination")+ " (R" + QString::fromUtf8("\u0304") + QString::fromUtf8("\u00b2")
			+ ')' + "</th> <th>" + QString::number(rsquareAdj, 'g', 15) + "</th> </tr>";
	}
	str +=  "</table>";

	uiGeneralTab.teResult->setText(str);
}

/*!
 * show the fit result log (plain text)
 */
void XYFitCurveDock::showFitResultLog(const XYFitCurve::FitResult& fitResult) {
	QString str;
	str += i18n("status:") + ' ' + fitResult.status + "<br>";
	str += i18n("iterations:") + ' ' + QString::number(fitResult.iterations) + "<br>";
	str += i18n("tolerance:") + ' ' + QString::number(m_fitData.eps) + "<br>";
	if (fitResult.elapsedTime > 1000)
		str += i18n("calculation time: %1 s", fitResult.elapsedTime/1000) + "<br>";
	else
		str += i18n("calculation time: %1 ms", fitResult.elapsedTime) + "<br>";
	str += i18n("degrees of freedom:") + ' ' + QString::number(fitResult.dof) + "<br>";
	str += i18n("number of parameters:") + ' ' + QString::number(fitResult.paramValues.size()) + "<br>";
	str += i18n("X range:") + ' ' + QString::number(uiGeneralTab.sbMin->value()) + " .. " + QString::number(uiGeneralTab.sbMax->value()) + "<br>";

	if (!fitResult.valid) {
		uiGeneralTab.teLog->setText(str);
		return; //result is not valid, there was an error which is shown in the status-string, nothing to show more.
	}

	const int np = fitResult.paramValues.size();
	const double rsquare = nsl_stats_rsquare(fitResult.sse,fitResult.sst);
	const double rsquareAdj = nsl_stats_rsquareAdj(rsquare, np, fitResult.dof);

	// Parameter
	str += "<br> <b>" + i18n("Parameters:") + "</b><br>";
	for (int i = 0; i < np; i++) {
		if (m_fitData.paramFixed.at(i))
			str += m_fitData.paramNamesUtf8.at(i) + QString(" = ") + QString::number(fitResult.paramValues.at(i)) + "<br>";
		else {
			str += m_fitData.paramNamesUtf8.at(i) + QString(" = ") + QString::number(fitResult.paramValues.at(i))
				+ QString::fromUtf8("\u00b1") + QString::number(fitResult.errorValues.at(i))
				+ " (" + QString::number(100.*fitResult.errorValues.at(i)/fabs(fitResult.paramValues.at(i)), 'g', 3) + " %)<br>";

			const double t = nsl_stats_tdist_t(fitResult.paramValues.at(i), fitResult.errorValues.at(i));
			const double p = nsl_stats_tdist_p(t, fitResult.dof);
			const double margin = nsl_stats_tdist_margin(0.05, fitResult.dof, fitResult.errorValues.at(i));
			str += " (" + i18n("t statistic:") + ' ' + QString::number(t, 'g', 3) + ", " + i18n("p value:") + ' ' + QString::number(p, 'g', 3)
				+ ", " + i18n("conf. interval:") + ' ' + QString::number(fitResult.paramValues.at(i) - margin)
                                + " .. " + QString::number(fitResult.paramValues.at(i) + margin) + ")<br>";
		}
	}

	// Goodness of fit
	str += "<br><b>" + i18n("Goodness of fit:") + "</b><br>";
	str += i18n("sum of squared residuals") + " (" + QString::fromUtf8("\u03c7") + QString::fromUtf8("\u00b2") + "): " + QString::number(fitResult.sse) + "<br>";
	if (fitResult.dof != 0) {
		str += i18n("reduced") + ' ' + QString::fromUtf8("\u03c7") + QString::fromUtf8("\u00b2") + ": " + QString::number(fitResult.rms) + "<br>";
		str += i18n("root mean square error") + " (RMSE): " + QString::number(fitResult.rsd) + "<br>";
		str += i18n("coefficient of determination") + " (R" + QString::fromUtf8("\u00b2") + "): " + QString::number(rsquare, 'g', 15) + "<br>";
		str += i18n("adj. coefficient of determination")+ " (R" + QString::fromUtf8("\u0304") + QString::fromUtf8("\u00b2")
			+ "): " + QString::number(rsquareAdj, 'g', 15) + "<br><br>";

		double p = nsl_stats_chisq_p(fitResult.sse, fitResult.dof);
		str += i18n("P > ") + QString::fromUtf8("\u03c7") + QString::fromUtf8("\u00b2") + ": " + QString::number(p, 'g', 3) + "<br>";
		const double F = nsl_stats_fdist_F(fitResult.sst, fitResult.rms);
		str += i18n("F statistic") + ": " + QString::number(F, 'g', 3) + "<br>";
		p = nsl_stats_fdist_p(F, np, fitResult.dof);
		str += i18n("P > F") + ": " + QString::number(p, 'g', 3) + "<br>";
	}
	str += i18n("mean absolute error:") + ' ' + QString::number(fitResult.mae) + "<br> <br>";

	// show all iterations
	str += "<b>" + i18n("Iterations:") + "</b><br>";
	for (const auto &s: m_fitData.paramNamesUtf8)
		str += s + ' ';
	str += QString::fromUtf8("\u03c7") + QString::fromUtf8("\u00b2");

	const QStringList iterations = fitResult.solverOutput.split(';');
	for (const auto &s: iterations)
		str += "<br>" + s;

	uiGeneralTab.teLog->setText(str);
}

/*!
 * show the result and details of fit
 */
void XYFitCurveDock::showFitResult() {
	const XYFitCurve::FitResult& fitResult = m_fitCurve->fitResult();
	if (!fitResult.available) {
		uiGeneralTab.teResult->clear();
		uiGeneralTab.teLog->clear();
		return;
	}

	//const XYFitCurve::FitData& fitData = m_fitCurve->fitData();

	const int np = fitResult.paramValues.size();
	const double rsquare = nsl_stats_rsquare(fitResult.sse,fitResult.sst);
	const double rsquareAdj = nsl_stats_rsquareAdj(rsquare, np, fitResult.dof);

	showFitResultSummary(fitResult);
	showFitResultLog(fitResult);

	// General
	uiGeneralTab.twGeneral->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	uiGeneralTab.twGeneral->item(0, 1)->setText(fitResult.status);

	uiGeneralTab.twGeneral->item(1, 1)->setText(QString::number(fitResult.iterations));
	uiGeneralTab.twGeneral->item(2, 1)->setText(QString::number(m_fitData.eps));
	if (fitResult.elapsedTime > 1000)
		uiGeneralTab.twGeneral->item(3, 1)->setText(QString::number(fitResult.elapsedTime/1000) + " s");
	else
		uiGeneralTab.twGeneral->item(3, 1)->setText(QString::number(fitResult.elapsedTime) + " ms");

	uiGeneralTab.twGeneral->item(4, 1)->setText(QString::number(fitResult.dof));
	uiGeneralTab.twGeneral->item(5, 1)->setText(QString::number(fitResult.paramValues.size()));
	uiGeneralTab.twGeneral->item(6, 1)->setText(QString::number(uiGeneralTab.sbMin->value()) + " .. " + QString::number(uiGeneralTab.sbMax->value()) );

	// Parameters
	uiGeneralTab.twParameters->setRowCount(np);
	//uiGeneralTab.twParameters->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	QStringList headerLabels;
	headerLabels << i18n("Name") << i18n("Value") << i18n("Error") << i18n("Error, %") << i18n("t statistic") << i18n("P > |t|") << i18n("Conf. Interval");
	uiGeneralTab.twParameters->setHorizontalHeaderLabels(headerLabels);
	for (int i = 0; i < np; i++) {
		QTableWidgetItem *item = new QTableWidgetItem(m_fitData.paramNamesUtf8.at(i));
		uiGeneralTab.twParameters->setItem(i, 0, item);
		item = new QTableWidgetItem(QString::number(fitResult.paramValues.at(i)));
		uiGeneralTab.twParameters->setItem(i, 1, item);
		if (!m_fitData.paramFixed.at(i)) {
			item = new QTableWidgetItem(QString::number(fitResult.errorValues.at(i), 'g', 6));
			uiGeneralTab.twParameters->setItem(i, 2, item);
			item = new QTableWidgetItem(QString::number(100.*fitResult.errorValues.at(i)/fabs(fitResult.paramValues.at(i)), 'g', 3));
			uiGeneralTab.twParameters->setItem(i, 3, item);

			// t values
			const double t = nsl_stats_tdist_t(fitResult.paramValues.at(i), fitResult.errorValues.at(i));
			item = new QTableWidgetItem(QString::number(t, 'g', 3));
			uiGeneralTab.twParameters->setItem(i, 4, item);

			// p values
			const double p = nsl_stats_tdist_p(t, fitResult.dof);
			item = new QTableWidgetItem(QString::number(p, 'g', 3));
			// color p values depending on value
			if (p > 0.05)
				item->setTextColor(Qt::red);
			else if (p > 0.01)
				item->setTextColor(Qt::darkGreen);
			else if (p > 0.001)
				item->setTextColor(Qt::darkCyan);
			else if (p > 0.0001)
				item->setTextColor(Qt::blue);
			else
				item->setTextColor(Qt::darkBlue);
			uiGeneralTab.twParameters->setItem(i, 5, item);

			// Conf. interval
			const double margin = nsl_stats_tdist_margin(0.05, fitResult.dof, fitResult.errorValues.at(i));
			item = new QTableWidgetItem(QString::number(fitResult.paramValues.at(i) - margin)
				+ " .. " + QString::number(fitResult.paramValues.at(i) + margin));
			uiGeneralTab.twParameters->setItem(i, 6, item);
		}
	}

	// Goodness of fit
	uiGeneralTab.twGoodness->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
	uiGeneralTab.twGoodness->item(0, 2)->setText(QString::number(fitResult.sse));

	if (fitResult.dof != 0) {
		uiGeneralTab.twGoodness->item(1, 2)->setText(QString::number(fitResult.rms));
		uiGeneralTab.twGoodness->item(2, 2)->setText(QString::number(fitResult.rsd));

		uiGeneralTab.twGoodness->item(3, 2)->setText(QString::number(rsquare, 'g', 15));
		uiGeneralTab.twGoodness->item(4, 2)->setText(QString::number(rsquareAdj, 'g', 15));

		// chi^2 and F test p-values
		double p = nsl_stats_chisq_p(fitResult.sse, fitResult.dof);
		uiGeneralTab.twGoodness->item(5, 2)->setText(QString::number(p, 'g', 3));
		double F = nsl_stats_fdist_F(fitResult.sst, fitResult.rms);
		uiGeneralTab.twGoodness->item(6, 2)->setText(QString::number(F, 'g', 3));
		p = nsl_stats_fdist_p(F, np, fitResult.dof);
		uiGeneralTab.twGoodness->item(7, 2)->setText(QString::number(p, 'g', 3));
	}

	uiGeneralTab.twGoodness->item(8, 2)->setText(QString::number(fitResult.mae));

	//enable the "recalculate"-button if the source data was changed since the last fit
	uiGeneralTab.pbRecalculate->setEnabled(m_fitCurve->isSourceDataChangedSinceLastRecalc());
}

//*************************************************************
//*********** SLOTs for changes triggered in XYCurve **********
//*************************************************************
//General-Tab
void XYFitCurveDock::curveDescriptionChanged(const AbstractAspect* aspect) {
	if (m_curve != aspect)
		return;

	m_initializing = true;
	if (aspect->name() != uiGeneralTab.leName->text()) {
		uiGeneralTab.leName->setText(aspect->name());
	} else if (aspect->comment() != uiGeneralTab.leComment->text()) {
		uiGeneralTab.leComment->setText(aspect->comment());
	}
	m_initializing = false;
}

void XYFitCurveDock::curveDataSourceTypeChanged(XYCurve::DataSourceType type) {
	m_initializing = true;
	uiGeneralTab.cbDataSourceType->setCurrentIndex(type);
	m_initializing = false;
}

void XYFitCurveDock::curveDataSourceCurveChanged(const XYCurve* curve) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbDataSourceCurve, curve);
	m_initializing = false;
}

void XYFitCurveDock::curveXDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbXDataColumn, column);
	m_initializing = false;
}

void XYFitCurveDock::curveYDataColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbYDataColumn, column);
	m_initializing = false;
}

void XYFitCurveDock::curveXErrorColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbXErrorColumn, column);
	m_initializing = false;
}

void XYFitCurveDock::curveYErrorColumnChanged(const AbstractColumn* column) {
	m_initializing = true;
	XYCurveDock::setModelIndexFromAspect(cbYErrorColumn, column);
	uiGeneralTab.lWeight->setEnabled(true);
	uiGeneralTab.cbWeight->setEnabled(true);
	m_initializing = false;
}

void XYFitCurveDock::curveFitDataChanged(const XYFitCurve::FitData& data) {
	m_initializing = true;
	m_fitData = data;
	if (m_fitData.modelCategory == nsl_fit_model_custom)
		uiGeneralTab.teEquation->setPlainText(m_fitData.model);
	else
		uiGeneralTab.cbModel->setCurrentIndex(m_fitData.modelType);

	uiGeneralTab.sbDegree->setValue(m_fitData.degree);
	this->showFitResult();
	m_initializing = false;
}

void XYFitCurveDock::dataChanged() {
	this->enableRecalculate();
}
