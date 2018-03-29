// C -> Fortran: Fortran passes everything by-reference

double randlc (double *x, double *a)
{
  return randlc_ (x, a);
}

void vranlc (int n, double *x, int a, double *y)
{
  vranlc_ (&n, x, &a, y);
}
