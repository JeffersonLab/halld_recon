*
#define MAX_CC 60
#define test_p
*
      subroutine init_island(in_filename, name_length)
*
      implicit none
      integer name_length
	  character in_filename(1000)
	  character(len=1000) filename
      real acell, ad2c
      integer i1, i2, id1, id2
      real fcell_hyc, fd2c
*
      common/profile_com/ acell(0:1,0:500,0:500), ad2c(0:1,0:500,0:500)
      logical profile_init_status
      common/profile_init/ profile_init_status
      data profile_init_status /.false./
*
	  do i1 = 1, name_length
		filename(i1:i1) = in_filename(i1)
	  enddo 
*
*	  write(*,*) 'DEBUG: opening ',filename
*
      open(22,file=filename,
     &          form='formatted', status='old')
	  do i1 = 0, 500
		do i2 = 0, i1
          read(22,'(i3,1x,i3,2(1x,e20.10))')
     &          id1, id2, fcell_hyc, fd2c
          acell(0,i1,i2) = fcell_hyc
		  acell(0,i2,i1) = fcell_hyc
		  ad2c(0,i2,i1) = fd2c
          ad2c(0,i1,i2) = fd2c
*          write(*,*) fcell_hyc,fd2c
        enddo
      enddo
      close(22)
*
      profile_init_status=.true.
      return
      end
/*
      subroutine init_island(new_acell, new_ad2c)
      implicit none
      real  new_acell(0:1,0:500,0:500)
	  real  new_ad2c(0:1,0:500,0:500)
      real acell, ad2c
      integer i1, i2
*
      common/profile_com/ acell(0:1,0:500,0:500), ad2c(0:1,0:500,0:500)
      logical profile_init_status
      common/profile_init/ profile_init_status
      data profile_init_status /.false./
 
        do i1 = 0, 500
        do i2 = 0, i1
c          acell(0,i1,i2) = new_acell(0,i1,i2)
c          acell(0,i2,i1) = new_acell(0,i2,i1)
c           ad2c(0,i2,i1) = new_ad2c(0,i2,i1)
c           ad2c(0,i1,i2) = new_ad2c(0,i1,i2)
          acell(0,i1,i2) = new_acell(i2,i1,0)
          acell(0,i2,i1) = new_acell(i1,i2,0)
           ad2c(0,i2,i1) = new_ad2c(i1,i2,0)
           ad2c(0,i1,i2) = new_ad2c(i2,i1,0)
        enddo
        enddo
*        
	  profile_init_status=.true.
	  return
	  end
*/
*
      subroutine main_island
      implicit none
#include "cphoto.inc"
#include "adcgam_bk.inc"
*
      integer i, j, icl, ipncl
      real ecl
      integer 
     &        nw            ,! number of hits in subdetector       INPUT
     &        ia(10800)     ,! adresses of counters          input/OUTPUT
     &        id(10800)     ,! energy in counter             input/OUTPUT
     &        ncl           ,! number of clusters                  OUTPUT
     &        lencl(200)     ! length of a cluster                 OUTPUT
*      real    tt(10800)      ! time in counter               input/OUTPUT
*
      real acell, ad2c
      common/profile_com/ acell(0:1,0:500,0:500), ad2c(0:1,0:500,0:500)
*
      integer i1, i2, id1, id2
      real fcell_hyc, fd2c
      logical first
      data first /.true./
      save first
      logical profile_init_status
      common/profile_init/ profile_init_status
*
      if(first) then
        first = .false.
*
	  if(profile_init_status.eqv..false.) then
	  	write(*,*) 'WARNING: CCAL shower profile data not loaded!'
	  endif
/*
        open(22,file='prof_pwo.dat',
     &          form='formatted', status='old')
        do i1 = 0, 500
        do i2 = 0, i1
          read(22,'(i3,1x,i3,2(1x,e20.10))')
     &      id1, id2, fcell_hyc, fd2c
          acell(0,i1,i2) = fcell_hyc
          acell(0,i2,i1) = fcell_hyc
           ad2c(0,i2,i1) = fd2c
           ad2c(0,i1,i2) = fd2c
        enddo
        enddo
        close(22)
*/
/*
        open(22,file='prof_lg.dat',
     &          form='formatted', status='old')
        do i1 = 0, 500
        do i2 = 0, i1
          read(22,'(i3,1x,i3,2(1x,e20.10))')
     &      id1, id2, fcell_hyc, fd2c
          acell(1,i1,i2) = fcell_hyc
          acell(1,i2,i1) = fcell_hyc
           ad2c(1,i2,i1) = fd2c
           ad2c(1,i1,i2) = fd2c
        enddo
        enddo
        close(22)
*/
      endif
*
      nadcgam = 0
*
      call data_hyc(nw,ia,id)             ! Prepare data in dimensionless form
c
      if(nw.gt.0) then                       ! if Photon has information 
c
        call clus_hyc(nw,ia,id,ncl,lencl)    ! Clusters search
c
        if(ncl.gt.0) then                    ! if cluster exist
           ipncl=1                           ! first cluster
           do icl=1,ncl                      ! loop over clusters
             ecl=0.            
             do i=1,lencl(icl)               ! loop over counters in a cluster
               ecl=ecl+id(ipncl+i-1)         ! energy of a cluster
             enddo                           ! loop over counters in a cluster
c             if(ecl/100..ge.min_energy) then ! if the energy is big enough
c
               call gams_hyc(lencl(icl),ia(ipncl),id(ipncl)) ! cluster
                                                                       ! processing
c
               if(nadcgam.gt.madcgam) then   ! if the number of gammas too big
                 nadcgam = madcgam
                 goto 99
               endif                         ! if the number of gammas too big
c             endif                          ! if the energy is big enough
              ipncl=ipncl+lencl(icl)         ! adress of the next cluster
           enddo                             ! loop over clusters
        endif                                ! if cluster exist
      endif                                  ! if Photon has information
*
99    continue
      call out_hyc
*
*      call dump_hyc
*
      return
      end
*
      subroutine data_hyc(nw,ia,id)
      implicit none
#include "cphoto.inc"
      integer 
     &          nw            ,! number of hits in subdetector        OUTPUT
     &          ia(*)         ,! adresses of counters id = nx*100+ny  OUTPUT
     &          id(*)          ! energy in counter ( Pht e- )         OUTPUT
*      real      tt(*)          ! time in counter (ns)                 OUTPUT
*
*             ---------------Include files---------------
*
*
*               -------------Local declarations------------
*
      integer  ech             ! hit array                            INPUT [0.1MeV]
      integer  i, j, ie
      common/ech_common/ ech(MCOL,MROW)
*      real     tch             ! hit time array
*      common/tch_common/ tch(MCOL,MROW)
*
      nw = 0                   ! init output
*
      do i = 1, NCOL
        do j = 1, NROW
          ie = ech(i,j)
          if(ie.gt.0)then
            nw     = nw + 1             ! one more hit
            ia(nw) = 100*i+j            ! code address
            id(nw) = ie                 ! energy in 10 Mev
*            tt(nw) = tch(i,j)
          endif
        enddo
      enddo
*
      return
      end
*
* ---------------------------------------------
*
      subroutine clus_hyc(nw,ia,id,ncl,lencl)
      implicit none
*
*             -----------Argument declarations-----------
*
      integer 
     &        nw            ,! number of hits in subdetector       INPUT
     &        ia(*)         ,! adresses of counters          input/OUTPUT
     &        id(*)         ,! energy in counter             input/OUTPUT
     &        ncl           ,! number of clusters                  OUTPUT
     &        lencl(*)       ! length of a cluster                 OUTPUT
*      real    tt(*)          ! time in counter               input/OUTPUT
*
*               -------------Local declarations------------
*
      integer 
     &        i,j,k,icl     ,! dummy loop index     
     &        ib,ie         ,!
     &        ias,iaf,iak   ,!
     &        last,lastcl   ,!
     &        next,leng     ,!
     &        iwork(10800)    ,! working array
     &        idwork(10800)   ,! help me
     &        maxcl          ! Maximum number of clusters 
*      real    twork(10800)   ! working array
*
*               --------------Data statements--------------
*
      data 
     &        maxcl /200/      
*
*               --------------Event Analysis Code-------------
*
      ncl=0
      if(nw.lt.1) return
      ncl=1
      lencl(1)=1
      if(nw.lt.2) return          ! only one hit
c
      call order_hyc(nw,ia,id) ! the addresses must be in increasing order.
c
      ncl=0                       ! zero output
      next=1
      do 10 k=2,nw+1
        if(k.le.nw) then
          iak=ia(k)
        endif
c
        if(iak-ia(k-1).le.1.and.k.le.nw) go to 10
