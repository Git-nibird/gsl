/* Author:  G. Jungman
 * RCS:     $Id$
 */
#include <stdio.h>
#include <math.h>
#include <gsl_errno.h>
#include <gsl_math.h>
#include <gsl_integration.h>
#include "gsl_sf_expint.h"
#include "gsl_sf_dilog.h"

#define locMin(a,b)  ((a) < (b) ? (a) : (b))
#define locMax(a,b)  ((a) > (b) ? (a) : (b))


/* data for summation loop for dilog()
 * CC(N)=(N(N+1)(N+2))**2  [with index shift for conversion to C]
 */
static double CC[30] = {
  36.0,         576.0,       36.0e+2,      144.0e+2,     441.0e+2,
  112896.0,     254016.0,    5184.0e+2,    9801.0e+2,    17424.0e+2,
  2944656.0,    4769856.0,   74529.0e+2,   112896.0e+2,  166464.0e+2,
  23970816.0,   33802596.0,  467856.0e+2,  636804.0e+2,  853776.0e+2,
  112911876.0,  147476736.0, 19044.0e+4,   24336.0e+4,   3080025.0e+2,
  386358336.0,  480661776.0, 5934096.0e+2, 7273809.0e+2, 8856576.0e+2
};

/* summation loop used by most cases in dilog() */
static double do_sum(const double y, const double by, double dl)
{
  int i;
  double a;
  double b = 4.0*y*y/by;
  double pre_dl;
  for(i=0; i<30; i++) {
    b = b*y;
    a = b/CC[i];
    pre_dl = dl;
    dl += a;
    if(dl == pre_dl) break;
  }
  return dl;
}


