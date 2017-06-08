#include <algorithm>
#include <numeric>
#include <string>
#include <set>
#include <map>
#include <sstream>
#include <functional>
#include <iostream>
#include <boost/test/unit_test.hpp>

#include <strtox/regex.h>
#include <libfuzzyinput/regexrandomgenerator.h>

namespace ast = strtox::regex;

// copied from https://stackoverflow.com/a/21419351/1188453

static std::string multiplicity_text(ast::multiplicity const& m) {
    std::ostringstream os;
    //
         if (m.minoccurs_ == 0 && m.unbounded())           os << '*'                                                ;
    else if (m.minoccurs_ == 1 && m.unbounded())           os << '+'                                                ;
    else if (m.unbounded())                               os << '{'  << m.minoccurs_ << ",}"                        ;
    // cannot be unbounded beyond this point
    else if (m.minoccurs_ == 1 && *m.maxoccurs_ == 1)       { ; }
    else if (m.minoccurs_ == *m.maxoccurs_)                 os << '{'  << m.minoccurs_ << '}'                         ;
    else if (m.minoccurs_ == 0 && *m.maxoccurs_ == 1)       os << '?'                                                ;
    else if (m.minoccurs_ == 0)                            os << "{," << *m.maxoccurs_ << '}'                        ;
    else                                                  os << '{'  << m.minoccurs_ << ","  << *m.maxoccurs_ << "}" ;
    // non-greedyness
    if (m.repeating() && !m.greedy_)
        os << "?";

    return os.str();
}

static std::ostream& escape_into(std::ostream& os, char v, bool escape_dquote = false) {
    switch(v)
    {
        case '\\': return os << "\\\\";
        case '\n': return os << "\\n";
        case '\t': return os << "\\t";
        case '\r': return os << "\\r";
        case '\b': return os << "\\b";
        case '\0': return os << "\\0";
        case '[': return os << "\\[";
        case ']': return os << "\\]";
        case '.': return os << "\\.";
        case '?': return os << "\\?";
        case '*': return os << "\\*";
        case '^': return os << "\\^";
        case '-': return os << "\\-";
        case '$': return os << "\\$";
        case '|': return os << "\\|";
        case '+': return os << "\\+";
        case '{': return os << "\\{";
        case '}': return os << "\\}";
        case '"':  return os << (escape_dquote? "\\\"" : "\"");
        default:
                   return os << v;
    }
}

static std::ostream& escape_into(std::ostream& os, std::string const& v, bool escape_dquote = false) {
    for(auto ch : v)
        escape_into(os, ch, escape_dquote);
    return os;
}

struct regex_tostring : boost::static_visitor<>
{
    std::ostream& os_;
    int id = 0;

    regex_tostring(std::ostream& os) : os_(os) {}
    ~regex_tostring() { os_ << "\n"; }

    void operator()(ast::alternative const & a) const {
        for (size_t i = 0; i<a.size(); ++i)
        {
            if (i) os_ << '|';
            (*this)(a[i]);
        }
    }

    void operator()(ast::atom const & v) const {
        boost::apply_visitor(*this, v.expr_);
        (*this)(v.mult_);
    }

    void operator()(ast::start_of_match    const & v) const { os_ << '^'; }
    void operator()(ast::end_of_match      const & v) const { os_ << '$'; }
    void operator()(ast::any_char          const & v) const { os_ << '.'; }
    void operator()(ast::group             const & v) const { os_ << '('; (*this)(v.root_); os_ << ')'; }
    void operator()(char                   const v)   const { escape_into(os_, v); }
    void operator()(std::string            const & v) const { escape_into(os_, v); }
    void operator()(std::vector<ast::atom> const & v) const { for(auto& atom : v) (*this)(atom); }
    void operator()(ast::multiplicity      const & m) const { os_ << multiplicity_text(m); }

    void operator()(ast::charset           const & v) const {
        os_ << '[';
        if (v.negated_) os_ << '^';
        for(auto& el : v.elements_) boost::apply_visitor(*this, el);
        os_ << ']';
    }
    void operator()(ast::charset::range    const & v) const {
        using std::get;
        escape_into(os_, get<0>(v)) << "-";
        escape_into(os_, get<1>(v));
    }
};

void check_roundtrip(ast::ast tree, std::string input)
{
    std::ostringstream os;
    regex_tostring str(os);

    boost::apply_visitor(str, tree);

    if (os.str() != input)
        std::cerr << "WARNING: '" << input << "' -> '" << os.str() << "'\n";
}


BOOST_AUTO_TEST_SUITE(Regex_Tests)

BOOST_AUTO_TEST_CASE(examples_tests)
{
  for (std::string pattern: {
//    "abc?",
//    "ab{2,3}c",
//    "[a-zA-Z]?",
//    "abc|d",
//    "a?",
//    "(XYZ)|(123)",
    "(\\+|-)?[0-9]{1,2}",
  })
  {
    std::cout << "// ================= " << pattern << " ========\n";
    ast::ast tree;
    if (strtox::parse_regex(pattern, tree))
    {
      check_roundtrip(tree, pattern);

      regex_tostring printer(std::cout);
      boost::apply_visitor(printer, tree);
      std::cout << "\n";

      fuzzy_input::RegexCombinations rng(pattern);
      std::cout << rng.size() << '\n';
      for (std::size_t i = 0; i < rng.size(); i++)
        std::cout << rng[i] << '\n';
    }
  }
}

BOOST_AUTO_TEST_SUITE_END()
