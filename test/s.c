#include <stdint.h>


void arith(int z)
{
  int a, b, c, d, e, f;
  /*char a, b, c, d, e, f;*/
  /*int64_t a, b, c, d, e, f;*/
  
  if(z > 0)
  {
    a = 22;
    b = 20;
  }
  else
  {
    a = -22;
    b = -10;
  }
  
  c = a+b;
  d = a-b;
  e = a*b;
  f = a/b;
}

void logical(int z)
{
  int a, b, c, d, e, f;
  /*int64_t a, b, c, d, e, f;*/
  
  if(z > 0)
  {
    a = 512;
    b = 8;
  }
  else
  {
    a = 4;
    b = 2;
  }
  
  c = a & b;
  d = a | b;
  e = a ^ b;
  f = ~a;
}

void branch(int z)
{
  int a, b, c, d, e, f, g, h;
  
  if(z > 0)
  {
    a = 255;
    b = 15;
  }
  else
  {
    a = -16;
    b = -10;
  }
  
  if(a > b)
  {
    c = a;
  }
  else
  {
    c = b;
  }
  
  if(a >= b)
  {
    d = a;
  }
  else
  {
    d = b;
  }
  
  if(a < b)
  {
    e = a;
  }
  else
  {
    e = b;
  }
  
  if(a <= b)
  {
    f = a;
  }
  else
  {
    f = b;
  }
  
  if(a == b)
  {
    g = a;
  }
  else
  {
    g = b;
  }
  
  if(a != b)
  {
    h = a;
  }
  else
  {
    h = b;
  }
}

void loop(int low, int high)
{
  int max = 0;
  int i;
  int l, h;
  if(high > 500)
    h = 500;
  else 
    h = high;
    
  if(low < 0)
    l = 0;
  else
    l = low;
  
  for(i = l; i <= h;++i)
    max = i;
}


/*
int main()
{
  return 0;
}
*/