/* based on:
   ALGORITHM 490 COLLECTED ALGORITHMS FROM ACM.
   ALGORITHM APPEARED IN COMM. ACM, VOL. 18, NO. 4N, P. 200.
   DOUBLE PRECISION FUNCTION DILOG(X)
   used: ZERO OF DILOG ON THE POSITIVE REAL AXIS, X0=12.59517...

   I checked this with Mathematica Re[PolyLog[2,x]]; it works. [GJ]
*/
int
gsl_sf_dilog_impl(const double x, double * result)
{
  if(x > 12.6) {
    double log_x = log(x);
    double Y  = 1.0/x;
    double BY = -1.0 - Y*(4.+Y);
    double dl = 3.28986813369645287 -
      0.5*log_x*log_x + (Y*(4.+5.75*Y)+3.*(1.+Y)*(1.-Y)*log(1.-Y))/BY;
    if(dl + 4.0*Y == dl) {
      *result = dl;
      return GSL_SUCCESS;
    }
    else {
      *result = do_sum(Y, BY, dl);
      return GSL_SUCCESS;
    }
  }
  else if(x >= 12.59) {
    /* DILOG COMPUTED FROM TAYLOR SERIES ABOUT ZERO OF DILOG, X0. */
    double X0 = 12.5951703698450184;
    double Y  = x/X0 - 1.;
    double Z  = 1.0/11.5951703698450184;
    double W  = Y*Z;
    double C1 = (3.*X0-2.)/6.;
    double C2 = ((11.*X0-15.)*X0+6.)/24.;
    double C3 = (((50.*X0-104.)*X0+84.)*X0-24.)/120.;
    double C4 = ((((274.*X0-770.)*X0+940.)*X0-540.)*X0+120.)/720.;
    *result = Y*(1.-Y*(0.5-Y*(1./3.-Y*(0.25-Y*(.2-Y/6.)))))*log(Z)
      - W*X0*Y*(0.5-W*(C1-W*(C2-W*(C3-W*C4))));
    return GSL_SUCCESS;
  }
  else if(x >= 2.0) {
    /* same as first case... */
    double log_x = log(x);
    double Y  = 1.0/x;
    double BY = -1.0 - Y*(4.0+Y);
    double dl = 3.28986813369645287 -
      0.5*log_x*log_x + (Y*(4.+5.75*Y)+3.*(1.+Y)*(1.-Y)*log(1.-Y))/BY;
    if(dl + 4.0*Y == dl) {
      *result = dl;
      return GSL_SUCCESS;
    }
    else {
      *result = do_sum(Y, BY, dl);
      return GSL_SUCCESS;
    }
  }
  else if(x > 1.0) {
    /* DILOG COMPUTED FROM REF. NO. 1, P.244, EQ(7) WITH
     * X=1/X + EQ(6), AND DESCRIPTION OF THIS ALGORITHM, EQ(4).
     */
    double Y = 1.0 - 1.0/x;
    double DX = log(x);
    double BY = 1.0 + Y*(4.+Y);
    double dl = 1.64493406684822643 + DX*(0.5*DX-log(x-1.)) 
      + (Y*(4.0+5.75*Y)-3.*(1.+Y)*DX/x)/BY;
    *result = do_sum(Y, BY, dl);
    return GSL_SUCCESS;
  }
  else if(x == 1.0) {
    /* DILOG COMPUTED FROM REF. NO. 1, P.244, EQ(2). */
    *result = 1.64493406684822643;
    return GSL_SUCCESS;
  }
  else if(x > 0.5) {
    /* DILOG COMPUTED FROM REF. NO. 1, P.244, EQ(7),
     * AND DESCRIPTION OF THIS ALGORITHM, EQ(4).
     */
    double Y = 1.0 - x;
    double DX = log(x);
    double BY = -1.0 - Y*(4.0+Y);
    double dl = 1.64493406684822643 
      - DX*log(Y) + (Y*(4.0+5.75*Y)+3.0*(1.0+Y)*DX*x)/BY;
    *result = do_sum(Y, BY, dl);
    return GSL_SUCCESS;
  }
  else if(x > 0.01) {
    /* DILOG COMPUTED FROM DESCRIPTION OF THIS ALGORITHM, EQ(4) */
    double Y = x;
    double BY = 1.0 + Y*(4.0+Y);
    double dl = (Y*(4.0+5.75*Y)+3.0*(1.0+Y)*(1.0-Y)*log(1.0-Y))/BY;
    *result = do_sum(Y, BY, dl);
    return GSL_SUCCESS;
  }
  else if(x < -1.0) {
    /* DILOG COMPUTED FROM REF. NO. 1, P.245, EQ(12) WITH
     * X=-X, AND DESCRIPTION OF THIS ALGORITHM, EQ(4).
     */
    double Y = 1.0/(1.0-x);
    double DX = log(-x);
    double DY = log(Y);
    double BY = 1.0 + Y*(4.0+Y);
    double dl = -1.64493406684822643 +
      0.5*DY*(DY+2.0*DX) + (Y*(4.0+5.75*Y) + 3.0*(1.0+Y)*(1.0-Y)*(DX+DY))/BY;
    if(dl + 4.0*Y == dl) {
      *result = dl;
      return GSL_SUCCESS;
    }
    else {
      *result = do_sum(Y, BY, dl);
      return GSL_SUCCESS;
    }
  }
  else if(x < -0.01) {
    /* DILOG COMPUTED FROM REF. NO. 1, P.244, EQ(8),
     * AND DESCRIPTION OF THIS ALGORITHM, EQ(4).
     */
    double Y = x/(x-1.0);
    double DX = log(1.0-x);
    double BY = -1.0 - Y*(4.0+Y);
    double dl = (Y*(4.0+5.75*Y)-3.0*(1.0+Y)*(1.0-Y)*DX)/BY - 0.5*DX*DX;
    *result = do_sum(Y, BY, dl);
    return GSL_SUCCESS;
  }
  else {
    /* DILOG COMPUTED FROM REF. NO. 1, P.244, EQ(1). */
    *result = x*(1.0+
	         x*(0.25+x*(1.0/9.0+
			    x*(0.0625+x*(4.0e-2+
				         x*(1.0/36.0+x*(1.0/49.0+x/64.0)))))));
    return GSL_SUCCESS; 
  }
}


/* Evaluate using a series representation for the
 * "correction" term:
 *    Re[Li2(z)] = Li2(|z|) - Sum[|z|^k / k^2 (1-cos(k arg(z))), {k,1,Inf}]
 *
 * This requires r < 1 and is slow to converge when r is near 1.
 * For large N, the accuracy is approximately Exp[-N ln(r)]/N^2,
 * for |N ln(r)| > 1. Therefore, the number of terms required is
 * roughly N ~ 10 / |ln r| for accuracy of Exp[-10]/N^2. So, in
 * practice this is not useful for 1-r < 10^(-2).
 *
 * Assumes r > 0. If arg(z) is zero, this will be adding
 * up lots of zeros, so better to trap that case outside.
 */
static
int
dilogc_series_1(double r, double cos_theta, double * result)
{
  int stat_dl;
  double dilog_abs;
  double alpha = 1.0 - cos_theta;
  double beta  = sin(acos(cos_theta));
  double ck  = cos_theta;
  double sk  = beta;
  double rk  = r;
  double sum = r*(1.0 - ck);
  int kmax = 100 + (int)(10.0/(-log(r)));
  int k;
  for(k=2; k<kmax; k++) {
    double ck_tmp = ck;
    ck = ck - (alpha*ck + beta*sk);
    sk = sk - (alpha*sk - beta*ck_tmp);
    rk *= r;
    sum += rk/((double)k*k) * (1.0 - ck);
  }
  
  /* Re(Li2(z)) = Li2(|z|) - sum */
  stat_dl = gsl_sf_dilog_impl(r, &dilog_abs);
  *result = dilog_abs - sum;
  return stat_dl;
}

