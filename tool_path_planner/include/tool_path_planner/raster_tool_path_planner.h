/*
 * Copyright (c) 2016, Southwest Research Institute
 * All rights reserved.
 *
 */

#ifndef RASTER_TOOL_PATH_PLANNER_H
#define RASTER_TOOL_PATH_PLANNER_H

#include <vtkPoints.h>
#include <vtkKdTreePointLocator.h>

#include <tool_path_planner/tool_path_planner.h>

namespace tool_path_planner
{
  class RasterToolPathPlanner : public ToolPathPlanner
  {
  public:

    /**
     * @brief constructor
     * @param use_ransac set flag to use ransac plane estimation to determine path normals
     */
    RasterToolPathPlanner(bool use_ransac=false);
    ~RasterToolPathPlanner(){}

    /**
     * @brief planPaths plans a set of paths for all meshes in a given list
     * @param meshes A vector of meshes to plan paths for
     * @param paths The resulting path data generated
     */
    void planPaths(const vtkSmartPointer<vtkPolyData> mesh, std::vector<ProcessPath>& paths);
    void planPaths(const std::vector<vtkSmartPointer<vtkPolyData> > meshes, std::vector< std::vector<ProcessPath> >& paths);
    void planPaths(const std::vector<pcl::PolygonMesh>& meshes, std::vector< std::vector<ProcessPath> >& paths);
    void planPaths(const pcl::PolygonMesh& mesh, std::vector<ProcessPath>& paths);

    /**
     * @brief setInputMesh Sets the input mesh to generate paths
     * @param mesh The input mesh to be operated on
     */
    void setInputMesh(vtkSmartPointer<vtkPolyData> mesh);

    /**
     * @brief getInputMesh Gets the input mesh used for generating paths
     * @return The stored input mesh
     */
    vtkSmartPointer<vtkPolyData> getInputMesh(){return input_mesh_;}

    /**
     * @brief setTool Sets the tool parameters used during path generation
     * @param tool The tool object with all of the parameters necessary for path generation
     */
    void setTool(ProcessTool tool){tool_ = tool;}

    /**
     * @brief getTool Gets the tool parameters used during path generation
     * @return The set of tool parameters
     */
    ProcessTool getTool(){return tool_;}

    /**
     * @brief getFirstPath Uses the input mesh, generates the first path by intersecting the mesh with a plane
     * @param path The first path generated
     */
    bool getFirstPath(ProcessPath& path);

    /**
     * @brief getNextPath Creates the next path offset from the current path
     * @param this_path The current path, from which to create an offset path
     * @param next_path The next path returned after calling the function
     * @param dist The distance to offset the next path from the current
     * @param test_self_intersection Disables check to see if new path intersects with any previously generated paths
     * @return True if the next path is successfully created, False if no path can be generated
     */
    bool getNextPath(const ProcessPath this_path, ProcessPath& next_path, double dist = 0.0, bool test_self_intersection = true);

    /**
     * @brief computePaths Will create and store all paths possible from the given mesh and starting path
     * @return True if paths were generated, False if the first path is not available (nothing to start from)
     */
    bool computePaths();

    /**
     * @brief getPaths Gets all of the paths generated
     * @return The paths generated from the computePaths() function
     */
    std::vector<ProcessPath> getPaths(){return paths_;}

    /**
     * @brief generateNormals For a set of new points, estimates the normal from the input mesh normals by averaging N nearest neighbors' normals
     * @param data The points to operate on, normal data inserted in place
     */
    void estimateNewNormals(vtkSmartPointer<vtkPolyData>& data);

    /**
     * @brief generateNormals For a set of new points, estimates the normal from the input mesh normals ransac plane fit
     * @param data The points to operate on, normal data inserted in place
     */
    void estimateNewNormalsRansac(vtkSmartPointer<vtkPolyData>& data);

    /**
     * @brief setDebugModeOn Turn on debug mode to visualize every step of the path planning process
     * @param debug Turns on debug if true, turns off debug if false
     */
    void setDebugMode(bool debug){debug_on_ = debug;}

    /**
     * @brief setLogDir Set the directory for saving polydata files to
     * @param dir The directory to save data to
     */
    void setLogDir(std::string dir){debug_viewer_.setLogDir(dir);}

    /**
     * @brief getLogDir Get the directory used for saving polydata files to
     * @return The directory currently used for saving data
     */
    std::string getLogDir(){return debug_viewer_.getLogDir();}