c
        ib=next                   ! first word of the (sub)cluster
        ie=k-1                    ! last word of the (sub)cluster
        next=k                    ! first word of the next (sub)cluster
        if(ncl.ge.maxcl)   return
        ncl=ncl+1
        lencl(ncl)=next-ib        ! length of the (sub)cluster
c
        if(ncl.eq.1)              go to 10
c
c     a job to glue the subclusters
c
        ias=ia(ib)
        iaf=ia(ie)
        last=ib-1
        lastcl=ncl-1
        do 1 icl=lastcl,1,-1
          leng=lencl(icl)
          if(ias-ia(last).gt.100) go to 10  ! no subclusters to be glued
          do i=last,last-leng+1,-1
            if(ias-ia(i).gt.100)  go to 1
            if(iaf-ia(i).ge.100)  then      ! subclusters to be glued
              if(icl.lt.ncl-1.and.leng.le.10800) then
                call ucopy(ia(last+1-leng),iwork(1),leng)
                call ucopy21(ia(last+1),ia(last+1-leng),(ib-1-last))
                call ucopy(iwork(1),ia(ib-leng),leng)
                call ucopy(id(last+1-leng),iwork(1),leng)
                call ucopy21(id(last+1),id(last+1-leng),(ib-1-last))
                call ucopy(iwork(1),id(ib-leng),leng)
*                call fcopy(tt(last+1-leng),twork(1),leng)
*                call fcopy21(tt(last+1),tt(last+1-leng),(ib-1-last))
*                call fcopy(twork(1),tt(ib-leng),leng)
                do j=icl,ncl-2
                  lencl(j)=lencl(j+1)
                enddo
              endif
              ib=ib-leng
              lencl(ncl-1)=lencl(ncl)+leng
              ncl=ncl-1
              go to 1
            endif
          enddo
    1   last=last-leng            ! last word of tested subcluster
   10 continue
c
      return
      end
*
* ---------------------------------------------
*
      subroutine order_hyc(nw,ia,id)      
      implicit none
*
*             -----------Argument declarations-----------
*
      integer 
     &          nw            ,! number of hits in subdetector       
     &          ia(*)         ,! first  INPUT/OUTPUT array
     &          id(*)          ! second INPUT/OUTPUT array
*      real      tt(*)          ! time in counter               input/OUTPUT
*
*             ---------------Include files---------------
*
*               -------------Local declarations------------
*
      logical 
     &       alarm
      integer 
     &       i,k,
     &       iat,idt
*      real   ttt
*
*               --------------Event Analysis Code-------------
*
      alarm=.false.
c
      go to 1
c
      entry       chord(nw,ia,id,*)   ! only to check the right order
c
      alarm=.true.
    1 if(nw.lt.2)  return             ! only one hit
      do 2 k=2,nw                     ! loop over hits
      if(ia(k).le.ia(k-1)) then
        if(alarm) return 1
        iat=ia(k)
        idt=id(k)
*        ttt=tt(k)
        do i=k-1,0,-1
          if(i.ge.1) then
            if(iat.lt.ia(i)) then
              ia(i+1)=ia(i)
              id(i+1)=id(i)
*              tt(i+1)=tt(i)
            else
              ia(i+1)=iat
              id(i+1)=idt
*              tt(i+1)=ttt
              go to 2
            endif
          else
            ia(1)=iat
            id(1)=idt
*            tt(1)=ttt
          endif
        enddo                         ! loop over hits
      endif
    2 continue
c
      return
      end
*
* ---------------------------------------------
*
      subroutine gams_hyc(nadc,ia,id)
      implicit none
*
*             -----------Argument declarations-----------
*                                            input
      integer  
     &       nadc             ,! number of counters in the cluster
     &       ia(nadc)         ,! adresses of counters
     &       id(nadc)          ! energy of counters
*      real   tt(nadc)          ! time of counters
*
*             ---------------include files---------------
*
*
#include "adcgam_bk.inc"
#include "cphoto.inc"
*
      integer icl_index, icl_iener      ! list of cluster elements
      common /icl_common/ icl_index(200,MAX_CC), icl_iener(200,MAX_CC)
*
*               -------------local declarations------------
*
      integer 
     &       ngam0            ,! number of gammas found before
     &       niter            ,! max number of iterations (6)
     &       npk              ,! number of peaks in the cluster               
     &       ipnpk(10)        ,! counter number with max energy
                               ! in a peak              
     &       igmpk(2,10)      ,!
     &       minpk            ,! min energy of a counter in a cluster  
     &       idelta           ,!
     &       itype            ,! type of peak
     &       leng             ,!
     &       ixypk,ixpk,iypk  ,!
     &       ic,iac,in        ,!               
     &       ixy,ixymax,ixymin,!
     &       iy,ix,iyc        ,!
     &       iwk              ,!
     &       ie,iia           ,!
     &       i,ig,ipk,iter    ,!
     &       iwrk(10800,0:12)  ! working array for resolved peaks
*
      integer
     &       idp(nadc,0:12)   ,! energy of each cell of the island belonging to the certain peaks
     &       ide,
     &       idecorr
      real
     &       fw(12)
*
      real
     &          chisq         ,! current value of chi2
     &          chisq1        ,! value of chi2 for preliminary gammas seperation
     &          chisq2        ,! value of chi2 for final gammas seperation
     &          ratio         ,!
     &          eg            ,!
     &          epk(10)       ,!
     &          xpk(10)       ,!
     &          ypk(10)       ,!
     &          a,dx,dy       ,!
     &          cell_hyc      ,!
     &          e1,x1,y1      ,!
     &          e2,x2,y2      ,!
     &          fwrk(10800,0:12)    ,! working array for resolved peaks
     &          fe, fia
*
      integer j, idsum
*
*               -----------external  declarations----------
*
*               --------------data statements--------------
*
cc    data minpk/30/           ! min cell energy to be good for the 2nd peak if it is a case
cc    data minpk/5/            ! min cell energy to be good for the 2nd peak if it is a case
      data idelta/0/           ! min cell energy part to be a member of separated cluster
      data chisq1/90.0/        ! /3.0/
      data chisq2/50.0/        ! /0.8/
      data niter/6/
      save chisq1, chisq2, niter, idelta
c
      call order_hyc(nadc,ia,id)
c
      ngam0=nadcgam
c
c  peaks search :
c
* li
      idsum = 0
      do ic = 1, nadc
        idsum = idsum + id(ic)
      enddo
      if(nadc.lt.3) then
        minpk = 1
      else
cc      minpk = max(1,nint(0.16+0.06*idsum))
        if(isect.ne.0) then
          minpk = max(1,nint(20.*log(1.+0.0001*idsum)))
        else
          minpk = max(1,nint( 7.*log(1.+0.0001*idsum)))
        endif
      endif
      minpk = minpk*100
* li
      npk=0
      do 1 ic=1,nadc
        iac=id(ic)
        if(iac.lt.minpk)       go to 1
        ixy=ia(ic)
        ixymax=ixy+100+1
        ixymin=ixy-100-1
        iyc=ixy-ixy/100*100
        in=ic+1
        do while (in.le.nadc.and.ia(in).le.ixymax)
          iy=ia(in)-ia(in)/100*100
          if(iabs(iy-iyc).le.1.and.id(in).ge.iac) go to 1
          in=in+1
        enddo
        in=ic-1
        do while (in.ge.1.and.ia(in).ge.ixymin)
          iy=ia(in)-ia(in)/100*100
          if(iabs(iy-iyc).le.1.and.id(in).gt.iac) go to 1
          in=in-1
        enddo
        npk=npk+1           !  peak found
        ipnpk(npk)=ic
        if(npk.eq.10.or.npk.ge.10000/nadc-3) go to 10
    1 continue
      if(npk.eq.0)             return
c
c   gamma search for one peak :
c
   10 if(npk.gt.1)             go to 100
      if(nadcgam.ge.madcgam-1)           return
c
      nadcgam=nadcgam+1
      chisq=chisq2
c
      ic=ipnpk(1)
      ix=ia(ic)/100
      iy=ia(ic)-ix*100
c
      call peak_type(ix,iy,itype)
c
      e2=0.
      call gamma_hyc(itype,nadc,ia,id,chisq,
     &               e1,x1,y1,
     &               e2,x2,y2)
        chi2_adcgam(1,nadcgam)=chisq          !  remember chi2 of the cluster
        type_adcgam(1,nadcgam)=itype          !  remember type of the cluster
      energy_adcgam(1,nadcgam)=e1
           x_adcgam(1,nadcgam)=x1
           y_adcgam(1,nadcgam)=y1
*        time_adcgam(1,nadcgam)=t1
        dime_adcgam(1,nadcgam)=nadc
          id_adcgam(1,nadcgam)=0
      status_adcgam(1,nadcgam)=itype
