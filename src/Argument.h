#ifndef LAZR_ARGUMENT_H
#define LAZR_ARGUMENT_H

#include <string>

class Argument {
  public:
    Argument(int parameter_id,
             int call_id,
             const std::string& package_name,
             const std::string& function_name,
             int parameter_position,
             const std::string& parameter_name,
             int argument_count,
             int vararg,
             int missing,
             const std::string& argument_type,
             const std::string& expression_type,
             const std::string& transitive_type = LAZR_NA_STRING,
             const std::string& value_type = LAZR_NA_STRING,
             int preforced = 0)
        : parameter_id_(parameter_id)
        , call_id_(call_id)
        , package_name_(package_name)
        , function_name_(function_name)
        , parameter_position_(parameter_position)
        , parameter_name_(parameter_name)
        , argument_count_(argument_count)
        , vararg_(vararg)
        , missing_(missing)
        , argument_type_(argument_type)
        , expression_type_(expression_type)
        , transitive_type_(transitive_type)
        , value_type_(value_type)
        , preforced_(preforced)
        , cap_force_(0)
        , cap_meta_(0)
        , cap_lookup_(0)
        , escaped_(0)
        , esc_force_(0)
        , esc_meta_(0)
        , esc_lookup_(0)
        , force_depth_(NA_INTEGER)
        , force_source_(LAZR_NA_STRING)
        , companion_position_(NA_INTEGER)
        , event_sequence_("") {
        cap_force_ = preforced;
    }

    int get_parameter_id() {
        return parameter_id_;
    }

    void force(const std::string& value_type,
               int force_depth,
               int companion_position) {
        if (escaped_) {
            ++esc_force_;
        } else {
            ++cap_force_;
        }
        force_depth_ = force_depth;

        add_event_('F');
        value_type_ = value_type;
        companion_position_ = companion_position;
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

    void to_sexp(int index,
                 SEXP r_parameter_id,
                 SEXP r_call_id,
                 SEXP r_package_name,
                 SEXP r_function_name,
                 SEXP r_parameter_position,
                 SEXP r_parameter_name,
                 SEXP r_argument_count,
                 SEXP r_vararg,
                 SEXP r_missing,
                 SEXP r_argument_type,
                 SEXP r_expression_type,
                 SEXP r_transitive_type,
                 SEXP r_value_type,
                 SEXP r_preforced,
                 SEXP r_cap_force,
                 SEXP r_cap_meta,
                 SEXP r_cap_lookup,
                 SEXP r_escaped,
                 SEXP r_esc_force,
                 SEXP r_esc_meta,
                 SEXP r_esc_lookup,
                 SEXP r_force_depth,
                 SEXP r_force_source,
                 SEXP r_companion_position,
                 SEXP r_event_sequence) {
        INTEGER(r_parameter_id)[index] = parameter_id_;
        INTEGER(r_call_id)[index] = call_id_;
        SET_STRING_ELT(r_package_name, index, make_char(package_name_));
        SET_STRING_ELT(r_function_name, index, make_char(function_name_));
        INTEGER(r_parameter_position)[index] = parameter_position_;
        SET_STRING_ELT(r_parameter_name, index, make_char(parameter_name_));
        INTEGER(r_argument_count)[index] = argument_count_;
        LOGICAL(r_vararg)[index] = vararg_;
        LOGICAL(r_missing)[index] = missing_;
        SET_STRING_ELT(r_argument_type, index, make_char(argument_type_));
        SET_STRING_ELT(r_expression_type, index, make_char(expression_type_));
        SET_STRING_ELT(r_transitive_type, index, make_char(transitive_type_));
        SET_STRING_ELT(r_value_type, index, make_char(value_type_));
        INTEGER(r_preforced)[index] = preforced_;
        INTEGER(r_cap_force)[index] = cap_force_;
        INTEGER(r_cap_meta)[index] = cap_meta_;
        INTEGER(r_cap_lookup)[index] = cap_lookup_;
        LOGICAL(r_escaped)[index] = escaped_;
        INTEGER(r_esc_force)[index] = esc_force_;
        INTEGER(r_esc_meta)[index] = esc_meta_;
        INTEGER(r_esc_lookup)[index] = esc_lookup_;
        INTEGER(r_force_depth)[index] = force_depth_;
        SET_STRING_ELT(r_force_source, index, make_char(force_source_));
        INTEGER(r_companion_position)[index] = companion_position_;
        SET_STRING_ELT(r_event_sequence, index, make_char(event_sequence_));
    }

  private:
    int parameter_id_;
    int call_id_;
    std::string package_name_;
    std::string function_name_;
    int parameter_position_;
    std::string parameter_name_;
    int argument_count_;
    int vararg_;
    int missing_;
    std::string argument_type_;
    std::string expression_type_;
    std::string transitive_type_;
    std::string value_type_;
    int preforced_;
    int cap_force_;
    int cap_meta_;
    int cap_lookup_;
    int escaped_;
    int esc_force_;
    int esc_meta_;
    int esc_lookup_;
    int force_depth_;
    std::string force_source_;
    int companion_position_;
    std::string event_sequence_;

    void add_event_(char event) {
        event_sequence_.push_back(event);
    }
};

#endif /* LAZR_ARGUMENT_H */
