// Copyright (c) 2008-2009, Jeremy Brewer
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above copyright notice,
//      this list of conditions and the following disclaimer.
//    * Redistributions in binary form must reproduce the above copyright
//      notice, this list of conditions and the following disclaimer in the
//      documentation and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// Useful macros and typedefs used nearly everywhere
// Author: Jeremy Brewer

#ifndef BASE_H__
#define BASE_H__

#include <cmath>
#include <cstring>

#include <iostream>
#include <iomanip>
#include <sstream>

// We only dump names from std in this manner.  All other using statements
// should be reserved for .cc files.
using namespace std;

namespace google_sky {

// Typedefs for integer types.
// NOTE: int is assumed to be a signed 32 bit integer.
typedef signed char int8;
typedef short int16;
typedef long long int64;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint;
typedef unsigned long long uint64;

// This macro should be called inside the private section of a class to
// disallow the copy constructor and assignment operator which are a major
// source of memory leaks.  Classes that define this macro cannot be copied
// and must be passed and returned by reference or pointer.
#define DISALLOW_COPY_AND_ASSIGN(classname) \
  classname(const classname &); \
  classname &operator=(const classname &);

// The CHECK macro is our primary way of performing assertions within the
// codebase.  It is best explained with an example:
//
// CHECK(some_expression) << "A failure occured with status " << status;
//
// Then if some_expression is false, the program will die with a message
// indicating the file and line number along with your error message.
//
// You should use CHECK for any condition under which the program cannot
// continue.  For instance, if a function takes the sqrt of a number, that
// function should use CHECK(x >= 0.0) << "Invalid input: " << x;
//
// Additionally, we define several similar checks for convenience below.
#define CHECK(expression) \
  google_sky::Asserter::DUMMY = \
      google_sky::Asserter::GetInstance((expression), __FILE__, __LINE__)

#define CHECK_EQ(a, b) CHECK((a) == (b))
#define CHECK_NE(a, b) CHECK((a) != (b))
#define CHECK_GT(a, b) CHECK((a) > (b))
#define CHECK_LT(a, b) CHECK((a) < (b))
#define CHECK_GTE(a, b) CHECK((a) >= (b))
#define CHECK_LTE(a, b) CHECK((a) <= (b))

#define ASSERT_EQ(expected, actual) CHECK_EQ(expected, actual) \
    << "Expected: " << (expected) << "\n  Actual: " << (actual)

#define ASSERT_STREQ(expected, actual) \
    CHECK(strcmp((expected), (actual)) == 0) \
    << "Expected: " << (expected) << "\n  Actual: " << (actual)

#define ASSERT_TRUE(expression) CHECK((expression)) << "Expected true"
#define ASSERT_FALSE(expression) CHECK((!expression)) << "Expected false"

#define ASSERT_FLOAT_EQ(expected, actual, eps) \
    CHECK(fabs((expected) - (actual)) < (eps)) \
    << "Expected: " << (expected) << "\n  Actual: " << (actual)

#define ASSERT_IS_NUMBER(a) CHECK(!isnan(a) && !isinf(a)) \
    << "Input is a bad floating point number: " << a

// These are faster then pow().

inline double Square(double x) { return x * x; }
inline double Cube(double x) { return x * x * x; }

// Floating point equality comparison function.  This is the preferred way to
// check for equality among floating point numbers.
//
// Adapted from code at the following URL:
//  http://www.cygnus-software.com/papers/comparingfloats/
//      Comparing%20floating%20point%20numbers.htm
inline bool FloatEqual(double a, double b, double max_relative_error,
                       double max_absolute_error) {
  if (fabs(a - b) < max_absolute_error) return true;
  double relative_error = (fabs(b) > fabs(a)) ? fabs((a - b) / b) :
                                                fabs((a - b) / a);
  return relative_error <= max_relative_error;
}

// Class for aiding in assertions.
//
// This class is designed to be used in place of assertions.  It allows the
// user to print arbitrary messages in the event of an assertion failure.
//
// This class isn't designed to be used directly.  Instead, use the CHECK()
// macro above.
class Asserter {
 public:
  ~Asserter() {}

  // To avoid an instantiation for every CHECK(), this class is implemented as
  // a singleton.  This method returns the only instance and sets the relevant
  // metadata in a single statement.
  inline static Asserter &GetInstance(bool expression, const char *file_name,
                                      int line_number) {
    static Asserter instance;
    instance.expression_ = expression;
    instance.file_name_ = file_name;
    instance.line_number_ = line_number;
    return instance;
  }

  // This method is a weird hack to essentially overload return types.  This
  // was done so that this class knows when it has finally received all of the
  // input from multiple calls from the << operator.  This method is then
  // called to evaluate the expression and either do nothing or die with the
  // appropriate error message.
  inline operator int() {
    if (expression_) return 0;
    cout.flush();
    cerr.flush();
    cerr << "\n\n *** Check failed at " << file_name_ << " line "
         << line_number_ << " ***\n\n";
    cerr << stream_.str();
    cerr << "\n\n";
    cerr.flush();
    exit(EXIT_FAILURE);
    return 0;
  }

  // Generic implementation of the << operator.  Builds up the user's error
  // message in an internal string stream.
  template <typename Type>
  inline Asserter& operator<<(Type value) {
    if (expression_) return *this;
    stream_ << value;
    return *this;
  }

  // Overloaded for float input to show more digits.
  inline Asserter& operator<<(float value) {
    if (expression_) return *this;
    stream_ << setprecision(7) << value;
    return *this;
  }

  // Overloaded for double input to show more digits.
  inline Asserter& operator<<(double value) {
    if (expression_) return *this;
    stream_ << setprecision(15) << value;
    return *this;
  }

  // This value is only used to trigger operator int() above.  Yes, this is a
  // fairly weird/ugly hack, but don't you love CHECK()?
  static int DUMMY;

 private:
  Asserter()
      : line_number_(DUMMY),  // Avoid compiler warnings when CHECK isn't used.
        stream_(stringstream::in | stringstream::out) {}

  bool expression_;
  const char *file_name_;
  int line_number_;
  stringstream stream_;

  DISALLOW_COPY_AND_ASSIGN(Asserter);
};

}  // namespace google_sky

#endif  // BASE_H__
