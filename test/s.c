
int add(int* ax, int* ay)
{
  return *ax + *ay;
}

int main()
{
  int x, y, z, w;
  int* px;

  px = &x;
  *px = 20;

  px = &y;
  *px = 10;

  z = add(px, px);
}
