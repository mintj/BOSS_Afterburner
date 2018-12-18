# echo "cleanup NTupleMap NTupleMap-00-00-00 in /besfs/users/deboer/BOSS_Afterburner/boss/workarea/Analysis"

if test "${CMTROOT}" = ""; then
  CMTROOT=/afs/ihep.ac.cn/bes3/offline/ExternalLib/SLC6/contrib/CMT/v1r25; export CMTROOT
fi
. ${CMTROOT}/mgr/setup.sh
cmtNTupleMaptempfile=`${CMTROOT}/mgr/cmt -quiet build temporary_name`
if test ! $? = 0 ; then cmtNTupleMaptempfile=/tmp/cmt.$$; fi
${CMTROOT}/mgr/cmt cleanup -sh -pack=NTupleMap -version=NTupleMap-00-00-00 -path=/besfs/users/deboer/BOSS_Afterburner/boss/workarea/Analysis  $* >${cmtNTupleMaptempfile}
if test $? != 0 ; then
  echo >&2 "${CMTROOT}/mgr/cmt cleanup -sh -pack=NTupleMap -version=NTupleMap-00-00-00 -path=/besfs/users/deboer/BOSS_Afterburner/boss/workarea/Analysis  $* >${cmtNTupleMaptempfile}"
  cmtcleanupstatus=2
  /bin/rm -f ${cmtNTupleMaptempfile}
  unset cmtNTupleMaptempfile
  return $cmtcleanupstatus
fi
cmtcleanupstatus=0
. ${cmtNTupleMaptempfile}
if test $? != 0 ; then
  cmtcleanupstatus=2
fi
/bin/rm -f ${cmtNTupleMaptempfile}
unset cmtNTupleMaptempfile
return $cmtcleanupstatus

