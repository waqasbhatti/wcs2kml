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

#include <string>
#include <vector>

#include "base.h"
#include "string_util.h"

namespace google_sky {

int Main(int argc, char **argv) {
  {
    cout << "Testing StringPrintf related functions... ";
    string s;
    string hello = "hello, world";
    s = StringPrintf("hello, %s", "world");
    ASSERT_EQ(s, hello);

    SStringPrintf(&s, "hello, %s", "world");
    ASSERT_EQ(s, hello);

    s = "hello, ";
    StringAppendF(&s, "%s", "world");
    ASSERT_EQ(s, hello);
    cout << "pass\n";
  }

  {
    cout << "Testing string contains... ";

    ASSERT_FALSE(StringContains("", "hi"));
    ASSERT_FALSE(StringContains("hi", ""));
    ASSERT_TRUE(StringContains("hello", "hello"));
    ASSERT_TRUE(StringContains("hello", "ell"));
    ASSERT_FALSE(StringContains("hello", "hi"));

    cout << "pass\n";
  }

  {
    cout << "Testing value conversions... ";
    int i;
    float f;
    double d;
    bool b;

    // Check that we catch bad conversions.
    ASSERT_FALSE(StringToInt(string(""), &i));
    ASSERT_FALSE(StringToInt(string("abcavsd"), &i));
    ASSERT_FALSE(StringToInt(string("abc42"), &i));
    ASSERT_FALSE(StringToInt(string("42avsd"), &i));

    ASSERT_FALSE(StringToFloat(string(""), &f));
    ASSERT_FALSE(StringToFloat(string("abcavsd"), &f));
    ASSERT_FALSE(StringToFloat(string("abc42"), &f));
    ASSERT_FALSE(StringToFloat(string("42avsd"), &f));

    ASSERT_FALSE(StringToDouble(string(""), &d));
    ASSERT_FALSE(StringToDouble(string("abcavsd"), &d));
    ASSERT_FALSE(StringToDouble(string("abc42"), &d));
    ASSERT_FALSE(StringToDouble(string("42avsd"), &d));

    ASSERT_FALSE(StringToBool(string(""), &b));
    ASSERT_FALSE(StringToBool(string("abcavsd"), &b));
    ASSERT_FALSE(StringToBool(string("abc42"), &b));
    ASSERT_FALSE(StringToBool(string("42avsd"), &b));

    // Check that we convert properly.
    ASSERT_TRUE(StringToInt(string("42"), &i));
    ASSERT_EQ(i, 42);

    ASSERT_TRUE(StringToFloat(string("2.34"), &f));
    ASSERT_FLOAT_EQ(f, 2.34, 1.0e-6);

    ASSERT_TRUE(StringToDouble(string("3.1415926535897931"), &d));
    ASSERT_FLOAT_EQ(d, 3.1415926535897931, 1.0e-12);

    ASSERT_TRUE(StringToBool(string("true"), &b));
    ASSERT_TRUE(b);
    ASSERT_TRUE(StringToBool(string("false"), &b));
    ASSERT_FALSE(b);

    cout << "pass\n";
  }

  {
    cout << "Testing string split and join... ";

    // Test splitting on whitespace.
    vector<string> words;

    // Check empty and whitespace only strings.
    StringSplitOnWhiteSpace(string(""), &words);
    ASSERT_EQ(words.size(), 0);

    StringSplitOnWhiteSpace(string("   \t\t\n   \n"), &words);
    ASSERT_EQ(words.size(), 0);

    // Check 1 word, no space.
    StringSplitOnWhiteSpace(string("hello"), &words);
    ASSERT_EQ(words.size(), 1);
    ASSERT_EQ(words[0], string("hello"));

    // Check 1 word, leading and trailing space.
    StringSplitOnWhiteSpace(string("  hello"), &words);
    ASSERT_EQ(words.size(), 1);
    ASSERT_EQ(words[0], string("hello"));

    StringSplitOnWhiteSpace(string("hello  "), &words);
    ASSERT_EQ(words.size(), 1);
    ASSERT_EQ(words[0], string("hello"));

    StringSplitOnWhiteSpace(string("  hello  "), &words);
    ASSERT_EQ(words.size(), 1);
    ASSERT_EQ(words[0], string("hello"));

    // Check multiple words.
    StringSplitOnWhiteSpace(string("  hello  world!\n"), &words);
    ASSERT_EQ(words.size(), 2);
    ASSERT_EQ(words[0], string("hello"));
    ASSERT_EQ(words[1], string("world!"));

    // Check splitting on a single char.
    // NOTE: Here, the behavior follows Python so there is a 1 element vector.
    //       This differs from above where it is an empty list.
    StringSplitOnChar(string(""), ',', &words);
    ASSERT_EQ(words.size(), 1);
    ASSERT_EQ(words[0], string(""));

    // Corner cases.
    StringSplitOnChar(string(",hello"), ',', &words);
    ASSERT_EQ(words.size(), 2);
    ASSERT_EQ(words[0], string(""));
    ASSERT_EQ(words[1], string("hello"));

    StringSplitOnChar(string("hello,"), ',', &words);
    ASSERT_EQ(words.size(), 2);
    ASSERT_EQ(words[0], string("hello"));
    ASSERT_EQ(words[1], string(""));

    StringSplitOnChar(string(",hello,"), ',', &words);
    ASSERT_EQ(words.size(), 3);
    ASSERT_EQ(words[0], string(""));
    ASSERT_EQ(words[1], string("hello"));
    ASSERT_EQ(words[2], string(""));

    StringSplitOnChar(string("a,b,c"), ',', &words);
    ASSERT_EQ(words.size(), 3);
    ASSERT_EQ(words[0], string("a"));
    ASSERT_EQ(words[1], string("b"));
    ASSERT_EQ(words[2], string("c"));

    StringSplitOnChar(string(",a,,b,c,"), ',', &words);
    ASSERT_EQ(words.size(), 6);
    ASSERT_EQ(words[0], string(""));
    ASSERT_EQ(words[1], string("a"));
    ASSERT_EQ(words[2], string(""));
    ASSERT_EQ(words[3], string("b"));
    ASSERT_EQ(words[4], string("c"));
    ASSERT_EQ(words[5], string(""));

    // Check join, the inverse of split.
    string joined;
    words.clear();

    // Test empty input.
    StringJoin(words, " ", &joined);
    ASSERT_EQ(joined, string(""));

    // Test 1 word only.
    words.push_back("hello");
    StringJoin(words, " ", &joined);
    ASSERT_EQ(joined, string("hello"));

    // Test 2 words.
    words.push_back("world!");
    StringJoin(words, " ", &joined);
    ASSERT_EQ(joined, string("hello world!"));

    // Test char * join.
    const char *words_char[2] = { "hello", "world!" };

    StringJoin(words_char, 0, " ", &joined);
    ASSERT_EQ(joined, string(""));

    StringJoin(words_char, 1, " ", &joined);
    ASSERT_EQ(joined, string("hello"));

    StringJoin(words_char, 2, " ", &joined);
    ASSERT_EQ(joined, string("hello world!"));

    cout << "pass\n";
  }

  {
    cout << "Testing startswith & endswith... ";

    ASSERT_FALSE(StringStartsWith("", ""));
    ASSERT_FALSE(StringStartsWith("hello", ""));
    ASSERT_FALSE(StringStartsWith("hello", "helloworld"));
    ASSERT_FALSE(StringStartsWith("hello", "hi"));
    ASSERT_TRUE(StringStartsWith("hello", "hello"));
    ASSERT_TRUE(StringStartsWith("hello", "hel"));
    ASSERT_TRUE(StringStartsWith("hello", "h"));

    ASSERT_FALSE(StringEndsWith("", ""));
    ASSERT_FALSE(StringEndsWith("hello", ""));
    ASSERT_FALSE(StringEndsWith("hello", "helloworld"));
    ASSERT_FALSE(StringEndsWith("hello", "li"));
    ASSERT_TRUE(StringEndsWith("hello", "hello"));
    ASSERT_TRUE(StringEndsWith("hello", "llo"));
    ASSERT_TRUE(StringEndsWith("hello", "o"));

    cout << "pass\n";
  }

  {
    cout << "Testing strip... ";

    string s;

    // Leading space.
    s = "";
    StringStripLeadingWhiteSpace(&s);
    ASSERT_EQ(s, string(""));

    s = "hello world";
    StringStripLeadingWhiteSpace(&s);
    ASSERT_EQ(s, string("hello world"));

    s = "  hello world";
    StringStripLeadingWhiteSpace(&s);
    ASSERT_EQ(s, string("hello world"));

    s = "  hello world  ";
    StringStripLeadingWhiteSpace(&s);
    ASSERT_EQ(s, string("hello world  "));

    // Trailing space.
    s = "";
    StringStripTrailingWhiteSpace(&s);
    ASSERT_EQ(s, string(""));

    s = "hello world";
    StringStripTrailingWhiteSpace(&s);
    ASSERT_EQ(s, string("hello world"));

    s = "  hello world";
    StringStripTrailingWhiteSpace(&s);
    ASSERT_EQ(s, string("  hello world"));

    s = "  hello world  ";
    StringStripTrailingWhiteSpace(&s);
    ASSERT_EQ(s, string("  hello world"));

    // Leading and trailing space.
    s = "";
    StringStripLeadingAndTrailingWhiteSpace(&s);
    ASSERT_EQ(s, string(""));

    s = "hello world";
    StringStripLeadingAndTrailingWhiteSpace(&s);
    ASSERT_EQ(s, string("hello world"));

    s = "  hello world";
    StringStripLeadingAndTrailingWhiteSpace(&s);
    ASSERT_EQ(s, string("hello world"));

    s = "  hello world  ";
    StringStripLeadingAndTrailingWhiteSpace(&s);
    ASSERT_EQ(s, string("hello world"));

    cout << "pass\n";
  }

  {
    cout << "Testing filename extension splitting... ";

    string prefix;
    string extension;
    StringSplitExtension(string(""), &prefix, &extension);
    ASSERT_EQ(prefix, string(""));
    ASSERT_EQ(extension, string(""));

    StringSplitExtension(string("a"), &prefix, &extension);
    ASSERT_EQ(prefix, string("a"));
    ASSERT_EQ(extension, string(""));

    StringSplitExtension(string("."), &prefix, &extension);
    ASSERT_EQ(prefix, string(""));
    ASSERT_EQ(extension, string("."));

    StringSplitExtension(string("hello"), &prefix, &extension);
    ASSERT_EQ(prefix, string("hello"));
    ASSERT_EQ(extension, string(""));

    StringSplitExtension(string("hello.jpg"), &prefix, &extension);
    ASSERT_EQ(prefix, string("hello"));
    ASSERT_EQ(extension, string(".jpg"));

    StringSplitExtension(string("hello.world.jpg"), &prefix, &extension);
    ASSERT_EQ(prefix, string("hello.world"));
    ASSERT_EQ(extension, string(".jpg"));

    cout << "pass\n";
  }

  cout << "Passed\n";
  return 0;
}

}  // namespace google_sky

int main(int argc, char **argv) {
  return google_sky::Main(argc, argv);
}