c
      if(e2.gt.0..and.nadcgam.le.madcgam-1) then    ! two gamma found
        nadcgam=nadcgam+1
          chi2_adcgam(1,nadcgam)=chisq        !  remember chi2 of the cluster
          type_adcgam(1,nadcgam)=itype+10     !  remember type of the cluster
          type_adcgam(1,nadcgam-1)=itype+10   !  remember type of the cluster
        energy_adcgam(1,nadcgam)=e2
             x_adcgam(1,nadcgam)=x2
             y_adcgam(1,nadcgam)=y2
*          time_adcgam(1,nadcgam)=t2
          dime_adcgam(1,nadcgam)=nadc
            id_adcgam(1,nadcgam)=2
            id_adcgam(1,nadcgam-1)=1
            xc_adcgam(1,nadcgam)=0.5*(x2-x1)
            yc_adcgam(1,nadcgam)=0.5*(y2-y1)
            xc_adcgam(1,nadcgam-1)=0.5*(x1-x2)
            yc_adcgam(1,nadcgam-1)=0.5*(y1-y2)
        status_adcgam(1,nadcgam)=itype
        do j = 1, nadc
          if(j.le.MAX_CC) then
            icl_index(nadcgam,j)   = ia(j)
            icl_index(nadcgam-1,j) = ia(j)
            icl_iener(nadcgam,j)   = nint(id(j)*e2/(e1+e2))
            icl_iener(nadcgam-1,j) = nint(id(j)*e1/(e1+e2))
          endif
        enddo
      else
        do j = 1, nadc
          if(j.le.MAX_CC) then
            icl_index(nadcgam,j) = ia(j)
            icl_iener(nadcgam,j) = id(j)
          endif
        enddo
      endif
c
      return               ! end of processing one-peak cluster  
c
c   gamma search for several peaks.  first step. (1 gamma in each peak)
c
c   preliminary estimation of (E,x,y) of peaks, split them in two hits
c   only if it is badly needed (chi2 improvement is too high)
c   if this split will occur: it is only for better (E,x,y) estimation
c   it will be rejoined and reanalyzed in the second step (li comment)
c
  100 if(nadcgam.ge.madcgam-1)           return
c
      ratio = 1.             ! start separation of peaks by an iterative
      do iter=1,niter        ! procedure
        do i=1,nadc
          iwrk(i,0)=0
          fwrk(i,0)=0.
        enddo
c
        do ipk=1,npk
          ic=ipnpk(ipk)
c         if(iter.ne.1)  ratio=float(iwrk(ic,ipk))/iwrk(ic,npk+1)
          if(iter.ne.1)  ratio=fwrk(ic,ipk)/fwrk(ic,npk+1)
          eg=id(ic)*ratio
          ixypk=ia(ic)
          ixpk=ixypk/100
          iypk=ixypk-ixpk*100
          epk(ipk)=eg
          xpk(ipk)=eg*ixpk
          ypk(ipk)=eg*iypk
          if(ic.ge.nadc)           go to 120
          do in=ic+1,nadc
            ixy=ia(in)
            ix=ixy/100
            iy=ixy-ix*100
            if(ixy-ixypk.gt.100+1) go to 120
            if(iabs(iy-iypk).le.1) then
c             if(iter.ne.1)  ratio=float(iwrk(in,ipk))/iwrk(in,npk+1)
              if(iter.ne.1)  ratio=fwrk(in,ipk)/fwrk(in,npk+1)
              eg=id(in)*ratio
              epk(ipk)=epk(ipk)+eg
              xpk(ipk)=xpk(ipk)+eg*ix
              ypk(ipk)=ypk(ipk)+eg*iy
            endif
          enddo
  120     if(ic.lt.2)              go to 140
          do in=ic-1,1,-1
            ixy=ia(in)
            ix=ixy/100
            iy=ixy-ix*100
            if(ixypk-ixy.gt.100+1) go to 140
            if(iabs(iy-iypk).le.1) then
c             if(iter.ne.1)  ratio=float(iwrk(in,ipk))/iwrk(in,npk+1)
              if(iter.ne.1)  ratio=fwrk(in,ipk)/fwrk(in,npk+1)
              eg=id(in)*ratio
              epk(ipk)=epk(ipk)+eg
              xpk(ipk)=xpk(ipk)+eg*ix
              ypk(ipk)=ypk(ipk)+eg*iy
            endif
          enddo
  140     continue
* li
          if(epk(ipk).gt.0.) then
            xpk(ipk)=xpk(ipk)/epk(ipk)
            ypk(ipk)=ypk(ipk)/epk(ipk)
*          else
* TOO MANY WARNINGS, be smarter about how many one prints out
*            write(*,'(a,f6.2,a,i2)')
*     &       'WRN: lost maximum with peak energy ', id(ic)*1.e-4,
*     &       ' at ITER = ', iter
          endif
* li
          do i=1,nadc
            ixy=ia(i)
            ix=ixy/100
            iy=ixy-ix*100
            dx=abs(ix-xpk(ipk))
            dy=abs(iy-ypk(ipk))
*
            a=epk(ipk)*cell_hyc(dx,dy)
*
            iwrk(i,ipk)=nint(a)
            fwrk(i,ipk)=a
            iwrk(i,0)=iwrk(i,0)+iwrk(i,ipk)
            fwrk(i,0)=fwrk(i,0)+fwrk(i,ipk)
          enddo
        enddo
        do i=1,nadc
          iwk=iwrk(i,0)
          if(iwk.le.0) iwk=1
          iwrk(i,npk+1)=iwk
*
cc
          if(fwrk(i,0).gt.1.e-2) then
            fwrk(i,npk+1)=fwrk(i,0)
          else
            fwrk(i,npk+1)=1.e-2
          endif
        enddo
      enddo       ! end of iteration to separate peaks in a cluster
c
      do 200 ipk=1,npk
        leng=0
        do i=1,nadc
cc        if(iwrk(i,0).gt.0) then
          if(fwrk(i,0).gt.1.e-2) then
            ixy=ia(i)
            ie=id(i)*float(iwrk(i,ipk))/iwrk(i,0)+.5  ! part of cell energy #i belonging to peak #ipk
            fe=id(i)*fwrk(i,ipk)/fwrk(i,0)            ! part of cell energy #i belonging to peak #ipk
            if(fe.gt.idelta) then
              leng=leng+1
              iwrk(leng,npk+1)=ixy
cc            iwrk(leng,npk+2)=ie                     ! part of cell energy #i belonging to peak #ipk
              iwrk(leng,npk+2)=nint(fe)               ! part of cell energy #i belonging to peak #ipk
              fwrk(leng,npk+1)=ixy
              fwrk(leng,npk+2)=fe                     ! part of cell energy #i belonging to peak #ipk
            endif
          endif
        enddo
        if(nadcgam.ge.madcgam-1)         go to 500
        igmpk(2,ipk)=0
        if(leng.eq.0)                    go to 200
        nadcgam=nadcgam+1
        chisq=chisq1
c
        ic=ipnpk(ipk)
        ix=ia(ic)/100
        iy=ia(ic)-ix*100
c
        call peak_type(ix,iy,itype)
c
        e2=0.
        call gamma_hyc(itype,leng,iwrk(1,npk+1),iwrk(1,npk+2),
     &                 chisq,e1,x1,y1,
     &                       e2,x2,y2)
        chi2_adcgam(1,nadcgam)=chisq      !  remember chi2 of the cluster
        type_adcgam(1,nadcgam)=itype      !  remember type of the cluster
        energy_adcgam(1,nadcgam)=e1
        x_adcgam(1,nadcgam)=x1
        y_adcgam(1,nadcgam)=y1
*         time_adcgam(1,nadcgam)=t1
*       id_adcgam  (1,nadcgam)=90
        dime_adcgam  (1,nadcgam)=leng
        status_adcgam(1,nadcgam)=itype
        igmpk(1,ipk)=nadcgam
        igmpk(2,ipk)=nadcgam
        if(e2.gt.0..and.nadcgam.le.madcgam-1) then    ! two gamma found
          nadcgam=nadcgam+1
          chi2_adcgam(1,nadcgam)=chisq      !  remember chi2 of the cluster
          type_adcgam(1,nadcgam)=itype      !  remember type of the cluster
          energy_adcgam(1,nadcgam)=e2
          x_adcgam(1,nadcgam)=x2
          y_adcgam(1,nadcgam)=y2
*        time_adcgam(1,nadcgam)=t2
         xc_adcgam(1,nadcgam)=0.5*(x2-x1)
         yc_adcgam(1,nadcgam)=0.5*(y2-y1)
         xc_adcgam(1,nadcgam-1)=0.5*(x1-x2)
         yc_adcgam(1,nadcgam-1)=0.5*(y1-y2)
          id_adcgam  (1,nadcgam)=92
          id_adcgam  (1,nadcgam-1)=91
          dime_adcgam  (1,nadcgam)=leng
         status_adcgam(1,nadcgam)=itype
          igmpk(2,ipk)=nadcgam
        endif
  200 continue
