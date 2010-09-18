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
  if (vec.capacity() <= N)
    return false;
  for (size_t ix = 0; ix != N; ++ix) {
    if (vec[ix] != arr[ix])
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

class NonPod {
public:
  NonPod(const NonPod& oth) {
    foo_ = oth.foo_ + 1;
    bar_ = oth.bar_;
  }

  ~NonPod() {
    ++dtor_called;
  }

  NonPod(char foo) : foo_(foo), bar_(kBar) {
  }

  int get_foo() const { return foo_; }

  bool has_the_bar() {
    return (kBar == bar_); 
  }

  static int dtor_called;
  const static long long kBar = 0x7369746965726568ll;

private:
  NonPod();

  char foo_;
  long long bar_;
};

int NonPod::dtor_called = 0;

}  //namespace

int TestFixedArray() {
  ipc::FixedArray<NonPod, 8> far;
  if (far.size())
    return 1;
  if (far.max_size() != 8)
    return 2;

  NonPod pod(2);
  far.push_back(pod);
  for (int ix = 0; ix != 7; ++ix) {
    if (!far.push_back(far[ix]))
      return 3;
  }
  if (far.push_back(pod))
    return 3;
  if (far.size() != 8)
    return 4;

  for (int ix = 0; ix != 8; ++ix) {
    if (far[ix].get_foo() != (ix + 3))
      return 5;
    if (!far[ix].has_the_bar())
      return 6;
  }

  far.clear();
  if (far.size())
    return 7;
  if (NonPod::dtor_called != 8)
    return 8;

  far.push_back(NonPod(5));
  if (far.size() != 1)
    return 9;
  if (far[0].get_foo() != 6)
    return 10;

  return 0;
}


#pragma warning(push)
#pragma warning(disable : 4245)

int TestPodVector() {
  {
    ipc::PodVector<int> vec;
    if ((vec.get() != 0) || (vec.capacity() != 0))
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

    vec.clear();
    if (vec.size())
      return 5;
  }

  {
    ipc::PodVector<double> vec;
    vec.Set(0, 0);
    if (vec.get() != 0)
      return 8;
  }

  {
    ipc::PodVector<wchar_t> vec;
    if (vec.get() != 0)
      return 9;
   
    wchar_t a[] = {6, 7, 5, 4, 3, 2, 1, 8, -1, -2};
    vec.Add(a, countof(a));
    if (!ArrEqual(a, vec))
      return 10;

    wchar_t b[] = {-3, -4, 5, 6, 0};
    vec.Add(b, countof(b));
    wchar_t c[] = {6, 7, 5, 4, 3, 2, 1, 8, -1, -2, -3, -4, 5, 6, 0};
    if (!ArrEqual(c, vec))
      return 11;

    vec.Set(b, countof(b));
    if (!ArrEqual(b, vec))
      return 12;
  }

  {
    ipc::PodVector<char> vec;
    if (vec.get() != 0)
      return 13;
    
    char a[] = {5};
    vec.Add(a, countof(a));
    if (!ArrEqual(a, vec))
      return 14;

    char b[] = {-3, -4, 5, 6, 0};
    vec.Add(b, countof(b));
    char c[] = {5, -3, -4, 5, 6, 0};
    if (!ArrEqual(c, vec))
      return 15;

    vec.Set(b, countof(b));
    if (!ArrEqual(b, vec))
      return 16;

    ipc::PodVector<char> vec2;
    vec2.Swap(vec);
    if (!ArrEqual(b, vec2))
      return 17;
    if (0 != vec.size())
      return 18;
  }

  typedef void* VoidPt;
  VoidPt a[] = { VoidPt(55), VoidPt(66), VoidPt(77) };

  {
    ipc::PodVector<void*> vec;
    vec.resize(countof(a));
    if (vec.size() != countof(a))
      return 22;
    for (size_t ix = 0; ix != countof(a); ++ix) {
      if (vec[ix] != (void*)(0))
        return 23;
    }
    for (size_t ix = 0; ix != countof(a); ++ix) {
      vec[ix] = a[ix];
    }
    if (!ArrEqual(a, vec))
      return 24;
  }

  {
    ipc::PodVector<void*> vec;
    vec.push_back(a[0]);
    vec.insert(vec.end(), &a[1], &a[countof(a)]);

    if (!ArrEqual(a, vec))
      return 26;

    vec.erase(vec.begin(), vec.begin() + 2);
    if (vec.size() != 1)
      return 27;
    if (vec[0] != a[2])
      return 28;
  }

  return 0;
}

template <typename ChT, size_t N1, size_t N2, size_t N3>
int TestStringImpl(const ChT (&txt)[N1], const ChT (&tzt)[N2], const ChT (&non)[N3]) {
  
  {
    ipc::HolderString<ChT> str;
    if (str.size() != 0)
      return 1;
    if (StringCompare(str.c_str(), non))
      return 2;
    if (str != non)
      return 3;
  }

  {
    ipc::HolderString<ChT> str(txt);
    if (str.size() != countof(txt) - 1)
      return 3;
    if (StringCompare(str.c_str(), txt))
      return 4;
  }

  {
    ipc::HolderString<ChT> str(txt);
    ipc::HolderString<ChT> str2(tzt);
    str.swap(str2);
    if (StringCompare(str.c_str(), tzt))
      return 6;
    if (StringCompare(str2.c_str(), txt))
      return 7;
  }

  {
    ipc::HolderString<ChT> str(txt);
    ipc::HolderString<ChT> str2(tzt);
    str2 = str;
    if (StringCompare(str.c_str(), txt))
      return 9;
    if (StringCompare(str2.c_str(), txt))
      return 10;
  }

  {
    ipc::HolderString<ChT> str;
    str = txt;
    if (StringCompare(str.c_str(), txt))
      return 11;
  }

  {
    ipc::HolderString<ChT> str;
    str.assign(txt, countof(txt) - 1);
    if (str.size() != countof(txt) - 1)
      return 13;
    if (StringCompare(str.c_str(), txt))
      return 14;
  }

  {
    ipc::HolderString<ChT> str(txt);
    if (str != txt)
      return 16;
    if (str == tzt)
      return 17;
  }

  {
    ipc::HolderString<ChT> str(txt);
    for (int ix =0; ix != countof(txt) -1; ++ix) {
      if (str[ix] != txt[ix])
        return 19;
    }
  }

  {
    ipc::HolderString<ChT> str(tzt);
    str.append(txt);
    if (str.size() != countof(txt) + countof(tzt) - 2)
      return 20;
  }

  {
    ipc::HolderString<ChT> str(non);
    if (StringCompare(str.c_str(), non))
      return 22;

    str.append(non);
    if (StringCompare(str.c_str(), non))
      return 23;

    size_t c = str.capacity();
    ChT x[] = {0, 0};
    for (size_t ix = 0; ix != c; ++ix) {
      x[0] = ChT(48 + ix);
      str.append(x);
      if (str.capacity() < (str.size() + 1))
        return 25;
    }

    if ((c + 1) >= str.capacity())
      return 26;
  }

  return 0;
}

int TestHolderString() {
  int rv1 = TestStringImpl("All the world I've seen before me passing by", "We the people", "");
  int rv2 = TestStringImpl(L"We the people", L"All the world I've seen before me passing by", L"");
  return rv1 + rv2;
}


#pragma warning(pop)