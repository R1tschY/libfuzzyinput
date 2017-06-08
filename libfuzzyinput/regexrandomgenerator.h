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

#pragma once

#include <strtox/stringview.h>
#include <strtox/regex.h>

#include <boost/iterator/iterator_facade.hpp>

namespace fuzzy_input {

class RegexCombinations;

struct RegexCombinationNode
{
  std::size_t combinations = 0;
  std::vector<RegexCombinationNode> children;
};

class RegexCombinationIterator
  : public boost::iterator_facade<
    RegexCombinationIterator, std::string, std::random_access_iterator_tag, std::string>
{
public:
  RegexCombinationIterator(const RegexCombinations& combinations, std::size_t i) :
    combinations_(combinations), index_(i)
  { }

private:
  friend class boost::iterator_core_access;

  std::ptrdiff_t distance_to(const RegexCombinationIterator& other) const
  { return other.index_ - index_; }

  void advance(std::size_t n) { index_ += n; }
  void decrement() { index_ -= 1; }
  void increment() { index_ += 1; }

  bool equal(const RegexCombinationIterator& other) const
  {
      return &combinations_ == &other.combinations_ && index_ == other.index_;
  }

  std::string dereference() const;

  const RegexCombinations& combinations_;
  std::size_t index_;
};


class RegexCombinations
{
public:
  RegexCombinations(strtox::string_view regex);

  std::string operator[](std::size_t n) const;
  std::size_t size() const { return tree_.combinations; }

  RegexCombinationIterator begin() const
  { return RegexCombinationIterator(*this, 0); }

  RegexCombinationIterator end() const
  { return RegexCombinationIterator(*this, size()); }

private:
  RegexCombinationNode tree_;
  strtox::regex::ast ast_;
};

/// \brief
class RegexRandomGenerator
{
public:
  RegexRandomGenerator(strtox::string_view re);

private:

};


inline
std::string RegexCombinationIterator::dereference() const
{
  return combinations_[index_];
}

} // namespace fuzzy_input
