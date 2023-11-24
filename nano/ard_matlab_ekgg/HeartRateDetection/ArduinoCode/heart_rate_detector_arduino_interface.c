/*
 * File: heart_rate_detector_arduino_interface.c
 *
 * MATLAB Coder version            : 2.8
 * C/C++ source code generated on  : 23-Oct-2017 11:57:38
 */

/* Include Files */
#include "heart_rate_detector_arduino_interface.h"

/* Variable Definitions */
static int x1;
static int x2;
static int x3;
static int x4;
static int x5;
static int x6;
static int x7;
static int x8;
static int x9;
static int x10;
static int b_y1;
static int y2;
static int x;
static int data[32];
static int n;
static int b_x1;
static int b_x2;
static int b_x3;
static int b_x4;
static int runningSum;
static boolean_T runningSum_not_empty;
static int x0;
static int c_x1;
static int c_x2;
static int c_x3;
static int c_x4;
static int b_x5;
static int b_x6;
static int b_x7;
static int b_x8;
static int b_x9;
static int b_x10;
static int x11;
static int x12;
static int x13;
static int x14;
static int x15;
static int maxPeak;
static boolean_T maxPeak_not_empty;
static int lastDatum;
static unsigned int timeSinceMax;
static int count;
static int lastQRSDelay;
static unsigned char b_n;
static int z[8];
static int lastY;

/* Function Declarations */
static int deriv1(int b_x0);
static void deriv1_init(void);
static void filter8_init(void);
static int heart_rate_detector(int u);
static int hpfilt(int u);
static void hpfilt_init(void);
static int lpfilt(int b_x);
static void lpfilt_init(void);
static void qrs_delay_init(void);

/* Function Definitions */

/*
 * deriv1 implements the derivative approximation represented by
 *   the difference equation:
 *
 *  y[n] = 2*x[n] + x[n-1] - x[n-3] - 2*x[n-4]
 *
 *   The filter has a delay of 2.
 * Arguments    : int b_x0
 * Return Type  : int
 */
static int deriv1(int b_x0)
{
  int y;
  y = (((b_x0 << 1) + b_x1) - b_x3) - (b_x4 << 1);
  b_x4 = b_x3;
  b_x3 = b_x2;
  b_x2 = b_x1;
  b_x1 = b_x0;
  return y;
}

/*
 * Arguments    : void
 * Return Type  : void
 */
static void deriv1_init(void)
{
  b_x1 = 0;
  b_x2 = 0;
  b_x3 = 0;
  b_x4 = 0;
}

/*
 * Arguments    : void
 * Return Type  : void
 */
static void filter8_init(void)
{
  int i;
  b_n = 0;
  for (i = 0; i < 8; i++) {
    z[i] = 0;
  }

  lastY = 0;
}

/*
 * heart_rate_detector QRS detection based on heart_rate_detector in picqrs.c from
 *  Open Source ECG Analysis (OSEA)
 *  http://www.eplimited.com/confirmation.htm
 * Arguments    : int u
 * Return Type  : int
 */
