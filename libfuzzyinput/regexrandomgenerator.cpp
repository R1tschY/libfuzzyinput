///
/// Copyright (c) 2016 R1tschY
/// 
/// Permission is hereby granted, free of charge, to any person obtaining a copy
/// of this software and associated documentation files (the "Software"), to 
/// deal in the Software without restriction, including without limitation the 
/// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or 
/// sell copies of the Software, and to permit persons to whom the Software is
/// furnished to do so, subject to the following conditions:
/// 
/// The above copyright notice and this permission notice shall be included in
/// all copies or substantial portions of the Software.
/// 
/// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
/// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
/// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
/// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER 
/// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING 
/// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
/// IN THE SOFTWARE.
///

#include "regexrandomgenerator.h"

#include <iostream>
#include <algorithm>

using namespace strtox;

namespace fuzzy_input {

namespace {

constexpr
int ipow(int base, unsigned exp)
{
  int result = 1;
  while (exp)
  {
    if (exp & 1)
      result *= base;
    exp >>= 1;
    base *= base;
  }

  return result;
}

namespace geometric_seq {

constexpr
int partial_sum(int q, std::size_t n)
{
  if (q == 1)
    return n + 1;

  return (1 - ipow(q, n + 1)) / (1 - q);
}

constexpr
int partial_sum_range(int q, std::size_t a, std::size_t b)
{
  if (q == 1)
    return b - a + 1;

  return q * (ipow(q, a) - ipow(q, b)) / (1 - q);
}

} // namespace geometric_seq

string_view any_char_chars = "abcdefghijklmnopqrstuvwxyz";

struct build_combinations_tree_visitor : boost::static_visitor<>
{
  RegexCombinationNode& tree_;

  build_combinations_tree_visitor(RegexCombinationNode& tree) : tree_(tree) {}

  void operator()(regex::alternative const & a)
  {
    for (auto& alt : a)
    {
      tree_.children.emplace_back();
      build_combinations_tree_visitor vis(tree_.children.back());
      vis(alt);
    }
    tree_.combinations = std::accumulate(tree_.children.cbegin(), tree_.children.cend(), 0,
      [=](int a, const RegexCombinationNode& b){ return a + b.combinations; });
  }

  void operator()(regex::atom const & v)
  {
    tree_.children.emplace_back();
    build_combinations_tree_visitor vis(tree_.children.back());
    boost::apply_visitor(vis, v.expr_);

    if (v.mult_.unbounded())
      throw std::runtime_error("unbounded repeat not supported");

    if (tree_.children.back().combinations == 1)
    {
      tree_.combinations = *v.mult_.maxoccurs_ - v.mult_.minoccurs_ + 1;
      return;
    }

    if (v.mult_.minoccurs_ > 0)
    {
      tree_.combinations = geometric_seq::partial_sum_range(
        tree_.children.back().combinations, v.mult_.minoccurs_ - 1, *v.mult_.maxoccurs_);
    }
    else
    {
      tree_.combinations = geometric_seq::partial_sum(
        tree_.children.back().combinations, *v.mult_.maxoccurs_);
    }
  }

  void operator()(regex::start_of_match const & v)
  {
    throw std::runtime_error("start of match '^' not supported");
  }

  void operator()(regex::end_of_match const & v)
  {
    throw std::runtime_error("end of match '$' not supported");
  }

  void operator()(regex::any_char const & v)
  {
    tree_.combinations += any_char_chars.length();
  }

  void operator()(regex::group const & v)
  {
    (*this)(v.root_);
  }

  void operator()(std::string const & v)
  {
    tree_.combinations = 1;
  }

  void operator()(regex::sequence const & v)
  {
    for (auto& atom : v)
    {
      tree_.children.emplace_back();
      build_combinations_tree_visitor vis(tree_.children.back());
      vis(atom);
    }
    tree_.combinations = std::accumulate(tree_.children.cbegin(), tree_.children.cend(), 1,
      [=](int a, const RegexCombinationNode& b){ return a * b.combinations; });
  }

  void operator()(regex::charset const & v)
  {
    if (v.negated_)
      throw std::runtime_error("negated charset not supported");

    for (auto& el : v.elements_)
    {
      tree_.children.emplace_back();
      build_combinations_tree_visitor vis(tree_.children.back());
      boost::apply_visitor(vis, el);
    }

    tree_.combinations = std::accumulate(tree_.children.cbegin(), tree_.children.cend(), 0,
      [=](int a, const RegexCombinationNode& b){ return a + b.combinations; });
  }

