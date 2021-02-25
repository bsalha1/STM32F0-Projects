
#if defined(STM32)
#include "stm32f0xx.h"
#endif
#include <stdio.h>
#include <string.h>

void hello(void);
void showmult2(int a, int b);
void showmult3(int a, int b, int c);
void listing(const char *school, int course, const char *verb,
             int enrollment, const char *season, int year);
int trivial(unsigned int n);
void reverse_puts(const char *s);
int sumsq(unsigned int a, unsigned int b);

static int c_trivial(unsigned int n)
{
    int tmp[100]; // a stack-allocated array of 100 words
    for(int x=0; x < sizeof tmp / sizeof tmp[0]; x += 1)
        tmp[x] = x+1;
    if (n >= sizeof tmp / sizeof tmp[0])
        n = sizeof tmp / sizeof tmp[0] - 1;
    return tmp[n];
}

static int c_sumsq(unsigned int a, unsigned int b)
{
  int tmp[100];
  if (a >= 100)
    a = 99;
  if (b >= 100)
    b = 99;
  int step = 1;
  if (a == b)
    step = 0;
  else if (a > b)
    step = -1;
  for(int x=0; x < sizeof tmp / sizeof tmp[0]; x += 1)
    tmp[x] = x * x;
  int sum = 0;
  for(int x=a; ; x += step) {
    sum += tmp[x];
    if (x == b)
      return sum;
  }
}

void onoff(const char *s)
{
#if defined(STM32)
  s++;
  const char p = *s++;
  int n = 0;
  while(*s >= '0' && *s <= '9') n = n*10 + *s++ - '0';
  if (n > 15) return;
  if ((p == 'a' && (n == 13 || n == 14)) || (p == 'c' && n > 12) ||
      (p == 'b' && (n == 10 || n == 11)) || p == 'e') return;
  RCC->AHBENR /*easteregg*/ |= RCC_AHBENR_GPIOAEN<<(p-'a');
  typeof(GPIOA) pts[]={GPIOA,GPIOB,GPIOC,GPIOD,0,GPIOF};
  typeof(GPIOA) pt=pts[p-'a'];
  pt->MODER = (pt->MODER & ~(3<<(2*n))) | (1<<(2*n));
  pt->ODR ^= 1<<n;
#endif
}

void test_hello(void) {
    hello();
}
void test_showmult2(void) {
  for(int i=1; i<5; i++)
    for(int j=1; j<5; j++)
      showmult2(i,j);
}

void test_showmult3(void) {
  for(int i=3; i<5; i++)
    for(int j=3; j<5; j++)
      for(int k=3; k<5; k++)
        showmult3(i,j,k);
}

void test_listing(void) {
  listing("ECE", 27000, "had", 420, "Spring", 2020);
  listing("ECE", 36200, "had", 210, "Spring", 2020);
  listing("ECE", 27000, "has", 240, "Fall", 2020);
  listing("ECE", 36200, "has", 280, "Fall", 2020);
}

void test_trivial(void) {
    printf("  trivial(-1) = %d\n",   trivial(-1));
    printf("c_trivial(-1) = %d\n", c_trivial(-1));
    for(int i=1; i<20; i+=3) {
        printf("  trivial(%d) = %d\n", i,   trivial(i));
        printf("c_trivial(%d) = %d\n", i, c_trivial(i));
    }
}

void test_reverse_puts(void) {
  reverse_puts("This is a test!");
  reverse_puts("Testing.  123");
  reverse_puts("ABCDEFGHIJKLMNOPQRSTUVWXYZ..zyxwvutsrqponmlkjihgfedcba");
}

void test_sumsq(void) {
    printf("  sumsq(-1,10) = %d\n",   sumsq(-1,10));
    printf("c_sumsq(-1,10) = %d\n", c_sumsq(-1,10));
    printf("  sumsq(13,-2) = %d\n",   sumsq(13,-2));
    printf("c_sumsq(13,-2) = %d\n", c_sumsq(13,-2));
    printf("  sumsq(-3,-2) = %d\n",   sumsq(-3,-2));
    printf("c_sumsq(-3,-2) = %d\n", c_sumsq(-3,-2));
    for(int i=1; i<10; i+=3)
        for(int j=2; j<12; j+=4) {
            printf("  sumsq(%d,%d) = %d\n", i, j,   sumsq(i,j));
            printf("c_sumsq(%d,%d) = %d\n", i, j, c_sumsq(i,j));
        }
}

void test_all(void) {
  test_hello(); test_showmult2(); test_showmult3(); test_listing();
  test_trivial(); test_reverse_puts(); test_sumsq();
}

void (*test[])(void) = {
  test_all, test_hello, test_showmult2, test_showmult3, test_listing,
  test_trivial, test_reverse_puts, test_sumsq
};

void serial_init();

int main()
{
#if defined(STM32)
  serial_init();
#endif
  for(;;) {
    char buffer[10];
    printf("Select test: ");
    fgets(buffer, 10, stdin); buffer[9] = '\0';
    int num;
    if (sscanf(buffer, "%d", &num) != 1 || num < 0 || num > 7) {
      if (buffer[0] == 'p' && buffer[1] >= 'a' && buffer[1] <= 'f')
        onoff(buffer);
      else {
        printf("Enter a number between 1 and 7.\n");
        continue;
      }
    }
    test[num]();
  }
  return 0;
}