c
c   this is the second step :   ( 1 or 2 gamma in each peak )
c   (e,x,y) of hits were prelimary estimated in the first step
c
      do i=1,nadc
        iwrk(i,0)=0
        fwrk(i,0)=0.
        idp(i,0) =0
      enddo
*
      do 230 ipk=1,npk
      do 231 i=1,nadc
        iwrk(i,ipk)=0
        fwrk(i,ipk)=0.
        idp(i,ipk) =0
c vfk
        if(igmpk(2,ipk).eq.0) go to 231
c vfk
        do ig=igmpk(1,ipk),igmpk(2,ipk)
          ixy=ia(i)
          ix=ixy/100
          iy=ixy-ix*100
          dx=ix-x_adcgam(1,ig)
          dy=iy-y_adcgam(1,ig)
c
          fia=energy_adcgam(1,ig)*cell_hyc(dx,dy)
          iia=nint(fia)
c
          iwrk(i,ipk)=iwrk(i,ipk)+iia   ! part of gamma #ig energy belonging to cell #i from peak #ipk
          fwrk(i,ipk)=fwrk(i,ipk)+fia   ! part of gamma #ig energy belonging to cell #i from peak #ipk
          idp(i,ipk)=idp(i,ipk)+iia
          iwrk(i,0)=iwrk(i,0)+iia
          fwrk(i,0)=fwrk(i,0)+fia
        enddo
  231   continue 
  230 continue
*
* il, recover working array, renormalize total sum to the original cell energy
*
      do i=1, nadc
        idp(i,0)=0
        do ipk=1,npk
          idp(i,0)=idp(i,0)+idp(i,ipk)
        enddo
*
        ide = id(i)-idp(i,0)
        if(ide.eq.0) goto 234
        if(fwrk(i,0).eq.0.) goto 234
*
        do ipk=1,npk
          fw(ipk) = fwrk(i,ipk)/fwrk(i,0)
        enddo
        idecorr = 0
        do ipk=1,npk
          fia = ide*fw(ipk)
          if(fwrk(i,ipk)+fia.gt.0.) then
            fwrk(i,ipk)=fwrk(i,ipk)+fia
            fwrk(i,0)  =fwrk(i,0)  +fia
          endif
          iia = nint(fia)
          if(iwrk(i,ipk)+iia.gt.0.) then
            iwrk(i,ipk)=iwrk(i,ipk)+iia
            iwrk(i,0)  =iwrk(i,0)  +iia
            idecorr    = idecorr + iia
          else if(iwrk(i,ipk)+iia.lt.0.) then
            print*, 'WARNING NEGATIVE CORR = ',
     &      ia(i), id(i)
          endif
        enddo
*
cc      if(iabs(ide-idecorr).gt.5) then
cc          print*, 'WARNING RENORM. CORR MISMATCH ',
cc     &      ia(i), id(i), ide, idecorr
cc      endif
234     continue
      enddo
*
* il
*
      nadcgam=ngam0                               ! reanalize last (two) gamma(s)
      do 300 ipk=1,npk
        leng=0
        do i=1,nadc
          if(iwrk(i,0).gt.0) then
cc          ie=nint(id(i)*float(iwrk(i,ipk))/iwrk(i,0))
            fe=id(i)*fwrk(i,ipk)/fwrk(i,0)
cc          if(ie.gt.idelta) then
            if(fe.gt.idelta) then
              leng=leng+1
              iwrk(leng,npk+1)=ia(i)
              fwrk(leng,npk+1)=ia(i)
cc            iwrk(leng,npk+2)=ie
              iwrk(leng,npk+2)=nint(fe)
              fwrk(leng,npk+2)=fe
            endif
          else
cc          if(id(i).gt.5) print*, 'drop ', ia(i), id(i)
          endif
        enddo
        if(nadcgam.ge.madcgam-1) then
          print*, 'gams_pht debug printout, goto line 500 has met'
          go to 500
        endif
        if(leng.eq.0)          go to 300
        nadcgam=nadcgam+1
        chisq=chisq2
c
        ic=ipnpk(ipk)
        ix=ia(ic)/100
        iy=ia(ic)-ix*100
c
        call peak_type(ix,iy,itype)
c
        e2=0.
        call gamma_hyc(itype,leng,iwrk(1,npk+1),iwrk(1,npk+2),
     &                 chisq,e1,x1,y1,
     &                       e2,x2,y2)
        chi2_adcgam(1,nadcgam)=chisq      !  remember chi2 of the cluster
        type_adcgam(1,nadcgam)=itype      !  remember type of the cluster
        energy_adcgam(1,nadcgam)=e1
        x_adcgam(1,nadcgam)=x1
        y_adcgam(1,nadcgam)=y1
*        time_adcgam(1,nadcgam)=t1
        dime_adcgam  (1,nadcgam)=leng
        id_adcgam  (1,nadcgam)=10
        status_adcgam(1,nadcgam)=itype
        if(e2.gt.0..and.nadcgam.le.madcgam-1) then    ! two gamma found
          nadcgam=nadcgam+1
          chi2_adcgam(1,nadcgam)=chisq      !  remember chi2 of the cluster
          type_adcgam(1,nadcgam)=itype+10      !  remember type of the cluster
          type_adcgam(1,nadcgam-1)=itype+10      !  remember type of the cluster
          energy_adcgam(1,nadcgam)=e2
*          time_adcgam(1,nadcgam)=t2
          x_adcgam(1,nadcgam)=x2
          y_adcgam(1,nadcgam)=y2
         xc_adcgam(1,nadcgam)=0.5*(x2-x1)
         yc_adcgam(1,nadcgam)=0.5*(y2-y1)
         xc_adcgam(1,nadcgam-1)=0.5*(x1-x2)
         yc_adcgam(1,nadcgam-1)=0.5*(y1-y2)
          dime_adcgam  (1,nadcgam)=leng
          id_adcgam  (1,nadcgam)=12
          id_adcgam  (1,nadcgam-1)=11
        status_adcgam(1,nadcgam)=itype
*
        do j = 1, leng
          if(j.le.MAX_CC) then
            icl_index(nadcgam,j)   = iwrk(j,npk+1)
            icl_index(nadcgam-1,j) = iwrk(j,npk+1)
            icl_iener(nadcgam,j)   = nint(iwrk(j,npk+2)*e2/(e1+e2))
            icl_iener(nadcgam-1,j) = nint(iwrk(j,npk+2)*e1/(e1+e2))
          endif
        enddo
      else
        do j = 1, leng
          if(j.le.MAX_CC) then
            icl_index(nadcgam,j)   = iwrk(j,npk+1)
            icl_iener(nadcgam,j)   = iwrk(j,npk+2)
          endif
        enddo
*
        endif
  300 continue
c
500   continue 
c
      return
      end
*
* ---------------------------------------------
*
      subroutine peak_type(ix,iy,itype)
      implicit none
#include "cphoto.inc"
*
*             -----------Argument declarations-----------
*
      integer 
     &          ix,iy   ,! row and column of a peak        INPUT
     &          itype    ! peak type                      OUTPUT
*
*               --------------Event Analysis Code-------------
*
      itype = 0
*
      IF(isect.eq.0) THEN
*
        if((ix.eq.NCOL/2-1.or.ix.eq.NCOL/2+2).and.
     &    iy.ge.NROW/2-1.and.iy.le.NROW/2+2) itype = 1        ! hole/outer boundary
*
        if((iy.eq.NROW/2-1.or.iy.eq.NROW/2+2).and.
     &    ix.ge.NCOL/2-1.and.ix.le.NCOL/2+2) itype = 1        ! hole/outer boundary
*
        if(ix.eq.1.or.ix.eq.NCOL.or.iy.eq.1.or.iy.eq.NROW)
     &                               itype = 2                ! transition (PWO/LG or LG/LG)
*
      ELSE IF(isect.eq.1) THEN
        if(ix.eq.1.or.iy.eq.1)        itype = 1
        if(ix.eq.NCOL.or.iy.eq.NROW)  itype = 2
      ELSE IF(isect.eq.2) THEN
        if(ix.eq.NCOL.or.iy.eq.1)     itype = 1
        if(ix.eq.1.or.iy.eq.NROW)     itype = 2
      ELSE IF(isect.eq.3) THEN
        if(ix.eq.NCOL.or.iy.eq.NROW)  itype = 1
        if(ix.eq.1.or.iy.eq.1)        itype = 2
      ELSE IF(isect.eq.4) THEN
        if(ix.eq.1.or.iy.eq.NROW)     itype = 1
        if(ix.eq.NCOL.or.iy.eq.1)     itype = 2
      ENDIF
