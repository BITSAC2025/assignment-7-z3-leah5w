/**
 * verify.cpp
 * @author kisslune 
 */

#include "Z3Mgr.h"

using namespace z3;
using namespace SVF;


/* A simple example

int main() {
    int* p;
    int q;
    int* r;
    int x;

    p = malloc();
    q = 5;
    *p = q;
    x = *p;
    assert(x==10);
}
*/
void Z3Tests::test0()
{
    //  int* p;
    expr p = getZ3Expr("p");
    //  int q;
    expr q = getZ3Expr("q");
    //  int* r;
    expr r = getZ3Expr("r");
    //  int x;
    expr x = getZ3Expr("x");
    //  p = malloc();
    addToSolver(p == getMemObjAddress("malloc"));
    //  q = 5;
    addToSolver(q == 5);
    //  *p = q;
    storeValue(p, q);
    //  x = *p;
    addToSolver(x == loadValue(p));

    // print the expression values
    printExprValues();

    addToSolver(x == getZ3Expr(10));
    // print the result of satisfiability check
    std::cout << solver.check() << std::endl;
}


/*
// Simple integers

    int main() {
        int a;
        int b;
        a = 0;
        b = a + 1;
        assert(b > 0);
    }
*/
void Z3Tests::test1()
{
    expr a = getZ3Expr("a");
    expr b = getZ3Expr("b");

    addToSolver(a == ctx.int_val(0));
    addToSolver(b == a + 1);

    addToSolver(b > ctx.int_val(0));
    std::cout << solver.check() << std::endl;
}

/*
  // One-level pointers

    int main() {
        int* p;
        int q;
        int b;
        p = malloc;
        *p = 0;
        q = *p;
        *p = 3;
        b = *p + 1;
        assert(b > 3);
    }
*/
void Z3Tests::test2()
{
    expr p = getZ3Expr("p");
    expr q = getZ3Expr("q");
    expr b = getZ3Expr("b");

    addToSolver(p == getMemObjAddress("malloc_p"));
    storeValue(p, ctx.int_val(0));

    addToSolver(q == loadValue(p));
    storeValue(p, ctx.int_val(3));
    addToSolver(b == loadValue(p) + 1);

    addToSolver(b > ctx.int_val(3));
    std::cout << solver.check() << std::endl;
}

/*
    // Mutiple-level pointers

    int main() {
        int** p;
        int* q;
        int* r;
        int x;

        p = malloc1(..);
        q = malloc2(..);
        *p = q;
        *q = 10;
        r = *p;
        x = *r;
        assert(x==10);
    }
*/
void Z3Tests::test3()
{
    expr p = getZ3Expr("p");
    expr q = getZ3Expr("q");
    expr r = getZ3Expr("r");
    expr x = getZ3Expr("x");

    addToSolver(p == getMemObjAddress("malloc_p"));
    addToSolver(q == getMemObjAddress("malloc_q"));

    storeValue(p, q);
    storeValue(q, ctx.int_val(10));

    addToSolver(r == loadValue(p));
    addToSolver(x == loadValue(r));

    addToSolver(x == ctx.int_val(10));
    std::cout << solver.check() << std::endl;
}

/*
   // Array and pointers

    int main() {
        int* p;
        int* x;
        int* y;
        int a;
        int b;
        p = malloc;
        x = &p[0];
        y = &p[1]
        *x = 10;
        *y = 11;
        a = *x;
        b = *y;
        assert((a + b)>20);
    }
*/
void Z3Tests::test4()
{
    expr p = getZ3Expr("p");
    expr x = getZ3Expr("x");
    expr y = getZ3Expr("y");
    expr a = getZ3Expr("a");
    expr b = getZ3Expr("b");

    addToSolver(p == getMemObjAddress("array"));
    addToSolver(x == getGepObjAddress(p, 0));
    addToSolver(y == getGepObjAddress(p, 1));

    storeValue(x, ctx.int_val(10));
    storeValue(y, ctx.int_val(11));

    addToSolver(a == loadValue(x));
    addToSolver(b == loadValue(y));

    addToSolver((a + b) > ctx.int_val(20));
    std::cout << solver.check() << std::endl;
}

/*
    // Branches

int main(int argv) {
    int a;
    int b;
    int b1;
    a = argv + 1;
    b = 5;
    if(a > 10)
        b = a;
    b1 = b;
    assert(b1 >= 5);
}
*/
void Z3Tests::test5()
{
    expr argv = getZ3Expr("argv");
    expr a = getZ3Expr("a");
    expr b = getZ3Expr("b");
    expr b1 = getZ3Expr("b1");

    addToSolver(a == argv + 1);
    expr cond = a > ctx.int_val(10);
    addToSolver(b == z3::ite(cond, a, ctx.int_val(5)));
    addToSolver(b1 == b);

    addToSolver(b1 >= ctx.int_val(5));
    std::cout << solver.check() << std::endl;
}

