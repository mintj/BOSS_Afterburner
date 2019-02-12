# echo "setup PipiJpsiAlg PipiJpsiAlg-00-00-03 in /afs/.ihep.ac.cn/bes3/offline/Boss/7.0.4/Analysis/Physics/PsiPrime"

if test "${CMTROOT}" = ""; then
  CMTROOT=/afs/ihep.ac.cn/bes3/offline/ExternalLib/SLC6/contrib/CMT/v1r25; export CMTROOT
fi
. ${CMTROOT}/mgr/setup.sh
cmtPipiJpsiAlgtempfile=`${CMTROOT}/mgr/cmt -quiet build temporary_name`
if test ! $? = 0 ; then cmtPipiJpsiAlgtempfile=/tmp/cmt.$$; fi
${CMTROOT}/mgr/cmt setup -sh -pack=PipiJpsiAlg -version=PipiJpsiAlg-00-00-03 -path=/afs/.ihep.ac.cn/bes3/offline/Boss/7.0.4/Analysis/Physics/PsiPrime  -no_cleanup $* >${cmtPipiJpsiAlgtempfile}
if test $? != 0 ; then
  echo >&2 "${CMTROOT}/mgr/cmt setup -sh -pack=PipiJpsiAlg -version=PipiJpsiAlg-00-00-03 -path=/afs/.ihep.ac.cn/bes3/offline/Boss/7.0.4/Analysis/Physics/PsiPrime  -no_cleanup $* >${cmtPipiJpsiAlgtempfile}"
  cmtsetupstatus=2
  /bin/rm -f ${cmtPipiJpsiAlgtempfile}
  unset cmtPipiJpsiAlgtempfile
  return $cmtsetupstatus
fi
cmtsetupstatus=0
. ${cmtPipiJpsiAlgtempfile}
if test $? != 0 ; then
  cmtsetupstatus=2
fi
/bin/rm -f ${cmtPipiJpsiAlgtempfile}
unset cmtPipiJpsiAlgtempfile
return $cmtsetupstatus