  void operator()(regex::charset::range const & v)
  {
    using namespace std;
    tree_.combinations += get<1>(v) - get<0>(v) + 1;
  }

  void operator()(char v) const
  {
    tree_.combinations += 1;
  }
};

struct get_combination_visitor : boost::static_visitor<>
{
  const RegexCombinationNode& tree_;
  std::string& output_;
  const std::size_t n_;

  get_combination_visitor(std::size_t n, const RegexCombinationNode& tree, std::string& output) :
    tree_(tree), output_(output), n_(n)
  {
    if (n_ >= tree_.combinations)
      throw std::runtime_error("internal error");
  }

  void operator()(regex::alternative const & a)
  {
    // search right alternative
    std::size_t newn = n_;
    for (std::size_t i = 0; i < a.size(); i += 1)
    {
      if (newn < tree_.children[i].combinations)
      {
        // found alternative
        get_combination_visitor vis(newn, tree_.children[i], output_);
        vis(a[i]);
        return;
      }

      // goto next alternative
      newn -= tree_.children[i].combinations;
    }

    throw std::runtime_error("internal error"); // you should not be here
  }

  void operator()(regex::atom const & v)
  {
    std::size_t s = 1;
    std::size_t m = 1;
    std::size_t n;
    std::size_t q = tree_.children.back().combinations;

    // initalize
    if (v.mult_.minoccurs_ == 0)
    {
      n = n_;
    }
    else if (v.mult_.minoccurs_ == 1)
    {
      n = n_ + 1;
    }
    else
    {
      n = n_ + geometric_seq::partial_sum(q, v.mult_.minoccurs_ - 1);
    }

    // generate
    while (n >= s)
    {
      m *= q;

      get_combination_visitor vis((n - s) % q, tree_.children.back(), output_);
      boost::apply_visitor(vis, v.expr_);

      s += m;
    }
  }

  void operator()(regex::sequence const & v)
  {
    std::size_t n = n_;

    for (std::size_t i = 0; i < v.size(); i += 1)
    {
      std::size_t cn = n % tree_.children[i].combinations;
      std::size_t n = n / tree_.children[i].combinations;

      get_combination_visitor vis(cn, tree_.children[i], output_);
      vis(v[i]);
    }
  }

  void operator()(regex::start_of_match const & v)
  {
    throw std::runtime_error("start of match '^' not supported");
  }

  void operator()(regex::end_of_match const & v)
  {
    throw std::runtime_error("end of match '$' not supported");
  }

  void operator()(regex::any_char const & v)
  {
    throw std::runtime_error("any char '.' not supported");
  }

  void operator()(regex::group const & v)
  {
    (*this)(v.root_);
  }

  void operator()(std::string const & v)
  {
    output_ += v;
  }

  void operator()(regex::charset const & v)
  {
    if (v.negated_)
      throw std::runtime_error("negated charset not supported");

    // search right alternative
    std::size_t newn = n_;
    for (std::size_t i = 0; i < v.elements_.size(); i += 1)
    {
      if (newn < tree_.children[i].combinations)
      {
        // found alternative
        get_combination_visitor vis(newn, tree_.children[i], output_);
        boost::apply_visitor(vis, v.elements_[i]);
        return;
      }

      // goto next alternative
      newn -= tree_.children[i].combinations;
    }

    throw std::runtime_error("internal error"); // you should not be here
  }

  void operator()(regex::charset::range const & v)
  {
    using namespace std;
    output_ += get<0>(v) + n_;
  }

  void operator()(char v) const
  {
    output_ += v;
  }
};

} // namespace



RegexCombinations::RegexCombinations(string_view regex)
{
  bool success = parse_regex(regex, ast_);
  if (!success)
  {
    throw std::invalid_argument("'regex' is not a supported regex expression");
  }

  build_combinations_tree_visitor vis(tree_);
  boost::apply_visitor(vis, ast_);
}

std::string RegexCombinations::operator[](std::size_t n) const
{
  if (n >= tree_.combinations)
    throw std::invalid_argument("'n' is out of range");

  std::string result;
  get_combination_visitor vis(n, tree_, result);
  boost::apply_visitor(vis, ast_);
  return result;
}

RegexRandomGenerator::RegexRandomGenerator(string_view re)
{

}

} // namespace FuzzyInput
