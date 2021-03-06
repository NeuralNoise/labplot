find_package(SharedMimeInfo REQUIRED)
set(KDE_FRONTEND true)
set(KDEFRONTEND_DIR kdefrontend)
set(BACKEND_DIR backend)
set(COMMONFRONTEND_DIR commonfrontend)
set(CANTOR_DIR cantor)
set(TOOLS_DIR tools)
set(CMAKE_AUTOMOC ON)

set(GUI_SOURCES
	${KDEFRONTEND_DIR}/GuiObserver.cpp
	${KDEFRONTEND_DIR}/GuiTools.cpp
	${KDEFRONTEND_DIR}/HistoryDialog.cpp
	${KDEFRONTEND_DIR}/LabPlot.cpp
	${KDEFRONTEND_DIR}/MainWin.cpp
	${KDEFRONTEND_DIR}/SettingsDialog.cpp
	${KDEFRONTEND_DIR}/SettingsGeneralPage.cpp
	${KDEFRONTEND_DIR}/SettingsWorksheetPage.cpp
	${KDEFRONTEND_DIR}/SettingsPage.h
	${KDEFRONTEND_DIR}/TemplateHandler.cpp
	${KDEFRONTEND_DIR}/ThemeHandler.cpp
	${KDEFRONTEND_DIR}/datasources/AsciiOptionsWidget.cpp
	${KDEFRONTEND_DIR}/datasources/BinaryOptionsWidget.cpp
	${KDEFRONTEND_DIR}/datasources/DatabaseManagerDialog.cpp
	${KDEFRONTEND_DIR}/datasources/DatabaseManagerWidget.cpp
	${KDEFRONTEND_DIR}/datasources/HDFOptionsWidget.cpp
	${KDEFRONTEND_DIR}/datasources/FileInfoDialog.cpp
	${KDEFRONTEND_DIR}/datasources/ImageOptionsWidget.cpp
	${KDEFRONTEND_DIR}/datasources/ImportDialog.cpp
	${KDEFRONTEND_DIR}/datasources/ImportFileWidget.cpp
	${KDEFRONTEND_DIR}/datasources/ImportFileDialog.cpp
	${KDEFRONTEND_DIR}/datasources/ImportSQLDatabaseDialog.cpp
	${KDEFRONTEND_DIR}/datasources/ImportSQLDatabaseWidget.cpp
	${KDEFRONTEND_DIR}/datasources/NetCDFOptionsWidget.cpp
	${KDEFRONTEND_DIR}/datasources/FITSOptionsWidget.cpp
	${KDEFRONTEND_DIR}/dockwidgets/AxisDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/NoteDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/CartesianPlotDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/CartesianPlotLegendDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/CustomPointDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/ColumnDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/MatrixDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/ProjectDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/SpreadsheetDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/XYCurveDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/XYEquationCurveDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/XYDataReductionCurveDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/XYDifferentiationCurveDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/XYIntegrationCurveDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/XYInterpolationCurveDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/XYSmoothCurveDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/XYFitCurveDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/XYFourierFilterCurveDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/XYFourierTransformCurveDock.cpp
	${KDEFRONTEND_DIR}/dockwidgets/WorksheetDock.cpp
	${KDEFRONTEND_DIR}/matrix/MatrixFunctionDialog.cpp
	${KDEFRONTEND_DIR}/spreadsheet/PlotDataDialog.cpp
	${KDEFRONTEND_DIR}/spreadsheet/EquidistantValuesDialog.cpp
	${KDEFRONTEND_DIR}/spreadsheet/ExportSpreadsheetDialog.cpp
	${KDEFRONTEND_DIR}/spreadsheet/DropValuesDialog.cpp
	${KDEFRONTEND_DIR}/spreadsheet/FunctionValuesDialog.cpp
	${KDEFRONTEND_DIR}/spreadsheet/RandomValuesDialog.cpp
	${KDEFRONTEND_DIR}/spreadsheet/SortDialog.cpp
	${KDEFRONTEND_DIR}/spreadsheet/StatisticsDialog.cpp
	${KDEFRONTEND_DIR}/worksheet/ExportWorksheetDialog.cpp
	${KDEFRONTEND_DIR}/worksheet/GridDialog.cpp
	${KDEFRONTEND_DIR}/worksheet/DynamicPresenterWidget.cpp
	${KDEFRONTEND_DIR}/worksheet/PresenterWidget.cpp
	${KDEFRONTEND_DIR}/worksheet/SlidingPanel.cpp
	${KDEFRONTEND_DIR}/widgets/ConstantsWidget.cpp
	${KDEFRONTEND_DIR}/widgets/ThemesComboBox.cpp
	${KDEFRONTEND_DIR}/widgets/ThemesWidget.cpp
	${KDEFRONTEND_DIR}/widgets/ExpressionTextEdit.cpp
	${KDEFRONTEND_DIR}/widgets/FitOptionsWidget.cpp
	${KDEFRONTEND_DIR}/widgets/FitParametersWidget.cpp
	${KDEFRONTEND_DIR}/widgets/FunctionsWidget.cpp
	${KDEFRONTEND_DIR}/widgets/LabelWidget.cpp
	${KDEFRONTEND_DIR}/widgets/DatapickerImageWidget.cpp
	${KDEFRONTEND_DIR}/widgets/DatapickerCurveWidget.cpp
	${KDEFRONTEND_DIR}/widgets/FITSHeaderEditWidget.cpp
	${KDEFRONTEND_DIR}/widgets/FITSHeaderEditNewKeywordDialog.cpp
	${KDEFRONTEND_DIR}/widgets/FITSHeaderEditAddUnitDialog.cpp
	${KDEFRONTEND_DIR}/widgets/FITSHeaderEditDialog.cpp
)
IF (USE_LIBORIGIN)
	LIST(APPEND GUI_SOURCES ${KDEFRONTEND_DIR}/datasources/ImportOpj.cpp)
