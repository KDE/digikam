# Copyright (c) 2013-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
# Copyright (C) 2013-2014 by Gowtham Ashok <gwty93 at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#
# This creates image test-cases with ImageMagick which artificially apply
# blur, noise, compression, and over-exposure to given test image.
#

echo "Enter jpg image name (without extension)"
read imagename
i=1

while [ $i -lt 10 ]
do

    compressionlevel=`expr $i \* 10`

    blurred=$imagename"_blurred_"$i
    compressed=$imagename"_compressed_"$i
    noised=$imagename"_noised_"$i
    underexposed=$imagename"_underexposed_"$i
    overexposed=$imagename"_overexposed_"$i

    convert $imagename.jpg -blur 0x$i $blurred.jpg
    convert $imagename.jpg -quality $compressionlevel% $compressed.jpg
    convert $imagename.jpg -fx "u*0.$i" $underexposed.jpg
    convert $imagename.jpg -fx "u*1.$i" $overexposed.jpg

    if [ $i -eq 1 ]; then
        convert $imagename.jpg +noise gaussian $noised.jpg
    else
        j=`expr $i - 1`
        noisedj=$imagename"_noised_"$j
        convert $noisedj.jpg +noise gaussian $noised.jpg
    fi

    echo "$compressionlevel% complete"
    i=`expr $i + 1`

done

echo "100% complete"
