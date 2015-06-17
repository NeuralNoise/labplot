/***************************************************************************
    File                 : Plot3D.cpp
    Project              : LabPlot
    Description          : 3D plot
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

#include "Plot3D.h"
#include "Plot3DPrivate.h"
#include "backend/core/AbstractColumn.h"
#include "backend/worksheet/plots/PlotArea.h"
#include "backend/worksheet/TextLabel.h"

#include <QDebug>
#include <QGraphicsItem>
#include <QGraphicsScene>
#include <QGraphicsProxyWidget>
#include <QGraphicsView>
#include <QPainter>
#include <QWidget>
#include <QFileInfo>

#include <KIcon>

#include <QVTKGraphicsItem.h>
#include <vtkSphereSource.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include <vtkLight.h>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkProperty.h>
#include <vtkTextProperty.h>
#include <vtkRendererCollection.h>
#include <vtkOBJReader.h>
#include <vtkSTLReader.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkInteractorStyleTrackballCamera.h>
#include <vtkTriangle.h>
#include <vtkCellArray.h>
#include <vtkOrientationMarkerWidget.h>
#include <vtkCubeAxesActor.h>


Plot3D::Plot3D(const QString& name, QGLContext *context)
	: AbstractPlot(name, new Plot3DPrivate(this, context)){
	qDebug() << Q_FUNC_INFO;
	init();
}

Plot3D::Plot3D(const QString &name, Plot3DPrivate *dd)
	: AbstractPlot(name, dd){
	qDebug() << Q_FUNC_INFO;
	init();
}

void Plot3D::init(){
	m_plotArea = new PlotArea(name() + " plot area");
	addChild(m_plotArea);

	Q_D(Plot3D);
	d->init();
}

Plot3D::~Plot3D(){

}

QIcon Plot3D::icon() const{
	// TODO: Replace by some 3D chart
	return KIcon("office-chart-line");
}

void Plot3D::setRect(const QRectF &rect){
	Q_D(Plot3D);
	d->rect = rect;
	d->retransform();
}

void Plot3D::setVisualizationType(VisualizationType type){
	Q_D(Plot3D);
	d->visType = type;
	d->isChanged = true;
}

void Plot3D::setDataSource(DataSource source){
	Q_D(Plot3D);
	d->sourceType = source;
	d->isChanged = true;
	if (source != DataSource_Spreadsheet){
		setXColumn(0);
		setYColumn(0);
		setZColumn(0);
	}
}

void Plot3D::setFile(const KUrl& path){
	Q_D(Plot3D);
	d->path = path;
	d->isChanged = true;
}

void Plot3D::setXColumn(AbstractColumn *column){
	qDebug() << Q_FUNC_INFO;
	Q_D(Plot3D);
	d->xColumn= column;
}

void Plot3D::setYColumn(AbstractColumn *column){
	qDebug() << Q_FUNC_INFO;
	Q_D(Plot3D);
	d->yColumn= column;
}

void Plot3D::setZColumn(AbstractColumn *column){
	qDebug() << Q_FUNC_INFO;
	Q_D(Plot3D);
	d->zColumn= column;
}

void Plot3D::setNodeColumn(int node, AbstractColumn* column){
	qDebug() << Q_FUNC_INFO;
	if (node >= 0 && node < 3){
		Q_D(Plot3D);
		d->nodeColumn[node] = column;
	}
}

void Plot3D::retransform(){
	Q_D(Plot3D);
	d->retransform();
	WorksheetElementContainer::retransform();
}

/////////////////////////////////////////////////////////////////////////////

Plot3DPrivate::Plot3DPrivate(Plot3D* owner, QGLContext *context)
	: AbstractPlotPrivate(owner)
	, q_ptr(owner)
	, context(context)
	, visType(Plot3D::VisualizationType_Triangles)
	, sourceType(Plot3D::DataSource_Empty)
	, isChanged(false)
	, xColumn(0)
	, yColumn(0)
	, zColumn(0){

	for (int i = 0; i < 3; ++i){
		nodeColumn[i] = 0;
	}
}

Plot3DPrivate::~Plot3DPrivate(){
}

void Plot3DPrivate::init(){
	Q_Q(Plot3D);
	vtkItem = new QVTKGraphicsItem(context, q->plotArea()->graphicsItem());

	vtkGenericOpenGLRenderWindow *renderWindow = vtkItem->GetRenderWindow();

	renderer = vtkSmartPointer<vtkRenderer>::New();
	renderWindow->AddRenderer(renderer);

	vtkSmartPointer<vtkInteractorStyleTrackballCamera> style = vtkSmartPointer<vtkInteractorStyleTrackballCamera>::New();

	renderWindow->GetInteractor()->SetInteractorStyle(style);

	vtkSmartPointer<vtkLight> light = vtkSmartPointer<vtkLight>::New();
	light->SetFocalPoint(1.875, 0.6125, 0);
	light->SetPosition(0.875, 1.6125, 1);
	renderer->AddLight(light);

	addAxes();
}

void Plot3DPrivate::clearActors(){
	foreach(vtkActor *actor, actors){
		renderer->RemoveActor(actor);
	}
	actors.clear();
}

void Plot3DPrivate::addSphere(){
	vtkSmartPointer<vtkSphereSource> sphereSource = vtkSmartPointer<vtkSphereSource>::New();
	sphereSource->Update();
	vtkSmartPointer<vtkPolyDataMapper> sphereMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	sphereMapper->SetInputConnection(sphereSource->GetOutputPort());
	vtkSmartPointer<vtkActor> sphereActor = vtkSmartPointer<vtkActor>::New();
	sphereActor->GetProperty()->SetFrontfaceCulling(true);
	sphereActor->SetMapper(sphereMapper);
	axes->SetBounds(sphereSource->GetOutput()->GetBounds());
	renderer->AddActor(sphereActor);
	actors.push_back(sphereActor);
}

namespace{
	template<class TReader>
	vtkSmartPointer<TReader> createReaderImpl(const KUrl& filePath){
		const QByteArray ascii = filePath.path().toAscii();
		const char *path = ascii.constData();
		vtkSmartPointer<TReader> reader = vtkSmartPointer<TReader>::New();
		reader->SetFileName(path);
		reader->Update();
		return reader;
	}
}

vtkSmartPointer<vtkPolyDataAlgorithm> Plot3DPrivate::createReader(const KUrl& filePath) const{
	const QString& fileName = filePath.fileName();
	qDebug() << Q_FUNC_INFO << "Read from the file:" << filePath.path().toAscii();
	const QString& fileType = fileName.split('.').last().toLower();

	if (fileType == "obj"){
		qDebug() << Q_FUNC_INFO << "Create obj reader";
		return createReaderImpl<vtkOBJReader>(filePath);
	} else if (fileType == "stl"){
		qDebug() << Q_FUNC_INFO << "Create STL reader";
		return createReaderImpl<vtkSTLReader>(filePath);
	}

	return vtkSmartPointer<vtkPolyDataAlgorithm>();
}

void Plot3DPrivate::readFromFile(){
	vtkSmartPointer<vtkPolyDataAlgorithm> reader = createReader(path);
	if (reader.Get() == 0)
		return;

	vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	mapper->SetInputConnection(reader->GetOutputPort());

	axes->SetBounds(reader->GetOutput()->GetBounds());

	vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
	actor->SetMapper(mapper);

	renderer->AddActor(actor);
	actors.push_back(actor);
}

void Plot3DPrivate::addAxes(){
	axes = vtkSmartPointer<vtkCubeAxesActor>::New();

	axes->SetCamera(renderer->GetActiveCamera());
	axes->DrawXGridlinesOn();
	axes->DrawYGridlinesOn();
	axes->DrawZGridlinesOn();

	const double colors[][3] = {{1.0, 0.0, 0.0}, {0.0, 1.0, 0.0}, {0.0, 0.0, 1.0}};

	const int fontSize = 72;
	for (int i = 0; i < 3; ++i){
		vtkTextProperty *titleProp = axes->GetTitleTextProperty(i);
		titleProp->SetColor(colors[i][0], colors[i][1], colors[i][2]);
		titleProp->SetBold(true);
		titleProp->SetFontSize(fontSize);

		vtkTextProperty *labelProp = axes->GetLabelTextProperty(i);
		labelProp->SetColor(colors[i][0], colors[i][1], colors[i][2]);
		labelProp->SetBold(true);
		labelProp->SetFontSize(fontSize);
	}

	axes->GetXAxesLinesProperty()->SetLineWidth(10);
	axes->GetYAxesLinesProperty()->SetLineWidth(10);
	axes->GetZAxesLinesProperty()->SetLineWidth(10);
	axes->GetAxisLabels(0);

	renderer->AddActor(axes);
}

void Plot3DPrivate::readFromColumns(){
	if (xColumn == 0 || yColumn == 0 || zColumn == 0){
		return;
	}

	for (int i = 0; i < 3; ++i)
		if (nodeColumn[i] == 0){
			qDebug() << Q_FUNC_INFO << "Node" << i << "== 0";
			return;
		}

	if (visType == Plot3D::VisualizationType_Triangles){
		qDebug() << Q_FUNC_INFO << "Triangles rendering";
		vtkSmartPointer<vtkCellArray> triangles = vtkSmartPointer<vtkCellArray>::New();
		vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();

		qDebug() << Q_FUNC_INFO << "Row count:" << xColumn->rowCount() << yColumn->rowCount() << zColumn->rowCount();

		const int numPoints = std::min(xColumn->rowCount(), std::min(yColumn->rowCount(), zColumn->rowCount()));
		for (int i = 0; i < numPoints; ++i){
			const int x = static_cast<int>(xColumn->valueAt(i));
			const int y = static_cast<int>(yColumn->valueAt(i));
			const int z = static_cast<int>(zColumn->valueAt(i));

			points->InsertNextPoint(x, y, z);
		}

		const int numTrianges = std::min(nodeColumn[0]->rowCount(), std::min(nodeColumn[1]->rowCount(), nodeColumn[2]->rowCount()));
		for (int i = 0; i < numTrianges; ++i){
			vtkSmartPointer<vtkTriangle> triangle = vtkSmartPointer<vtkTriangle>::New();
			const int id1 = static_cast<int>(nodeColumn[0]->valueAt(i));
			const int id2 = static_cast<int>(nodeColumn[1]->valueAt(i));
			const int id3 = static_cast<int>(nodeColumn[2]->valueAt(i));

			if (id1 < 1 || id2 < 1 || id3 < 1 || id1 > numPoints || id2 > numPoints || id3 > numPoints)
				// TODO: Return error
				continue;

			triangle->GetPointIds()->SetId(0, id1);
			triangle->GetPointIds()->SetId(1, id2);
			triangle->GetPointIds()->SetId(2, id3);

			triangles->InsertNextCell(triangle);
		}

		qDebug() << Q_FUNC_INFO << "Amount of triangles:" << triangles->GetSize();

		vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();

		polydata->SetPoints(points);
		polydata->SetPolys(triangles);

		axes->SetBounds(polydata->GetBounds());
		vtkSmartPointer<vtkPolyDataMapper> mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
		mapper->SetInputData(polydata);

		vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
		actor->SetMapper(mapper);

		renderer->AddActor(actor);
		actors.push_back(actor);
	}
}

void Plot3DPrivate::retransform(){
	prepareGeometryChange();
	setPos(rect.x()+rect.width()/2, rect.y()+rect.height()/2);

	//plotArea position is always (0, 0) in parent's coordinates, don't need to update here
	Q_Q(Plot3D);
	vtkItem->setGeometry(q->plotArea()->rect());
	q->plotArea()->setRect(rect);

	if (isChanged){
		clearActors();
		if (sourceType == Plot3D::DataSource_Empty){
			qDebug() << Q_FUNC_INFO << "Add Sphere";
			addSphere();
		}else if(sourceType == Plot3D::DataSource_File){
			qDebug() << Q_FUNC_INFO << "Read file";
			readFromFile();
		}else if(sourceType == Plot3D::DataSource_Spreadsheet){
			readFromColumns();
		}

		isChanged = false;
	}

	WorksheetElementContainerPrivate::recalcShapeAndBoundingRect();
}