// RUN: %clang_cc1 -std=c++2c -triple x86_64-unknown-linux-gnu -emit-llvm -o - %s | FileCheck %s

template <typename T, __SIZE_TYPE__ size>
struct Array {
  T data[size]{};
  constexpr const T* begin() const { return data; }
  constexpr const T* end() const { return data + size; }
};

int f1() {
  static constexpr Array<int, 3> integers{1, 2, 3};
  int sum = 0;
  template for (auto x : integers) sum += x;
  return sum;
}

int f2() {
  static constexpr Array<int, 3> integers{1, 2, 3};
  int sum = 0;
  template for (constexpr auto x : integers) sum += x;
  return sum;
}

int f3() {
  static constexpr Array<int, 0> integers{};
  int sum = 0;
  template for (constexpr auto x : integers) {
    static_assert(false, "not expanded");
    sum += x;
  }
  return sum;
}

int f4() {
  static constexpr Array<int, 2> a{1, 2};
  static constexpr Array<int, 2> b{3, 4};
  int sum = 0;

  template for (auto x : a)
    template for (auto y : b)
      sum += x + y;

  template for (constexpr auto x : a)
    template for (constexpr auto y : b)
      sum += x + y;

  return sum;
}

struct Private {
  static constexpr Array<int, 3> integers{1, 2, 3};
  friend constexpr int friend_func();

private:
  constexpr const int* begin() const { return integers.begin(); }
  constexpr const int* end() const { return integers.end(); }

public:
  static int member_func();
};

int Private::member_func() {
  int sum = 0;
  static constexpr Private p1;
  template for (auto x : p1) sum += x;
  return sum;
}

struct CustomIterator {
  struct iterator {
    int n;

    constexpr iterator operator+(int m) const {
      return {n + m};
    }

    constexpr void operator++() { ++n; }

    constexpr int operator*() const {
      return n;
    }

    friend constexpr bool operator!=(iterator a, iterator b) {
      return a.n != b.n;
    }

    friend constexpr int operator-(iterator a, iterator b) {
      return a.n - b.n;
    }
  };

   constexpr iterator begin() const { return iterator(1); }
   constexpr iterator end() const { return iterator(5); }
};

int custom_iterator() {
  static constexpr CustomIterator c;
  int sum = 0;
  template for (auto x : c) sum += x;
  template for (constexpr auto x : c) sum += x;
  return sum;
}

// CHECK: @_ZZ2f1vE8integers = internal constant %struct.Array { [3 x i32] [i32 1, i32 2, i32 3] }, align 4
// CHECK: @_ZZ2f2vE8integers = internal constant %struct.Array { [3 x i32] [i32 1, i32 2, i32 3] }, align 4
// CHECK: @_ZZ2f3vE8integers = internal constant %struct.Array.0 zeroinitializer, align 4
// CHECK: @_ZZ2f4vE1a = internal constant %struct.Array.1 { [2 x i32] [i32 1, i32 2] }, align 4
// CHECK: @_ZZ2f4vE1b = internal constant %struct.Array.1 { [2 x i32] [i32 3, i32 4] }, align 4
// CHECK: @_ZZN7Private11member_funcEvE2p1 = internal constant %struct.Private zeroinitializer, align 1
// CHECK: @_ZZ15custom_iteratorvE1c = internal constant %struct.CustomIterator zeroinitializer, align 1
// CHECK: @__const._Z15custom_iteratorv.__begin1 = private {{.*}} constant %"struct.CustomIterator::iterator" { i32 1 }, align 4
// CHECK: @__const._Z15custom_iteratorv.__end1 = private {{.*}} constant %"struct.CustomIterator::iterator" { i32 5 }, align 4
// CHECK: @__const._Z15custom_iteratorv.__begin1.1 = private {{.*}} constant %"struct.CustomIterator::iterator" { i32 1 }, align 4
// CHECK: @__const._Z15custom_iteratorv.__end1.2 = private {{.*}} constant %"struct.CustomIterator::iterator" { i32 5 }, align 4
// CHECK: @__const._Z15custom_iteratorv.__iter1 = private {{.*}} constant %"struct.CustomIterator::iterator" { i32 1 }, align 4
// CHECK: @__const._Z15custom_iteratorv.__iter1.3 = private {{.*}} constant %"struct.CustomIterator::iterator" { i32 2 }, align 4
// CHECK: @__const._Z15custom_iteratorv.__iter1.4 = private {{.*}} constant %"struct.CustomIterator::iterator" { i32 3 }, align 4
// CHECK: @__const._Z15custom_iteratorv.__iter1.5 = private {{.*}} constant %"struct.CustomIterator::iterator" { i32 4 }, align 4
// CHECK: @_ZN7Private8integersE = {{.*}} constant %struct.Array { [3 x i32] [i32 1, i32 2, i32 3] }, comdat, align 4