static int heart_rate_detector(int u)
{
  int heart_rate;
  int lp;
  int hp;
  int d;
  int avg;
  int pk;
  int c[8];
  int m;
  int k;
  unsigned int y;
  unsigned int q;
  unsigned int b_x;
  heart_rate = 0;
  lp = lpfilt(u);
  hp = hpfilt(lp);
  d = deriv1(hp);

  /*  mvwint() implements a moving window integrator, averaging */
  /*  the signal values over the last 16 */
  avg = 0;
  if (!runningSum_not_empty) {
    runningSum = 0;
    runningSum_not_empty = true;
    x0 = 0;
    c_x1 = 0;
    c_x2 = 0;
    c_x3 = 0;
    c_x4 = 0;
    b_x5 = 0;
    b_x6 = 0;
    b_x7 = 0;
    b_x8 = 0;
    b_x9 = 0;
    b_x10 = 0;
    x11 = 0;
    x12 = 0;
    x13 = 0;
    x14 = 0;
    x15 = 0;
  } else {
    runningSum -= x15;
    x15 = x14;
    x14 = x13;
    x13 = x12;
    x12 = x11;
    x11 = b_x10;
    b_x10 = b_x9;
    b_x9 = b_x8;
    b_x8 = b_x7;
    b_x7 = b_x6;
    b_x6 = b_x5;
    b_x5 = c_x4;
    c_x4 = c_x3;
    c_x3 = c_x2;
    c_x2 = c_x1;
    c_x1 = x0;
    if (d < 0) {
      x0 = -d;
    } else {
      x0 = d;
    }

    runningSum += x0;
    avg = runningSum >> 4;
  }

  /*  peak() takes a datum as input and returns a peak height */
  /*  when the signal returns to half its peak height, or it has been */
  /*  95 ms since the peak height was detected. */
  pk = 0;
  if (!maxPeak_not_empty) {
    maxPeak = 0;
    maxPeak_not_empty = true;
    lastDatum = 0;
    timeSinceMax = 0U;
  } else {
    if (timeSinceMax > 0U) {
      timeSinceMax++;
    }

    if ((avg > lastDatum) && (avg > maxPeak)) {
      /*  rising slope */
      maxPeak = avg;
      if (maxPeak > 2048) {
        /*  reset timeSinceMax  */
        /*  Not sure what maxPeak > 2 means with scaling? */
        timeSinceMax = 1U;
      }
    } else if (avg < (maxPeak >> 1)) {
      /*  middle of falling slope */
      /*  Less than half the peak height */
      pk = maxPeak;
      maxPeak = 0;
      timeSinceMax = 0U;
    } else {
      if (timeSinceMax > 19U) {
        pk = maxPeak;
        maxPeak = 0;
        timeSinceMax = 0U;
      }
    }

    lastDatum = avg;
  }

  /*  Normal low range for a QRS detection is 60ms. */
  /*  Characteristics of the Normal ECG. Frank G. Yanowitz, */
  /*  MD. Professor of Medicine. University of Utah School of */
  /*  Medicine. http://ecg.utah.edu. */
  count++;
  if (pk != 0) {
    z[(unsigned char)(b_n + 1U) - 1] = pk;
    b_n = (unsigned char)((unsigned char)(b_n + 1U) & 7);
    for (m = 0; m < 8; m++) {
      c[m] = z[m] >> 3;
    }

    lastY = c[0];
    for (k = 0; k < 7; k++) {
      lastY += c[k + 1];
    }
  }

  if ((pk > lastY) && (count > 12)) {
    lastQRSDelay = count;
    count = 0;
  }

  if (lastQRSDelay != 0) {
    /*  (1000 ms/s * 60s/m) / (samples per ms) / (QRSDelay samples) */
    if (lastQRSDelay == 0) {
      heart_rate = MAX_int16_T;
    } else if (lastQRSDelay == 1) {
      heart_rate = 12000;
    } else if (lastQRSDelay == -1) {
      heart_rate = -12000;
    } else {
      if (lastQRSDelay >= 0) {
        y = (unsigned int)lastQRSDelay;
      } else if (lastQRSDelay == MIN_int16_T) {
        y = 32768U;
      } else {
        y = (unsigned int)-lastQRSDelay;
      }

      q = 12000U / y;
      b_x = 12000U - q * y;
      if ((b_x > 0U) && (b_x >= (y >> 1U) + (y & 1U))) {
        q++;
      }

      heart_rate = (int)q;
      if (lastQRSDelay < 0) {
        heart_rate = -(int)q;
      }
    }
  }

  /* { */
  /*     coder.inline('never'); */
  /*     R = my_replacements(replacement_lib); */
  /*     x = cast(u, 'like', T.data); */
  /*     heart_rate = cast(0,'like',T.heart_rate_output); */
  /*     lp = R.lpfilt(x); */
  /*     hp = R.hpfilt(lp); */
  /*     d = R.deriv1(hp); */
  /*     a = abs(d); */
  /*     avg = R.mvwint(a); */
  /*     pk = R.peak(avg); */
  /*  */
  /*     [QRSDelay, threshold] = R.qrs_delay(pk); */
  /*     if QRSDelay ~= 0 */
  /*         % (1000 ms/s * 60s/m) / (samples per ms) / (QRSDelay samples) */
  /*         heart_rate(:) = cast(60000 / SAMPLES_PER_MS,'like',T.int) / QRSDelay; */
  /*     end */
  /* } */
  return heart_rate;
}

