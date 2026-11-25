"""Writes the data files of the vis_readers1 example to examples/assets/data.

Run with a Python that has numpy and vtk (9.x):

    python examples/vis_readers1/make_data.py

  stars.csv  a spiral galaxy: x, y, z, brightness, age (4000 stars)
  blobs.npy  a 40^3 float32 density field of four blobs, indexed [k][j][i]
  boy.vtu    Boy's surface with its Gaussian curvature (VTK XML, appended zlib data)
"""
import os
import numpy as np
import vtk

out = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "assets", "data")
os.makedirs(out, exist_ok=True)
rng = np.random.default_rng(7)

# Spiral galaxy
n = 4000
arm = rng.integers(0, 2, n)
r = rng.gamma(2.0, 0.35, n)
theta = 1.8 * np.log1p(4 * r) + np.pi * arm + rng.normal(0, 0.25, n)
x = r * np.cos(theta)
z = r * np.sin(theta)
y = rng.normal(0, 0.05 * np.exp(-r), n)
brightness = np.exp(-r) * rng.uniform(0.3, 1.0, n)
age = np.clip(r * 3 + rng.normal(0, 0.8, n), 0, None)
with open(os.path.join(out, "stars.csv"), "w", newline="") as f:
    f.write("# a spiral galaxy for vis_readers1\n")
    f.write("x,y,z,brightness,age\n")
    for row in zip(x, y, z, brightness, age):
        f.write(",".join(f"{v:.4f}" for v in row) + "\n")

# Density field
m = 40
c = np.linspace(-1, 1, m, dtype=np.float32)
Z, Y, X = np.meshgrid(c, c, c, indexing="ij")  # [k][j][i]
density = np.zeros_like(X)
for (bx, by, bz, s) in [(0.3, 0.2, 0.0, 0.25), (-0.35, -0.1, 0.25, 0.22), (0.0, -0.35, -0.3, 0.2), (-0.1, 0.4, -0.2, 0.18)]:
    density += np.exp(-((X - bx) ** 2 + (Y - by) ** 2 + (Z - bz) ** 2) / (2 * s * s))
np.save(os.path.join(out, "blobs.npy"), density.astype(np.float32))

# Boy's surface with curvature
boy = vtk.vtkParametricBoy()
source = vtk.vtkParametricFunctionSource()
source.SetParametricFunction(boy)
source.SetUResolution(90)
source.SetVResolution(90)
source.Update()
clean = vtk.vtkCleanPolyData()
clean.SetInputConnection(source.GetOutputPort())
curvature = vtk.vtkCurvatures()
curvature.SetInputConnection(clean.GetOutputPort())
curvature.SetCurvatureTypeToMean()
curvature.Update()
surface = curvature.GetOutput()
values = surface.GetPointData().GetArray("Mean_Curvature")
values.SetName("curvature")
pd = surface.GetPointData()
for name in [pd.GetArrayName(i) for i in range(pd.GetNumberOfArrays())]:
    if name != "curvature":
        pd.RemoveArray(name)
pd.SetActiveScalars("curvature")
append = vtk.vtkAppendFilter()
append.AddInputData(surface)
append.Update()
w = vtk.vtkXMLUnstructuredGridWriter()
w.SetInputData(append.GetOutput())
w.SetDataModeToAppended()
w.EncodeAppendedDataOff()
w.SetCompressorTypeToZLib()
w.SetFileName(os.path.join(out, "boy.vtu"))
w.Write()
print("written to", os.path.abspath(out))
