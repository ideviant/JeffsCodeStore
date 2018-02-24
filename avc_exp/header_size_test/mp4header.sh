for file in top500/*.mp4; do
	if mp4header ${file}.moov ${file}; then
		zip ${file}.zip ${file}.moov
		# BSD stat
		echo ${file}, $(stat -f%z ${file}.moov), $(stat -f%z ${file}.zip)
		# GNU stat
		#echo ${file}, $(stat -c%s ${file}.moov) $(stat -c%s ${file}.zip)
	fi
done

for file in top500/*.mp4_?d.mp4; do
	mp4header ${file/top500/top500_moov}.moov ${file}
done

for file in top500_moov/*.moov; do
	zip ${file}.zip ${file}
done


for file in top500_moov/*.mp4_hd.mp4.moov; do
	echo ${file}, $(stat -c%s ${file}), $(stat -c%s ${file}.zip), ,$(stat -c%s ${file/mp4_hd/mp4_ld}), $(stat -c%s ${file/mp4_hd/mp4_ld}.zip) 
done