#ifndef LAZR_ARGUMENT_H
#define LAZR_ARGUMENT_H

#include <string>

class Argument {
  public:
    Argument(int arg_id,
             int call_id,
             int fun_id,
             int call_env_id,
             const std::string& arg_name,
             int formal_pos,
             int arg_count,
             int vararg,
             int missing,
             const std::string& arg_type,
             const std::string& expr_type,
             const std::string& val_type,
             int preforced)
        : arg_id_(arg_id)
        , call_id_(call_id)
        , fun_id_(fun_id)
        , call_env_id_(call_env_id)
        , arg_name_(arg_name)
        , formal_pos_(formal_pos)
        , actual_pos_(NA_INTEGER)
        , force_pos_(NA_INTEGER)
        , arg_count_(arg_count)
        , vararg_(vararg)
        , missing_(missing)
        , arg_type_(arg_type)
        , expr_type_(expr_type)
        , val_type_(val_type)
        , preforced_(preforced)
        , cap_force_(0)
        , cap_meta_(0)
        , cap_lookup_(0)
        , escaped_(0)
        , esc_force_(0)
        , esc_meta_(0)
        , esc_lookup_(0)
        , force_depth_(NA_INTEGER)
        , comp_pos_(NA_INTEGER)
        , event_seq_("")
        , self_effect_seq_("")
        , effect_seq_("")
        , ref_seq_({}) {
        cap_force_ = preforced;
    }

    int get_id() {
        return arg_id_;
    }

    int get_call_id() {
        return call_id_;
    }

    int get_position() {
        return formal_pos_;
    }

    void set_force_position(int force_pos) {
        force_pos_ = force_pos;
    }

    void set_actual_position(int actual_pos) {
        actual_pos_ = actual_pos;
    }

    void force(int force_depth, int comp_pos) {
        if (escaped_) {
            ++esc_force_;
        } else {
            ++cap_force_;
        }
        force_depth_ = force_depth;

        add_event_('F');
        comp_pos_ = comp_pos;
    }

    void set_value_type(const std::string& val_type) {
        val_type_ = val_type;
    }

    void lookup() {
        if (escaped_) {
            ++esc_lookup_;
        } else {
            ++cap_lookup_;
        }

        add_event_('L');
    }

    void metaprogram() {
        if (escaped_) {
            ++esc_meta_;
        } else {
            ++cap_meta_;
        }

        add_event_('M');
    }

    void escaped() {
        if (escaped_) {
            return;
        }

        escaped_ = true;
        add_event_('E');
    }

    void side_effect(const char type, bool transitive) {
        update_effect_seq_(type, transitive);
    }

    void reflection(const std::string& name) {
        if (ref_seq_.empty()) {
            ref_seq_.push_back({name, 1});
        }

        else if (ref_seq_.back().first == name) {
            ++ref_seq_.back().second;
        }

        else {
            ref_seq_.push_back({name, 1});
        }
    }

    void to_sexp(int index,
                 SEXP r_arg_id,
                 SEXP r_call_id,
                 SEXP r_fun_id,
                 SEXP r_call_env_id,
                 SEXP r_arg_name,
                 SEXP r_formal_pos,
                 SEXP r_force_pos,
                 SEXP r_actual_pos,
                 SEXP r_arg_count,
                 SEXP r_vararg,
                 SEXP r_missing,
                 SEXP r_arg_type,
                 SEXP r_expr_type,
                 SEXP r_val_type,
                 SEXP r_preforced,
                 SEXP r_cap_force,
                 SEXP r_cap_meta,
                 SEXP r_cap_lookup,
                 SEXP r_escaped,
                 SEXP r_esc_force,
                 SEXP r_esc_meta,
                 SEXP r_esc_lookup,
                 SEXP r_force_depth,
                 SEXP r_comp_pos,
                 SEXP r_event_seq,
                 SEXP r_self_effect_seq,
                 SEXP r_effect_seq,
                 SEXP r_ref_seq) {
        SET_INTEGER_ELT(r_arg_id, index, arg_id_);
        SET_INTEGER_ELT(r_call_id, index, call_id_);
        SET_INTEGER_ELT(r_fun_id, index, fun_id_);
        SET_INTEGER_ELT(r_call_env_id, index, call_env_id_);
        SET_STRING_ELT(r_arg_name, index, make_char(arg_name_));
        SET_INTEGER_ELT(r_formal_pos, index, formal_pos_);
        SET_INTEGER_ELT(r_force_pos, index, force_pos_);
        SET_INTEGER_ELT(r_actual_pos, index, actual_pos_);
        SET_INTEGER_ELT(r_arg_count, index, arg_count_);
        SET_LOGICAL_ELT(r_vararg, index, vararg_);
        SET_LOGICAL_ELT(r_missing, index, missing_);
        SET_STRING_ELT(r_arg_type, index, make_char(arg_type_));
        SET_STRING_ELT(r_expr_type, index, make_char(expr_type_));
        SET_STRING_ELT(r_val_type, index, make_char(val_type_));
        SET_INTEGER_ELT(r_preforced, index, preforced_);
        SET_INTEGER_ELT(r_cap_force, index, cap_force_);
        SET_INTEGER_ELT(r_cap_meta, index, cap_meta_);
        SET_INTEGER_ELT(r_cap_lookup, index, cap_lookup_);
        SET_LOGICAL_ELT(r_escaped, index, escaped_);
        SET_INTEGER_ELT(r_esc_force, index, esc_force_);
        SET_INTEGER_ELT(r_esc_meta, index, esc_meta_);
        SET_INTEGER_ELT(r_esc_lookup, index, esc_lookup_);
        SET_INTEGER_ELT(r_force_depth, index, force_depth_);
        SET_INTEGER_ELT(r_comp_pos, index, comp_pos_);
        SET_STRING_ELT(r_event_seq, index, make_char(event_seq_));
        SET_STRING_ELT(r_self_effect_seq, index, make_char(self_effect_seq_));
        SET_STRING_ELT(r_effect_seq, index, make_char(effect_seq_));
        SET_STRING_ELT(r_ref_seq, index, make_char(to_string(ref_seq_)));
    }

  private:
    int arg_id_;
    int call_id_;
    int fun_id_;
    int call_env_id_;
    std::string arg_name_;
    int formal_pos_;
    int force_pos_;
    int actual_pos_;
    int arg_count_;
    int vararg_;
    int missing_;
    std::string arg_type_;
    std::string expr_type_;
    std::string val_type_;
    int preforced_;
    int cap_force_;
    int cap_meta_;
    int cap_lookup_;
    int escaped_;
    int esc_force_;
    int esc_meta_;
    int esc_lookup_;
    int force_depth_;
    int comp_pos_;
    std::string event_seq_;
    std::string self_effect_seq_;
    std::string effect_seq_;
    std::vector<std::pair<std::string, int>> ref_seq_;

    void add_event_(char event) {
        event_seq_.push_back(event);
    }

    void update_effect_seq_(const char type, bool transitive) {
        effect_seq_.push_back(type);
        if (!transitive) {
            self_effect_seq_.push_back(type);
        }
    }
};

#endif /* LAZR_ARGUMENT_H */