*
      return
      end
*
* ---------------------------------------------
*
      subroutine gamma_hyc(itype,nadc,ia,id,chisq,
     &                                e1,x1,y1,
     &                                e2,x2,y2)
      implicit none
*
*
*             -----------Argument declarations-----------
*
*                                            input
      integer  
     &       itype            ,! type of the peak
     &       nadc             ,! number of counters in the peak
     &       ia(nadc)         ,! adresses of counters
     &       id(nadc)          ! energy of counters
      real
*     &       tt(nadc)         ,! time of counters
     &       chisq             ! reference value of chisq(input) 
                               ! improved  value of chisq(output) 
c
      integer nzero,           ! number of neib. elements, supposed to have 0 energy
     &        iaz(10800)       ! array  of neib. elements
c
c                                            output
c
      real
     &       e1,x1,y1         ,! energy and coordinates of first gamma
     &       e2,x2,y2          ! energy and coordinates of second gamma(if any)
                               ! coordinates are in terms of cell size
*      real   t1,t2             ! time of both gammas
*
*             ---------------include files---------------
*
#include "cphoto.inc"
*
*               -------------Local declarations------------
*
      integer*4 
     &     i,ix,iy,
     &     dof                 !  number of degrees of freedom  
      real
     &     dxy                ,!  initial step for iteration
     &     stepmin            ,!  minimum step for iteration
     &     stepx,stepy        ,!  current steps
     &     parx,pary          ,!
     &     chimem,chisq0,chi0 ,! 
     &     chir,chil,chiu,chid,!
     &     chiold             ,!
     &     chi00              ,!
     &     x0,y0              ,!
     &     ee,xx,yy           ,!
     &     d2,xm2,xm2cut
*
*               --------------Data statements--------------
*
      data 
     &     dxy/.05/           ,! in the units of sell size           
     &     stepmin/.002/      ,!
     &     xm2cut /1.7/        ! used to check of physical meaning of separation
*
*               --------------Event Analysis Code-------------
*
      e2=0.                             ! one gamma in a peak assumed
      x2=0.
      y2=0.
*      t2=0.
*
      call fill_zeros(nadc,ia,nzero,iaz)
c
      call mom1_pht(nadc,ia,id,nzero,iaz,e1,x1,y1)! calculate initial values of e,x,y
      if(nadc.le.0) return                 ! if nadc=0 e1=0.
c
      chimem=chisq                         ! remember reference value of chi2
c
      call chisq1_hyc(nadc,ia,id,nzero,iaz,e1,x1,y1,chi0)
                                            ! calculate initial value of chi2
      chisq0=chi0                           ! 
      dof=nzero+nadc-2                      ! number of degrees of freedom
      if(dof.lt.1) dof=1                    !
      chisq=chi0/dof                        ! 
      x0=x1                                 !
      y0=y1                                 !
c
c  Start of Iteration
c
    1 continue
      call chisq1_hyc(nadc,ia,id,nzero,iaz,e1,x0+dxy,y0,chir) ! values of x and y step
      call chisq1_hyc(nadc,ia,id,nzero,iaz,e1,x0-dxy,y0,chil) !
      call chisq1_hyc(nadc,ia,id,nzero,iaz,e1,x0,y0+dxy,chiu) !
      call chisq1_hyc(nadc,ia,id,nzero,iaz,e1,x0,y0-dxy,chid) !
      if(chi0.gt.chir.or.chi0.gt.chil) then
        stepx=dxy
        if(chir.gt.chil) stepx=-stepx
      else
        stepx=0.
        parx=chir+chil-2.*chi0
        if(parx.gt.0.) stepx=-dxy*(chir-chil)/(2.*parx)
      endif
      if(chi0.gt.chiu.or.chi0.gt.chid) then
        stepy=dxy
        if(chiu.gt.chid) stepy=-stepy
      else
        stepy=0.
        pary=chiu+chid-2.*chi0
        if(pary.gt.0.) stepy=-dxy*(chiu-chid)/(2.*pary)
      endif                                       
c
c     if steps at minimum - end iteration
c 
      if(abs(stepx).lt.stepmin.and.abs(stepy).lt.stepmin) go to 2
c
      call chisq1_hyc(nadc,ia,id,nzero,iaz,e1,x0+stepx,y0+stepy,chi00)
c
c     if chi2 at minimum - end iteration
c 
      if(chi00.ge.chi0) go to 2
c
      chi0=chi00
      x0=x0+stepx             ! next approximation
      y0=y0+stepy             !
      go to 1
    2 continue                     ! end of iteration
c
      if(chi0.lt.chisq0) then      ! if chi2 improved then
        x1=x0                      ! fix improved values
        y1=y0                      !
        chisq=chi0/dof             !
      endif
*
      if(chisq.le.chimem) return   ! if chi2 is less than maximim allowed for 
                                   ! one gamma in a peak- not try to separate
                                   ! the peak into two gammas
c
c  Try to improve chi2 by introducing second gamma
c
      chiold=chisq
      call tgamma_hyc(nadc,ia,id,nzero,iaz,chisq,ee,xx,yy,
     &					e2,x2,y2)
c
      if(e2.gt.0.) then            ! if chi2 improved -decide if the
                                   ! separation has physical meaning by
                                   ! calculating the effective mass
                                   ! of two gammas
        d2=(xx-x2)**2*xsize**2 + (yy-y2)**2*ysize**2
        xm2=ee*e2*d2
c
        if(xm2.gt.0.) xm2=sqrt(xm2)/1270.*0.1  ! mass in MeV.; 1270 = zhycal
c
cc      if(xm2.gt.xm2cut*(ee+e2)*0.01) then
cc      if(xm2.gt.xm2cut) then
        if(xm2.gt.xm2cut*xsize) then     ! if separation have physical meaning
          e1=ee                    ! fix the parameters of first gamma
          x1=xx                    !
          y1=yy                    !
        else
          e2=0.                    ! if no physical meaning e2=0.(second
          chisq=chiold             ! gamma is killed)
        endif
      endif
      return
      end
*
* ---------------------------------------------
*
      subroutine fill_zeros(nadc,ia,nneib,ian)
      implicit none
#include "cphoto.inc"
      integer 
     &          nadc          ,! number of hits in cluster           INPUT
     &          ia(nadc)       ! adresses of counters                INPUT
*
      integer i, j, ix, iy, nneib, nneibnew, ian(10800)
      integer stat_ch
      common/stat_ch_common/ stat_ch(MCOL,MROW)
*
      nneib = 0
      do i=1,nadc
        ix = ia(i) / 100
        iy = ia(i) - ix * 100
        if(ix.gt.1) then  ! fill left neib
          nneib = nneib + 1
          ian(nneib) = iy   + (ix-1)*100
*
        if(iy.gt.1) then  ! fill bottom left neib
          nneib = nneib + 1
          ian(nneib) = iy-1 + (ix-1)*100
        endif
        if(iy.lt.NROW) then  ! fill top left neib
          nneib = nneib + 1
          ian(nneib) = iy+1 + (ix-1)*100
        endif
*
        endif
        if(ix.lt.NCOL) then  ! fill right neib
          nneib = nneib + 1
          ian(nneib) = iy + (ix+1)*100
*
        if(iy.gt.1) then  ! fill bottom right neib
          nneib = nneib + 1
          ian(nneib) = iy-1 + (ix+1)*100
        endif
        if(iy.lt.NROW) then  ! fill top right neib
          nneib = nneib + 1
          ian(nneib) = iy+1 + (ix+1)*100
        endif
*
        endif
        if(iy.gt.1) then  ! fill bottom neib
          nneib = nneib + 1
          ian(nneib) = iy-1 + ix*100
        endif
        if(iy.lt.NROW) then  ! fill top neib
          nneib = nneib + 1
          ian(nneib) = iy+1 + ix*100
        endif
      enddo
*
      do i = 1, nneib
        do j = 1, nadc
          if(ia(j).eq.ian(i)) then
            ian(i) = -1
          endif
        enddo
      enddo
*
      do i = 1, nneib
        if(ian(i).eq.-1) goto 301
        do j = i+1, nneib
          if(ian(j).eq.ian(i)) then
            ian(j) = -1
          endif
        enddo
301     continue
      enddo
*
      nneibnew = 0
      do i = 1, nneib
        ix = ian(i) / 100
        iy = ian(i) - ix * 100
        if(ian(i).ne.-1) then
        if(stat_ch(ix,iy).eq.0) then    ! suggested by Weizhi:
          nneibnew = nneibnew + 1
          ian(nneibnew) = ian(i)
        endif
        endif
      enddo
      nneib = nneibnew
*
      return
      end
