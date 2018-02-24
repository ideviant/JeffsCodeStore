
#! 测试 mmov 的压缩效果
# for file in top500/*.mp4; do
# 	if mp4header ${file}.moov ${file}; then
# 		zip ${file}.zip ${file}.moov
# 		# BSD stat
# 		echo ${file}, $(stat -f%z ${file}.moov), $(stat -f%z ${file}.zip)
# 		# GNU stat
# 		#echo ${file}, $(stat -c%s ${file}.moov) $(stat -c%s ${file}.zip)
# 	fi
# done
# 
# for file in top500/*.mp4_?d.mp4; do
# 	mp4header ${file/top500/top500_moov}.moov ${file}
# done
# 
# for file in top500_moov/*.moov; do
# 	zip ${file}.zip ${file}
# done
# 
# for file in top500_moov/*.mp4_hd.mp4.moov; do
# 	echo ${file}, $(stat -c%s ${file}), $(stat -c%s ${file}.zip), ,$(stat -c%s ${file/mp4_hd/mp4_ld}), $(stat -c%s ${file/mp4_hd/mp4_ld}.zip) 
# done



#! 测试 MP4Box -inter 400 的压缩效果
# for hdfile in top500/*.mp4_hd.mp4; do
#     mkdir -p tmp
#     file=${hdfile%.mp4_hd.mp4}
#     ldfile=${file}.mp4_ld.mp4
#     hdout=${hdfile/top500/tmp}
#     ldout=${ldfile/top500/tmp}
#     hdmoov=${hdfile/top500/moov_inter400}.moov
#     ldmoov=${ldfile/top500/moov_inter400}.moov
#     MP4Box -noprog -quiet -inter 400 ${hdfile} -out ${hdout}
#     MP4Box -noprog -quiet -inter 400 ${ldfile} -out ${ldout}
#     ./mp4header ${hdmoov}  ${hdout}
#     ./mp4header ${ldmoov}  ${ldout}
#     echo ${file}, $(stat -c%s ${hdmoov/moov_inter400/moov}), $(stat -c%s ${hdmoov}), $(stat -c%s ${ldmoov/moov_inter400/moov}), $(stat -c%s ${ldmoov}) ;
#     rm -r tmp
# done

echo filename, \
    raw264_size, \
    rawaac_size, \
    file_size, \
    moov_size, \
    flv_size, \
    meta_size, \
    mdi_flv_size, \
    mdi_meta_size

VIDEODIR=topvideo
RAWDIR=rawstream
MOOVDIR=moov
FF_FLVDIR=no_key_flv
MD_FLVDIR=index_flv

if ! [ -d "${RAWDIR}" ]; then mkdir -p "${RAWDIR}"; fi
if ! [ -d "${MOOVDIR}" ]; then mkdir -p "${MOOVDIR}"; fi
if ! [ -d "${FF_FLVDIR}" ]; then mkdir -p "${FF_FLVDIR}"; fi
if ! [ -d "${MD_FLVDIR}" ]; then mkdir -p "${MD_FLVDIR}"; fi


for hdfile in ${VIDEODIR}/*.mp4_hd.mp4; do
	hdname=${hdfile#${VIDEODIR}/}	# strip prefix
    hdname=${hdname%.mp4}			# strip suffix

    # set name
    raw264=${RAWDIR}/${hdname}.264
    rawaac=${RAWDIR}/${hdname}.aac
    hdmoov=${MOOVDIR}/${hdname}.moov
    ff_flv=${FF_FLVDIR}/${hdname}.flv
    ff_flv_md=${FF_FLVDIR}/${hdname}.flv.metadata
    md_flv=${MD_FLVDIR}/${hdname}.flv
    md_flv_md=${MD_FLVDIR}/${hdname}.flv.metadata

    ffmpeg -i ${hdfile} -an -c:v copy -f rawvideo -y ${raw264}
    ffmpeg -i ${hdfile} -vn -c:a copy -f rawaudio -y ${rawaac}
    ffmpeg -i ${hdfile} -c:v copy -y ${ff_flv}
    yamdi -i ${ff_flv} -o ${md_flv}

    ./mp4header ${hdmoov}  ${hdfile}
    ./flvheader ${ff_flv_md} ${ff_flv}
    ./flvheader ${md_flv_md} ${md_flv}

    echo ${hdname}, \
        $(stat -c%s ${raw264}), \
        $(stat -c%s ${rawaac}), \
        $(stat -c%s ${hdfile}), \
        $(stat -c%s ${hdmoov}), \
        $(stat -c%s ${ff_flv}), \
        $(stat -c%s ${ff_flv_md}), \
        $(stat -c%s ${md_flv}), \
        $(stat -c%s ${md_flv_md})
done