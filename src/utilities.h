#ifndef LAZR_UTILITIES_H
#define LAZR_UTILITIES_H

#include <vector>
#include <string>
#include "Rincludes.h"

extern const std::string LAZR_NA_STRING;

std::string get_type_as_string(SEXP r_object);

SEXP integer_vector_wrap(const std::vector<int>& vector);

SEXP real_vector_wrap(const std::vector<double>& vector);

SEXP character_vector_wrap(const std::vector<std::string>& vector);

SEXP logical_vector_wrap(const std::vector<int>& vector);

void set_class(SEXP r_object, const std::string& class_name);

SEXP create_data_frame(const std::vector<std::string>& names,
                       const std::vector<SEXP>& columns);

SEXP make_char(const std::string& input);

std::string charptr_to_string(const char* charptr);

std::string to_string(const std::vector<std::pair<std::string, int>>& seq);

std::string to_string(const std::vector<int>& seq);

#endif /* LAZR_UTILITIES_H */