*
* ---------------------------------------------
*
      subroutine mom1_pht(nadc,ia,id,nzero,iaz,a0,x0,y0)
      implicit none
*
*
*             -----------Argument declarations-----------
*
      integer 
     &          nadc          ,! number of hits in cluster           INPUT
     &          ia(nadc)      ,! adresses of counters                INPUT
     &          id(nadc)       ! energy in counter                   INPUT
*      real      tt(nadc)       ! time of counter                     INPUT
*
      real   
     *          a0,x0,y0       ! First momenta                      OUTPUT
*      real      t              ! energy weighted time               OUTPUT
*
      integer   nzero,         ! number of neib. elements, supposed to have 0 energy
     &          iaz(nzero)     ! array  of neib. elements
*
*               -------------Local declarations------------
*
      integer*4 
     &     i                                   ,!index for cycle 
     &     ix,iy                                !
      real a                                   ,!
     &     corr                                ,! Correction to energy
     &     cell_hyc                             ! calc. energy in cell
*     &     sh_en                               ,! total shower energy  
*     &     tweight                              ! weighted time sum 
*
*               --------------Event Analysis Code-------------
*
      a0=0.
      x0=0.
      y0=0.
*      t=0.
*      sh_en=0.
      if(nadc.le.0) return
      do i=1,nadc
        a=id(i)
        ix=ia(i)/100
        iy=ia(i)-ix*100
        a0=a0+a
        x0=x0+a*ix
        y0=y0+a*iy
*        sh_en=sh_en+a
*        tweight=tt(i)*a
      enddo
      if(a0.le.0.) return
      x0=x0/a0
      y0=y0/a0
*      t=tweight/sh_en
      corr=0.              ! correction for delta
      do i=1,nadc
        ix=ia(i)/100
        iy=ia(i)-ix*100
        corr=corr+cell_hyc(float(ix)-x0,float(iy)-y0)
      enddo
*
      do i=1,nzero
        ix=iaz(i)/100
        iy=iaz(i)-ix*100
        corr=corr+cell_hyc(float(ix)-x0,float(iy)-y0)
      enddo
      corr = corr / 1.006
*
      if(corr.lt..8) then
cc      print*, 'corr = ', corr, a0, x0, y0 ! to many if around central hole
        corr=.8
      else if(corr.gt.1.) then
        corr=1.
      endif
      a0=a0/corr
*
      return
      end
*
* ---------------------------------------------
*
      subroutine chisq1_hyc(nadc,ia,id,nneib,ian,e1,x1,y1,chisq)
      implicit none
*
*             -----------Argument declarations-----------
*
      integer
     &          nadc          ,! number of hits in cluster           INPUT
     &          ia(nadc)      ,! adresses of counters                INPUT
     &          id(nadc)       ! energy in counter                   INPUT
*
      real
     &          e1,x1,y1      ,!                                     INPUT
     &          chisq          !                                    OUTPUT
*
      integer   nneib,         ! number of neib. elements, supposed to have 0 energy
     &          ian(nneib)     ! array  of neib. elements
*
*               -------------local declarations------------
*
      integer 
     &        i,ix,iy                ! 
      real
c    &        const                 ,!
     &        a,d                   ,! 
     &        cell_hyc               ! calc. energy in cell
*
*               --------------data statements--------------
*
c     data const/1.5/
c     save const
*
      real fcell, sigma2
*
*               --------------Event Analysis Code-------------
*
      chisq = 0.
c
      do i=1,nadc
        ix = ia(i) / 100
        iy = ia(i) - ix * 100
        if (e1.ne.0.) then
          if(abs(x1-ix).gt.6.0 .or. abs(y1-iy).gt.6.0) then
          else
            fcell = cell_hyc(x1-ix,y1-iy)
            chisq = chisq
     &          + e1*(fcell-id(i)/e1)**2 / sigma2(x1-ix,y1-iy,fcell,e1)
          endif
*
        else
          chisq = chisq + id(i)**2 / 9.
          print*, 'case 0 ch'
        endif
*
      enddo
*
      do i=1,nneib
        ix = ian(i) / 100
        iy = ian(i) - ix * 100
        if (e1.ne.0.) then
          if(abs(x1-ix).gt.6.0 .or. abs(y1-iy).gt.6.0) then
          else
            fcell = cell_hyc(x1-ix,y1-iy)
            chisq = chisq
     &          + e1*fcell**2 / sigma2(x1-ix,y1-iy,fcell,e1)
          endif
*
        else
          chisq = chisq + id(i)**2 / 9.
          print*, 'case 0 ch'
        endif
*
      enddo
*
      return
      end
*
      real function sigma2(dx,dy,fc,e)            ! sigma_e^2/e = sigma_f^2*e, units are 10MeV
      implicit none
*
      real dx, dy, fc, e
*
      real alp, bet1, bet2, d2c
      parameter (alp = 0.816, bet1 = 32.1, bet2 = 1.72)
*
      if(dx*dx+dy*dy.gt.25.) then
        sigma2 = 100.
        return
      endif
*
      sigma2 =  alp*fc + (bet1+bet2*sqrt(e/100.))*d2c(dx,dy)
     &       +  0.2/(e/100.)       ! 0.2 for ped-sigma and 10/sqrt(12)
#ifdef test_p
      sigma2 =  sigma2 / (0.0001*e)**0.166
#endif
      sigma2 =  100. * sigma2
*
      return
      end
*
      real function d2c(x,y)  ! (df/dx)^2 + (df/dy)^2
      implicit none
      real x, y
#include "cphoto.inc"
*
      integer i, j, i0
      real ax, ay, wx, wy
      real acell, ad2c
      common/profile_com/ acell(0:1,0:500,0:500), ad2c(0:1,0:500,0:500)
*
      ax  = abs(x*100.)
      ay  = abs(y*100.)
      i   = int(ax)
      j   = int(ay)

      if(i.lt.500 .and. j.lt.500) then
*
        i0  = min(1,isect)
        wx  = ax - i
        wy  = ay - j
*
        d2c = ad2c(i0,i,  j)   * (1.-wx) * (1.-wy) +
     &        ad2c(i0,i+1,j)   *     wx  * (1.-wy) +
     &        ad2c(i0,i,  j+1) * (1.-wx) *     wy  +
     &        ad2c(i0,i+1,j+1) *     wx  *     wy
      else
        d2c = 1.
      endif
*
      return
      end
*
#ifdef test_p
      subroutine chisq2t_hyc(ecell,e1,dx1,dy1,e2,dx2,dy2,f1,f2,chisqt)
      implicit none
*
      real  ecell, e1, dx1, dy1, e2, dx2, dy2, f1, f2, chisqt, sigma2
      real s
*
      if(e1.ne.0..and.e2.ne.0.) then
        s = e1*sigma2(dx1,dy1,f1,e1)/(0.0001*e1)**0.166 +
     &      e2*sigma2(dx2,dy2,f2,e2)/(0.0001*e2)**0.166
      else if(e1.eq.0..and.e2.eq.0.) then
        s = 90000.
      else if(e1.eq.0.) then
        s = e2*sigma2(dx2,dy2,f2,e2)/(0.0001*e2)**0.166
      else
        s = e1*sigma2(dx1,dy1,f1,e1)/(0.0001*e1)**0.166
      endif
*
      chisqt = (e1*f1+e2*f2-ecell)**2 / s
*
      return
      end
#else
      subroutine chisq2t_hyc(ecell,e1,dx1,dy1,e2,dx2,dy2,f1,f2,chisqt)
      implicit none
*
      real  ecell, e1, dx1, dy1, e2, dx2, dy2, f1, f2, chisqt, sigma2
      real s
*
      if(e1.ne.0..and.e2.ne.0.) then
        s = e1*sigma2(dx1,dy1,f1,e1) + e2*sigma2(dx2,dy2,f2,e2)
      else if(e1.eq.0..and.e2.eq.0.) then
        s = 90000.
      else if(e1.eq.0.) then
        s = e2*sigma2(dx2,dy2,f2,e2)
      else
        s = e1*sigma2(dx1,dy1,f1,e1)
      endif
*
      chisqt = (e1*f1+e2*f2-ecell)**2 / s
*
      return
      end
#endif
*
* ---------------------------------------------
*
      subroutine tgamma_hyc(nadc,ia,id,nzero,iaz,chisq,
     &                           e1,x1,y1,e2,x2,y2)
      implicit none
*
*
*             -----------Argument declarations-----------
*
*                                            input
      integer  
     &       nadc             ,! number of counters in the peak
     &       ia(nadc)         ,! adresses of counters
     &       id(nadc)          ! energy of counters
*      real   tt(nadc)          ! time of counters
      real
     &       chisq             ! initial   value of chisq(input) 
                               ! improved  value of chisq(output) 
