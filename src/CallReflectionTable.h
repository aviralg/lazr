#ifndef LAZR_CALL_REFLECTION_TABLE_H
#define LAZR_CALL_REFLECTION_TABLE_H

#include <vector>
#include <string>
#include <instrumentr/instrumentr.h>

class CallReflectionTable {
  public:
    CallReflectionTable() {
    }

    ~CallReflectionTable() {
    }

    void insert(int ref_call_id,
                const std::string& ref_type,
                int source_fun_id,
                int source_call_id,
                int sink_fun_id,
                int sink_call_id,
                int sink_arg_id,
                int sink_formal_pos,
                int depth) {
        ref_call_id_.push_back(ref_call_id);
        ref_type_.push_back(ref_type);
        source_fun_id_.push_back(source_fun_id);
        source_call_id_.push_back(source_call_id);
        sink_fun_id_.push_back(sink_fun_id);
        sink_call_id_.push_back(sink_call_id);
        sink_arg_id_.push_back(sink_arg_id);
        sink_formal_pos_.push_back(sink_formal_pos);
        depth_.push_back(depth);
    }

    SEXP to_sexp() {
        int size = ref_type_.size();

        SEXP r_ref_call_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_ref_type = PROTECT(allocVector(STRSXP, size));
        SEXP r_source_fun_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_source_call_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_sink_fun_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_sink_call_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_sink_arg_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_sink_formal_pos = PROTECT(allocVector(INTSXP, size));
        SEXP r_depth = PROTECT(allocVector(INTSXP, size));

        for (int index = 0; index < size; ++index) {
            SET_INTEGER_ELT(r_ref_call_id, index, ref_call_id_[index]);
            SET_STRING_ELT(r_ref_type, index, make_char(ref_type_[index]));
            SET_INTEGER_ELT(r_source_fun_id, index, source_fun_id_[index]);
            SET_INTEGER_ELT(r_source_call_id, index, source_call_id_[index]);
            SET_INTEGER_ELT(r_sink_fun_id, index, sink_fun_id_[index]);
            SET_INTEGER_ELT(r_sink_call_id, index, sink_call_id_[index]);
            SET_INTEGER_ELT(r_sink_arg_id, index, sink_arg_id_[index]);
            SET_INTEGER_ELT(r_sink_formal_pos, index, sink_formal_pos_[index]);
            SET_INTEGER_ELT(r_depth, index, depth_[index]);
        }

        std::vector<SEXP> columns({r_ref_call_id,
                                   r_ref_type,
                                   r_source_fun_id,
                                   r_source_call_id,
                                   r_sink_fun_id,
                                   r_sink_call_id,
                                   r_sink_arg_id,
                                   r_sink_formal_pos,
                                   r_depth});

        std::vector<std::string> names({"ref_call_id",
                                        "ref_type",
                                        "source_fun_id",
                                        "source_call_id",
                                        "sink_fun_id",
                                        "sink_call_id",
                                        "sink_arg_id",
                                        "sink_formal_pos",
                                        "depth"});

        SEXP df = create_data_frame(names, columns);

        UNPROTECT(9);

        return df;
    }

  private:
    std::vector<int> ref_call_id_;
    std::vector<std::string> ref_type_;
    std::vector<int> source_fun_id_;
    std::vector<int> source_call_id_;
    std::vector<int> sink_fun_id_;
    std::vector<int> sink_call_id_;
    std::vector<int> sink_arg_id_;
    std::vector<int> sink_formal_pos_;
    std::vector<int> depth_;
};

#endif /* LAZR_CALL_REFLECTION_TABLE_H */
