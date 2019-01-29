*
        integer    ladcgam          ! length of  characteristic
        parameter (ladcgam   = 11)  ! 
        integer    madcgam          ! maximum number of particles
        parameter (madcgam   = 50)
        integer    nadcgam          ! number of gammas
        real*4      adcgam(ladcgam,madcgam)
*
        real*4
     $ energy_adcgam(ladcgam,madcgam) ,! energy
     $      x_adcgam(ladcgam,madcgam) ,! global x-coord 
     $      y_adcgam(ladcgam,madcgam) ,! global y-coord 
     $      z_adcgam(ladcgam,madcgam) ,! global z-coord 
     $     xc_adcgam(ladcgam,madcgam) ,! center of gravity x-coord 
     $     yc_adcgam(ladcgam,madcgam) ,! center of gravity y-coord 
     $   chi2_adcgam(ladcgam,madcgam)  ! chi2 
        integer*4
     $   type_adcgam(ladcgam,madcgam) ,! type(hit place)
     $   dime_adcgam(ladcgam,madcgam) ,! number of counters in cluster
     $     id_adcgam(ladcgam,madcgam) ,! identification number
     $ status_adcgam(ladcgam,madcgam)  ! status (location of peak)
*
        equivalence
     $ (energy_adcgam, adcgam( 1,1) ) ,! r4
     $ (     x_adcgam, adcgam( 2,1) ) ,! r4
     $ (     y_adcgam, adcgam( 3,1) ) ,! r4
     $ (     z_adcgam, adcgam( 4,1) ) ,! r4
     $ (    xc_adcgam, adcgam( 5,1) ) ,! r4
     $ (    yc_adcgam, adcgam( 6,1) ) ,! r4
     $ (  chi2_adcgam, adcgam( 7,1) ) ,! r4
     $ (  type_adcgam, adcgam( 8,1) ) ,! i4
     $ (  dime_adcgam, adcgam( 9,1) ) ,! i4
     $ (    id_adcgam, adcgam(10,1) ) ,! i4
     $ (status_adcgam, adcgam(11,1) )  ! i4
*
        common /adcgam_cbk/ nadcgam, adcgam
*
        save   /adcgam_cbk/
*
