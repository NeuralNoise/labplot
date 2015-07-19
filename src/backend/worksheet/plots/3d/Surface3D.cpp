/***************************************************************************
    File                 : Surface3D.cpp
    Project              : LabPlot
    Description          : 3D surface class
    --------------------------------------------------------------------
    Copyright            : (C) 2015 by Minh Ngo (minh@fedoraproject.org)

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

#include "Surface3D.h"
#include "Surface3DPrivate.h"
#include "Plot3D.h"
#include "backend/lib/commandtemplates.h"

#include <QDebug>

#include <KLocale>

#include <vtkRenderer.h>

Surface3D::Surface3D(vtkRenderer& renderer)
	: AbstractAspect("Surface")
	, d_ptr(new Surface3DPrivate(renderer, this)) {
}

void Surface3D::init() {
	Q_D(Surface3D);
	d->init();
}

Surface3D::~Surface3D() {
}

DemoDataHandler& Surface3D::demoDataHandler() {
	Q_D(Surface3D);
	return *d->demoHandler;
}

SpreadsheetDataHandler& Surface3D::spreadsheetDataHandler() {
	Q_D(Surface3D);
	return *d->spreadsheetHandler;
}

MatrixDataHandler& Surface3D::matrixDataHandler() {
	Q_D(Surface3D);
	return *d->matrixHandler;
}

FileDataHandler& Surface3D::fileDataHandler() {
	Q_D(Surface3D);
	return *d->fileHandler;
}

void Surface3D::remove(){
	AbstractAspect::remove();
	Q_D(Surface3D);
	d->hide();
	emit parametersChanged();
}

void Surface3D::update() {
	Q_D(Surface3D);
	d->update();
}

//##############################################################################
//##########################  getter methods  ##################################
//##############################################################################

BASIC_SHARED_D_READER_IMPL(Surface3D, Surface3D::VisualizationType, visualizationType, visualizationType)
BASIC_SHARED_D_READER_IMPL(Surface3D, Surface3D::DataSource, dataSource, sourceType)

//##############################################################################
//#################  setter methods and undo commands ##########################
//##############################################################################

STD_SETTER_CMD_IMPL_F_S(Surface3D, SetVisualizationType, Surface3D::VisualizationType, visualizationType, update)
void Surface3D::setVisualizationType(VisualizationType type) {
	Q_D(Surface3D);
	if (type != d->visualizationType)
		exec(new Surface3DSetVisualizationTypeCmd(d, type, i18n("%1: visualization type changed")));
}

STD_SETTER_CMD_IMPL_F_S(Surface3D, SetDataSource, Surface3D::DataSource, sourceType, update)
void Surface3D::setDataSource(DataSource source) {
	Q_D(Surface3D);
	if (source != d->sourceType)
		exec(new Surface3DSetDataSourceCmd(d, source, i18n("%1: data source type changed")));
}

////////////////////////////////////////////////////////////////////////////////

Surface3DPrivate::Surface3DPrivate(vtkRenderer& renderer, Surface3D *parent)
	: q(parent)
	, renderer(renderer)
	, visualizationType(Surface3D::VisualizationType_Triangles)
	, sourceType(Surface3D::Surface3D::DataSource_Empty) {
	init();
}

void Surface3DPrivate::init() {
	demoHandler = new DemoDataHandler;
	q->addChild(demoHandler);
	demoHandler->setHidden(true);
	
	spreadsheetHandler = new SpreadsheetDataHandler;
	q->addChild(spreadsheetHandler);
	spreadsheetHandler->setHidden(true);
	
	matrixHandler = new MatrixDataHandler;
	q->addChild(matrixHandler);
	matrixHandler->setHidden(true);

	fileHandler = new FileDataHandler;
	q->addChild(fileHandler);
	fileHandler->setHidden(true);

	update();
	q->connect(demoHandler, SIGNAL(parametersChanged()), SLOT(update()));
	q->connect(spreadsheetHandler, SIGNAL(parametersChanged()), SLOT(update()));
	q->connect(matrixHandler, SIGNAL(parametersChanged()), SLOT(update()));
	q->connect(fileHandler, SIGNAL(parametersChanged()), SLOT(update()));
}

Surface3DPrivate::~Surface3DPrivate() {
	hide();
}

QString Surface3DPrivate::name() const {
	return i18n("3D Surface");
}

void Surface3DPrivate::hide() {
	if (surfaceActor)
		renderer.RemoveActor(surfaceActor);
}

void Surface3DPrivate::update() {
	hide();
	if (sourceType == Surface3D::DataSource_Empty) {
		surfaceActor = demoHandler->actor(visualizationType);
	} else if (sourceType == Surface3D::DataSource_File) {
		surfaceActor = fileHandler->actor(visualizationType);
	} else if (sourceType == Surface3D::DataSource_Matrix) {
		surfaceActor = matrixHandler->actor(visualizationType);
	} else if (sourceType == Surface3D::DataSource_Spreadsheet) {
		surfaceActor = spreadsheetHandler->actor(visualizationType);
	}

	renderer.AddActor(surfaceActor);
	emit q->parametersChanged();
}