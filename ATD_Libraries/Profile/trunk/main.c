/* Little sample file to demonstrate the profiler.*/

#include <io.h>
//#define DONT_PROFILE
#include "profile.h"

/* Sample: time some calculations. */
int main(int argc, char *argv[]) {
  int i, i_res, i_many = (1 << 24) ;
  /* Redirect stdout to a logfile.*/
  FILE* fp = fopen ("log.txt", "wa") ;
  _dup2 (_fileno (fp), _fileno (stdout)) ;
  /* Time additions */
  PROFILE_START ("Adding") ;
  for (i = 1; i < i_many; ++i) {
    i_res = i + i ;
  }
  PROFILE_STOP ("Adding") ;
  /* Time subtractions */
  PROFILE_START ("Subtracting") ;
  PROFILE_START ("Subtracting") ;
  for (i = 1; i < i_many; ++i) {
    i_res = i - i ;
  }
  PROFILE_STOP ("Subtracting") ;
  /* Print first results */
  PROFILE_PRINT () ;
  /* Reset profiler */
  PROFILE_RESET () ;
  /* Time divisions */
  PROFILE_START ("Dividing") ;
  for (i = 1; i < i_many; ++i) {
    i_res = i / i ;
  }
  PROFILE_STOP ("Dividing") ;
  /* Time subtractions */
  PROFILE_START ("Multipling") ;
  for (i = 1; i < i_many; ++i) {
    i_res = i * i ;
  }
  PROFILE_STOP ("Multipling") ;
  PROFILE_PRINT () ;
  /* End */
  fclose (fp) ;
  return 1 ;
}