  private:

    bool use_ransac_normal_estimation_;


    bool debug_on_;  /**< Turns on/off the debug display which views the path planning output one step at a time */
    vtk_viewer::VTKViewer debug_viewer_;  /**< The vtk viewer for displaying debug output */
    vtkSmartPointer<vtkKdTreePointLocator> kd_tree_; /**< kd tree for finding nearest neighbor points */
    vtkSmartPointer<vtkPolyData> input_mesh_; /**< input mesh to operate on */
    std::vector<ProcessPath> paths_; /**< series of intersecting lines on the given mesh */
    ProcessTool tool_; /**< The tool parameters which defines how to generate the tool paths (spacing, offset, etc.) */

    /**
     * @brief getCellCentroidData Gets the data for a cell in the input_mesh_
     * @param id The cell id to get data for
     * @param center The center of the given cell
     * @param norm The cell normal
     * @param area The cell area
     * @return True if the cell for the given id exists, False if it does not exist
     */
    bool getCellCentroidData(int id, double* center, double* norm, double& area);

    /**
     * @brief createStartCurve Creates a initial "curve" to begin the path planning from
     * @return A set of VTK points and normals which are then used to create a cutting surface
     */
    vtkSmartPointer<vtkPolyData> createStartCurve();

    /**
     * @brief smoothData Takes in a spline and returns a series of evenly spaced points with normals and derivatives
     * @param spline The input spline to operate on
     * @param points The set of evenly spaced points, with normals
     * @param derivatives The set of evenly spaced points, with derivative data inserted into the "normals" position
     */
    void smoothData(vtkSmartPointer<vtkParametricSpline> spline, vtkSmartPointer<vtkPolyData>& points, vtkSmartPointer<vtkPolyData>& derivatives);

    /**
     * @brief generateNormals Generates point normals for a given mesh, normals are located on polydata vertices
     * @param data The mesh to operate on, normal data inserted in place
     */
    void generateNormals(vtkSmartPointer<vtkPolyData>& data);

    /**
     * @brief createOffsetLine Given a line with normals, generate a new line which is offset by a given distance
     * @param line Start line
     * @param derivatives Derivatives of the start line
     * @param dist The amount and direction to offset
     * @return The newly created line
     */
    vtkSmartPointer<vtkPolyData> createOffsetLine(vtkSmartPointer<vtkPolyData> line, vtkSmartPointer<vtkPolyData> derivatives, double dist);

    /**
     * @brief createSurfaceFromSpline Using a line with normals, generate a surface with points above and below the line (used for mesh intersection calculation)
     * @param line The input line with normals
     * @param dist The amount to extend above and below the line for generating the new surface
     * @return The new surface, in the form of a mesh
     */
    vtkSmartPointer<vtkPolyData> createSurfaceFromSpline(vtkSmartPointer<vtkPolyData> line, double dist);

    /**
     * @brief sortPoints Sorts points in order to form a contiguous line with the shortest length possible
     * @param points The input points to reorder
     */
    void sortPoints(vtkSmartPointer<vtkPoints>& points);

    /**
     * @brief findIntersectionLine Given an cutting mesh, finds the intersection of the mesh and the input_mesh_
     * @param cut_surface The mesh to intersect with the input_mesh_
     * @param points The points found on the intersection of the two meshes
     * @param spline A smoothed spline which is generated from the points found from the intersection
     * @return True if the two meshes intersect, False if they do not (no point or spline data)
     */
    bool findIntersectionLine(vtkSmartPointer<vtkPolyData> cut_surface,
                                                      vtkSmartPointer<vtkPolyData>& points,
                                                      vtkSmartPointer<vtkParametricSpline>& spline);

    /**
     * @brief checkPathForHoles Checks a given path to determine if it needs to be broken up if there is a large hole in the middle
     * @param path The input path to be checked for large holes/gaps
     * @param out_paths The output paths, after any splitting is performed.  Is empty if no splitting is needed
     * @return True if a large hole is detected and path was broken up, false if no splitting is needed
     */
    bool checkPathForHoles(const ProcessPath path, std::vector<ProcessPath>& out_paths);

    /**
     * @brief resamplePoints Resamples a set of points to make them evenly spaced, creates and samples a spline through the original point set
     * @param points The input points to modify
     */
    void resamplePoints(vtkSmartPointer<vtkPoints>& points);
  };

}

#endif // PATH_PLANNER_H
