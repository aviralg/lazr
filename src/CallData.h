#ifndef STRICTR_CALL_DATA_H
#define STRICTR_CALL_DATA_H

#include <string>
#include <vector>
#include "utilities.h"

class CallData {
  public:
    CallData() {
    }

    void push_back(int call_id = NA_INTEGER,
                   const std::string& package_name = STRICTR_NA_STRING,
                   const std::string& function_name = STRICTR_NA_STRING,
                   int successful = NA_INTEGER,
                   const std::string& result_type = STRICTR_NA_STRING,
                   const std::string& force_order = STRICTR_NA_STRING,
                   int c_call_count = NA_INTEGER,
                   int r_call_count = NA_INTEGER,
                   double c_execution_time = NA_REAL,
                   double r_execution_time = NA_REAL) {
        call_id_seq_.push_back(call_id);
        package_name_seq_.push_back(package_name);
        function_name_seq_.push_back(function_name);
        successful_seq_.push_back(successful);
        result_type_seq_.push_back(result_type);
        force_order_seq_.push_back(force_order);
        c_call_count_seq_.push_back(c_call_count);
        r_call_count_seq_.push_back(r_call_count);
        c_execution_time_seq_.push_back(c_execution_time);
        r_execution_time_seq_.push_back(r_execution_time);
    }

    SEXP to_sexp() {
        std::vector<SEXP> columns({integer_vector_wrap(call_id_seq_),
                                   character_vector_wrap(package_name_seq_),
                                   character_vector_wrap(function_name_seq_),
                                   character_vector_wrap(result_type_seq_),
                                   character_vector_wrap(force_order_seq_),
                                   integer_vector_wrap(c_call_count_seq_),
                                   integer_vector_wrap(r_call_count_seq_),
                                   real_vector_wrap(c_execution_time_seq_),
                                   real_vector_wrap(r_execution_time_seq_)});

        std::vector<std::string> names({"call_id",
                                        "package_name",
                                        "function_name",
                                        "result_type",
                                        "force_order",
                                        "c_call_count",
                                        "r_call_count",
                                        "c_execution_time",
                                        "r_execution_time"});

        return create_data_frame(names, columns);
    }

  private:
    std::vector<int> call_id_seq_;
    std::vector<std::string> package_name_seq_;
    std::vector<std::string> function_name_seq_;
    std::vector<int> successful_seq_;
    std::vector<std::string> result_type_seq_;
    std::vector<std::string> force_order_seq_;
    std::vector<int> c_call_count_seq_;
    std::vector<int> r_call_count_seq_;
    std::vector<double> c_execution_time_seq_;
    std::vector<double> r_execution_time_seq_;
};

#endif /* STRICTR_CALL_DATA_H */
