

void Add(int* a, int* b)
{
  *a += *b;
}

void Inc(int* z)
{
  int v = 1;
  Add(z, &v);
}

void A(int* x, int* y)
{
  Add(x, y);
  Inc(y);
}

void main()
{
  int sum;
  int i;
  
  sum = 0;
  i = 1;
  
  while(i < 11)
  {
    A(&sum, &i);
  }
}
