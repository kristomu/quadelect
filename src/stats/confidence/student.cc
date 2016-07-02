// Given the degree of confidence desired, this class produces z-values for 
// confidence intervals on the t-distribution with n degrees of freedom.

// Algorithms taken from 
// http://stuff.mit.edu/afs/sipb.mit.edu/project/seven/build/AFNI98/mri_stats.c,
// ultimately AS 63 appl. statist. (1973), vol.22, no.3, for the incomplete 
// beta function and AS 103 for the inverse.

#include "student.h"

using namespace std;

////

// Logarithm of the complete beta function.
double t_confidence_int::lnbeta(double p , double q) const {
	return (gamma(p) + gamma(q) - gamma(p+q));
}

double t_confidence_int::incbeta(double x, double p, double q, 
		double beta) const {
	
	const double ACU = 1e-15; // Desired accuracy.

	double psq, temp, rx;
	int ns;

	if( p <= 0.0 || q <= 0.0 ) return(-1.0);  /* error! */
	if( x <= 0.0 || x >= 1.0 ) return(x);

	/**  change tail if necessary and determine s **/

	psq = p+q ;
	if(p < psq*x )
		return(1 - incbeta(1-x, q, p, beta));

	double term = 1, ai = 1, betain = 1;

	ns     = q + (1-x)*psq;

	/** use soper's reduction formulae **/
	rx = x/(1-x);

	// Cleaned up a lot! -KM-

	while (fabs(term) > ACU || fabs(term) > ACU * betain) {
		if (ns >= 0) {
			temp = q-ai ;
			if(ns == 0) rx = x;
		} else
			temp = psq++;

		term    = term*temp*rx/(p+ai) ;
		betain += term;

		++ai;
		--ns;
	}

	return(betain * exp(p * log(x) + (q-1) * log(1-x) - beta) / p);
}


// Yuck. Make proper later.
double t_confidence_int::incbeta_inverse(double alpha, double p, double q, 
		double beta) const {

   double fpu , xinbta , a,r,y,t,s,h,w , acu ,
          yprev,prev,sq , g,adj,tx,xin ;

   tx = 0;
   fpu = 1e-15;

   if(p <= 0.0 || q <= 0.0 || alpha < 0.0 || alpha > 1.0) return (-1.0);
   if(alpha == 0 || alpha == 1) return(alpha);

   /** change tail if necessary **/

   if( alpha > 0.5)
	   return(1 - incbeta_inverse(1-alpha, q, p, beta));

   a    = alpha ;

   /** calculate the initial approximation **/

   r = sqrt(-log(a*a));
   y = r - (2.30753 + 0.27061*r) / (1.0+(0.99229+0.04481*r)*r) ;
   if(p > 1 && q > 1) {
	r = (y*y-3)/6.0 ;
	s = 1.0/(p+p-1) ;
	t = 1.0/(q+q-1) ;
	h = 2.0/(s+t) ;
	w = y*sqrt(h+r)/h-(t-s)*(r+5.0/6.0-2.0/(3.0*h)) ;
	xinbta = p/(p+q*exp(w+w)) ;
   } else {
	r = q+q ;
	t = 1.0/(9.0*q) ;
	t = r * pow( (1.0-t+y*sqrt(t)) , 3.0 ) ;
	if( t > 0.0 ) {
		t = (4.0*p+r-2.0)/t ;
		if( t <= 1.0 )
			xinbta = exp((log(a*p)+beta)/p);
		else
			xinbta = 1.0-2.0/(t+1.0);
	} else 
		xinbta = 1.0-exp((log((1.0-a)*q)+beta)/q);
   }

     /** solve for x by a modified newton-raphson method **/

    r     = 1.0-p ;
    t     = 1.0-q ;
    yprev = 0.0 ;
    sq    = 1.0 ;
    prev  = 1.0 ;
    if(xinbta < 0.0001) xinbta = 0.0001 ;
    if(xinbta > 0.9999) xinbta = 0.9999 ;

    acu = fpu;
    double ft_tolerance;

do {
      y = incbeta( xinbta , p,q,beta ) ;
      if( y < 0.0 ) return -1.0 ;
      xin = xinbta ;
      y = (y-a)*exp(beta+r*log(xin)+t*log(1.0-xin)) ;
      if(y*yprev <= 0.0) prev = max(sq, fpu) ;
      g = 1.0 ;

lab9:
      adj = g*y ;
      sq = adj*adj ;
      if(sq < prev)
	      tx = xinbta-adj ;
      if(tx >= 0.0 && tx <= 1.0) goto lab11 ;

lab10:
      g = g/3.0 ; goto lab9 ;

lab11:
      if(tx == 0.0  || tx == 1.0 ) goto lab10 ;
      ft_tolerance = fabs(xinbta-tx);

      xinbta = tx ;
      yprev = y ;
} while (prev > acu && y*y > acu && ft_tolerance > fpu);

      return xinbta ;
}


double t_confidence_int::student_p2t(double pp, double dof) const {
   double bb , binv , tt ;

   if( pp <= 0.0 || pp >= 0.9999999 || dof < 1.0 ) return 0.0 ;

   bb   = lnbeta( 0.5*dof , 0.5 ) ;
   binv = incbeta_inverse( pp, 0.5*dof , 0.5 , bb ) ;
   tt   = sqrt( dof*(1.0/binv-1.0) ) ;
   return tt ;
}

// And now for something much less hackish.

double t_confidence_int::z_score(double uncertainty, double dof) const {
	if (uncertainty < 0 || uncertainty > 1) return(NAN);
	if (dof <= 0) return(NAN);

	return(student_p2t(uncertainty, dof));
}

double t_confidence_int::interval(double mean, double variance, int num_samples,
		double uncertainty) const {

	bool debug = false;

	if (debug) {
		cout << "Desired uncertainty: " << uncertainty << endl;
		cout << "DOF: " << num_samples - 1 << endl;
	}

	// First determine the multiple of sigma for the interval.
	double zscore = z_score(uncertainty, num_samples-1);

	if (debug)
		cout << "Z: " << zscore << endl;

	// Were there bad parameters? If so, give up.
	if (!finite(zscore))
		return(zscore);

	if (debug)
		cout << "Variance: " << variance << endl;

	// Now we have (sample mean) +/- z * sqrt(variance / num_samples).

	return(zscore * sqrt(variance / num_samples));
}

/*main() {

	t_confidence_int test;

	cout << test.z_score_cache(0.05, 5.3) << endl;
	cout << test.z_score_cache(0.45, 5.3) << endl;
	cout << test.z_score_cache(0.95, 5.3) << endl;

	for (double c = 0; c < 1; c += 0.1) 
		cout << test.z_score_cache(c, 5) << " for " << c << endl;
}*/
