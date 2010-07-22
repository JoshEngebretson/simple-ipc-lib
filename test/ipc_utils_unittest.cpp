// Copyright (c) 2010 Google Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "ipc_utils.h"

namespace {

template <typename T, size_t N>
bool ArrEqual(const T (&arr)[N], ipc::PodVector<T>& vec) {
  if (vec.size() != N)
    return false;
  for (size_t ix = 0; ix != N; ++ix) {
    if (vec.get()[ix] != arr[ix])
      return false;
  }
  return true;
}

template <typename ChT>
int StringCompare(const ChT* s1, const ChT* s2);

template <>
int StringCompare<char>(const char* s1, const char* s2) {
  return strcmp(s1, s2);
}

template <>
int StringCompare<wchar_t>(const wchar_t* s1, const wchar_t* s2) {
  return wcscmp(s1, s2);
}

}  //namespace


#pragma warning(push)
#pragma warning(disable : 4245)

int TestPodVector() {
  {
    ipc::PodVector<int> vec;
    if (vec.get() != 0)
      return 1;
    
    int a[] = {5, 4, 3, 2, 1, 0, -1, -2};
    vec.Add(a, countof(a));
    if (!ArrEqual(a, vec))
      return 2;

    int b[] = {-3, -4, 5, 6 };
    vec.Add(b, countof(b));
    int c[] = {5, 4, 3, 2, 1, 0, -1, -2, -3, -4, 5, 6};
    if (!ArrEqual(c, vec))
      return 3;

    vec.Set(b, countof(b));
    if (!ArrEqual(b, vec))
      return 4;
  }

  {
    ipc::PodVector<double> vec;
    vec.Set(0, 0);
    if (vec.get() != 0)
      return 5;
  }

  {
    ipc::PodVector<wchar_t> vec;
    if (vec.get() != 0)
      return 6;
   
    wchar_t a[] = {6, 7, 5, 4, 3, 2, 1, 8, -1, -2};
    vec.Add(a, countof(a));
    if (!ArrEqual(a, vec))
      return 7;

    wchar_t b[] = {-3, -4, 5, 6, 0};
    vec.Add(b, countof(b));
    wchar_t c[] = {6, 7, 5, 4, 3, 2, 1, 8, -1, -2, -3, -4, 5, 6, 0};
    if (!ArrEqual(c, vec))
      return 8;

    vec.Set(b, countof(b));
    if (!ArrEqual(b, vec))
      return 9;
  }

  {
    ipc::PodVector<char> vec;
    if (vec.get() != 0)
      return 10;
    
    char a[] = {5};
    vec.Add(a, countof(a));
    if (!ArrEqual(a, vec))
      return 11;

    char b[] = {-3, -4, 5, 6, 0};
    vec.Add(b, countof(b));
    char c[] = {5, -3, -4, 5, 6, 0};
    if (!ArrEqual(c, vec))
      return 12;

    vec.Set(b, countof(b));
    if (!ArrEqual(b, vec))
      return 13;

    ipc::PodVector<char> vec2;
    vec2.Swap(vec);
    if (!ArrEqual(b, vec2))
      return 13;
    if (0 != vec.size())
      return 14;
  }

  return 0;
}

template <typename ChT, size_t N1, size_t N2>
int TestStringImpl(const ChT (&txt)[N1], const ChT (&tzt)[N2]) {
  
  {
    ipc::HolderString<ChT> str;
    if (str.size() != 0)
      return 1;
  }

  {
    ipc::HolderString<ChT> str(txt);
    if (str.size() != countof(txt) - 1)
      return 2;
    if (StringCompare(str.c_str(), txt))
      return 3;
  }

  {
    ipc::HolderString<ChT> str(txt);
    ipc::HolderString<ChT> str2(tzt);
    str.swap(str2);
    if (StringCompare(str.c_str(), tzt))
      return 4;
    if (StringCompare(str2.c_str(), txt))
      return 5;
  }

  {
    ipc::HolderString<ChT> str(txt);
    ipc::HolderString<ChT> str2(tzt);
    str2 = str;
    if (StringCompare(str.c_str(), txt))
      return 6;
    if (StringCompare(str2.c_str(), txt))
      return 7;
  }

  {
    ipc::HolderString<ChT> str;
    str = txt;
    if (StringCompare(str.c_str(), txt))
      return 8;
  }

  {

    ipc::HolderString<ChT> str;
    str.assign(txt, countof(txt) - 1);
    if (str.size() != countof(txt) - 1)
      return 9;
    if (StringCompare(str.c_str(), txt))
      return 10;
  }

  {
    ipc::HolderString<ChT> str(txt);
    if (str != txt)
      return 11;
    if (str == tzt)
      return 12;
  }

 {
    ipc::HolderString<ChT> str(txt);
    for (int ix =0; ix != countof(txt) -1; ++ix) {
      if (str[ix] != txt[ix])
        return 13;
    }
  }

 {
    ipc::HolderString<ChT> str(tzt);
    str.append(txt);
    if (str.size() != countof(txt) + countof(tzt) - 2)
      return 14;
 }

  return 0;
}

int TestHolderString() {
  int rv1 = TestStringImpl("All the world I�ve seen before me passing by", "We the people");
  int rv2 = TestStringImpl(L"We the people", L"All the world I�ve seen before me passing by");
  return rv1 + rv2;
}


#pragma warning(pop)