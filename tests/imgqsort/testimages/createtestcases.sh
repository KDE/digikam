#This creates testcases which artificially apply blur, noise and compression to given test image


#ls|grep .JPG|cat>tests.txt
#ls|grep .jpg|cat>>tests.txt
#ls|grep .png|cat>>tests.txt
#echo "Collected pictures and stored them in tests.txt"
#count=0
#while read line
#do
#     currentfile=`echo $line`
#     image[$count]="$currentfile"
#     echo "Count incremented"
#     count=`expr $count + 1`
#done <tests.txt

 
echo "Enter image name(ending with .jpg)"
read imagename
i=1
while [ $i -lt 10 ]
do
compressionlevel=`expr $i \* 10`

blurred=$imagename"_blurred_"$i
compressed=$imagename"_compressed_"$i
noised=$imagename"_noised_"$i

convert  $imagename.jpg -blur 0x$i $blurred.jpg
convert  $imagename.jpg  -quality $compressionlevel% $compressed.jpg
if [ $i -eq 1 ]
        then
                convert  $imagename.jpg +noise gaussian $noised.jpg
else
        j=`expr $i - 1`
        noisedj=$imagename"_noised_"$j
        convert $noisedj.jpg +noise gaussian $noised.jpg
fi
echo "$compressionlevel% complete"
i=`expr $i + 1`
done
echo "100% complete"
