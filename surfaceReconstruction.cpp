#include "surfaceReconstruction.h"

#define NumberOfNeighborsConsideredForEvaluation 24
#define PercentageOfPointsToRemove 5.0

surfaceReconstruction::surfaceReconstruction(std::string inputFile, std::string saveFile, unsigned int reconstructionChoice)
{
    pointSet points;

    std::ifstream stream(inputFile, std::ios_base::binary);
    if (!stream)
    {
        std::cerr << "Error: cannot read file " << inputFile << std::endl;
        return;
    }
    stream >> points;
    std::cout << "Read " << points.size() << " point(s)" << std::endl;
    if (points.empty())
    {
        return;
    }

    CGAL::remove_outliers<CGAL::Sequential_tag>(points, NumberOfNeighborsConsideredForEvaluation, points.parameters().threshold_percent(PercentageOfPointsToRemove));
    std::cout << points.number_of_removed_points() << " point(s) are outliers." << std::endl;
    // Applying point set processing algorithm to a CGAL::Point_set_3
    // object does not erase the points from memory but place them in
    // the garbage of the object: memory can be freeed by the user.
    points.collect_garbage();
    // Compute average spacing using neighborhood of 6 points
    double spacing = CGAL::compute_average_spacing<CGAL::Sequential_tag>(points, 6);
    // Simplify using a grid of size 2 * average spacing

    CGAL::grid_simplify_point_set(points, 2. * spacing);
    std::cout << points.number_of_removed_points() << " point(s) removed after simplification." << std::endl;
    points.collect_garbage();
    CGAL::jet_smooth_point_set<CGAL::Sequential_tag>(points, 24);

    if (reconstructionChoice == 0) // Poisson
    { // DO NOT WORK - FIX / DELETE
        CGAL::jet_estimate_normals<CGAL::Sequential_tag>(points, 24); // Use 24 neighbors
        // Orientation of normals, returns iterator to first unoriented point
        typename pointSet::iterator unoriented_points_begin = CGAL::mst_orient_normals(points, 24); // Use 24 neighbors
        points.remove(unoriented_points_begin, points.end());
        CGAL::Surface_mesh<kernel::Point_3> output_mesh;
        CGAL::poisson_surface_reconstruction_delaunay(points.begin(), points.end(), points.point_map(), points.normal_map(), output_mesh, spacing);
        std::ofstream f("out.ply", std::ios_base::binary);
        CGAL::set_binary_mode(f);
        CGAL::write_ply(f, output_mesh);
        f.close();
    }
    else if (reconstructionChoice == 1) // Advancing front
    {
        typedef std::array<std::size_t, 3> Facet; // Triple of indices
        std::vector<Facet> facets;
        // The function is called using directly the points raw iterators
        CGAL::advancing_front_surface_reconstruction(points.points().begin(), points.points().end(), std::back_inserter(facets));
        std::cout << facets.size() << " facet(s) generated by reconstruction." << std::endl;
        // copy points for random access
        std::vector<kernel::Point_3> vertices;
        vertices.reserve(points.size());
        std::copy(points.points().begin(), points.points().end(), std::back_inserter(vertices));
        CGAL::Surface_mesh<kernel::Point_3> output_mesh;
        CGAL::Polygon_mesh_processing::polygon_soup_to_polygon_mesh(vertices, facets, output_mesh);
        std::ofstream f(saveFile);
        f << output_mesh;
        f.close();
    }
    else if (reconstructionChoice == 2) // Scale space
    {
        CGAL::Scale_space_surface_reconstruction_3<kernel> reconstruct(points.points().begin(), points.points().end());
        // Smooth using 4 iterations of Jet Smoothing
        reconstruct.increase_scale(4, CGAL::Scale_space_reconstruction_3::Jet_smoother<kernel>());
        // Mesh with the Advancing Front mesher with a maximum facet length of 0.5
        reconstruct.reconstruct_surface(CGAL::Scale_space_reconstruction_3::Advancing_front_mesher<kernel>(0.5));
        std::ofstream f(saveFile);
        f << "OFF" << std::endl << points.size() << " " << reconstruct.number_of_facets() << " 0" << std::endl;
        for (pointSet::Index idx : points)
        {
            f << points.point(idx) << std::endl;
        }
        for (const auto& facet : CGAL::make_range(reconstruct.facets_begin(), reconstruct.facets_end()))
        {
            f << "3 " << facet << std::endl;
        }
        f.close();
    }
    else // Handle error
    {
        std::cerr << "Error: invalid reconstruction id: " << reconstructionChoice << std::endl;
    }
}

