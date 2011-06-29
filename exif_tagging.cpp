/*
 * exif_tagging.cpp
 *
 *  Created on: Jun 17, 2011
 *      Author: danny
 */
#include "exif_tagging.hpp"
#include <stdexcept>
#include <cmath>
#include <exiv2/exiv2.hpp>
#include <boost/assign/list_of.hpp>

void add_exif_coordinate( Exiv2::ExifData &data, const std::string &attribute, double value)
{
    Exiv2::URationalValue::AutoPtr rational(new Exiv2::URationalValue);

    // Add more elements through the extended interface of rational value
    double frac = 0.0;
    double intpart = 0.0;
    frac = std::modf( value, &intpart);
    rational->value_.push_back(std::make_pair( (unsigned int)intpart, (unsigned int)1));

    // intpart = minutes * 10000
    frac = std::modf( frac * 600000, &intpart);
    rational->value_.push_back(std::make_pair( (unsigned int)intpart, (unsigned int)10000));

    rational->value_.push_back(std::make_pair(0,1));

    // Add the key and value pair to the Exif data
    Exiv2::ExifKey key = Exiv2::ExifKey( attribute);
    data.add(key, rational.get());
}

bool tag_location( const std::string &filename, const location &loc)
{
    bool result = false;
    Exiv2::Image::AutoPtr image = Exiv2::ImageFactory::open( filename);
    if (!image.get()) throw std::runtime_error( "could not open image file " + filename + " for metadata tags\n");
    image->readMetadata();
    Exiv2::ExifData data = image->exifData();

    if (data.findKey( Exiv2::ExifKey("Exif.GPSInfo.GPSLatitude")) == data.end())
    {
        add_exif_coordinate( data, "Exif.GPSInfo.GPSLatitude", std::abs( loc.latitude));
        add_exif_coordinate( data, "Exif.GPSInfo.GPSLongitude", std::abs( loc.longitude));
        data["Exif.GPSInfo.GPSLatitudeRef"] = loc.latitude < 0 ? "S":"N";
        data["Exif.GPSInfo.GPSLongitudeRef"] = loc.longitude < 0 ? "W":"E";
        Exiv2::byte version[] = { 2, 0, 0, 0};
        data["Exif.GPSInfo.GPSVersionID"].getValue()->read( version, 4, Exiv2::invalidByteOrder);
        image->setExifData( data);
        image->writeMetadata();
        result = true;
    }

    return result;
}