*
      integer   nzero,         ! number of neib. elements, supposed to have 0 energy
     &          iaz(nzero)     ! array  of neib. elements
*
*                                            output
*
      real
     &       e1,x1,y1         ,! energy and coordinates of first gamma
     &       e2,x2,y2          ! energy and coordinates of second gamma
                               ! coordinates are in terms of sell size
*
*               -------------Local declarations------------
*
      integer*4 
     &     i,ix,iy,iter       ,! dummy loop index
     &     dof                 ! number of degrees of freedom   
      real
     &     dxy                ,!
     &     dxc,dyc            ,!
     &     dx0,dy0            ,!
     &     dx1,dy1            ,!
     &     dx2,dy2            ,!
     &     u,r,rsq,rsq2       ,!  
     &     epsc,eps0,eps1,eps2,!
     &     stpmin,epsmax      ,!
c    &     const              ,!
     &     delch              ,!
     &     step               ,! 
     &     cosi,scal          ,!
     &     chisq2,chisqc      ,! 
     &     dchi,dchida,dchi0  ,!
     &     a0,a1,a2           ,!
     &     ex,d,dd            ,!
     &     e1c,x1c,y1c        ,!
     &     e2c,x2c,y2c        ,!
     &     gr,gre,grx,gry     ,!
     &     grc                ,!
     &     grec,grxc,gryc     ,!
     &     gx1,gy1            ,!
     &     gx2,gy2            ,!
     &     e0,x0,y0           ,!
     &     xx,yy,yx            !
*
      real f1c, f2c, f1x, f2x, f1y, f2y,
     &     chisqt, chisqt0, chisqt1, chisqt2,
     &     chisqtx1, chisqtx2, chisqty1, chisqty2,
     &     dchidax1, dchidax2, dchiday1, dchiday2
*
      real cell_hyc            ! fraction of energy in a cell
*
*               --------------Data statements--------------
*
      data 
     &     stpmin/.5/        ,!
     &     epsmax/.9999/     ,!
     &     delch /10./        ! /8./
c    &     const /1.5/
*
*               --------------Event Analysis Code-------------
*
      call mom2_pht(nadc,ia,id,nzero,iaz,e0,x0,y0,xx,yy,yx)
*
      e2=0.
      x2=0.
      y2=0.
c
      if(nadc.le.0)        return
c
c     choosing of the starting point
c
      dxy = xx-yy
      rsq2= dxy**2+4.*yx**2
      if(rsq2.lt.1.e-20) rsq2=1.e-20
      rsq = sqrt(rsq2)
      dxc =-sqrt((rsq+dxy)*2.)
      dyc = sqrt((rsq-dxy)*2.)
      if(yx.ge.0.) dyc=-dyc
      r=sqrt(dxc**2+dyc**2)
      epsc=0.
      do i=1,nadc
        ix = ia(i)/100
        iy = ia(i)-ix*100
        u  = (ix-x0)*dxc/r+(iy-y0)*dyc/r
        epsc=epsc-0.01*id(i)*u*abs(u)
      enddo
      epsc=epsc/(0.01*e0*rsq)
      if(epsc.gt.0.8) epsc=0.8
      if(epsc.lt.-.8) epsc=-.8
      dxc=dxc/sqrt(1.-epsc**2)
      dyc=dyc/sqrt(1.-epsc**2)
c
c     start of iterations
c
      step = 0.1
      cosi = 0.0
      chisq2=1.e35
c
c
c
      do iter = 1, 100
c
        call c3to5_pht(e0,x0,y0,epsc,dxc,dyc,e1c,x1c,y1c,e2c,x2c,y2c)
        eps1 = (1.+epsc)/2.
        eps2 = (1.-epsc)/2.
        chisqc=0.
        do i=1,nadc+nzero     !  chi**2 calculation
*
          if(i.le.nadc) then
            ex  = id(i)
            ix = ia(i)/100
            iy = ia(i)-ix*100
          else
            ex  = 0
            ix  = iaz(i-nadc)/100
            iy  = iaz(i-nadc)-ix*100
          endif
*
          dx1 = x1c - ix
          dy1 = y1c - iy
          dx2 = x2c - ix
          dy2 = y2c - iy
          f1c = cell_hyc(dx1,dy1)
          f2c = cell_hyc(dx2,dy2)
          call chisq2t_hyc(ex,e1c,dx1,dy1,e2c,dx2,dy2,f1c,f2c,chisqt)
          chisqc = chisqc+chisqt
        enddo
        if(chisqc.ge.chisq2) then  !  new step if no improvement
          if(iter.gt.1) then
            dchi = chisqc-chisq2
            dchi0= gr*step
            step = .5*step/sqrt(1.+dchi/dchi0)
          endif
          step = .5*step
        else               !  calculation of gradient
          grec = 0.
          grxc = 0.
          gryc = 0.
          do i=1,nadc+nzero
            if(i.le.nadc) then
              ex  = id(i)
              ix  = ia(i)/100
              iy  = ia(i)-ix*100
            else
              ex  = 0
              ix  = iaz(i-nadc)/100
              iy  = iaz(i-nadc)-ix*100
            endif
            dx1 = x1c - ix
            dy1 = y1c - iy
            dx2 = x2c - ix
            dy2 = y2c - iy
*
            f1c = cell_hyc(dx1,dy1)
            f2c = cell_hyc(dx2,dy2)
*
            a1  = e1c*f1c
            a2  = e2c*f2c
            a0  = a1 + a2
            call
     &      chisq2t_hyc(ex,e1c,dx1,dy1,e2c,dx2,dy2,f1c,f2c,chisqt0)
            call
     &      chisq2t_hyc(ex,e1c+1.,dx1,dy1,e2c,dx2,dy2,f1c,f2c,chisqt1)
            call
     &      chisq2t_hyc(ex,e1c,dx1,dy1,e2c+1.,dx2,dy2,f1c,f2c,chisqt2)
*
            f1x = cell_hyc(x1c+.05-ix,dy1)
            f2x = cell_hyc(x2c+.05-ix,dy2)
            f1y = cell_hyc(dx1,y1c+.05-iy)
            f2y = cell_hyc(dx2,y2c+.05-iy)
*
            call
     &  chisq2t_hyc(ex,e1c,dx1+0.05,dy1,e2c,dx2,dy2,f1x,f2c,chisqtx1)
            call
     &  chisq2t_hyc(ex,e1c,dx1,dy1,e2c,dx2+0.05,dy2,f1c,f2x,chisqtx2)
            call
     &  chisq2t_hyc(ex,e1c,dx1,dy1+0.05,e2c,dx2,dy2,f1y,f2c,chisqty1)
            call
     &  chisq2t_hyc(ex,e1c,dx1,dy1,e2c,dx2,dy2+0.05,f1c,f2y,chisqty2)
*
            dchidax1 = 20.*(chisqtx1 - chisqt0)
            dchidax2 = 20.*(chisqtx2 - chisqt0)
            dchiday1 = 20.*(chisqty1 - chisqt0)
            dchiday2 = 20.*(chisqty2 - chisqt0)
            dchida   = 0.5*(chisqt1 + chisqt2 - chisqt0)
*
            gx1 = (e1c*f1x-a1) * dchidax1
            gx2 = (e2c*f2x-a2) * dchidax2
            gy1 = (e1c*f1y-a1) * dchiday1
            gy2 = (e2c*f2y-a2) * dchiday2
*
            grec = grec + dchida*(f1c-f2c)*e0
     &           - ( (gx1+gx2)*dxc + (gy1+gy2)*dyc )
*
            grxc = grxc + (gx1*eps2-gx2*eps1)
            gryc = gryc + (gy1*eps2-gy2*eps1)
*
          enddo
          grc  = sqrt(grec**2+grxc**2+gryc**2)
          if(grc.lt.1.e-6) grc=1.e-6
          if(iter.gt.1) then
            cosi = (gre*grec+grx*grxc+gry*gryc)/(gr*grc)
            scal = abs(gr/grc-cosi)
            if(scal.lt..1) scal=.1
            step=step/scal
          endif
          chisq2=chisqc
          eps0= epsc
          dx0 = dxc
          dy0 = dyc
          gre = grec
          grx = grxc
          gry = gryc
          gr  = grc
        endif
        epsc= eps0- step*gre/gr
        do while (abs(epsc).gt.epsmax)
          step=step/2.
          epsc= eps0- step*gre/gr
        enddo
        dxc = dx0 - step*grx/gr
        dyc = dy0 - step*gry/gr
        if(step*gr.lt.stpmin) go to 101
      enddo                                ! end of iterations
c
  101 continue
      if(chisq*(nadc+nzero-2)-chisq2.lt.delch) return
      dof=nzero+nadc-5
      if(dof.lt.1) dof=1
      chisq=chisq2/dof