/* E_2(i x)
 * used below
 */
static
double
E2_imag(double x)
{
  if(fabs(x) > 100.0) {
    return -1.0/x * (sin(x) - 2.0/x*cos(x));
  }
  else {
    double Si;
    gsl_sf_Si_impl(x, &Si);
    return cos(x) + x * (Si - 0.5*M_PI);
  }
}


/* Evaluate using a series which is small for r near 1.
 * This is easily derived from the above representation:
 *
 *  Li2(z) = Li2(|z|) - 1/4 a(2Pi-a) + Sum[(1-r^k)/k^2(1-cos(k a)), {k,1,Inf}]
 *
 * where a = argz(z), 0 <= a <= 2Pi.
 *
 * Assumes r < 1. If arg(z) is zero, this will be adding
 * up lots of zeros, so better to trap that case outside.
 */
static
int
dilogc_series_2(double r, double cos_theta, double *result)
{
  int stat_dl;
  double dilog_abs;
  double sum_end;
  double theta = acos(cos_theta);
  double alpha = 1.0 - cos_theta;
  double beta  = sin(theta);
  double ck  = cos_theta;
  double sk  = beta;
  double rk  = r;
  double sum = (1.0-r)*(1.0 - ck);
  int kmax = locMax( 10, (int)(10.0/(-log(r))) );
  int k;
  for(k=2; k<kmax; k++) {
    double ck_tmp = ck;
    ck = ck - (alpha*ck + beta*sk);
    sk = sk - (alpha*sk - beta*ck_tmp);
    rk *= r;
    sum += (1.0-rk)/((double)k*k) * (1.0 - ck);
  }

  /* Clean up the last part of the sum,
   * approximated by an integral.
   */
  sum_end = 1.0/kmax * (1.0 - E2_imag(kmax*theta));

  stat_dl = gsl_sf_dilog_impl(r, &dilog_abs);
  *result = dilog_abs - 0.25*theta*(2.0*M_PI - theta) + sum + sum_end;
  return stat_dl;  
}


int
gsl_sf_dilogc_impl(const double r, const double cos_theta, double * result)
{
  if(r < 0.0 || fabs(cos_theta) > 1.0) {
    *result = 0.0;
    return GSL_EDOM;
  }
  else if(cos_theta == 1.0) {
    return gsl_sf_dilog_impl(r, result);
  }
  else {
    /* Restrict to r in (0,1) using the standard functional relation.
     */
    double t = ( r > 1.0 ? 1.0/r : r);
    double tmp_result;
    int stat_tmp;
    
    if(t < 0.99) {
      stat_tmp = dilogc_series_1(t, cos_theta, &tmp_result);
    }
    else {
      stat_tmp = dilogc_series_2(t, cos_theta, &tmp_result);
    }

    /* Invert the standard functional relation if r was > 1.
     */
    if(r > 1.0) {
      double lgr = log(r);
      double arg = M_PI - acos(cos_theta);
      *result = -tmp_result - 0.5*lgr*lgr + 0.5*arg*arg - M_PI*M_PI/6.0;
    }
    else {
      *result = tmp_result;
    }
    
    return stat_tmp;
  }
}


int gsl_sf_dilog_e(const double x, double * result)
{
  int status = gsl_sf_dilog_impl(x, result);
  if(status != GSL_SUCCESS) {
    GSL_ERROR("gsl_sf_dilog_e", status);
  }
  return status;
}

int gsl_sf_dilogc_e(const double x, const double c, double * result)
{
  int status = gsl_sf_dilogc_impl(x, c, result);
  if(status != GSL_SUCCESS) {
    GSL_ERROR("gsl_sf_dilogc_e", status);
  }
  return status;
}


double gsl_sf_dilog(const double x)
{
  double y;
  int status = gsl_sf_dilog_impl(x, &y);
  if(status != GSL_SUCCESS) {
    GSL_WARNING("gsl_sf_dilog", status);
  }
  return y;
}

double gsl_sf_dilogc(const double x, const double c)
{
  double y;
  int status = gsl_sf_dilogc_impl(x, c, &y);
  if(status != GSL_SUCCESS) {
    GSL_WARNING("gsl_sf_dilogc", status);
  }
  return y;
}
