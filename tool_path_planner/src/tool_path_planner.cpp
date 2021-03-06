/*
 * Copyright (c) 2016, Southwest Research Institute
 * All rights reserved.
 *
 */

#include <limits>
#include <cmath>

#include <Eigen/Core>

#include <vtkParametricFunctionSource.h>
#include <vtkOBBTree.h>
#include <vtkIntersectionPolyDataFilter.h>
#include <vtkDelaunay2D.h>
#include <vtkMath.h>
#include <vtkSpline.h>
#include <vtkPolyDataNormals.h>
#include <vtkKdTreePointLocator.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkTriangle.h>
#include <vtk_viewer/vtk_utils.h>
#include <vtkReverseSense.h>
#include <vtkImplicitDataSet.h>
#include <vtkCutter.h>
#include <vtkCellLocator.h>
#include <vtkGenericCell.h>
#include <vtkTriangleFilter.h>

#include <tool_path_planner/tool_path_planner.h>

namespace tool_path_planner
{

  double squared_distance(std::vector<double>& pt1, std::vector<double>& pt2)
  {
    if(pt1.size() != 3 || pt2.size() != 3)
    {
      return 0;
    }
    return (pow(pt1[0] - pt2[0], 2.0) + pow(pt1[1] - pt2[1], 2.0 ) + pow((pt1[2] - pt2[2]), 2.0 ));
  }

  void flipPointOrder(ProcessPath& path)
  {
    vtkSmartPointer<vtkPoints> points = vtkSmartPointer<vtkPoints>::New();
    vtkSmartPointer<vtkPoints> points2 = vtkSmartPointer<vtkPoints>::New();
    points = path.line->GetPoints();

    // flip point order
    for(int i = points->GetNumberOfPoints() - 1; i >= 0; --i)
    {
      points2->InsertNextPoint(points->GetPoint(i));
    }
    path.line->SetPoints(points2);

    // flip normal order
    vtkSmartPointer<vtkDataArray> norms = path.line->GetPointData()->GetNormals();
    vtkSmartPointer<vtkDoubleArray> new_norms = vtkSmartPointer<vtkDoubleArray>::New();
    new_norms->SetNumberOfComponents(3);

    for(int i = norms->GetNumberOfTuples() - 1; i >= 0; --i)
    {
      double* ptr = norms->GetTuple(i);
      new_norms->InsertNextTuple(ptr);
    }
    path.line->GetPointData()->SetNormals(new_norms);

    // flip derivative directions
    points = path.derivatives->GetPoints();
    vtkSmartPointer<vtkPoints> dpoints2 = vtkSmartPointer<vtkPoints>::New();

    // flip point order
    for(int i = points->GetNumberOfPoints() - 1; i >= 0; --i)
    {
      dpoints2->InsertNextPoint(points->GetPoint(i));
    }
    path.derivatives->SetPoints(dpoints2);


    vtkDataArray* ders = path.derivatives->GetPointData()->GetNormals();
    vtkSmartPointer<vtkDoubleArray> new_ders = vtkSmartPointer<vtkDoubleArray>::New();
    new_ders->SetNumberOfComponents(3);
    for(int i = ders->GetNumberOfTuples() -1; i >= 0; --i)
    {
      double* pt = ders->GetTuple(i);
      pt[0] *= -1;
      pt[1] *= -1;
      pt[2] *= -1;
      new_ders->InsertNextTuple(pt);
    }
    path.derivatives->GetPointData()->SetNormals(new_ders);

    // reset points in spline
    path.spline->SetPoints(points);
  }

  int findClosestPoint(std::vector<double>& pt,  std::vector<std::vector<double> >& pts)
  {
    double min = std::numeric_limits<double>::max();
    int index = -1;
    for(int i = 0; i < pts.size(); ++i)
    {
      double d = squared_distance(pt, pts[i]);
      if(d < min)
      {
        index = i;
        min = d;
      }
    }
    return index;
  }

}
