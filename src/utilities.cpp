#include "utilities.h"

const std::string LAZR_NA_STRING("***LAZR_NA_STRING***");

std::string get_type_as_string(SEXP r_object) {
    return type2char(TYPEOF(r_object));
}

SEXP integer_vector_wrap(const std::vector<int>& vector) {
    int size = vector.size();
    SEXP r_vector = PROTECT(allocVector(INTSXP, size));

    for (int i = 0; i < size; ++i) {
        INTEGER(r_vector)[i] = vector[i];
    }

    UNPROTECT(1);
    return r_vector;
}

SEXP real_vector_wrap(const std::vector<double>& vector) {
    int size = vector.size();
    SEXP r_vector = PROTECT(allocVector(REALSXP, size));

    for (int i = 0; i < size; ++i) {
        REAL(r_vector)[i] = vector[i];
    }

    UNPROTECT(1);
    return r_vector;
}

SEXP character_vector_wrap(const std::vector<std::string>& vector) {
    int size = vector.size();
    SEXP r_vector = PROTECT(allocVector(STRSXP, size));

    for (int i = 0; i < size; ++i) {
        const std::string& value = vector[i];
        SET_STRING_ELT(r_vector,
                       i,
                       value == LAZR_NA_STRING ? NA_STRING
                                               : mkChar(value.c_str()));
    }

    UNPROTECT(1);
    return r_vector;
}

SEXP logical_vector_wrap(const std::vector<int>& vector) {
    int size = vector.size();
    SEXP r_vector = PROTECT(allocVector(LGLSXP, size));

    for (int i = 0; i < size; ++i) {
        LOGICAL(r_vector)[i] = vector[i];
    }

    UNPROTECT(1);
    return r_vector;
}

void set_class(SEXP r_object, const std::string& class_name) {
    setAttrib(r_object, R_ClassSymbol, mkString(class_name.c_str()));
}

SEXP create_data_frame(const std::vector<std::string>& names,
                       const std::vector<SEXP>& columns) {
    int column_count = names.size();
    int row_count = LENGTH(columns[0]);

    SEXP r_list = PROTECT(allocVector(VECSXP, column_count));

    for (int i = 0; i < column_count; ++i) {
        SET_VECTOR_ELT(r_list, i, columns[i]);
    }

    Rf_setAttrib(r_list, R_NamesSymbol, character_vector_wrap(names));

    set_class(r_list, "data.frame");

    SEXP r_row_names = PROTECT(allocVector(STRSXP, row_count));

    for (int i = 0; i < row_count; ++i) {
        SET_STRING_ELT(r_row_names, i, mkChar(std::to_string(i + 1).c_str()));
    }

    Rf_setAttrib(r_list, R_RowNamesSymbol, r_row_names);

    UNPROTECT(2);

    return r_list;
}

SEXP make_char(const std::string& input) {
    return input == LAZR_NA_STRING ? NA_STRING : mkChar(input.c_str());
}

std::string charptr_to_string(const char* charptr) {
    return charptr == nullptr ? LAZR_NA_STRING : std::string(charptr);
}

std::string to_string(const std::vector<std::pair<std::string, int>>& seq) {
    std::string str;
    int size = seq.size();

    for (int i = 0; i < size - 1; ++i) {
        const auto& pair = seq[i];
        str.append(pair.first);
        str.push_back(':');
        str.append(std::to_string(pair.second));
        str.push_back('|');
    }
    if (size != 0) {
        const auto& pair = seq[size - 1];
        str.append(pair.first);
        str.push_back(':');
        str.append(std::to_string(pair.second));
    }

    return str;
}

std::string to_string(const std::vector<int>& seq) {
    std::string str;
    int size = seq.size();

    for (int i = 0; i < size - 1; ++i) {
        str.append(std::to_string(seq[i]));
        str.append("|");
    }

    if (size != 0) {
        str.append(std::to_string(seq[size - 1]));
    }

    return str;
}

SEXP run_length_encoding_helper(const char* element) {
    if (element == NULL) {
        return NA_STRING;
    }

    std::string new_element;

    int len = strlen(element);

    if (len == 0) {
        return mkChar(element);
    }

    int i = 0;
    while (i < len) {
        new_element.push_back(element[i]);
        new_element.push_back('+');
        ++i;
        while (i < len && element[i] == element[i - 1]) {
            ++i;
        }
    }

    return mkChar(new_element.c_str());
}

SEXP r_lazr_run_length_encoding(SEXP r_input) {
    int length = Rf_length(r_input);
    SEXP r_output = PROTECT(allocVector(STRSXP, length));

    for (int i = 0; i < length; ++i) {
        SEXP r_char = STRING_ELT(r_input, i);
        const char* element = r_char == NA_STRING ? NULL : CHAR(r_char);
        SEXP r_new_char = run_length_encoding_helper(element);
        SET_STRING_ELT(r_output, i, r_new_char);
    }

    UNPROTECT(1);
    return r_output;
}
