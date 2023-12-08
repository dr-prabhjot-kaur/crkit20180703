set(vtkIOExodus_LOADED 1)
set(vtkIOExodus_DEPENDS "vtkFiltersGeneral;vtkIOXML;vtkexodusII;vtksys")
set(vtkIOExodus_LIBRARIES "vtkIOExodus")
set(vtkIOExodus_INCLUDE_DIRS "${VTK_INSTALL_PREFIX}/include/vtk-6.3")
set(vtkIOExodus_LIBRARY_DIRS "")
set(vtkIOExodus_RUNTIME_LIBRARY_DIRS "${VTK_INSTALL_PREFIX}/lib")
set(vtkIOExodus_WRAP_HIERARCHY_FILE "${CMAKE_CURRENT_LIST_DIR}/vtkIOExodusHierarchy.txt")

