# GPScoordinatesInterpolation
### Take GPS/GNSS ellipsoid altitude compare it with a geoid model and return the topographic altitude of any given area

Consider:

1. Define the geoid Model:
    You will need a geoid model that covers the area of interest, which provides the geoid height at different geographic coordinates. 
    The geoid model can be stored in a file or a database, and it should be in a format that allows for easy access and interpolation.
    (I'm using the Geoid Model for Portugal as an example)
    
2. Get the GPS Coordinates:
    You will need to get the GPS coordinates of the point of interest, including the latitude, longitude, and ellipsoid height. 
    The ellipsoid height is the height above the reference ellipsoid and is usually provided by the GPS receiver.

3. Interpolate the geoid height:
    The interpolation method should be chosen based on the accuracy requirements and the size of the dataset. 
    In this implementation I choose cubic spline Interpolation, which provides smoother results and better accuracy,
    but its more computational extensive 
    
    
> Here are some resources to find out how this algorithm works:<br>
https://support.pix4d.com/hc/en-us/articles/202559869-Orthometric-vs-ellipsoidal-height<br>
https://en.wikipedia.org/wiki/Spline_interpolation<br>
https://en.wikiversity.org/wiki/Cubic_Spline_Interpolation
