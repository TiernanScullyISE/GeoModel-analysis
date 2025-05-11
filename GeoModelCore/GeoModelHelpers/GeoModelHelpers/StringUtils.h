/*
  Copyright (C) 2002-2024 CERN for the benefit of the ATLAS collaboration
*/
#ifndef GeoModelHelpers_STRINGUTILS_H
#define GeoModelHelpers_STRINGUTILS_H

#include <string>
#include <string_view>
#include <vector>
#include <functional>

namespace GeoStrUtils{


    /// @brief Replaces all occurances of pattern <A> by a substring <B>
    /// @param str String to modify
    /// @param exp Expression to be replaced
    /// @param rep Replace it with
    /// @return 
    std::string replaceExpInString(std::string str, 
                                   const std::string_view exp, 
                                   const std::string_view rep);
    
    /// @brief If a string contains a bash environment variable
    ///        E.g. ${GMX_FILES}/Blub.xml then the ${} expression is
    ///        replaced by the variable content 
    /// @param str 
    /// @return 
    std::string resolveEnviromentVariables(const std::string_view str);

    std::string getEnvVar(const std::string_view key);

    /// @brief returns the longest substring that's common between the
    ///        two given strings starting from character 0
    std::string_view longestCommonString(const std::string_view firstStr,
                                         const std::string_view secondStr);
    
    /// @brief Returns a string consisting of N white spaces
    std::string whiteSpaces(const unsigned int n, const std::string_view space =" ");

    /// Converts a string into an integer  
    int atoi(std::string_view str);
    /// Converts a string into a double / float
    double atof(std::string_view str);
    /// Removes all trailing and starting whitespaces from a string
    std::string_view eraseWhiteSpaces(std::string_view str);

    /// Splits the string into smaller substrings
    std::vector<std::string> tokenize(const std::string& the_str,
                                      std::string_view delimiter);
    
    std::vector<double> tokenizeDouble(const std::string& the_str,
                                       std::string_view delimiter);
                                   
    std::vector<int> tokenizeInt(const std::string& the_str,
                                 std::string_view delimiter);


    template <class ObjType> std::string chainUp(unsigned int numEles,
                                                 const std::function<ObjType(unsigned int)>& func,
                                                 const std::string_view glue=";");
    
    template <class ObjType> std::string chainUp(const std::vector<ObjType>& vector,
                                                 const std::string_view glue =";");

    /// Function to set the precision of the conversion from double to string
    std::string to_string_with_precision(const double a_value, const unsigned n = 16);

    void printStdVectorStrings(const std::vector<std::string>& vec);
    /// @brief Converts the numerical address value of a bare pointer to a hex 
    /// string representation. Used in GeoModelIO/GeoModelWrite
    std::string getAddressStringFromPointer(const auto *ptr);
}
#include "GeoModelHelpers/StringUtils.icc"
#endif