const int ntc_abe[6] = {1500, 2200, 22000, 220000, 150};
const int ptc_abg[7] = {2200, 1000, 220, 150, 100, 22, 22};
const int ptc_xyz[8][3] = {
  {1, 1, 1},
  {0, 1, 1},
  {1, 0, 1},
  {0, 0, 1},
  {1, 1, 0},
  {0, 1, 0},
  {1, 0, 0},
  {0, 0, 0},
};

double inter(const double x, const int a, const int b) {
  return x * (b - a) + a;
}

double ptc_function_int(const int x, const int y, const int z) {
  const double a = ptc_abg[0];
  const double b = ptc_abg[1];
  const double c = ptc_abg[2];
  const double d = ptc_abg[3];
  const double e = ptc_abg[4];
  const double f = ptc_abg[5];
  const double g = ptc_abg[6];
  return g + 1 / (z / e + 1 / (f + 1 / (y / c + 1 / (d + 1 / (x / a + 1 / b)))));
}

void ptc_float(const double x, const double y, const double z, double &a, double &b, double &c, double &A, double &B, double &C, double &D) {
  if (x <= y) {
    if (y <= z) {
      a = x, b = y, c = z;
    } else if (x <= z) {
      a = x, b = z, c = y;
    } else {
      a = z, b = x, c = y;
    }
  } else {
    if (y > z) {
      a = z, b = y, c = x;
    } else if (x > z) {
      a = y, b = z, c = x;
    } else {
      a = y, b = x, c = z;
    }
  }

  A = ptc_function_int(x >= a, y >= a, z >= a);
  B = ptc_function_int(x >= b, y >= b, z >= b);
  C = ptc_function_int(x >= c, y >= c, z >= c);
  D = ptc_function_int(x >= 1, y >= 1, z >= 1);
}

double ptc_function_float(const double x, const double y, const double z) {
  double a, b, c, A, B, C, D;
  ptc_float(x, y, z, a, b, c, A, B, C, D);

  return 1 / (a / A + (b - a) / B + (c - b) / C + (1 - c) / D);
}

double ptc_function_inter(const double x0, const int i) {
  const int *row0 = ptc_xyz[i];
  const int *row1 = ptc_xyz[i + 1];
  const int m = row0[0];
  const int n = row1[0];
  const int o = row0[1];
  const int p = row1[1];
  const int q = row0[2];
  const int r = row1[2];

  const double x = inter(x0, m, n);
  const double y = inter(x0, o, p);
  const double z = inter(x0, q, r);

  return ptc_function_float(x, y, z);
}

double ptc_derivative_inter(const double x0, const int i) {
  const int *row0 = ptc_xyz[i];
  const int *row1 = ptc_xyz[i + 1];
  const int m = row0[0];
  const int n = row1[0];
  const int o = row0[1];
  const int p = row1[1];
  const int q = row0[2];
  const int r = row1[2];

  const double x = inter(x0, m, n);
  const double y = inter(x0, o, p);
  const double z = inter(x0, q, r);

  int a1, a2;
  int b1, b2;
  int c1, c2;

  if (x <= y) {
    if (y <= z) {
      a1 = m, b1 = o, c1 = q;
      a2 = n, b2 = p, c2 = r;
    } else if (x <= z) {
      a1 = m, b1 = q, c1 = o;
      a2 = n, b2 = r, c2 = p;
    } else {
      a1 = q, b1 = m, c1 = o;
      a2 = r, b2 = n, c2 = p;
    }
  } else {
    if (y > z) {
      a1 = q, b1 = o, c1 = m;
      a2 = r, b2 = p, c2 = n;
    } else if (x > z) {
      a1 = o, b1 = q, c1 = m;
      a2 = p, b2 = r, c2 = n;
    } else {
      a1 = o, b1 = m, c1 = q;
      a2 = p, b2 = n, c2 = r;
    }
  }

  double a, b, c, A, B, C, D;
  ptc_float(x, y, z, a, b, c, A, B, C, D);

  const double u = (a2 - a1) / A + (a1 - a2 - b1 + b2) / B + (b1 - b2 - c1 + c2) / C + (c1 - c2) / D;
  const double v = (x0 * (a2 - a1) + a1) / A + (-x * (a2 - a1) - a1 + x0 * (b2 - b1) + b1) / B + (-x0 * (b2 - b1) - b1 + x0 * (c2 - c1) + c1) / C + (-x0 * (c2 - c1) - c1 + 1) / D;

  return -u / (v * v);
}