/*
// Compare and pointers
int main() {
   int *a = malloc1;
   int *b = malloc2;
   *a = 5;
   *b = 10;
   int *p;
   if (*a < *b) {
       p = a;
   } else {
       p = b;
   }
   assert(*p == 5);
}
*/
void Z3Tests::test6()
{
    expr aPtr = getZ3Expr("aPtr");
    expr bPtr = getZ3Expr("bPtr");
    expr p = getZ3Expr("p");

    addToSolver(aPtr == getMemObjAddress("malloc_a"));
    addToSolver(bPtr == getMemObjAddress("malloc_b"));

    storeValue(aPtr, ctx.int_val(5));
    storeValue(bPtr, ctx.int_val(10));

    expr valA = loadValue(aPtr);
    expr valB = loadValue(bPtr);
    addToSolver(p == z3::ite(valA < valB, aPtr, bPtr));

    addToSolver(loadValue(p) == ctx.int_val(5));
    std::cout << solver.check() << std::endl;
}

/*
 int main() {
	int a = 1, b = 2, c = 3;
	int d;
  if (a > 0) {
  	d = b + c;
  }
  else {
  	d = b - c;
  }
  assert(d == 5);
 }
 */
void Z3Tests::test7()
{
    expr a = getZ3Expr("a");
    expr b = getZ3Expr("b");
    expr c = getZ3Expr("c");
    expr d = getZ3Expr("d");

    addToSolver(a == ctx.int_val(1));
    addToSolver(b == ctx.int_val(2));
    addToSolver(c == ctx.int_val(3));

    addToSolver(d == z3::ite(a > ctx.int_val(0), b + c, b - c));
    addToSolver(d == ctx.int_val(5));

    std::cout << solver.check() << std::endl;
}

/*
 int main() {
    int arr[2] = {0, 1};
    int a = 10;
    int *p;
    if (a > 5) {
        p = &arr[0];
    }
    else {
        p = &arr[1];
    }
    assert(*p == 0);
 }
 */
void Z3Tests::test8()
{
    expr arr = getZ3Expr("arr");
    expr p = getZ3Expr("p");
    expr a = getZ3Expr("a");

    addToSolver(arr == getMemObjAddress("arr_base"));
    z3::expr arr0 = getGepObjAddress(arr, 0);
    z3::expr arr1 = getGepObjAddress(arr, 1);

    storeValue(arr0, ctx.int_val(0));
    storeValue(arr1, ctx.int_val(1));

    addToSolver(a == ctx.int_val(10));
    addToSolver(p == z3::ite(a > ctx.int_val(5), arr0, arr1));

    addToSolver(loadValue(p) == ctx.int_val(0));
    std::cout << solver.check() << std::endl;
}

/*
    // Struct and pointers

    struct A{ int f0; int* f1;};
    int main() {
       struct A* p;
       int* x;
       int* q;
       int** r;
       int* y;
       int z;

       p = malloc1;
       x = malloc2;
       *x = 5;
       q = &(p->f0);
       *q = 10;
       r = &(p->f1);
       *r = x;
       y = *r;
       z = *q + *y;
       assert(z == 15);
    }
*/
void Z3Tests::test9()
{
    expr p = getZ3Expr("p");
    expr x = getZ3Expr("x");
    expr q = getZ3Expr("q");
    expr r = getZ3Expr("r");
    expr y = getZ3Expr("y");
    expr z = getZ3Expr("z");

    addToSolver(p == getMemObjAddress("structA"));
    addToSolver(x == getMemObjAddress("malloc_x"));

    storeValue(x, ctx.int_val(5));

    addToSolver(q == getGepObjAddress(p, 0));
    storeValue(q, ctx.int_val(10));

    addToSolver(r == getGepObjAddress(p, 1));
    storeValue(r, x);

    addToSolver(y == loadValue(r));
    addToSolver(z == loadValue(q) + loadValue(y));

    addToSolver(z == ctx.int_val(15));
    std::cout << solver.check() << std::endl;
}

/*
int foo(int z) {
    k = z;
    return k;
}
int main() {
  int x;
  int y;
  y = foo(2);
  x = foo(3);
  assert(x == 3 && y == 2);
}
*/
void Z3Tests::test10()
{
    expr y = getZ3Expr("y");
    expr x = getZ3Expr("x");
    expr k_after_foo_y = getZ3Expr("k_after_foo_y");
    expr k_after_foo_x = getZ3Expr("k_after_foo_x");

    addToSolver(k_after_foo_y == ctx.int_val(2));
    addToSolver(y == k_after_foo_y);

    addToSolver(k_after_foo_x == ctx.int_val(3));
    addToSolver(x == k_after_foo_x);

    addToSolver(x == ctx.int_val(3) && y == ctx.int_val(2));
    std::cout << solver.check() << std::endl;
}


int main()
{
    Z3Tests tests;
    tests.test0();
    tests.resetSolver();
    tests.test1();
    tests.resetSolver();
    tests.test2();
    tests.resetSolver();
    tests.test3();
    tests.resetSolver();
    tests.test4();
    tests.resetSolver();
    tests.test5();
    tests.resetSolver();
    tests.test6();
    tests.resetSolver();
    tests.test7();
    tests.resetSolver();
    tests.test8();
    tests.resetSolver();
    tests.test9();
    tests.resetSolver();
    tests.test10();

    return 0;
}