ENDIF ()

set(UI_SOURCES
	${KDEFRONTEND_DIR}/ui/constantswidget.ui
	${KDEFRONTEND_DIR}/ui/functionswidget.ui
	${KDEFRONTEND_DIR}/ui/fitoptionswidget.ui
	${KDEFRONTEND_DIR}/ui/fitparameterswidget.ui
	${KDEFRONTEND_DIR}/ui/labelwidget.ui
	${KDEFRONTEND_DIR}/ui/settingsgeneralpage.ui
	${KDEFRONTEND_DIR}/ui/settingsworksheetpage.ui
	${KDEFRONTEND_DIR}/ui/settingsprintingpage.ui
	${KDEFRONTEND_DIR}/ui/datasources/asciioptionswidget.ui
	${KDEFRONTEND_DIR}/ui/datasources/binaryoptionswidget.ui
	${KDEFRONTEND_DIR}/ui/datasources/databasemanagerwidget.ui
	${KDEFRONTEND_DIR}/ui/datasources/hdfoptionswidget.ui
	${KDEFRONTEND_DIR}/ui/datasources/imageoptionswidget.ui
	${KDEFRONTEND_DIR}/ui/datasources/importfilewidget.ui
	${KDEFRONTEND_DIR}/ui/datasources/importsqldatabasewidget.ui
	${KDEFRONTEND_DIR}/ui/datasources/netcdfoptionswidget.ui
	${KDEFRONTEND_DIR}/ui/datasources/fitsoptionswidget.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/axisdock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/cartesianplotdock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/cartesianplotlegenddock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/columndock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/custompointdock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/notedock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/matrixdock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/projectdock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/spreadsheetdock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xycurvedock.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xycurvedockgeneraltab.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xydatareductioncurvedockgeneraltab.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xydifferentiationcurvedockgeneraltab.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xyintegrationcurvedockgeneraltab.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xyinterpolationcurvedockgeneraltab.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xysmoothcurvedockgeneraltab.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xyfitcurvedockgeneraltab.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xyfourierfiltercurvedockgeneraltab.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xyfouriertransformcurvedockgeneraltab.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/xyequationcurvedockgeneraltab.ui
	${KDEFRONTEND_DIR}/ui/dockwidgets/worksheetdock.ui
	${KDEFRONTEND_DIR}/ui/matrix/matrixfunctionwidget.ui
	${KDEFRONTEND_DIR}/ui/spreadsheet/plotdatawidget.ui
	${KDEFRONTEND_DIR}/ui/spreadsheet/equidistantvalueswidget.ui
	${KDEFRONTEND_DIR}/ui/spreadsheet/exportspreadsheetwidget.ui
	${KDEFRONTEND_DIR}/ui/spreadsheet/dropvalueswidget.ui
	${KDEFRONTEND_DIR}/ui/spreadsheet/functionvalueswidget.ui
	${KDEFRONTEND_DIR}/ui/spreadsheet/randomvalueswidget.ui
	${KDEFRONTEND_DIR}/ui/worksheet/exportworksheetwidget.ui
	${KDEFRONTEND_DIR}/ui/datapickerimagewidget.ui
	${KDEFRONTEND_DIR}/ui/datapickercurvewidget.ui
	${KDEFRONTEND_DIR}/ui/fitsheadereditwidget.ui
	${KDEFRONTEND_DIR}/ui/fitsheadereditnewkeywordwidget.ui
	${KDEFRONTEND_DIR}/ui/fitsheadereditaddunitwidget.ui
)

