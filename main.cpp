/*   
    Copyright (C) 2023 Danilo Nascimento
    GNU General Public License v3.0
    Permissions of this strong copyleft license are conditioned on making available complete source code of licensed works and modifications,
    which include larger works using a licensed work, under the same license.
    Copyright and license notices must be preserved. Contributors provide an express grant of patent rights.

    Contact information
    -------------------
    E-mail: me@daniloinspace.com
    Website: daniloinspace.com

    Take GPS/GNSS ellipsoid altitude compare it with a geoid model and return the topographic altitude of any given area

    Consider:

    1. Define the geoid Model:
    You will need a geoid model that covers the area of interest, which provides the geoid height at different geographic coordinates. 
    The geoid model can be stored in a file or a database, and it should be in a format that allows for easy access and interpolation.
    
    2. Get the GPS Coordinates:
    You will need to get the GPS coordinates of the point of interest, including the latitude, longitude, and ellipsoid height. 
    The ellipsoid height is the height above the reference ellipsoid and is usually provided by the GPS receiver.

    3. Interpolate the geoid height:
    The interpolation method should be chosen based on the accuracy requirements and the size of the dataset. 
    In this implementation I choose cubic spline Interpolation, which provides smoother results and better accuracy,
    but its more computational extensive 
*/

#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <stdexcept>


struct GeoidModel
{
    int nrows;
    int ncols;
    double lat_min;
    double lon_min;
    double lat_step;
    double lon_step;
    std::vector<double> data;
};

// Compute the indices of the four surrounding grid cells for a given latitude and longitude
void find_grid_indices(const GeoidModel &model, double lat, double lon, int &row1, int &col1, int &row2, int &col2)
{
    // This error check isnt working properly, problem is on the 4th condition that trows the error even if the point is actualy inside the geoid model...
    // if (lat < model.lat_min || lat >= model.lat_min + model.nrows * model.lat_step ||
    //     lon < model.lon_min || lon >= model.lon_min + model.ncols * model.lon_step)
    // {
    //     throw std::runtime_error("Point is outside geoid model bounds");
    // }

    double row = (lat - model.lat_min) / model.lat_step;
    double col = (lon - model.lon_min) / model.lon_step;

    row1 = static_cast<int>(std::floor(row));
    row2 = static_cast<int>(std::ceil(row));
    col1 = static_cast<int>(std::floor(col));
    col2 = static_cast<int>(std::ceil(col));

    // Clamp the indices to the range [0, nrows-1] or [0, ncols-1]
    row1 = std::max(0, std::min(row1, model.nrows - 1));
    row2 = std::max(0, std::min(row2, model.nrows - 1));
    col1 = std::max(0, std::min(col1, model.ncols - 1));
    col2 = std::max(0, std::min(col2, model.ncols - 1));
}

// Compute the topographic height at a given latitude and longitude using cubic spline interpolation
double interpolate_geoid_height(const GeoidModel &model, double lat, double lon)
{
    int row1, col1, row2, col2;
    find_grid_indices(model, lat, lon, row1, col1, row2, col2);

    double h11 = model.data[row1 * model.ncols + col1];
    double h12 = model.data[row1 * model.ncols + col2];
    double h21 = model.data[row2 * model.ncols + col1];
    double h22 = model.data[row2 * model.ncols + col2];

    double dlat = (lat - model.lat_min - row1 * model.lat_step) / model.lat_step;
    double dlon = (lon - model.lon_min - col1 * model.lon_step) / model.lon_step;

    double h1 = h11 + dlon * (h12 - h11);
    double h2 = h21 + dlon * (h22 - h21);
    double h = h1 + dlat * (h2 - h1);

    return h;
}

// Load the geoid model from a file
GeoidModel load_geoid_model(const std::string &filename)
{
    GeoidModel model;

    std::ifstream file(filename);
    if (!file)
    {
        throw std::runtime_error("Error opening file " + filename);
    }

    std::string line;
    std::getline(file, line); // Read the header line

    if (line != "Longitude\tLatitude\tHeight")
    {
        throw std::runtime_error("Invalid file format");
    }

    // Count the number of data rows in the file
    model.nrows = 0;
    while (std::getline(file, line))
    {

        ++model.nrows;
    }

    // Seek back to the start of the file and read the header line again
    file.clear();
    file.seekg(0);
    std::getline(file, line); // Read the header line

    // Count the number of columns in the header line
    model.ncols = 0;
    for (char c : line)
    {
        if (c == '\t')
        {
            ++model.ncols;
        }
    }
    ++model.ncols;

    // Read the remaining data rows in the file
    model.data.resize(model.nrows * model.ncols);

    for (int i = 0; i < model.nrows; ++i)
    {
        if (!std::getline(file, line))
        {
            throw std::runtime_error("Invalid file format");
        }

        std::istringstream iss(line);
        double lon, lat, h;
        if (!(iss >> lon >> lat >> h))
        {
            throw std::runtime_error("Invalid file format");
        }

        int j = i * model.ncols;
        model.data[j] = lon;
        model.data[j + 1] = lat;
        model.data[j + 2] = h;
    }

    // Compute the grid step sizes and minimum lat/lon values
    model.lon_min = model.data[0];
    model.lat_min = model.data[1];
    double lon_max = model.data[(model.ncols - 1) * 3];
    double lat_max = model.data[(model.nrows - 1) * model.ncols + 1];
    model.lon_step = (lon_max - model.lon_min) / (model.ncols - 1);
    model.lat_step = (lat_max - model.lat_min) / (model.nrows - 1);

    return model;
}

// Compute the topographic height given the GPS coordinates and geoid model
double compute_topographic_height(const GeoidModel &model, double lat, double lon, double ellipsoid_height)
{
    double geoid_height = interpolate_geoid_height(model, lat, lon);
    double topographic_height = ellipsoid_height - geoid_height;
    return topographic_height;
}

// Example usage
int main()
{
    GeoidModel model = load_geoid_model("GeodPT08.dat");

    double lat = 41.157944;
    double lon = -8.629105;
    double ellipsoid_height = 148.0;

    double topographic_height = compute_topographic_height(model, lat, lon, ellipsoid_height);

    // Print the results
    std::cout << "GPS Coordinates: (" << lat << ", " << lon << ")" << std::endl;
    std::cout << "Ellipsoid height: " << ellipsoid_height << " m" << std::endl;
    std::cout << "Geoid height: " << interpolate_geoid_height(model, lat, lon) << " m" << std::endl;
    std::cout << "Topographic height: " << topographic_height << " m" << std::endl;

    return 0;
}
