set(vtkRenderingQt_LOADED 1)
set(vtkRenderingQt_DEPENDS "vtkFiltersSources;vtkFiltersTexture;vtkGUISupportQt;vtkRenderingLabel")
set(vtkRenderingQt_LIBRARIES "vtkRenderingQt")
set(vtkRenderingQt_INCLUDE_DIRS "${VTK_INSTALL_PREFIX}/include/vtk-6.3")
set(vtkRenderingQt_LIBRARY_DIRS "")
set(vtkRenderingQt_RUNTIME_LIBRARY_DIRS "${VTK_INSTALL_PREFIX}/lib")
set(vtkRenderingQt_WRAP_HIERARCHY_FILE "${CMAKE_CURRENT_LIST_DIR}/vtkRenderingQtHierarchy.txt")

if(NOT Qt5Widgets_DIR)
  set(Qt5Widgets_DIR "/opt/el7/pkgs/crkit/nightly/20180703/crkit/qt-5.6.2/lib/cmake/Qt5Widgets")
endif()
find_package(Qt5Widgets REQUIRED QUIET)

