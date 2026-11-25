"""Writes the reader test files in tests/data with VTK's own writers and numpy.

Run with a Python that has vtk (9.x) and numpy:

    python tests/data/make_test_data.py

The data has values the tests can check (see tests/test_ivfvis_readers.cpp):
  grid:   4 x 3 x 2 points, origin (1, 2, 3), spacing (0.5, 1, 2);
          scalars f = x + 10 y + 100 z, vectors v = (x, y, z) at each point
  sphere: a coarse triangulated sphere with point scalars height = z
  quads:  a 2 x 2 plane of quads (8 triangles), point scalars s = point index
"""
import os
import numpy as np
import vtk
from vtk.util import numpy_support

out = os.path.dirname(os.path.abspath(__file__))


def path(name):
    return os.path.join(out, name)


# --- Grid ----------------------------------------------------------------------------------
image = vtk.vtkImageData()
image.SetDimensions(4, 3, 2)
image.SetOrigin(1.0, 2.0, 3.0)
image.SetSpacing(0.5, 1.0, 2.0)
n = image.GetNumberOfPoints()
f = np.zeros(n, dtype=np.float32)
v = np.zeros((n, 3), dtype=np.float32)
for i in range(n):
    x, y, z = image.GetPoint(i)
    f[i] = x + 10 * y + 100 * z
    v[i] = (x, y, z)
fa = numpy_support.numpy_to_vtk(f, deep=True)
fa.SetName("f")
va = numpy_support.numpy_to_vtk(v, deep=True)
va.SetName("v")
image.GetPointData().SetScalars(fa)
image.GetPointData().SetVectors(va)

for version, binary, name in [(42, False, "grid_42_ascii.vtk"), (42, True, "grid_42_binary.vtk"),
                              (51, True, "grid_51_binary.vtk")]:
    w = vtk.vtkStructuredPointsWriter()
    w.SetInputData(image)
    w.SetFileName(path(name))
    w.SetFileVersion(version)
    if binary:
        w.SetFileTypeToBinary()
    else:
        w.SetFileTypeToASCII()
    w.Write()


def xml_variants(writer_class, data, stem, ext):
    variants = [
        ("ascii", lambda w: w.SetDataModeToAscii()),
        ("binary", lambda w: (w.SetDataModeToBinary(), w.SetCompressorTypeToNone())),
        ("binary_zlib", lambda w: (w.SetDataModeToBinary(), w.SetCompressorTypeToZLib())),
        ("appended_raw", lambda w: (w.SetDataModeToAppended(), w.EncodeAppendedDataOff(), w.SetCompressorTypeToNone())),
        ("appended_zlib", lambda w: (w.SetDataModeToAppended(), w.EncodeAppendedDataOff(), w.SetCompressorTypeToZLib())),
        ("appended_base64", lambda w: (w.SetDataModeToAppended(), w.EncodeAppendedDataOn(), w.SetCompressorTypeToZLib())),
    ]
    for suffix, setup in variants:
        w = writer_class()
        w.SetInputData(data)
        setup(w)
        if suffix.endswith("zlib") or suffix == "appended_base64":
            w.SetHeaderTypeToUInt32() if suffix == "binary_zlib" else w.SetHeaderTypeToUInt64()
        w.SetFileName(path(f"{stem}_{suffix}.{ext}"))
        w.Write()


xml_variants(vtk.vtkXMLImageDataWriter, image, "grid", "vti")

# --- Sphere (triangles) --------------------------------------------------------------------
sphere = vtk.vtkSphereSource()
sphere.SetThetaResolution(8)
sphere.SetPhiResolution(6)
sphere.Update()
poly = sphere.GetOutput()
poly.GetPointData().Initialize()
z = np.array([poly.GetPoint(i)[2] for i in range(poly.GetNumberOfPoints())], dtype=np.float32)
za = numpy_support.numpy_to_vtk(z, deep=True)
za.SetName("height")
poly.GetPointData().SetScalars(za)

for version, binary, name in [(42, False, "sphere_42_ascii.vtk"), (51, True, "sphere_51_binary.vtk"),
                              (51, False, "sphere_51_ascii.vtk")]:
    w = vtk.vtkPolyDataWriter()
    w.SetInputData(poly)
    w.SetFileName(path(name))
    w.SetFileVersion(version)
    if binary:
        w.SetFileTypeToBinary()
    else:
        w.SetFileTypeToASCII()
    w.Write()

append = vtk.vtkAppendFilter()
append.AddInputData(poly)
append.Update()
xml_variants(vtk.vtkXMLUnstructuredGridWriter, append.GetOutput(), "sphere", "vtu")

w = vtk.vtkUnstructuredGridWriter()
w.SetInputData(append.GetOutput())
w.SetFileName(path("sphere_ugrid_42_ascii.vtk"))
w.SetFileVersion(42)
w.Write()

# --- Quads ---------------------------------------------------------------------------------
plane = vtk.vtkPlaneSource()
plane.SetResolution(2, 2)
plane.Update()
quads = plane.GetOutput()
quads.GetPointData().Initialize()
s = np.arange(quads.GetNumberOfPoints(), dtype=np.float64)
sa = numpy_support.numpy_to_vtk(s, deep=True)
sa.SetName("s")
quads.GetPointData().AddArray(sa)
append = vtk.vtkAppendFilter()
append.AddInputData(quads)
append.Update()
w = vtk.vtkXMLUnstructuredGridWriter()
w.SetInputData(append.GetOutput())
w.SetDataModeToAscii()
w.SetFileName(path("quads_ascii.vtu"))
w.Write()

# --- CSV and NumPy -------------------------------------------------------------------------
with open(path("points.csv"), "w", newline="") as fcsv:
    fcsv.write("# test points\n")
    fcsv.write("x,y,z,temperature,label\n")
    for i in range(5):
        fcsv.write(f"{i},{2 * i},{3 * i},{20.5 + i},p{i}\n")
with open(path("table_semicolon.csv"), "w", newline="") as fcsv:
    fcsv.write("1;2;3\n4;5;6\n7;8;9\n")

field = np.arange(4 * 3 * 2, dtype=np.float32).reshape(2, 3, 4)  # [k][j][i]: x fastest
np.save(path("field_f32.npy"), field)
np.save(path("field_f64_fortran.npy"), np.asfortranarray(np.arange(24, dtype=np.float64).reshape(4, 3, 2, order="F")))
np.save(path("points_i32.npy"), np.arange(15, dtype=np.int32).reshape(5, 3))
np.save(path("vectors_f32.npy"), np.ones((2, 3, 4, 3), dtype=np.float32))
np.save(path("bigendian_f8.npy"), np.arange(6, dtype=">f8"))
print("written to", out)
