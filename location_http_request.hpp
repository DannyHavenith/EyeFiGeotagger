/*
 * location_http_request.hpp
 *
 *  Created on: Jun 17, 2011
 *      Author: danny
 */

#ifndef LOCATION_HTTP_REQUEST_HPP_
#define LOCATION_HTTP_REQUEST_HPP_
#include <string>

/// send a given json location request to the google location web-api and return the json
/// result string.
std::string send_request( const std::string &json_request);

#endif /* LOCATION_HTTP_REQUEST_HPP_ */
