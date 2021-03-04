#ifndef STRICTR_ARGUMENT_DATA_H
#define STRICTR_ARGUMENT_DATA_H

#include <string>
#include <vector>
#include "utilities.h"

class ArgumentData {
  public:
    ArgumentData() {
    }

    void push_back(int parameter_id = NA_INTEGER,
                   int call_id = NA_INTEGER,
                   const std::string& package_name = STRICTR_NA_STRING,
                   const std::string& function_name = STRICTR_NA_STRING,
                   int parameter_position = NA_INTEGER,
                   const std::string& parameter_name = STRICTR_NA_STRING,
                   int vararg = NA_INTEGER,
                   int missing = NA_INTEGER,
                   const std::string& expression_type = STRICTR_NA_STRING,
                   const std::string& transitive_type = STRICTR_NA_STRING,
                   const std::string& value_type = STRICTR_NA_STRING,
                   int forced = NA_INTEGER,
                   int metaprogrammed = NA_INTEGER,
                   int lookup_count = NA_INTEGER,
                   int force_depth = NA_INTEGER,
                   int force_source = NA_INTEGER,
                   int escaped = NA_INTEGER,
                   const std::string& event_sequence = STRICTR_NA_STRING,
                   double evaluation_time = NA_REAL) {
        parameter_id_seq_.push_back(parameter_id);
        call_id_seq_.push_back(call_id);
        package_name_seq_.push_back(package_name);
        function_name_seq_.push_back(function_name);
        parameter_position_seq_.push_back(parameter_position);
        parameter_name_seq_.push_back(parameter_name);
        vararg_seq_.push_back(vararg);
        missing_seq_.push_back(missing);
        expression_type_seq_.push_back(expression_type);
        transitive_type_seq_.push_back(transitive_type);
        value_type_seq_.push_back(value_type);
        forced_seq_.push_back(forced);
        metaprogrammed_seq_.push_back(metaprogrammed);
        lookup_count_seq_.push_back(lookup_count);
        force_depth_seq_.push_back(force_depth);
        force_source_seq_.push_back(force_source);
        escaped_seq_.push_back(escaped);
        event_sequence_seq_.push_back(event_sequence);
        evaluation_time_seq_.push_back(evaluation_time);
    }

    SEXP to_sexp() {
        std::vector<SEXP> columns({integer_vector_wrap(parameter_id_seq_),
                                   integer_vector_wrap(call_id_seq_),
                                   character_vector_wrap(package_name_seq_),
                                   character_vector_wrap(function_name_seq_),
                                   integer_vector_wrap(parameter_position_seq_),
                                   character_vector_wrap(parameter_name_seq_),
                                   logical_vector_wrap(vararg_seq_),
                                   logical_vector_wrap(missing_seq_),
                                   character_vector_wrap(expression_type_seq_),
                                   character_vector_wrap(transitive_type_seq_),
                                   character_vector_wrap(value_type_seq_),
                                   logical_vector_wrap(forced_seq_),
                                   logical_vector_wrap(metaprogrammed_seq_),
                                   integer_vector_wrap(lookup_count_seq_),
                                   integer_vector_wrap(force_depth_seq_),
                                   integer_vector_wrap(force_source_seq_),
                                   logical_vector_wrap(escaped_seq_),
                                   character_vector_wrap(event_sequence_seq_),
                                   real_vector_wrap(evaluation_time_seq_)});
        std::vector<std::string> names({"parameter_id",
                                        "call_id",
                                        "package_name",
                                        "function_name",
                                        "parameter_position",
                                        "parameter_name",
                                        "vararg",
                                        "missing",
                                        "expression_type",
                                        "transitive_type",
                                        "value_type",
                                        "forced",
                                        "metaprogrammed",
                                        "lookup_count",
                                        "force_depth",
                                        "force_source",
                                        "escaped",
                                        "event_sequence",
                                        "evaluation_time"});

        return create_data_frame(names, columns);
    }

  private:
    std::vector<int> parameter_id_seq_;
    std::vector<int> call_id_seq_;
    std::vector<std::string> package_name_seq_;
    std::vector<std::string> function_name_seq_;
    std::vector<int> parameter_position_seq_;
    std::vector<std::string> parameter_name_seq_;
    std::vector<int> vararg_seq_;
    std::vector<int> missing_seq_;
    std::vector<std::string> expression_type_seq_;
    std::vector<std::string> transitive_type_seq_;
    std::vector<std::string> value_type_seq_;
    std::vector<int> forced_seq_;
    std::vector<int> metaprogrammed_seq_;
    std::vector<int> lookup_count_seq_;
    std::vector<int> force_depth_seq_;
    std::vector<int> force_source_seq_;
    std::vector<int> escaped_seq_;
    std::vector<std::string> event_sequence_seq_;
    std::vector<double> evaluation_time_seq_;
};

#endif /* STRICTR_ARGUMENT_DATA_H */
