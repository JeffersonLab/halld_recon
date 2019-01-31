#define MAX_HHITS 1728
*
      block data mc_com
      implicit none
*
      integer mcrun, mcevent, nout, iout, aout, ih32
      real    hegen
      common /read_mcfile_com/ mcrun, mcevent, hegen(64), nout,
     &        iout(MAX_HHITS), aout(MAX_HHITS)
*
      integer mcread_flag, mc_eof_flag, nreadmc
      common /mcread_stat_com/ mcread_flag, mc_eof_flag, nreadmc
*
      integer run, event, latch, tdif, live2, nout_dst,
     &        id_counters, count_counters
      real    ebeam
      common /read_dstfile_com/ run, event, ebeam, latch, tdif, live2,
     &        nout_dst, id_counters(MAX_HHITS),
     &        count_counters(MAX_HHITS)
*
      end
*
