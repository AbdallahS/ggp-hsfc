/*****************************************************************************************
 * A boost based exception class
 *****************************************************************************************/

#ifndef HSFCEXCEPTION_H
#define HSFCEXCEPTION_H

#include <string>
#include <boost/exception/all.hpp>

namespace HSFC
{

typedef boost::error_info<struct tag_Message, std::string> ErrorMsgInfo;
class HSFCException : public boost::exception, public std::exception {};

};


#endif // HSFC_H
