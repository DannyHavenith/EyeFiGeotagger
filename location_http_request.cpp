/*
 * location_http_request.cpp
 *
 *  Created on: Jun 17, 2011
 *      Author: danny
 */
#define BOOST_NETWORK_NO_LIB
#include "location_http_request.hpp"
#include <boost/network.hpp>

using std::string;

string send_request( const string &json_request)
{
    using namespace boost::network;
    using namespace boost::network::http;

    client::request request("http://www.google.com/loc/json");
    request << header("Connection", "close")
            << header( "Content-Type", "application/json")
            << header( "Content-length", boost::lexical_cast<std::string>((unsigned int)json_request.size()));


    client client;
    client::response response_ = client.post(request, json_request);
    return body(response_);

}