// CHECK-LABEL: define {{.*}} i32 @_Z2f1v()
// CHECK: entry:
// CHECK-NEXT:   %sum = alloca i32, align 4
// CHECK-NEXT:   %__range1 = alloca ptr, align 8
// CHECK-NEXT:   %__begin1 = alloca ptr, align 8
// CHECK-NEXT:   %__end1 = alloca ptr, align 8
// CHECK-NEXT:   %__iter1 = alloca ptr, align 8
// CHECK-NEXT:   %x = alloca i32, align 4
// CHECK-NEXT:   %__iter12 = alloca ptr, align 8
// CHECK-NEXT:   %x4 = alloca i32, align 4
// CHECK-NEXT:   %__iter17 = alloca ptr, align 8
// CHECK-NEXT:   %x9 = alloca i32, align 4
// CHECK-NEXT:   store i32 0, ptr %sum, align 4
// CHECK-NEXT:   store ptr @_ZZ2f1vE8integers, ptr %__range1, align 8
// CHECK-NEXT:   %call = call {{.*}} ptr @_ZNK5ArrayIiLm3EE5beginEv(ptr {{.*}} @_ZZ2f1vE8integers)
// CHECK-NEXT:   store ptr %call, ptr %__begin1, align 8
// CHECK-NEXT:   %call1 = call {{.*}} ptr @_ZNK5ArrayIiLm3EE3endEv(ptr {{.*}} @_ZZ2f1vE8integers)
// CHECK-NEXT:   store ptr %call1, ptr %__end1, align 8
// CHECK-NEXT:   %0 = load ptr, ptr %__begin1, align 8
// CHECK-NEXT:   %add.ptr = getelementptr inbounds i32, ptr %0, i64 0
// CHECK-NEXT:   store ptr %add.ptr, ptr %__iter1, align 8
// CHECK-NEXT:   %1 = load ptr, ptr %__iter1, align 8
// CHECK-NEXT:   %2 = load i32, ptr %1, align 4
// CHECK-NEXT:   store i32 %2, ptr %x, align 4
// CHECK-NEXT:   %3 = load i32, ptr %x, align 4
// CHECK-NEXT:   %4 = load i32, ptr %sum, align 4
// CHECK-NEXT:   %add = add nsw i32 %4, %3
// CHECK-NEXT:   store i32 %add, ptr %sum, align 4
// CHECK-NEXT:   br label %expand.next
// CHECK: expand.next:
// CHECK-NEXT:   %5 = load ptr, ptr %__begin1, align 8
// CHECK-NEXT:   %add.ptr3 = getelementptr inbounds i32, ptr %5, i64 1
// CHECK-NEXT:   store ptr %add.ptr3, ptr %__iter12, align 8
// CHECK-NEXT:   %6 = load ptr, ptr %__iter12, align 8
// CHECK-NEXT:   %7 = load i32, ptr %6, align 4
// CHECK-NEXT:   store i32 %7, ptr %x4, align 4
// CHECK-NEXT:   %8 = load i32, ptr %x4, align 4
// CHECK-NEXT:   %9 = load i32, ptr %sum, align 4
// CHECK-NEXT:   %add5 = add nsw i32 %9, %8
// CHECK-NEXT:   store i32 %add5, ptr %sum, align 4
// CHECK-NEXT:   br label %expand.next6
// CHECK: expand.next6:
// CHECK-NEXT:   %10 = load ptr, ptr %__begin1, align 8
// CHECK-NEXT:   %add.ptr8 = getelementptr inbounds i32, ptr %10, i64 2
// CHECK-NEXT:   store ptr %add.ptr8, ptr %__iter17, align 8
// CHECK-NEXT:   %11 = load ptr, ptr %__iter17, align 8
// CHECK-NEXT:   %12 = load i32, ptr %11, align 4
// CHECK-NEXT:   store i32 %12, ptr %x9, align 4
// CHECK-NEXT:   %13 = load i32, ptr %x9, align 4
// CHECK-NEXT:   %14 = load i32, ptr %sum, align 4
// CHECK-NEXT:   %add10 = add nsw i32 %14, %13
// CHECK-NEXT:   store i32 %add10, ptr %sum, align 4
// CHECK-NEXT:   br label %expand.end
// CHECK: expand.end:
// CHECK-NEXT:   %15 = load i32, ptr %sum, align 4
// CHECK-NEXT:   ret i32 %15