set(BACKEND_SOURCES
	${BACKEND_DIR}/core/Folder.cpp
	${BACKEND_DIR}/core/AbstractAspect.cpp
	${BACKEND_DIR}/core/AbstractColumn.cpp
	${BACKEND_DIR}/core/AbstractColumnPrivate.cpp
	${BACKEND_DIR}/core/abstractcolumncommands.cpp
	${BACKEND_DIR}/core/AbstractFilter.cpp
	${BACKEND_DIR}/core/AbstractSimpleFilter.cpp
	${BACKEND_DIR}/core/column/Column.cpp
	${BACKEND_DIR}/core/column/ColumnPrivate.cpp
	${BACKEND_DIR}/core/column/ColumnStringIO.cpp
	${BACKEND_DIR}/core/column/columncommands.cpp
	${BACKEND_DIR}/core/AbstractScriptingEngine.cpp
	${BACKEND_DIR}/core/AbstractScript.cpp
	${BACKEND_DIR}/core/ScriptingEngineManager.cpp
	${BACKEND_DIR}/core/Project.cpp
	${BACKEND_DIR}/core/AbstractPart.cpp
	${BACKEND_DIR}/core/Workbook.cpp
	${BACKEND_DIR}/core/AspectTreeModel.cpp
	${BACKEND_DIR}/core/datatypes/DateTime2DoubleFilter.h
	${BACKEND_DIR}/core/datatypes/DayOfWeek2DoubleFilter.h
	${BACKEND_DIR}/core/datatypes/Double2DateTimeFilter.h
	${BACKEND_DIR}/core/datatypes/Double2DayOfWeekFilter.h
	${BACKEND_DIR}/core/datatypes/Double2MonthFilter.h
	${BACKEND_DIR}/core/datatypes/Month2DoubleFilter.h
	${BACKEND_DIR}/core/datatypes/SimpleCopyThroughFilter.h
	${BACKEND_DIR}/core/datatypes/String2DayOfWeekFilter.h
	${BACKEND_DIR}/core/datatypes/String2DoubleFilter.h
	${BACKEND_DIR}/core/datatypes/String2MonthFilter.h
	${BACKEND_DIR}/core/datatypes/DateTime2StringFilter.cpp
	${BACKEND_DIR}/core/datatypes/String2DateTimeFilter.cpp
	${BACKEND_DIR}/core/datatypes/Double2StringFilter.cpp
	${BACKEND_DIR}/core/plugin/PluginLoader.cpp
	${BACKEND_DIR}/core/plugin/PluginManager.cpp
	${BACKEND_DIR}/datasources/AbstractDataSource.cpp
	${BACKEND_DIR}/datasources/FileDataSource.cpp
	${BACKEND_DIR}/datasources/filters/AbstractFileFilter.cpp
	${BACKEND_DIR}/datasources/filters/AsciiFilter.cpp
	${BACKEND_DIR}/datasources/filters/BinaryFilter.cpp
	${BACKEND_DIR}/datasources/filters/HDFFilter.cpp
	${BACKEND_DIR}/datasources/filters/ImageFilter.cpp
	${BACKEND_DIR}/datasources/filters/NetCDFFilter.cpp
	${BACKEND_DIR}/datasources/filters/FITSFilter.cpp

	${BACKEND_DIR}/gsl/ExpressionParser.cpp
	${BACKEND_DIR}/matrix/Matrix.cpp
	${BACKEND_DIR}/matrix/matrixcommands.cpp
	${BACKEND_DIR}/matrix/MatrixModel.cpp
 	${BACKEND_DIR}/nsl/nsl_dft.c
 	${BACKEND_DIR}/nsl/nsl_diff.c
	${BACKEND_DIR}/nsl/nsl_filter.c
	${BACKEND_DIR}/nsl/nsl_fit.c
	${BACKEND_DIR}/nsl/nsl_geom.c
	${BACKEND_DIR}/nsl/nsl_geom_linesim.c
	${BACKEND_DIR}/nsl/nsl_int.c
	${BACKEND_DIR}/nsl/nsl_interp.c
	${BACKEND_DIR}/nsl/nsl_sf_kernel.c
	${BACKEND_DIR}/nsl/nsl_sf_poly.c
	${BACKEND_DIR}/nsl/nsl_sf_stats.c
	${BACKEND_DIR}/nsl/nsl_sf_window.c
	${BACKEND_DIR}/nsl/nsl_smooth.c
	${BACKEND_DIR}/nsl/nsl_sort.c
	${BACKEND_DIR}/nsl/nsl_stats.c
	${BACKEND_DIR}/spreadsheet/Spreadsheet.cpp
	${BACKEND_DIR}/spreadsheet/SpreadsheetModel.cpp
	${BACKEND_DIR}/lib/XmlStreamReader.cpp
	${BACKEND_DIR}/note/Note.cpp
	${BACKEND_DIR}/worksheet/WorksheetElement.cpp
	${BACKEND_DIR}/worksheet/TextLabel.cpp
	${BACKEND_DIR}/worksheet/Worksheet.cpp
	${BACKEND_DIR}/worksheet/WorksheetElementContainer.cpp
	${BACKEND_DIR}/worksheet/WorksheetElementGroup.cpp
	${BACKEND_DIR}/worksheet/plots/AbstractPlot.cpp
	${BACKEND_DIR}/worksheet/plots/AbstractCoordinateSystem.cpp
	${BACKEND_DIR}/worksheet/plots/PlotArea.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/Axis.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/CartesianCoordinateSystem.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/CartesianPlot.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/CartesianPlotLegend.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/CustomPoint.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/Symbol.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYEquationCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYDataReductionCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYDifferentiationCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYIntegrationCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYInterpolationCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYSmoothCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYFitCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYFourierFilterCurve.cpp
	${BACKEND_DIR}/worksheet/plots/cartesian/XYFourierTransformCurve.cpp
	${BACKEND_DIR}/lib/SignallingUndoCommand.cpp
	${BACKEND_DIR}/datapicker/DatapickerPoint.cpp
	${BACKEND_DIR}/datapicker/DatapickerImage.cpp
	${BACKEND_DIR}/datapicker/Datapicker.cpp
	${BACKEND_DIR}/datapicker/Transform.cpp
	${BACKEND_DIR}/datapicker/ImageEditor.cpp
	${BACKEND_DIR}/datapicker/Segment.cpp
	${BACKEND_DIR}/datapicker/Segments.cpp
	${BACKEND_DIR}/datapicker/DatapickerCurve.cpp
)

