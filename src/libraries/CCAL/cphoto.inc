      INTEGER
     &        MCOL,           ! Max number of col. in the calorimeter matrix
     &        MROW            ! Max number of rows in the calorimeter matrix
*
      PARAMETER (MCOL = 34 , MROW = 34)
*
      integer
     &        min_dime,       ! Min number of cells in cluster cut
     &        ncol, nrow,     ! number of col. and rows in (sub)detector matrix
     &        isect           ! hycal sector number
      real
     &        xsize, ysize,   ! Cell sizes
     &        mine, maxe,     ! Min and max cluster energy cut
     &        minm,           ! Min energy for the central (max. deposition) cluster cell cut
     &        zhycal          ! approximate distance target to hycal
*
      common /set_common/ xsize, ysize, mine, maxe, min_dime,
     &                    minm, ncol, nrow, zhycal, isect
*
