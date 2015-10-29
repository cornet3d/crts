#include <cstdio>
#include   <sys/time.h>
#include   <sys/resource.h>
#include "fwrap.h"

using namespace std;


double    cputime(void)
{
    double   current_time;

#ifdef RUSAGE_SELF		
    struct rusage temp;
 
    getrusage(RUSAGE_SELF, &temp);

    current_time = (temp.ru_utime.tv_sec + temp.ru_stime.tv_sec +
            1.0e-6*(temp.ru_utime.tv_usec + temp.ru_stime.tv_usec));

#elif defined(CLOCKS_PER_SEC)
    current_time = clock()/((double) CLOCKS_PER_SEC);

#else
    fprintf(stderr,"\nERROR: Timing routines not available\n\n");
    current_time = 0.0;
#endif

    return (current_time);
}


double    FNAMEOF_fcpu_t(void)
{
    return cputime();
}

