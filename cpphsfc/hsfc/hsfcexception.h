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
class HSFCException : public boost::exception, public std::exception 
{
public:
    char const* what() const throw();
};

// Distinguish between an exception thrown because of bad input (ValueError)
// and an exception thrown because something went wrong internally.
class HSFCValueError : public HSFCException {};
class HSFCInternalError : public HSFCException {};

};


#endif // HSFC_H
