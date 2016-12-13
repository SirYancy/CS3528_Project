#include <string>

#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

//! Trim whitespace from right of string.
/*! Trims whitespace from right side of string.
 *  \param s String to trim.
 *  \param delimiters String of delimiters to trim.
 *  \return Trimmed string.
 */
inline std::string trim_right_copy(
  const std::string& s,
  const std::string& delimiters = " \f\n\r\t\v" )
{
  return s.substr( 0, s.find_last_not_of( delimiters ) + 1 );
}

//! Trim whitespace from left of string.
/*! Trims whitespace from left side of string.
 *  \param s String to trim.
 *  \param delimiters String of delimiters to trim.
 *  \return Trimmed string.
 */
inline std::string trim_left_copy(
  const std::string& s,
  const std::string& delimiters = " \f\n\r\t\v" )
{
  return s.substr( s.find_first_not_of( delimiters ) );
}

//! Trim whitespace from front and back of string.
/*! Trims whitespace from front and back of string.
 *  \param s String to trim.
 *  \param delimiters String of delimiters to trim.
 *  \return Trimmed string.
 */
inline std::string trim_copy(
  const std::string& s,
  const std::string& delimiters = " \f\n\r\t\v" )
{
  return trim_left_copy( trim_right_copy( s, delimiters ), delimiters );
}

//! String translation function
/*! Function to create a string to uppercase. Not internationally safe.
 * \param s String to case fold.
 * \return A string that has been case folded to UPPER.
 */
std::string strUpper(const std::string& s);

#endif // UTILS_H_INCLUDED