// CHECK-LABEL: define {{.*}} i32 @_Z2f2v()
// CHECK: entry:
// CHECK-NEXT:   %sum = alloca i32, align 4
// CHECK-NEXT:   %__range1 = alloca ptr, align 8
// CHECK-NEXT:   %__begin1 = alloca ptr, align 8
// CHECK-NEXT:   %__end1 = alloca ptr, align 8
// CHECK-NEXT:   %__iter1 = alloca ptr, align 8
// CHECK-NEXT:   %x = alloca i32, align 4
// CHECK-NEXT:   %__iter11 = alloca ptr, align 8
// CHECK-NEXT:   %x2 = alloca i32, align 4
// CHECK-NEXT:   %__iter15 = alloca ptr, align 8
// CHECK-NEXT:   %x6 = alloca i32, align 4
// CHECK-NEXT:   store i32 0, ptr %sum, align 4
// CHECK-NEXT:   store ptr @_ZZ2f2vE8integers, ptr %__range1, align 8
// CHECK-NEXT:   store ptr @_ZZ2f2vE8integers, ptr %__begin1, align 8
// CHECK-NEXT:   store ptr getelementptr (i8, ptr @_ZZ2f2vE8integers, i64 12), ptr %__end1, align 8
// CHECK-NEXT:   store ptr @_ZZ2f2vE8integers, ptr %__iter1, align 8
// CHECK-NEXT:   store i32 1, ptr %x, align 4
// CHECK-NEXT:   %0 = load i32, ptr %sum, align 4
// CHECK-NEXT:   %add = add nsw i32 %0, 1
// CHECK-NEXT:   store i32 %add, ptr %sum, align 4
// CHECK-NEXT:   br label %expand.next
// CHECK: expand.next:
// CHECK-NEXT:   store ptr getelementptr (i8, ptr @_ZZ2f2vE8integers, i64 4), ptr %__iter11, align 8
// CHECK-NEXT:   store i32 2, ptr %x2, align 4
// CHECK-NEXT:   %1 = load i32, ptr %sum, align 4
// CHECK-NEXT:   %add3 = add nsw i32 %1, 2
// CHECK-NEXT:   store i32 %add3, ptr %sum, align 4
// CHECK-NEXT:   br label %expand.next4
// CHECK: expand.next4:
// CHECK-NEXT:   store ptr getelementptr (i8, ptr @_ZZ2f2vE8integers, i64 8), ptr %__iter15, align 8
// CHECK-NEXT:   store i32 3, ptr %x6, align 4
// CHECK-NEXT:   %2 = load i32, ptr %sum, align 4
// CHECK-NEXT:   %add7 = add nsw i32 %2, 3
// CHECK-NEXT:   store i32 %add7, ptr %sum, align 4
// CHECK-NEXT:   br label %expand.end
// CHECK: expand.end:
// CHECK-NEXT:   %3 = load i32, ptr %sum, align 4
// CHECK-NEXT:   ret i32 %3


// CHECK-LABEL: define {{.*}} i32 @_Z2f3v()
// CHECK: entry:
// CHECK-NEXT:   %sum = alloca i32, align 4
// CHECK-NEXT:   %__range1 = alloca ptr, align 8
// CHECK-NEXT:   %__begin1 = alloca ptr, align 8
// CHECK-NEXT:   %__end1 = alloca ptr, align 8
// CHECK-NEXT:   store i32 0, ptr %sum, align 4
// CHECK-NEXT:   store ptr @_ZZ2f3vE8integers, ptr %__range1, align 8
// CHECK-NEXT:   store ptr @_ZZ2f3vE8integers, ptr %__begin1, align 8
// CHECK-NEXT:   store ptr @_ZZ2f3vE8integers, ptr %__end1, align 8
// CHECK-NEXT:   %0 = load i32, ptr %sum, align 4
// CHECK-NEXT:   ret i32 %0


