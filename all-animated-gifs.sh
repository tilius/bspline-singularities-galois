rm -rf eps/
mkdir -p eps gif

gnomon() {
	for i in `seq 0 5 355`; do
		echo $i
		./render-non-rect-support non-rect-support-`printf "%.3i" $i` $i | gnuplot
	done
	convert -delay 10 -density 150 -loop 0 -resize 1024x768 eps/non-rect-support-*.eps gif/non-rect-support.gif
}

overview() {
	tag=$1
	shift
	for i in `seq 0 5 120`; do
		echo $tag $i
		i0=`printf "%.3i" $i`
		./render-bsplines nurbs-overview-$tag-$i0 $i Regular $@ | gnuplot
	done
	for i in `seq 125 5 180`; do
		cp eps/nurbs-overview-$tag-120.eps eps/nurbs-overview-$tag-${i}.eps
	done
	convert -delay 10 -density 150 -loop 0 -resize 1024x768 eps/nurbs-overview-$tag-*.eps gif/nurbs-overview-${tag}.gif
}

overview part1 1 9 18 21
overview part2 36 41 44
#gnomon
