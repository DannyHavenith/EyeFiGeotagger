//============================================================================
// Name        : Geotagger.cpp
// Author      : Danny Havenith
// Version     :
// Copyright   : 
//============================================================================
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/join.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/tokenizer.hpp>
#include <boost/regex.hpp>

#include "location_http_request.hpp"
#include "exif_tagging.hpp"

using namespace std;


typedef std::map<string, string> access_point_strengths;

/// parse a log file for all access points encountered when taking image 'image_name'.
/// The log will be parsed and all access points after the last power-on are considered close enough, additionally
/// any new access points found within one minute of taking the picture are considered relevant.
access_point_strengths read_logfile( const string &image_name, istream &logfile)
{
    string buffer;
    access_point_strengths access_points;

    // find any access points between power on and the picture time...
    unsigned int picture_timestamp = 0; // when is the picture taken.
    bool picture_found = false;
    while( !picture_found && getline( logfile, buffer) )
    {
        vector<string> fields;
        boost::split( fields, buffer, boost::algorithm::is_any_of( ","));
        if (fields.size() >= 3)
        {
            if (fields[2] == "NEWAP" || fields[2] == "AP")
            {
                access_points[ fields[3]] = fields[4];
            }
            if (fields[2] == "POWERON")
            {
                access_points.clear();
            }
            if (fields[2] == "NEWPHOTO" && fields[3] == image_name)
            {
                picture_timestamp = boost::lexical_cast< unsigned int>( fields[1]);
                picture_found = true;
            }
        }
    }

    // find any new access points encountered within one minute of the picture
    bool within_time = true;
    while ( picture_timestamp && getline( logfile, buffer) && within_time)
    {
        vector<string> fields;
        boost::split( fields, buffer, boost::algorithm::is_any_of( ","));
        if (fields.size() >= 3)
        {
            unsigned int timestamp = boost::lexical_cast< unsigned int>( fields[1]);
            if ( timestamp >= picture_timestamp + 60 || fields[2] == "POWERON")
            {
                within_time = false;
            }
            if (fields[2] == "NEWAP" || fields[2] == "AP")
            {
                if (access_points.find( fields[3]) == access_points.end())
                {
                    access_points[ fields[3]] = fields[4];
                }
            }
        }
    }
    return access_points;
}

/// turn a string of the form "12ab34cd" into a string of the form "12-ab-34-cd"
string normalize_mac( const string &raw_mac)
{
    using namespace boost;

    // chop the string into pieces of 2
    int offsets[] = {2};
    offset_separator f(offsets, offsets+1, true);
    tokenizer<offset_separator> tokenized(raw_mac,f);

    // re-combine with dash as separator
    return join( tokenized, "-");
}

/// create a json request string for a single access point
string access_point_json( const access_point_strengths::value_type &v)
{
    return "{ \"mac_address\":\"" + normalize_mac(v.first) + "\", \"signal_strength\":" + v.second + "}";
}

/// create a json request string for all the given access points
string access_points_json( const access_point_strengths &access_points)
{
    string result;
    int count = 0;
    BOOST_FOREACH( const access_point_strengths::value_type &v, access_points)
    {
        if (count++)
        {
            result += ",";
        }
        result += access_point_json( v);
    }
    return "{\"version\":\"1.1.0\",\"request_address\":false,\"wifi_towers\":["
            + result
            + "]}";
}


double extract_value( const std::string &json_string, const std::string &attribute)
{
    double result = 0.0;
    boost::regex finder(".*\"" + attribute + "\":([0-9.]+).*");
    boost::smatch results;
    if (boost::regex_match(json_string, results, finder))
    {
        result = boost::lexical_cast<double>( results[1]);
    }
    else
    {
        throw std::runtime_error("could not find attribute " + attribute + " in the geo-information");
    }
    return result;
}

int main( int argc, char *argv[])
{
    using namespace boost::filesystem;
    try
    {

        // typical session:
//    {"version":"1.1.0","request_address":false,"wifi_towers":[{ "mac_address":"00-14-bf-09-0f-2e", "signal_strength":28},{ "mac_address":"00-1b-2f-58-8f-aa", "signal_strength":9}]}
//    {"location":{"latitude":51.5908213,"longitude":5.3155023,"accuracy":57.0},"access_token":"2:T0XE0ciQm6W9jp4N:Jm1NagrV1PHVZyFb"}

        if (argc < 2)
        {
            std::cerr << "usage: " << argv[0] << " <jpeg filename>\n";
            return -1;
        }

        // find the image file and verify it exists
        path image_path( argv[1]);
        if (!exists( image_path))
        {
            std::cerr << "image not found: " << image_path << "\n";
            return -2;
        }

        // find the log file and verify it exists
        path log_path( argv[1] + string(".log"));
        if (!exists( log_path))
        {
            std::cerr << "no logfile found for image " << image_path.filename() << "\n";
            return -3;
        }

        // parse the log file for access points
        boost::filesystem::ifstream logfile( log_path);
        access_point_strengths access_points = read_logfile( image_path.filename().c_str(), logfile);

        // if access points were found, try to geo-locate them and add the appropriate tags to the picture
        if ( !access_points.empty())
        {
            std::cout << "found " << access_points.size() << ((access_points.size()==1)?" access point\n":" access points\n");
            string json_request = access_points_json( access_points);

            string json_reply = send_request( json_request);
            tag_location(
                    argv[1],
                    location(
                            extract_value( json_reply, "longitude"),
                            extract_value( json_reply, "latitude")
                     )
              );
        }
        else
        {
            std::cout << "no access points for " << image_path.filename() << "\n";
        }
    }
    catch (std::exception &e)
    {
        std::cerr << "something went wrong: " << e.what() << "\n";
    }
	return 0;
}