// CHECK-LABEL: define {{.*}} i32 @_Z2f4v()
// CHECK: entry:
// CHECK-NEXT:   %sum = alloca i32, align 4
// CHECK-NEXT:   %__range1 = alloca ptr, align 8
// CHECK-NEXT:   %__begin1 = alloca ptr, align 8
// CHECK-NEXT:   %__end1 = alloca ptr, align 8
// CHECK-NEXT:   %__iter1 = alloca ptr, align 8
// CHECK-NEXT:   %x = alloca i32, align 4
// CHECK-NEXT:   %__range2 = alloca ptr, align 8
// CHECK-NEXT:   %__begin2 = alloca ptr, align 8
// CHECK-NEXT:   %__end2 = alloca ptr, align 8
// CHECK-NEXT:   %__iter2 = alloca ptr, align 8
// CHECK-NEXT:   %y = alloca i32, align 4
// CHECK-NEXT:   %__iter26 = alloca ptr, align 8
// CHECK-NEXT:   %y8 = alloca i32, align 4
// CHECK-NEXT:   %__iter112 = alloca ptr, align 8
// CHECK-NEXT:   %x14 = alloca i32, align 4
// CHECK-NEXT:   %__range215 = alloca ptr, align 8
// CHECK-NEXT:   %__begin216 = alloca ptr, align 8
// CHECK-NEXT:   %__end218 = alloca ptr, align 8
// CHECK-NEXT:   %__iter220 = alloca ptr, align 8
// CHECK-NEXT:   %y22 = alloca i32, align 4
// CHECK-NEXT:   %__iter226 = alloca ptr, align 8
// CHECK-NEXT:   %y28 = alloca i32, align 4
// CHECK-NEXT:   %__range133 = alloca ptr, align 8
// CHECK-NEXT:   %__begin134 = alloca ptr, align 8
// CHECK-NEXT:   %__end135 = alloca ptr, align 8
// CHECK-NEXT:   %__iter136 = alloca ptr, align 8
// CHECK-NEXT:   %x37 = alloca i32, align 4
// CHECK-NEXT:   %__range238 = alloca ptr, align 8
// CHECK-NEXT:   %__begin239 = alloca ptr, align 8
// CHECK-NEXT:   %__end240 = alloca ptr, align 8
// CHECK-NEXT:   %__iter241 = alloca ptr, align 8
// CHECK-NEXT:   %y42 = alloca i32, align 4
// CHECK-NEXT:   %__iter245 = alloca ptr, align 8
// CHECK-NEXT:   %y46 = alloca i32, align 4
// CHECK-NEXT:   %__iter150 = alloca ptr, align 8
// CHECK-NEXT:   %x51 = alloca i32, align 4
// CHECK-NEXT:   %__range252 = alloca ptr, align 8
// CHECK-NEXT:   %__begin253 = alloca ptr, align 8
// CHECK-NEXT:   %__end254 = alloca ptr, align 8
// CHECK-NEXT:   %__iter255 = alloca ptr, align 8
// CHECK-NEXT:   %y56 = alloca i32, align 4
// CHECK-NEXT:   %__iter259 = alloca ptr, align 8
// CHECK-NEXT:   %y60 = alloca i32, align 4
// CHECK-NEXT:   store i32 0, ptr %sum, align 4
// CHECK-NEXT:   store ptr @_ZZ2f4vE1a, ptr %__range1, align 8
// CHECK-NEXT:   %call = call {{.*}} ptr @_ZNK5ArrayIiLm2EE5beginEv(ptr {{.*}} @_ZZ2f4vE1a)
// CHECK-NEXT:   store ptr %call, ptr %__begin1, align 8
// CHECK-NEXT:   %call1 = call {{.*}} ptr @_ZNK5ArrayIiLm2EE3endEv(ptr {{.*}} @_ZZ2f4vE1a)
// CHECK-NEXT:   store ptr %call1, ptr %__end1, align 8
// CHECK-NEXT:   %0 = load ptr, ptr %__begin1, align 8
// CHECK-NEXT:   %add.ptr = getelementptr inbounds i32, ptr %0, i64 0
// CHECK-NEXT:   store ptr %add.ptr, ptr %__iter1, align 8
// CHECK-NEXT:   %1 = load ptr, ptr %__iter1, align 8
// CHECK-NEXT:   %2 = load i32, ptr %1, align 4
// CHECK-NEXT:   store i32 %2, ptr %x, align 4
// CHECK-NEXT:   store ptr @_ZZ2f4vE1b, ptr %__range2, align 8
// CHECK-NEXT:   %call2 = call {{.*}} ptr @_ZNK5ArrayIiLm2EE5beginEv(ptr {{.*}} @_ZZ2f4vE1b)
// CHECK-NEXT:   store ptr %call2, ptr %__begin2, align 8
// CHECK-NEXT:   %call3 = call {{.*}} ptr @_ZNK5ArrayIiLm2EE3endEv(ptr {{.*}} @_ZZ2f4vE1b)
// CHECK-NEXT:   store ptr %call3, ptr %__end2, align 8
// CHECK-NEXT:   %3 = load ptr, ptr %__begin2, align 8
// CHECK-NEXT:   %add.ptr4 = getelementptr inbounds i32, ptr %3, i64 0
// CHECK-NEXT:   store ptr %add.ptr4, ptr %__iter2, align 8
// CHECK-NEXT:   %4 = load ptr, ptr %__iter2, align 8
// CHECK-NEXT:   %5 = load i32, ptr %4, align 4
// CHECK-NEXT:   store i32 %5, ptr %y, align 4
// CHECK-NEXT:   %6 = load i32, ptr %x, align 4
// CHECK-NEXT:   %7 = load i32, ptr %y, align 4
// CHECK-NEXT:   %add = add nsw i32 %6, %7
// CHECK-NEXT:   %8 = load i32, ptr %sum, align 4
// CHECK-NEXT:   %add5 = add nsw i32 %8, %add
// CHECK-NEXT:   store i32 %add5, ptr %sum, align 4
// CHECK-NEXT:   br label %expand.next
// CHECK: expand.next:
// CHECK-NEXT:   %9 = load ptr, ptr %__begin2, align 8
// CHECK-NEXT:   %add.ptr7 = getelementptr inbounds i32, ptr %9, i64 1
// CHECK-NEXT:   store ptr %add.ptr7, ptr %__iter26, align 8
// CHECK-NEXT:   %10 = load ptr, ptr %__iter26, align 8
// CHECK-NEXT:   %11 = load i32, ptr %10, align 4
// CHECK-NEXT:   store i32 %11, ptr %y8, align 4
// CHECK-NEXT:   %12 = load i32, ptr %x, align 4
// CHECK-NEXT:   %13 = load i32, ptr %y8, align 4
// CHECK-NEXT:   %add9 = add nsw i32 %12, %13
// CHECK-NEXT:   %14 = load i32, ptr %sum, align 4
// CHECK-NEXT:   %add10 = add nsw i32 %14, %add9
// CHECK-NEXT:   store i32 %add10, ptr %sum, align 4
// CHECK-NEXT:   br label %expand.end
// CHECK: expand.end:
// CHECK-NEXT:   br label %expand.next11
// CHECK: expand.next11:
// CHECK-NEXT:   %15 = load ptr, ptr %__begin1, align 8
// CHECK-NEXT:   %add.ptr13 = getelementptr inbounds i32, ptr %15, i64 1
// CHECK-NEXT:   store ptr %add.ptr13, ptr %__iter112, align 8
// CHECK-NEXT:   %16 = load ptr, ptr %__iter112, align 8
// CHECK-NEXT:   %17 = load i32, ptr %16, align 4
// CHECK-NEXT:   store i32 %17, ptr %x14, align 4
// CHECK-NEXT:   store ptr @_ZZ2f4vE1b, ptr %__range215, align 8
// CHECK-NEXT:   %call17 = call {{.*}} ptr @_ZNK5ArrayIiLm2EE5beginEv(ptr {{.*}} @_ZZ2f4vE1b)
// CHECK-NEXT:   store ptr %call17, ptr %__begin216, align 8
// CHECK-NEXT:   %call19 = call {{.*}} ptr @_ZNK5ArrayIiLm2EE3endEv(ptr {{.*}} @_ZZ2f4vE1b)
// CHECK-NEXT:   store ptr %call19, ptr %__end218, align 8
// CHECK-NEXT:   %18 = load ptr, ptr %__begin216, align 8
// CHECK-NEXT:   %add.ptr21 = getelementptr inbounds i32, ptr %18, i64 0
// CHECK-NEXT:   store ptr %add.ptr21, ptr %__iter220, align 8
// CHECK-NEXT:   %19 = load ptr, ptr %__iter220, align 8
// CHECK-NEXT:   %20 = load i32, ptr %19, align 4
// CHECK-NEXT:   store i32 %20, ptr %y22, align 4
// CHECK-NEXT:   %21 = load i32, ptr %x14, align 4
// CHECK-NEXT:   %22 = load i32, ptr %y22, align 4
// CHECK-NEXT:   %add23 = add nsw i32 %21, %22
// CHECK-NEXT:   %23 = load i32, ptr %sum, align 4
// CHECK-NEXT:   %add24 = add nsw i32 %23, %add23
// CHECK-NEXT:   store i32 %add24, ptr %sum, align 4
// CHECK-NEXT:   br label %expand.next25
// CHECK: expand.next25:
// CHECK-NEXT:   %24 = load ptr, ptr %__begin216, align 8
// CHECK-NEXT:   %add.ptr27 = getelementptr inbounds i32, ptr %24, i64 1
// CHECK-NEXT:   store ptr %add.ptr27, ptr %__iter226, align 8
// CHECK-NEXT:   %25 = load ptr, ptr %__iter226, align 8
// CHECK-NEXT:   %26 = load i32, ptr %25, align 4
// CHECK-NEXT:   store i32 %26, ptr %y28, align 4
// CHECK-NEXT:   %27 = load i32, ptr %x14, align 4
// CHECK-NEXT:   %28 = load i32, ptr %y28, align 4
// CHECK-NEXT:   %add29 = add nsw i32 %27, %28
// CHECK-NEXT:   %29 = load i32, ptr %sum, align 4
// CHECK-NEXT:   %add30 = add nsw i32 %29, %add29
// CHECK-NEXT:   store i32 %add30, ptr %sum, align 4
// CHECK-NEXT:   br label %expand.end31
// CHECK: expand.end31:
// CHECK-NEXT:   br label %expand.end32
// CHECK: expand.end32:
// CHECK-NEXT:   store ptr @_ZZ2f4vE1a, ptr %__range133, align 8
// CHECK-NEXT:   store ptr @_ZZ2f4vE1a, ptr %__begin134, align 8
// CHECK-NEXT:   store ptr getelementptr (i8, ptr @_ZZ2f4vE1a, i64 8), ptr %__end135, align 8
// CHECK-NEXT:   store ptr @_ZZ2f4vE1a, ptr %__iter136, align 8
// CHECK-NEXT:   store i32 1, ptr %x37, align 4
// CHECK-NEXT:   store ptr @_ZZ2f4vE1b, ptr %__range238, align 8
// CHECK-NEXT:   store ptr @_ZZ2f4vE1b, ptr %__begin239, align 8
// CHECK-NEXT:   store ptr getelementptr (i8, ptr @_ZZ2f4vE1b, i64 8), ptr %__end240, align 8
// CHECK-NEXT:   store ptr @_ZZ2f4vE1b, ptr %__iter241, align 8
// CHECK-NEXT:   store i32 3, ptr %y42, align 4
// CHECK-NEXT:   %30 = load i32, ptr %sum, align 4
// CHECK-NEXT:   %add43 = add nsw i32 %30, 4
// CHECK-NEXT:   store i32 %add43, ptr %sum, align 4
// CHECK-NEXT:   br label %expand.next44
// CHECK: expand.next44:
// CHECK-NEXT:   store ptr getelementptr (i8, ptr @_ZZ2f4vE1b, i64 4), ptr %__iter245, align 8
// CHECK-NEXT:   store i32 4, ptr %y46, align 4
// CHECK-NEXT:   %31 = load i32, ptr %sum, align 4
// CHECK-NEXT:   %add47 = add nsw i32 %31, 5
// CHECK-NEXT:   store i32 %add47, ptr %sum, align 4
// CHECK-NEXT:   br label %expand.end48
// CHECK: expand.end48:
// CHECK-NEXT:   br label %expand.next49
// CHECK: expand.next49:
// CHECK-NEXT:   store ptr getelementptr (i8, ptr @_ZZ2f4vE1a, i64 4), ptr %__iter150, align 8
// CHECK-NEXT:   store i32 2, ptr %x51, align 4
// CHECK-NEXT:   store ptr @_ZZ2f4vE1b, ptr %__range252, align 8
// CHECK-NEXT:   store ptr @_ZZ2f4vE1b, ptr %__begin253, align 8
// CHECK-NEXT:   store ptr getelementptr (i8, ptr @_ZZ2f4vE1b, i64 8), ptr %__end254, align 8
// CHECK-NEXT:   store ptr @_ZZ2f4vE1b, ptr %__iter255, align 8
// CHECK-NEXT:   store i32 3, ptr %y56, align 4
// CHECK-NEXT:   %32 = load i32, ptr %sum, align 4
// CHECK-NEXT:   %add57 = add nsw i32 %32, 5
// CHECK-NEXT:   store i32 %add57, ptr %sum, align 4
// CHECK-NEXT:   br label %expand.next58
// CHECK: expand.next58:
// CHECK-NEXT:   store ptr getelementptr (i8, ptr @_ZZ2f4vE1b, i64 4), ptr %__iter259, align 8
// CHECK-NEXT:   store i32 4, ptr %y60, align 4
// CHECK-NEXT:   %33 = load i32, ptr %sum, align 4
// CHECK-NEXT:   %add61 = add nsw i32 %33, 6
// CHECK-NEXT:   store i32 %add61, ptr %sum, align 4
// CHECK-NEXT:   br label %expand.end62
// CHECK: expand.end62:
// CHECK-NEXT:   br label %expand.end63
// CHECK: expand.end63:
// CHECK-NEXT:   %34 = load i32, ptr %sum, align 4
// CHECK-NEXT:   ret i32 %34