c
      call c3to5_pht(e0,x0,y0,eps0,dx0,dy0,e1,x1,y1,e2,x2,y2)
c
      return
      end
*
* ---------------------------------------------
*
      subroutine c3to5_pht(e0,x0,y0,eps,dx,dy,e1,x1,y1,e2,x2,y2)
c
c  eps=(e1-e2)/e0, (e0=e1+e2),  x0*e0=x1*e1+x2*e2,  dx=x1-x2.
c     
      implicit none  
*
*               -----------Argument declarations-----------
*
      real
     &        e0,x0,y0,eps,dx,dy,                 ! input
     &        e1,x1,y1,e2,x2,y2                   ! output
*
      e1=e0*(1+eps)/2.
      e2=e0-e1
      x1=x0+dx*(1.-eps)/2.
      y1=y0+dy*(1.-eps)/2.
      x2=x0-dx*(1.+eps)/2.
      y2=y0-dy*(1.+eps)/2.
c
      return
      end
*
* ---------------------------------------------
*
      subroutine mom2_pht(nadc,ia,id,nzero,iaz,a0,x0,y0,xx,yy,yx)
      implicit none
*
*             -----------Argument declarations-----------
*
      integer*4
     &          nadc          ,! number of hits in cluster           INPUT
     &          ia(nadc)      ,! adresses of counters                INPUT
     &          id(nadc)       ! energy in counter                   INPUT
*      real      tt(nadc)       ! time of counter                     INPUT
*
      real   
     *          a0,x0,y0      ,! First momenta                      OUTPUT
     *          xx,yy,yx       ! Second momenta                     OUTPUT
*
      integer   nzero,         ! number of neib. elements, supposed to have 0 energy
     &          iaz(nzero)     ! array  of neib. elements
*
*               -------------Local declarations------------
*
      integer*4 
     &     i                                   ,!index for cycle 
     &     ix,iy                                !
      real a                                   ,!
     &     corr                                ,! Correction to energy
     &     cell_hyc                             ! calc. energy in cell
*     &     sh_en                               ,! total shower energy  
*     &     tweight                              ! weighted time sum 
*
*               --------------Event Analysis Code-------------
*
      a0=0.
      x0=0.
      y0=0.
      xx=0.
      yy=0.
      yx=0.
*      t=0.
*      sh_en=0.
      if(nadc.le.0) return
      do i=1,nadc        !  first momenta
        a=id(i)
        ix=ia(i)/100
        iy=ia(i)-ix*100
        a0=a0+a
        x0=x0+a*ix
        y0=y0+a*iy
*        sh_en=sh_en+a
*        tweight=tt(i)*a
      enddo
      if(a0.le.0.) return
      x0=x0/a0
      y0=y0/a0
*      t=tweight/sh_en
      do i=1,nadc        !  second momenta
        a=id(i)/a0
        ix=ia(i)/100
        iy=ia(i)-ix*100
        xx=xx+a*(ix-x0)**2
        yy=yy+a*(iy-y0)**2
        yx=yx+a*(ix-x0)*(iy-y0)
      enddo
      corr=0.              ! correction for delta
      do i=1,nadc
        ix=ia(i)/100
        iy=ia(i)-ix*100
        corr=corr+cell_hyc(float(ix)-x0,float(iy)-y0)
      enddo
      do i=1,nzero
        ix=iaz(i)/100
        iy=iaz(i)-ix*100
        corr=corr+cell_hyc(float(ix)-x0,float(iy)-y0)
      enddo
      corr = corr /1.006
      if(corr.lt..8) then
        corr=.8
      else if(corr.gt.1.) then
        corr=1.
      endif
      a0=a0/corr
c
      return
      end
*
* ---------------------------------------------
*
      real function cell_hyc(x,y)
      implicit none
#include "cphoto.inc"
*
      real x, y, x1, y1, x2, y2, cellsize
*
      integer i, j, i0
      real ax, ay, wx, wy
      real acell, ad2c
      common/profile_com/ acell(0:1,0:500,0:500), ad2c(0:1,0:500,0:500)
*
      ax  = abs(x*100.)
      ay  = abs(y*100.)
      i   = int(ax)
      j   = int(ay)
*
      if(i.lt.500 .and. j.lt.500) then
*
        i0  = min(1,isect)
        wx  = ax - i
        wy  = ay - j
*
* correction suggested by Weizhi:
*
        cell_hyc = acell(i0,i,  j)   * (1.-wx) * (1.-wy) +
     &             acell(i0,i+1,j)   *     wx  * (1.-wy) +
     &             acell(i0,i,  j+1) * (1.-wx) *     wy  +
     &             acell(i0,i+1,j+1) *     wx  *     wy
*
c       cell_hyc = acell(i0,i,  j)   *     wx  *     wy  +
c    &             acell(i0,i+1,j)   * (1.-wx) *     wy  +
c    &             acell(i0,i,  j+1) *     wx  * (1.-wy) +
c    &             acell(i0,i+1,j+1) * (1.-wx) * (1.-wy)
*
      else
        cell_hyc = 0.
      endif
      return
      end
*
* ---------------------------------------------
*
      subroutine out_hyc
      implicit none
*
*             ---------------include files---------------
*
#include "cphoto.inc"
#include "adcgam_bk.inc"
*
*               -------------local declarations------------
*
      integer i, nold, j, k
*
      integer tmpadcgam(ladcgam), wbuf
*
      integer icl_index, icl_iener      ! list of cluster elements
      common /icl_common/ icl_index(200,MAX_CC), icl_iener(200,MAX_CC)
*
*               --------------event Analysis code-------------
*
      do i = 1, nadcgam
        energy_adcgam(1,i) = energy_adcgam(1,i)/10000.
             x_adcgam(1,i) = (NCOL+1-2.*x_adcgam(1,i))*xsize/2.
             y_adcgam(1,i) = (NROW+1-2.*y_adcgam(1,i))*ysize/2.
            xc_adcgam(1,i) = -xc_adcgam(1,i)*xsize
            yc_adcgam(1,i) = -yc_adcgam(1,i)*ysize
          type_adcgam(1,i) =   id_adcgam(1,i)
      enddo
*
*     sort in increasing order
*
      do i = 1, nadcgam
      do j = i+1, nadcgam
        if(energy_adcgam(1,i).lt.energy_adcgam(1,j)) then
          call ucopy(adcgam(1,i),  tmpadcgam,ladcgam)
          call ucopy(adcgam(1,j),adcgam(1,i),ladcgam)
          call ucopy(tmpadcgam,  adcgam(1,j),ladcgam)
          do k = 1, MAX_CC
            wbuf           = icl_index(i,k)
            icl_index(i,k) = icl_index(j,k)
            icl_index(j,k) = wbuf
            wbuf           = icl_iener(i,k)
            icl_iener(i,k) = icl_iener(j,k)
            icl_iener(j,k) = wbuf
          enddo
        endif
      enddo
      enddo
*
      return
      end
*
* ---------------------------------------------
*
      subroutine dump_hyc
      implicit none
c
#include "adcgam_bk.inc"
c 
c               -------------Local declarations------------
c
      integer*4 
     $     ncl                                    ! loop index
c
c------------------------ Executable code starts here -------------------
c
c
c==========================================
        write(*,100) nadcgam
        write(*,101)
        do 1000 ncl=1,nadcgam
          write(*,102)
     &           ncl,
     &      energy_adcgam(1,ncl),
     &           x_adcgam(1,ncl),
     &           y_adcgam(1,ncl),
     &        chi2_adcgam(1,ncl),
     &        dime_adcgam(1,ncl),
     &        type_adcgam(1,ncl),
     &      status_adcgam(1,ncl)
 1000   continue
c
      return                   ! and quit 
* 
c          ---------------Format statements-------------- 
c 
 100    format('number of gammas = ',i4)
 101    format
     & ('  n   energy   x      y     chi2  dime',
     &  ' type status')
 102    format(i4,2x,4f7.2,3i5)
c
      end
*
*
      SUBROUTINE UCOPY(A,B,N)
      implicit none
      INTEGER   A(*), B(*), N, I
C--                NO OVERLAP  OR  BEGINNING OF A ON END OF B
      IF (N.LE.0) RETURN
      DO 21 I = 1, N
   21 B(I) = A(I)
      RETURN
      END
*
      SUBROUTINE UCOPY21(A,B,N)
      implicit none
      INTEGER A(*), B(*), N, I, IA, IB, c(10800)
C
      IF(N.LE.0) return
      IF(N.GT.10800) THEN
        PRINT*, 'ERR% UCOPY21 ARRAY TOO LONG ', N
        CALL EXIT(1)
      ENDIF
      DO 21 I = 1, N
   21 c(I) = A(I)
      DO 22 I = 1, N
   22 B(I) = c(I)
      RETURN
      END
*
