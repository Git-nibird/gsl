#include <config.h>
#include <gsl_errno.h>
#include <gsl_histogram.h>

int
gsl_histogram_increment (gsl_histogram * h, double x)
{
  int status = gsl_histogram_accumulate(h, x, 1.0) ;
  return status ;
}

int
gsl_histogram_accumulate (gsl_histogram * h, double x, double weight)
{
  const size_t n = h->nbins ;
  const double * range = h->range ;
  size_t index = 0 ;

  int status = gsl_histogram_find (n, range, x, &index) ;

  if (status) 
    {
      return status ;
    }

  if (index > n) 
    {
      GSL_ERROR("index lies outside valid range of 0 .. nbins - 1", 
		GSL_ESANITY) ;
    }

  h->bin[index] += weight ;
  
  return 0 ;
}