// CHECK-LABEL: define {{.*}} i32 @_ZN7Private11member_funcEv()
// CHECK: entry:
// CHECK-NEXT:   %sum = alloca i32, align 4
// CHECK-NEXT:   %__range1 = alloca ptr, align 8
// CHECK-NEXT:   %__begin1 = alloca ptr, align 8
// CHECK-NEXT:   %__end1 = alloca ptr, align 8
// CHECK-NEXT:   %__iter1 = alloca ptr, align 8
// CHECK-NEXT:   %x = alloca i32, align 4
// CHECK-NEXT:   %__iter12 = alloca ptr, align 8
// CHECK-NEXT:   %x4 = alloca i32, align 4
// CHECK-NEXT:   %__iter17 = alloca ptr, align 8
// CHECK-NEXT:   %x9 = alloca i32, align 4
// CHECK-NEXT:   store i32 0, ptr %sum, align 4
// CHECK-NEXT:   store ptr @_ZZN7Private11member_funcEvE2p1, ptr %__range1, align 8
// CHECK-NEXT:   %call = call {{.*}} ptr @_ZNK7Private5beginEv(ptr {{.*}} @_ZZN7Private11member_funcEvE2p1)
// CHECK-NEXT:   store ptr %call, ptr %__begin1, align 8
// CHECK-NEXT:   %call1 = call {{.*}} ptr @_ZNK7Private3endEv(ptr {{.*}} @_ZZN7Private11member_funcEvE2p1)
// CHECK-NEXT:   store ptr %call1, ptr %__end1, align 8
// CHECK-NEXT:   %0 = load ptr, ptr %__begin1, align 8
// CHECK-NEXT:   %add.ptr = getelementptr inbounds i32, ptr %0, i64 0
// CHECK-NEXT:   store ptr %add.ptr, ptr %__iter1, align 8
// CHECK-NEXT:   %1 = load ptr, ptr %__iter1, align 8
// CHECK-NEXT:   %2 = load i32, ptr %1, align 4
// CHECK-NEXT:   store i32 %2, ptr %x, align 4
// CHECK-NEXT:   %3 = load i32, ptr %x, align 4
// CHECK-NEXT:   %4 = load i32, ptr %sum, align 4
// CHECK-NEXT:   %add = add nsw i32 %4, %3
// CHECK-NEXT:   store i32 %add, ptr %sum, align 4
// CHECK-NEXT:   br label %expand.next
// CHECK: expand.next:
// CHECK-NEXT:   %5 = load ptr, ptr %__begin1, align 8
// CHECK-NEXT:   %add.ptr3 = getelementptr inbounds i32, ptr %5, i64 1
// CHECK-NEXT:   store ptr %add.ptr3, ptr %__iter12, align 8
// CHECK-NEXT:   %6 = load ptr, ptr %__iter12, align 8
// CHECK-NEXT:   %7 = load i32, ptr %6, align 4
// CHECK-NEXT:   store i32 %7, ptr %x4, align 4
// CHECK-NEXT:   %8 = load i32, ptr %x4, align 4
// CHECK-NEXT:   %9 = load i32, ptr %sum, align 4
// CHECK-NEXT:   %add5 = add nsw i32 %9, %8
// CHECK-NEXT:   store i32 %add5, ptr %sum, align 4
// CHECK-NEXT:   br label %expand.next6
// CHECK: expand.next6:
// CHECK-NEXT:   %10 = load ptr, ptr %__begin1, align 8
// CHECK-NEXT:   %add.ptr8 = getelementptr inbounds i32, ptr %10, i64 2
// CHECK-NEXT:   store ptr %add.ptr8, ptr %__iter17, align 8
// CHECK-NEXT:   %11 = load ptr, ptr %__iter17, align 8
// CHECK-NEXT:   %12 = load i32, ptr %11, align 4
// CHECK-NEXT:   store i32 %12, ptr %x9, align 4
// CHECK-NEXT:   %13 = load i32, ptr %x9, align 4
// CHECK-NEXT:   %14 = load i32, ptr %sum, align 4
// CHECK-NEXT:   %add10 = add nsw i32 %14, %13
// CHECK-NEXT:   store i32 %add10, ptr %sum, align 4
// CHECK-NEXT:   br label %expand.end
// CHECK: expand.end:
// CHECK-NEXT:   %15 = load i32, ptr %sum, align 4
// CHECK-NEXT:   ret i32 %15


