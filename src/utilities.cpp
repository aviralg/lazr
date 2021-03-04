#include "utilities.h"

const std::string STRICTR_NA_STRING("***STRICTR_NA_STRING***");

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
                       value == STRICTR_NA_STRING ? NA_STRING
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
