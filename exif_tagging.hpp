/*
 * exif_tagging.hpp
 *
 *  Created on: Jun 17, 2011
 *      Author: danny
 */

#ifndef EXIF_TAGGING_HPP_
#define EXIF_TAGGING_HPP_
#include <string>

struct location
{
    location( double longitude, double latitude)
    : longitude( longitude), latitude( latitude)
    {}

    double longitude;
    double latitude;
};

/// given the filename of a jpeg file and a location struct, add
/// the longitude and latitude to the Exif tags of
/// the jpeg file.
bool tag_location( const std::string &filename, const location &loc);

#endif /* EXIF_TAGGING_HPP_ */