// CHECK-LABEL: define {{.*}} i32 @_Z15custom_iteratorv()
// CHECK: entry:
// CHECK-NEXT:   %sum = alloca i32, align 4
// CHECK-NEXT:   %__range1 = alloca ptr, align 8
// CHECK: %__begin1 = alloca %"struct.CustomIterator::iterator", align 4
// CHECK: %__end1 = alloca %"struct.CustomIterator::iterator", align 4
// CHECK: %__iter1 = alloca %"struct.CustomIterator::iterator", align 4
// CHECK-NEXT:   %x = alloca i32, align 4
// CHECK: %__iter12 = alloca %"struct.CustomIterator::iterator", align 4
// CHECK-NEXT:   %x5 = alloca i32, align 4
// CHECK: %__iter19 = alloca %"struct.CustomIterator::iterator", align 4
// CHECK-NEXT:   %x12 = alloca i32, align 4
// CHECK: %__iter116 = alloca %"struct.CustomIterator::iterator", align 4
// CHECK-NEXT:   %x19 = alloca i32, align 4
// CHECK-NEXT:   %__range122 = alloca ptr, align 8
// CHECK: %__begin123 = alloca %"struct.CustomIterator::iterator", align 4
// CHECK: %__end124 = alloca %"struct.CustomIterator::iterator", align 4
// CHECK: %__iter125 = alloca %"struct.CustomIterator::iterator", align 4
// CHECK-NEXT:   %x26 = alloca i32, align 4
// CHECK: %__iter129 = alloca %"struct.CustomIterator::iterator", align 4
// CHECK-NEXT:   %x30 = alloca i32, align 4
// CHECK: %__iter133 = alloca %"struct.CustomIterator::iterator", align 4
// CHECK-NEXT:   %x34 = alloca i32, align 4
// CHECK: %__iter137 = alloca %"struct.CustomIterator::iterator", align 4
// CHECK-NEXT:   %x38 = alloca i32, align 4
// CHECK-NEXT:   store i32 0, ptr %sum, align 4
// CHECK-NEXT:   store ptr @_ZZ15custom_iteratorvE1c, ptr %__range1, align 8
// CHECK-NEXT:   call void @llvm.memcpy.p0.p0.i64(ptr align 4 %__begin1, ptr align 4 @__const._Z15custom_iteratorv.__begin1, i64 4, i1 false)
// CHECK-NEXT:   call void @llvm.memcpy.p0.p0.i64(ptr align 4 %__end1, ptr align 4 @__const._Z15custom_iteratorv.__end1, i64 4, i1 false)
// CHECK-NEXT:   %call = call i32 @_ZNK14CustomIterator8iteratorplEi(ptr {{.*}} %__begin1, i32 {{.*}} 0)
// CHECK: %coerce.dive = getelementptr inbounds nuw %"struct.CustomIterator::iterator", ptr %__iter1, i32 0, i32 0
// CHECK-NEXT:   store i32 %call, ptr %coerce.dive, align 4
// CHECK-NEXT:   %call1 = call {{.*}} i32 @_ZNK14CustomIterator8iteratordeEv(ptr {{.*}} %__iter1)
// CHECK-NEXT:   store i32 %call1, ptr %x, align 4
// CHECK-NEXT:   %0 = load i32, ptr %x, align 4
// CHECK-NEXT:   %1 = load i32, ptr %sum, align 4
// CHECK-NEXT:   %add = add nsw i32 %1, %0
// CHECK-NEXT:   store i32 %add, ptr %sum, align 4
// CHECK-NEXT:   br label %expand.next
// CHECK: expand.next:
// CHECK-NEXT:   %call3 = call i32 @_ZNK14CustomIterator8iteratorplEi(ptr {{.*}} %__begin1, i32 {{.*}} 1)
// CHECK: %coerce.dive4 = getelementptr inbounds nuw %"struct.CustomIterator::iterator", ptr %__iter12, i32 0, i32 0
// CHECK-NEXT:   store i32 %call3, ptr %coerce.dive4, align 4
// CHECK-NEXT:   %call6 = call {{.*}} i32 @_ZNK14CustomIterator8iteratordeEv(ptr {{.*}} %__iter12)
// CHECK-NEXT:   store i32 %call6, ptr %x5, align 4
// CHECK-NEXT:   %2 = load i32, ptr %x5, align 4
// CHECK-NEXT:   %3 = load i32, ptr %sum, align 4
// CHECK-NEXT:   %add7 = add nsw i32 %3, %2
// CHECK-NEXT:   store i32 %add7, ptr %sum, align 4
// CHECK-NEXT:   br label %expand.next8
// CHECK: expand.next8:
// CHECK-NEXT:   %call10 = call i32 @_ZNK14CustomIterator8iteratorplEi(ptr {{.*}} %__begin1, i32 {{.*}} 2)
// CHECK: %coerce.dive11 = getelementptr inbounds nuw %"struct.CustomIterator::iterator", ptr %__iter19, i32 0, i32 0
// CHECK-NEXT:   store i32 %call10, ptr %coerce.dive11, align 4
// CHECK-NEXT:   %call13 = call {{.*}} i32 @_ZNK14CustomIterator8iteratordeEv(ptr {{.*}} %__iter19)
// CHECK-NEXT:   store i32 %call13, ptr %x12, align 4
// CHECK-NEXT:   %4 = load i32, ptr %x12, align 4
// CHECK-NEXT:   %5 = load i32, ptr %sum, align 4
// CHECK-NEXT:   %add14 = add nsw i32 %5, %4
// CHECK-NEXT:   store i32 %add14, ptr %sum, align 4
// CHECK-NEXT:   br label %expand.next15
// CHECK: expand.next15:
// CHECK-NEXT:   %call17 = call i32 @_ZNK14CustomIterator8iteratorplEi(ptr {{.*}} %__begin1, i32 {{.*}} 3)
// CHECK: %coerce.dive18 = getelementptr inbounds nuw %"struct.CustomIterator::iterator", ptr %__iter116, i32 0, i32 0
// CHECK-NEXT:   store i32 %call17, ptr %coerce.dive18, align 4
// CHECK-NEXT:   %call20 = call {{.*}} i32 @_ZNK14CustomIterator8iteratordeEv(ptr {{.*}} %__iter116)
// CHECK-NEXT:   store i32 %call20, ptr %x19, align 4
// CHECK-NEXT:   %6 = load i32, ptr %x19, align 4
// CHECK-NEXT:   %7 = load i32, ptr %sum, align 4
// CHECK-NEXT:   %add21 = add nsw i32 %7, %6
// CHECK-NEXT:   store i32 %add21, ptr %sum, align 4
// CHECK-NEXT:   br label %expand.end
// CHECK: expand.end:
// CHECK-NEXT:   store ptr @_ZZ15custom_iteratorvE1c, ptr %__range122, align 8
// CHECK-NEXT:   call void @llvm.memcpy.p0.p0.i64(ptr align 4 %__begin123, ptr align 4 @__const._Z15custom_iteratorv.__begin1.1, i64 4, i1 false)
// CHECK-NEXT:   call void @llvm.memcpy.p0.p0.i64(ptr align 4 %__end124, ptr align 4 @__const._Z15custom_iteratorv.__end1.2, i64 4, i1 false)
// CHECK-NEXT:   call void @llvm.memcpy.p0.p0.i64(ptr align 4 %__iter125, ptr align 4 @__const._Z15custom_iteratorv.__iter1, i64 4, i1 false)
// CHECK-NEXT:   store i32 1, ptr %x26, align 4
// CHECK-NEXT:   %8 = load i32, ptr %sum, align 4
// CHECK-NEXT:   %add27 = add nsw i32 %8, 1
// CHECK-NEXT:   store i32 %add27, ptr %sum, align 4
// CHECK-NEXT:   br label %expand.next28
// CHECK: expand.next28:
// CHECK-NEXT:   call void @llvm.memcpy.p0.p0.i64(ptr align 4 %__iter129, ptr align 4 @__const._Z15custom_iteratorv.__iter1.3, i64 4, i1 false)
// CHECK-NEXT:   store i32 2, ptr %x30, align 4
// CHECK-NEXT:   %9 = load i32, ptr %sum, align 4
// CHECK-NEXT:   %add31 = add nsw i32 %9, 2
// CHECK-NEXT:   store i32 %add31, ptr %sum, align 4
// CHECK-NEXT:   br label %expand.next32
// CHECK: expand.next32:
// CHECK-NEXT:   call void @llvm.memcpy.p0.p0.i64(ptr align 4 %__iter133, ptr align 4 @__const._Z15custom_iteratorv.__iter1.4, i64 4, i1 false)
// CHECK-NEXT:   store i32 3, ptr %x34, align 4
// CHECK-NEXT:   %10 = load i32, ptr %sum, align 4
// CHECK-NEXT:   %add35 = add nsw i32 %10, 3
// CHECK-NEXT:   store i32 %add35, ptr %sum, align 4
// CHECK-NEXT:   br label %expand.next36
// CHECK: expand.next36:
// CHECK-NEXT:   call void @llvm.memcpy.p0.p0.i64(ptr align 4 %__iter137, ptr align 4 @__const._Z15custom_iteratorv.__iter1.5, i64 4, i1 false)
// CHECK-NEXT:   store i32 4, ptr %x38, align 4
// CHECK-NEXT:   %11 = load i32, ptr %sum, align 4
// CHECK-NEXT:   %add39 = add nsw i32 %11, 4
// CHECK-NEXT:   store i32 %add39, ptr %sum, align 4
// CHECK-NEXT:   br label %expand.end40
// CHECK: expand.end40:
// CHECK-NEXT:   %12 = load i32, ptr %sum, align 4
// CHECK-NEXT:   ret i32 %12