set(COMMONFRONTEND_SOURCES
	${COMMONFRONTEND_DIR}/matrix/MatrixView.cpp
	${COMMONFRONTEND_DIR}/note/NoteView.cpp
	${COMMONFRONTEND_DIR}/spreadsheet/SpreadsheetCommentsHeaderModel.cpp
	${COMMONFRONTEND_DIR}/spreadsheet/SpreadsheetHeaderView.cpp
	${COMMONFRONTEND_DIR}/spreadsheet/SpreadsheetItemDelegate.cpp
	${COMMONFRONTEND_DIR}/spreadsheet/SpreadsheetView.cpp
	${COMMONFRONTEND_DIR}/workbook/WorkbookView.cpp
	${COMMONFRONTEND_DIR}/worksheet/WorksheetView.cpp
	${COMMONFRONTEND_DIR}/ProjectExplorer.cpp
	${COMMONFRONTEND_DIR}/core/PartMdiView.cpp
	${COMMONFRONTEND_DIR}/widgets/TreeViewComboBox.cpp
	${COMMONFRONTEND_DIR}/widgets/qxtspanslider.cpp
	${COMMONFRONTEND_DIR}/datapicker/DatapickerView.cpp
	${COMMONFRONTEND_DIR}/datapicker/DatapickerImageView.cpp
)

IF (${CANTOR_LIBS_FOUND})
	set(CANTOR_SOURCES
		${KDEFRONTEND_DIR}/dockwidgets/CantorWorksheetDock.cpp
		${BACKEND_DIR}/cantorWorksheet/VariableParser.cpp
		${BACKEND_DIR}/cantorWorksheet/CantorWorksheet.cpp
		${COMMONFRONTEND_DIR}/cantorWorksheet/CantorWorksheetView.cpp
	)
	set(CANTOR_UI_SOURCES ${KDEFRONTEND_DIR}/ui/dockwidgets/cantorworksheetdock.ui)
	set(UI_SOURCES ${UI_SOURCES} ${CANTOR_UI_SOURCES})
