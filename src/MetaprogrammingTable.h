#ifndef LAZR_METAPROGRAMMING_TABLE_H
#define LAZR_METAPROGRAMMING_TABLE_H

#include <vector>
#include <string>
#include <instrumentr/instrumentr.h>

class MetaprogrammingTable {
  public:
    MetaprogrammingTable() {
    }

    ~MetaprogrammingTable() {
    }

    void insert(const std::string& meta_type,
                int source_fun_id,
                int source_call_id,
                int source_arg_id,
                int source_formal_pos,
                int sink_fun_id,
                int sink_call_id,
                int depth) {
        /* to avoid double counting, expression metaprogramming case is not
           added after a substitute with same fields */
        if (meta_type == "expression") {
            if (meta_type_.size() != 0 && meta_type_.back() == "substitute" &&
                source_fun_id_.back() == source_fun_id &&
                source_call_id_.back() == source_call_id &&
                source_arg_id_.back() == source_arg_id &&
                source_formal_pos_.back() == source_formal_pos &&
                sink_fun_id_.back() == sink_fun_id &&
                sink_call_id_.back() == sink_call_id &&
                depth_.back() == depth) {
                return;
            }
        }
        meta_type_.push_back(meta_type);
        source_fun_id_.push_back(source_fun_id);
        source_call_id_.push_back(source_call_id);
        source_arg_id_.push_back(source_arg_id);
        source_formal_pos_.push_back(source_formal_pos);
        sink_fun_id_.push_back(sink_fun_id);
        sink_call_id_.push_back(sink_call_id);
        depth_.push_back(depth);
    }

    SEXP to_sexp() {
        int size = meta_type_.size();

        SEXP r_meta_type = PROTECT(allocVector(STRSXP, size));
        SEXP r_source_fun_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_source_call_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_source_arg_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_source_formal_pos = PROTECT(allocVector(INTSXP, size));
        SEXP r_sink_fun_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_sink_call_id = PROTECT(allocVector(INTSXP, size));
        SEXP r_depth = PROTECT(allocVector(INTSXP, size));

        for (int index = 0; index < size; ++index) {
            SET_STRING_ELT(r_meta_type, index, make_char(meta_type_[index]));
            SET_INTEGER_ELT(r_source_fun_id, index, source_fun_id_[index]);
            SET_INTEGER_ELT(r_source_call_id, index, source_call_id_[index]);
            SET_INTEGER_ELT(r_source_arg_id, index, source_arg_id_[index]);
            SET_INTEGER_ELT(
                r_source_formal_pos, index, source_formal_pos_[index]);
            SET_INTEGER_ELT(r_sink_fun_id, index, sink_fun_id_[index]);
            SET_INTEGER_ELT(r_sink_call_id, index, sink_call_id_[index]);
            SET_INTEGER_ELT(r_depth, index, depth_[index]);
        }

        std::vector<SEXP> columns({r_meta_type,
                                   r_source_fun_id,
                                   r_source_call_id,
                                   r_source_arg_id,
                                   r_source_formal_pos,
                                   r_sink_fun_id,
                                   r_sink_call_id,
                                   r_depth});

        std::vector<std::string> names({"meta_type",
                                        "source_fun_id",
                                        "source_call_id",
                                        "source_arg_id",
                                        "source_formal_pos",
                                        "sink_fun_id",
                                        "sink_call_id",
                                        "depth"});

        SEXP df = create_data_frame(names, columns);

        UNPROTECT(8);

        return df;
    }

  private:
    std::vector<std::string> meta_type_;
    std::vector<int> source_fun_id_;
    std::vector<int> source_call_id_;
    std::vector<int> source_arg_id_;
    std::vector<int> source_formal_pos_;
    std::vector<int> sink_fun_id_;
    std::vector<int> sink_call_id_;
    std::vector<int> depth_;
};

#endif /* LAZR_METAPROGRAMMING_TABLE_H */