/*
 * hpfilt() implements the high pass filter represented by the following
 *   difference equation:
 *
 *        x[n] = x[n-1] + u[n] - u[n-32]
 *        y[n] = u[n-16] - x[n] ;
 *
 *   Note that the filter delay is 15.5 samples
 * Arguments    : int u
 * Return Type  : int
 */
static int hpfilt(int u)
{
  int y;

  /*  Circular buffer n */
  x = (x + u) - data[n];
  y = data[(n - 16) & 31] - (x >> 5);
  data[n] = u;
  n = (n + 1) & 31;
  return y;
}

/*
 * Arguments    : void
 * Return Type  : void
 */
static void hpfilt_init(void)
{
  x = 0;
  memset(&data[0L], 0, sizeof(int) << 5);
  n = 0;
}

/*
 * lpfilt() implements the digital filter represented by the difference
 *   equation:
 *
 *   y[n] = 2*y[n-1] - y[n-2] + x[n] - 2*x[n-5] + x[n-10]
 *
 *  Note that the filter delay is five samples.
 *
 *   y(n) = 2*y(n-1) - y(n-2) + x(n) - 2*x(n-5) + x(n-10)
 *
 *        1*y(n) - 2*y(n-1) + 1*y(n-2) = 1*x(n) + 0*x(n-1) + 0*x(n-2) + 0*x(n-3) +  0*x(n-4) - 2*x(n-5) + 0*x(n-6) + 0*x(n-7) + 0*x(n-8)  + 0*x(n-9) + 1*x(n-10)
 *   a = [1       -2          1];   b = [1        0          0          0           0         -2          0          0          0           0          1];
 *
 *  Lowpass filter coefficients.  The roots of a are on the unit circle, but
 *  they exactly cancel with roots of b.
 *  Dividing numerator and denominator gives q = [1 2 3 4 5 4 3 2 1].
 *  It's counting on perfect cancellation in order to get a filter that only
 *  has powers of two for coefficients.
 *  b = [1  0   0   0    0  -2   0   0   0    0   1];
 *  a = [1  -2  1];
 *  roots_a = roots(a)
 *  [q,r] = deconv(b,a) %#ok<*NOPTS>
 * Arguments    : int b_x
 * Return Type  : int
 */
static int lpfilt(int b_x)
{
  int y;
  int b_y0;
  b_y0 = ((((b_y1 << 1) - y2) + b_x) - (x5 << 1)) + x10;
  y2 = b_y1;
  b_y1 = b_y0;
  y = b_y0 >> 5;
  x10 = x9;
  x9 = x8;
  x8 = x7;
  x7 = x6;
  x6 = x5;
  x5 = x4;
  x4 = x3;
  x3 = x2;
  x2 = x1;
  x1 = b_x;
  return y;
}

/*
 * Arguments    : void
 * Return Type  : void
 */
static void lpfilt_init(void)
{
  x1 = 0;
  x2 = 0;
  x3 = 0;
  x4 = 0;
  x5 = 0;
  x6 = 0;
  x7 = 0;
  x8 = 0;
  x9 = 0;
  x10 = 0;
  b_y1 = 0;
  y2 = 0;
}

/*
 * Arguments    : void
 * Return Type  : void
 */
static void qrs_delay_init(void)
{
  count = 0;
  lastQRSDelay = 0;
}

/*
 * Maintain the int16 output for the Arduino interface.
 *  Expects input x to be signed 16-bit fixed-point or integer.
 * Arguments    : int b_x
 * Return Type  : int
 */
int heart_rate_detector_arduino_interface(int b_x)
{
  int hr;
  hr = heart_rate_detector(b_x);
  return hr;
}

/*
 * Arguments    : void
 * Return Type  : void
 */
void heart_rate_detector_arduino_interface_initialize(void)
{
  maxPeak_not_empty = false;
  runningSum_not_empty = false;
  lpfilt_init();
  hpfilt_init();
  deriv1_init();
  qrs_delay_init();
  filter8_init();
}

/*
 * Arguments    : void
 * Return Type  : void
 */
void heart_rate_detector_arduino_interface_terminate(void)
{
  /* (no terminate code required) */
}

/*
 * File trailer for heart_rate_detector_arduino_interface.c
 *
 * [EOF]
 */