ELSE (${CANTOR_LIBS_FOUND})
	set(CANTOR_SOURCES "")
ENDIF (${CANTOR_LIBS_FOUND})

set(TOOLS_SOURCES
	${TOOLS_DIR}/TeXRenderer.cpp
	${TOOLS_DIR}/EquationHighlighter.cpp
)

bison_target(GslParser
	${BACKEND_DIR}/gsl/parser.y
	${CMAKE_CURRENT_BINARY_DIR}/gsl_parser.c
)
set(GENERATED_SOURCES
	${BISON_GslParser_OUTPUTS}
)

add_subdirectory( pics )
add_subdirectory( pics/gsl_distributions )
add_subdirectory( pics/fit_models )
add_subdirectory( pics/colorchooser )
add_subdirectory( themes )

##############################################################################
set( LABPLOT_SRCS ${GUI_SOURCES} ${PLOTS_SOURCES} )
INCLUDE_DIRECTORIES( . ${BACKEND_DIR}/gsl ${GSL_INCLUDE_DIR} ${GSL_INCLUDEDIR}/.. )
ki18n_wrap_ui( LABPLOT_SRCS ${UI_SOURCES} )
add_executable( labplot2 ${LABPLOT_SRCS} ${BACKEND_SOURCES} ${CANTOR_SOURCES} ${DATASOURCES_SOURCES} ${COMMONFRONTEND_SOURCES} ${TOOLS_SOURCES} ${GENERATED_SOURCES} ${QTMOC_HDRS} )
target_link_libraries( labplot2 KF5::KDELibs4Support KF5::Archive KF5::XmlGui Qt5::Svg ${GSL_LIBRARIES} ${GSL_CBLAS_LIBRARIES} ${QT_QTSQL_LIBRARIES} )
#TODO: KF5::NewStuff
IF (KF5SyntaxHighlighting_FOUND)
	target_link_libraries( labplot2 KF5::SyntaxHighlighting )
ENDIF ()
IF (CANTOR_LIBS_FOUND)
	target_link_libraries( labplot2 ${CANTOR_LIBS} )
ENDIF ()
IF (HDF5_FOUND)
	target_link_libraries( labplot2 ${HDF5_C_LIBRARIES} )
ENDIF ()
IF (FFTW_FOUND)
	target_link_libraries( labplot2 ${FFTW_LIBRARIES} )
ENDIF ()
IF (NETCDF_FOUND)
	target_link_libraries( labplot2 ${NETCDF_LIBRARY} )
ENDIF ()
IF (CFITSIO_FOUND)
	target_link_libraries( labplot2 ${CFITSIO_LIBRARY} )
ENDIF ()
IF (USE_LIBORIGIN)
IF (WIN32)
	IF (("${CMAKE_CXX_COMPILER_ID} x" MATCHES "MSVC") OR MSVC)
	target_link_libraries( labplot2 ${PROJECT_BINARY_DIR}/lib/origin.lib )
	ELSE ()
	target_link_libraries( labplot2 ${PROJECT_BINARY_DIR}/lib/liborigin.a )
	ENDIF ()
ELSE ()
target_link_libraries( labplot2 ${PROJECT_BINARY_DIR}/liborigin/liborigin.a )
ENDIF ()
ENDIF ()

############## installation ################################

install( TARGETS labplot2 DESTINATION ${INSTALL_TARGETS_DEFAULT_ARGS} )
install( FILES ${KDEFRONTEND_DIR}/labplot2ui.rc DESTINATION  ${KXMLGUI_INSTALL_DIR}/${PROJECT_NAME} )
install( FILES ${KDEFRONTEND_DIR}/labplot2ui.rc ${KDEFRONTEND_DIR}/splash.png ${KDEFRONTEND_DIR}/labplot2.ico DESTINATION ${DATA_INSTALL_DIR}/${PROJECT_NAME} )
install( PROGRAMS org.kde.labplot2.desktop DESTINATION ${XDG_APPS_INSTALL_DIR} )
install( FILES labplot2.xml DESTINATION ${XDG_MIME_INSTALL_DIR} )
install( FILES labplot2_themes.knsrc DESTINATION ${CONFIG_INSTALL_DIR} )
update_xdg_mimetypes( ${XDG_MIME_INSTALL_DIR} )