double newton(const double x0, const double y0, double (*function)(const double x, const int i), double (*derivative)(const double x, const int i), const int i, const int n) {
  double x = x0;

  for (int k = 0; k < n; k++) {
    const double y = function(x, i);
    const double dx = derivative(x, i);
    x -= (y - y0) / dx;
  }

  return x;
}

CONTROL_STATUS control(const double resistance, double &freq1, double &freq2, double &freq3, const SENSOR &sensor) {
  switch (typeOutput(sensor)) {
    case OUTPUT_TYPE::NTC:
      return control_ntc(resistance, freq1, freq2, freq3);

    case OUTPUT_TYPE::PTC:
      return control_ptc(resistance, freq1, freq2, freq3);
  }

  return CONTROL_STATUS::ERROR;
}

CONTROL_STATUS control_ntc(const double r, double &freq1, double &freq2, double &freq3) {
  const double a = ntc_abe[0];
  const double b = ntc_abe[1];
  const double c = ntc_abe[2];
  const double d = ntc_abe[3];
  const double e = ntc_abe[4];

  if (r - e >= d) {
    freq1 = freq2 = freq3 = 0;
    return CONTROL_STATUS::IS_HIGH;
  }

  if ((r - e) * (d + c) >= c * d) {
    // 1/((1-x)/(1/(1/d)+e)+x/(1/(1/c+1/d)+e))
    freq1 = freq2 = 0;
    freq3 = (c * (d + e) + e * d) * (d - r + e) / (d * d * r);
    return CONTROL_STATUS::GOOD;
  }

  if ((r - e) * (c * d + b * d + b * c) >= b * c * d) {
    // 1/((1-x)/(1/(1/c+1/d)+e)+x/(1/(1/b+1/c+1/d)+e))
    freq1 = 0;
    freq2 = (b * c * (d + e) + e * b * d + e * c * d) * (c * (d - r + e) + d * (e - r)) / (c * c * d * d * r);
    freq3 = 1;
    return CONTROL_STATUS::GOOD;
  }

  if ((r - e) * (b * c * d + a * c * d + a * b * d + a * b * c) >= a * b * c * d) {
    // 1/((1-x)/(1/(1/b+1/c+1/d)+e)+x/(1/(1/a+1/b+1/c+1/d)+e))
    freq1 = (a * b * (c * (d + e) + d * e) + a * c * d * e + b * c * d * e) * (b * c * (d + e - r) + b * d * (e - r) + c * d * (e - r)) / (b * b * c * c * d * d * r);
    freq2 = freq3 = 1;
    return CONTROL_STATUS::GOOD;
  }

  freq1 = freq2 = freq3 = 1;
  return CONTROL_STATUS::IS_LOW;
}

CONTROL_STATUS control_ptc(const double r, double &freq1, double &freq2, double &freq3) {
  if (r <= ptc_function_inter(0, 0)) {
    freq1 = freq2 = freq3 = 1;
    return CONTROL_STATUS::IS_HIGH;
  }

  for (int i = 0; i < 7; i++) {
    if (r > ptc_function_inter(1, i)) {
      continue;
    }

    const double x = newton(0.9999, r, &ptc_function_inter, &ptc_derivative_inter, i, 8);

    const int *row0 = ptc_xyz[i];
    const int *row1 = ptc_xyz[i + 1];

    freq1 = inter(x, row0[0], row1[0]);
    freq2 = inter(x, row0[1], row1[1]);
    freq3 = inter(x, row0[2], row1[2]);
    return CONTROL_STATUS::GOOD;
  }

  freq1 = freq2 = freq3 = 0;
  return CONTROL_STATUS::IS_LOW;
}
