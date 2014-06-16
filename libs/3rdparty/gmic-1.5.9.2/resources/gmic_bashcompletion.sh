# 
#  Bash completion rules for 'gmic'. 
# 
# This file has been generated automatically. 
# Do not edit! 
# 
# This file should be copied/renamed in '/etc/bash_completion.d/gmic'. 
# 
 
_gmic_def() 
{ 
	local cur prev opts coms 
	COMPREPLY=() 
	cur="${COMP_WORDS[COMP_CWORD]}" 
	prev="${COMP_WORDS[COMP_CWORD-1]}" 
	opts="\
	      -debug --debug\
	      -h --h\
	      -help --help\
	      -version --version\
	      -apply_camera --apply_camera\
	      -apply_files --apply_files\
	      -camera --camera\
	      -m --m\
	      -command --command\
	      -cupid --cupid\
	      -cursor --cursor\
	      -d --d\
	      -display --display\
	      -d0 --d0\
	      -display0 --display0\
	      -d3d --d3d\
	      -display3d --display3d\
	      -da --da\
	      -display_array --display_array\
	      -dfft --dfft\
	      -display_fft --display_fft\
	      -dg --dg\
	      -display_graph --display_graph\
	      -dh --dh\
	      -display_histogram --display_histogram\
	      -display_parametric --display_parametric\
	      -dp --dp\
	      -display_polar --display_polar\
	      -drgba --drgba\
	      -display_rgba --display_rgba\
	      -dt --dt\
	      -display_tensors --display_tensors\
	      -dw --dw\
	      -display_warp --display_warp\
	      -document_gmic --document_gmic\
	      -e --e\
	      -echo --echo\
	      -echo_file --echo_file\
	      -echo_stdout --echo_stdout\
	      -function1d --function1d\
	      -gmicky --gmicky\
	      -gmicky_wilber --gmicky_wilber\
	      -heart --heart\
	      -i --i\
	      -input --input\
	      -o --o\
	      -output --output\
	      -output_ggr --output_ggr\
	      -on --on\
	      -outputn --outputn\
	      -op --op\
	      -outputp --outputp\
	      -ow --ow\
	      -outputw --outputw\
	      -plot --plot\
	      -p --p\
	      -print --print\
	      -rainbow_lut --rainbow_lut\
	      -roddy --roddy\
	      -remove_duplicates --remove_duplicates\
	      -remove_empty --remove_empty\
	      -select --select\
	      -sh --sh\
	      -shared --shared\
	      -srand --srand\
	      -testimage2d --testimage2d\
	      -text2img --text2img\
	      -type --type\
	      -uncommand --uncommand\
	      -uniform_distribution --uniform_distribution\
	      -up --up\
	      -update --update\
	      -v --v\
	      -verbose --verbose\
	      -wait --wait\
	      -warn --warn\
	      -w --w\
	      -window --window\
	      -k --k\
	      -keep --keep\
	      -mv --mv\
	      -move --move\
	      -nm --nm\
	      -name --name\
	      -rm --rm\
	      -remove --remove\
	      -rv --rv\
	      -reverse --reverse\
	      -sort_list --sort_list\
	      -sort_str --sort_str\
	      -abs --abs\
	      -acos --acos\
	      -+ --+\
	      -add --add\
	      -and --and\
	      -asin --asin\
	      -atan --atan\
	      -atan2 --atan2\
	      -<< --<<\
	      -bsl --bsl\
	      ->> -->>\
	      -bsr --bsr\
	      -cos --cos\
	      -cosh --cosh\
	      -/ --/\
	      -div --div\
	      -== --==\
	      -eq --eq\
	      -exp --exp\
	      ->= -->=\
	      -ge --ge\
	      -> -->\
	      -gt --gt\
	      -<= --<=\
	      -le --le\
	      -< --<\
	      -lt --lt\
	      -log --log\
	      -log10 --log10\
	      -log2 --log2\
	      -max --max\
	      -// --//\
	      -mdiv --mdiv\
	      -min --min\
	      -% --%\
	      -mod --mod\
	      -** --**\
	      -mmul --mmul\
	      -* --*\
	      -mul --mul\
	      -!= --!=\
	      -neq --neq\
	      -or --or\
	      -^ --^\
	      -pow --pow\
	      -rol --rol\
	      -ror --ror\
	      -sign --sign\
	      -sin --sin\
	      -sinc --sinc\
	      -sinh --sinh\
	      -sqr --sqr\
	      -sqrt --sqrt\
	      -- ---\
	      -sub --sub\
	      -tan --tan\
	      -tanh --tanh\
	      -xor --xor\
	      -apply_curve --apply_curve\
	      -apply_gamma --apply_gamma\
	      -balance_gamma --balance_gamma\
	      -complex2polar --complex2polar\
	      -compress_rle --compress_rle\
	      -cumul --cumul\
	      -c --c\
	      -cut --cut\
	      -discard --discard\
	      -eigen2tensor --eigen2tensor\
	      -endian --endian\
	      -equalize --equalize\
	      -f --f\
	      -fill --fill\
	      -float2int8 --float2int8\
	      -int82float --int82float\
	      -index --index\
	      -image_integral --image_integral\
	      -map --map\
	      -map_clut --map_clut\
	      -mix_channels --mix_channels\
	      -negative --negative\
	      -noise --noise\
	      -norm --norm\
	      -n --n\
	      -normalize --normalize\
	      -normalize_sum --normalize_sum\
	      -not --not\
	      -orientation --orientation\
	      -otsu --otsu\
	      -polar2complex --polar2complex\
	      -quantize --quantize\
	      -rand --rand\
	      -replace --replace\
	      -replace_inf --replace_inf\
	      -replace_nan --replace_nan\
	      -replace_seq --replace_seq\
	      -replace_str --replace_str\
	      -round --round\
	      -roundify --roundify\
	      -= --=\
	      -set --set\
	      -threshold --threshold\
	      -t2 --t2\
	      -threshold2 --threshold2\
	      -uncompress_rle --uncompress_rle\
	      -unrepeat --unrepeat\
	      -vector2tensor --vector2tensor\
	      -apply_channels --apply_channels\
	      -autoindex --autoindex\
	      -bayer2rgb --bayer2rgb\
	      -cmy2rgb --cmy2rgb\
	      -cmyk2rgb --cmyk2rgb\
	      -colormap --colormap\
	      -compose_channels --compose_channels\
	      -direction2rgb --direction2rgb\
	      -ditheredbw --ditheredbw\
	      -fc --fc\
	      -fill_color --fill_color\
	      -gradient2rgb --gradient2rgb\
	      -hsi2rgb --hsi2rgb\
	      -hsi82rgb --hsi82rgb\
	      -hsl2rgb --hsl2rgb\
	      -hsl82rgb --hsl82rgb\
	      -hsv2rgb --hsv2rgb\
	      -hsv82rgb --hsv82rgb\
	      -lab2lch --lab2lch\
	      -lab2rgb --lab2rgb\
	      -lab82rgb --lab82rgb\
	      -lch2lab --lch2lab\
	      -lch2rgb --lch2rgb\
	      -lch82rgb --lch82rgb\
	      -luminance --luminance\
	      -mix_rgb --mix_rgb\
	      -pseudogray --pseudogray\
	      -replace_color --replace_color\
	      -rgb2bayer --rgb2bayer\
	      -rgb2cmy --rgb2cmy\
	      -rgb2cmyk --rgb2cmyk\
	      -rgb2hsi --rgb2hsi\
	      -rgb2hsi8 --rgb2hsi8\
	      -rgb2hsl --rgb2hsl\
	      -rgb2hsl8 --rgb2hsl8\
	      -rgb2hsv --rgb2hsv\
	      -rgb2hsv8 --rgb2hsv8\
	      -rgb2lab --rgb2lab\
	      -rgb2lab8 --rgb2lab8\
	      -rgb2lch --rgb2lch\
	      -rgb2lch8 --rgb2lch8\
	      -rgb2luv --rgb2luv\
	      -rgb2srgb --rgb2srgb\
	      -rgb2xyz --rgb2xyz\
	      -rgb2xyz8 --rgb2xyz8\
	      -rgb2ycbcr --rgb2ycbcr\
	      -rgb2yuv --rgb2yuv\
	      -rgb2yuv8 --rgb2yuv8\
	      -remove_opacity --remove_opacity\
	      -select_color --select_color\
	      -sepia --sepia\
	      -solarize --solarize\
	      -split_opacity --split_opacity\
	      -srgb2rgb --srgb2rgb\
	      -to_a --to_a\
	      -to_color --to_color\
	      -to_colormode --to_colormode\
	      -to_gray --to_gray\
	      -to_graya --to_graya\
	      -to_pseudogray --to_pseudogray\
	      -to_rgb --to_rgb\
	      -to_rgba --to_rgba\
	      -transfer_colors --transfer_colors\
	      -xyz2rgb --xyz2rgb\
	      -xyz82rgb --xyz82rgb\
	      -ycbcr2rgb --ycbcr2rgb\
	      -yuv2rgb --yuv2rgb\
	      -yuv82rgb --yuv82rgb\
	      -a --a\
	      -append --append\
	      -append_tiles --append_tiles\
	      -autocrop --autocrop\
	      -autocrop_components --autocrop_components\
	      -autocrop_seq --autocrop_seq\
	      -channels --channels\
	      -columns --columns\
	      -z --z\
	      -crop --crop\
	      -diagonal --diagonal\
	      -elevate --elevate\
	      -expand_x --expand_x\
	      -expand_xy --expand_xy\
	      -expand_xyz --expand_xyz\
	      -expand_y --expand_y\
	      -expand_z --expand_z\
	      -montage --montage\
	      -mirror --mirror\
	      -permute --permute\
	      -r --r\
	      -resize --resize\
	      -pow2 --pow2\
	      -rr2d --rr2d\
	      -resize_ratio2d --resize_ratio2d\
	      -r2dx --r2dx\
	      -resize2dx --resize2dx\
	      -r2dy --r2dy\
	      -resize2dy --resize2dy\
	      -r3dx --r3dx\
	      -resize3dx --resize3dx\
	      -r3dy --r3dy\
	      -resize3dy --resize3dy\
	      -r3dz --r3dz\
	      -resize3dz --resize3dz\
	      -rotate --rotate\
	      -rotate_tileable --rotate_tileable\
	      -rows --rows\
	      -scale2x --scale2x\
	      -scale3x --scale3x\
	      -seamcarve --seamcarve\
	      -shift --shift\
	      -shrink_x --shrink_x\
	      -shrink_xy --shrink_xy\
	      -shrink_xyz --shrink_xyz\
	      -shrink_y --shrink_y\
	      -shrink_z --shrink_z\
	      -slices --slices\
	      -sort --sort\
	      -s --s\
	      -split --split\
	      -split_tiles --split_tiles\
	      -y --y\
	      -unroll --unroll\
	      -upscale_smart --upscale_smart\
	      -warp --warp\
	      -bandpass --bandpass\
	      -bilateral --bilateral\
	      -b --b\
	      -blur --blur\
	      -blur_angular --blur_angular\
	      -blur_linear --blur_linear\
	      -blur_radial --blur_radial\
	      -blur_selective --blur_selective\
	      -blur_x --blur_x\
	      -blur_xy --blur_xy\
	      -blur_xyz --blur_xyz\
	      -blur_y --blur_y\
	      -blur_z --blur_z\
	      -bokeh --bokeh\
	      -compose_freq --compose_freq\
	      -convolve --convolve\
	      -convolve_fft --convolve_fft\
	      -correlate --correlate\
	      -cross_correlation --cross_correlation\
	      -curvature --curvature\
	      -deblur --deblur\
	      -deblur_goldmeinel --deblur_goldmeinel\
	      -deblur_richardsonlucy --deblur_richardsonlucy\
	      -deconvolve_fft --deconvolve_fft\
	      -deinterlace --deinterlace\
	      -denoise --denoise\
	      -denoise_haar --denoise_haar\
	      -deriche --deriche\
	      -dilate --dilate\
	      -dilate_circ --dilate_circ\
	      -dilate_oct --dilate_oct\
	      -divergence --divergence\
	      -dog --dog\
	      -diffusiontensors --diffusiontensors\
	      -edges --edges\
	      -erode --erode\
	      -erode_circ --erode_circ\
	      -erode_oct --erode_oct\
	      -fft --fft\
	      -g --g\
	      -gradient --gradient\
	      -gradient_orientation --gradient_orientation\
	      -gradient_norm --gradient_norm\
	      -haar --haar\
	      -heat_flow --heat_flow\
	      -hessian --hessian\
	      -iee --iee\
	      -ifft --ifft\
	      -ihaar --ihaar\
	      -inn --inn\
	      -inpaint --inpaint\
	      -inpaint_flow --inpaint_flow\
	      -inpaint_holes --inpaint_holes\
	      -kuwahara --kuwahara\
	      -laplacian --laplacian\
	      -lic --lic\
	      -map_tones --map_tones\
	      -map_tones_fast --map_tones_fast\
	      -meancurvature_flow --meancurvature_flow\
	      -median --median\
	      -normalize_local --normalize_local\
	      -normalized_cross_correlation --normalized_cross_correlation\
	      -phase_correlation --phase_correlation\
	      -pde_flow --pde_flow\
	      -periodize_poisson --periodize_poisson\
	      -red_eye --red_eye\
	      -remove_hotpixels --remove_hotpixels\
	      -remove_pixels --remove_pixels\
	      -sharpen --sharpen\
	      -smooth --smooth\
	      -split_freq --split_freq\
	      -solidify --solidify\
	      -solidify_linear --solidify_linear\
	      -solidify_watershed --solidify_watershed\
	      -solve_poisson --solve_poisson\
	      -split_details --split_details\
	      -structuretensors --structuretensors\
	      -syntexturize --syntexturize\
	      -tv_flow --tv_flow\
	      -unsharp --unsharp\
	      -unsharp_octave --unsharp_octave\
	      -vanvliet --vanvliet\
	      -watermark_fourier --watermark_fourier\
	      -watershed --watershed\
	      -area --area\
	      -area_fg --area_fg\
	      -at_line --at_line\
	      -barycenter --barycenter\
	      -detect_skin --detect_skin\
	      -displacement --displacement\
	      -distance --distance\
	      -float2fft8 --float2fft8\
	      -fft82float --fft82float\
	      -fftpolar --fftpolar\
	      -histogram --histogram\
	      -histogram3 --histogram3\
	      -histogram_cumul --histogram_cumul\
	      -histogram_pointwise --histogram_pointwise\
	      -hough --hough\
	      -ifftpolar --ifftpolar\
	      -isophotes --isophotes\
	      -label --label\
	      -label_fg --label_fg\
	      -max_patch --max_patch\
	      -min_patch --min_patch\
	      -minimal_path --minimal_path\
	      -mse --mse\
	      -patches --patches\
	      -plot2value --plot2value\
	      -pointcloud --pointcloud\
	      -psnr --psnr\
	      -segment_watershed --segment_watershed\
	      -skeleton --skeleton\
	      -ssd_patch --ssd_patch\
	      -thinning --thinning\
	      -tones --tones\
	      -topographic_map --topographic_map\
	      -axes --axes\
	      -ball --ball\
	      -chessboard --chessboard\
	      -cie1931 --cie1931\
	      -circle --circle\
	      -ellipse --ellipse\
	      -flood --flood\
	      -gaussian --gaussian\
	      -graph --graph\
	      -grid --grid\
	      -j --j\
	      -image --image\
	      -line --line\
	      -mandelbrot --mandelbrot\
	      -marble --marble\
	      -maze --maze\
	      -maze_mask --maze_mask\
	      -object3d --object3d\
	      -pack_sprites --pack_sprites\
	      -piechart --piechart\
	      -plasma --plasma\
	      -point --point\
	      -polka_dots --polka_dots\
	      -polygon --polygon\
	      -quiver --quiver\
	      -rectangle --rectangle\
	      -rorschach --rorschach\
	      -sierpinski --sierpinski\
	      -snowflake --snowflake\
	      -spiralbw --spiralbw\
	      -spline --spline\
	      -t --t\
	      -text --text\
	      -text_outline --text_outline\
	      -triangle_shade --triangle_shade\
	      -truchet --truchet\
	      -turbulence --turbulence\
	      -yinyang --yinyang\
	      -dijkstra --dijkstra\
	      -eigen --eigen\
	      -invert --invert\
	      -solve --solve\
	      -svd --svd\
	      -transpose --transpose\
	      -trisolve --trisolve\
	      -+3d --+3d\
	      -add3d --add3d\
	      -animate3d --animate3d\
	      -apply_camera3d --apply_camera3d\
	      -apply_pose3d --apply_pose3d\
	      -arrow3d --arrow3d\
	      -axes3d --axes3d\
	      -b3d --b3d\
	      -background3d --background3d\
	      -box3d --box3d\
	      -c3d --c3d\
	      -center3d --center3d\
	      -circle3d --circle3d\
	      -circles3d --circles3d\
	      -col3d --col3d\
	      -color3d --color3d\
	      -colorcube3d --colorcube3d\
	      -cone3d --cone3d\
	      -cubes3d --cubes3d\
	      -cup3d --cup3d\
	      -cylinder3d --cylinder3d\
	      -distribution3d --distribution3d\
	      -/3d --/3d\
	      -div3d --div3d\
	      -db3d --db3d\
	      -double3d --double3d\
	      -elevation3d --elevation3d\
	      -empty3d --empty3d\
	      -extrude3d --extrude3d\
	      -f3d --f3d\
	      -focale3d --focale3d\
	      -gaussians3d --gaussians3d\
	      -gmic3d --gmic3d\
	      -gyroid3d --gyroid3d\
	      -histogram3d --histogram3d\
	      -image6cube3d --image6cube3d\
	      -imageblocks3d --imageblocks3d\
	      -imagecube3d --imagecube3d\
	      -imageplane3d --imageplane3d\
	      -imagepyramid3d --imagepyramid3d\
	      -imagerubik3d --imagerubik3d\
	      -imagesphere3d --imagesphere3d\
	      -isoline3d --isoline3d\
	      -isosurface3d --isosurface3d\
	      -label_points3d --label_points3d\
	      -lathe3d --lathe3d\
	      -l3d --l3d\
	      -light3d --light3d\
	      -line3d --line3d\
	      -lissajous3d --lissajous3d\
	      -m3d --m3d\
	      -mode3d --mode3d\
	      -md3d --md3d\
	      -moded3d --moded3d\
	      -*3d --*3d\
	      -mul3d --mul3d\
	      -n3d --n3d\
	      -normalize3d --normalize3d\
	      -o3d --o3d\
	      -opacity3d --opacity3d\
	      -parametric3d --parametric3d\
	      -pca_patch3d --pca_patch3d\
	      -plane3d --plane3d\
	      -point3d --point3d\
	      -pointcloud3d --pointcloud3d\
	      -pose3d --pose3d\
	      -p3d --p3d\
	      -primitives3d --primitives3d\
	      -projections3d --projections3d\
	      -pyramid3d --pyramid3d\
	      -quadrangle3d --quadrangle3d\
	      -rv3d --rv3d\
	      -reverse3d --reverse3d\
	      -r3d --r3d\
	      -rotate3d --rotate3d\
	      -rotation3d --rotation3d\
	      -sierpinski3d --sierpinski3d\
	      -skeleton3d --skeleton3d\
	      -snapshot3d --snapshot3d\
	      -sl3d --sl3d\
	      -specl3d --specl3d\
	      -ss3d --ss3d\
	      -specs3d --specs3d\
	      -sphere3d --sphere3d\
	      -spherical3d --spherical3d\
	      -spline3d --spline3d\
	      -s3d --s3d\
	      -split3d --split3d\
	      -sprite3d --sprite3d\
	      -sprites3d --sprites3d\
	      -star3d --star3d\
	      -streamline3d --streamline3d\
	      --3d ---3d\
	      -sub3d --sub3d\
	      -superformula3d --superformula3d\
	      -text_pointcloud3d --text_pointcloud3d\
	      -text3d --text3d\
	      -t3d --t3d\
	      -texturize3d --texturize3d\
	      -torus3d --torus3d\
	      -triangle3d --triangle3d\
	      -volume3d --volume3d\
	      -weird3d --weird3d\
	      -apply_parallel --apply_parallel\
	      -apply_parallel_channels --apply_parallel_channels\
	      -apply_parallel_overlap --apply_parallel_overlap\
	      -check --check\
	      -check3d --check3d\
	      -continue --continue\
	      -break --break\
	      -do --do\
	      -done --done\
	      -elif --elif\
	      -else --else\
	      -endif --endif\
	      -endl --endl\
	      -endlocal --endlocal\
	      -error --error\
	      -x --x\
	      -exec --exec\
	      -if --if\
	      -l --l\
	      -local --local\
	      -mutex --mutex\
	      -onfail --onfail\
	      -parallel --parallel\
	      -progress --progress\
	      -q --q\
	      -quit --quit\
	      -repeat --repeat\
	      -return --return\
	      -rprogress --rprogress\
	      -skip --skip\
	      -u --u\
	      -status --status\
	      -while --while\
	      -array --array\
	      -array_fade --array_fade\
	      -array_mirror --array_mirror\
	      -array_random --array_random\
	      -frame --frame\
	      -frame_blur --frame_blur\
	      -frame_cube --frame_cube\
	      -frame_fuzzy --frame_fuzzy\
	      -frame_painting --frame_painting\
	      -frame_pattern --frame_pattern\
	      -frame_round --frame_round\
	      -frame_x --frame_x\
	      -frame_xy --frame_xy\
	      -frame_xyz --frame_xyz\
	      -frame_y --frame_y\
	      -img2ascii --img2ascii\
	      -imagegrid --imagegrid\
	      -linearize_tiles --linearize_tiles\
	      -map_sprites --map_sprites\
	      -puzzle --puzzle\
	      -quadratize_tiles --quadratize_tiles\
	      -rotate_tiles --rotate_tiles\
	      -shift_tiles --shift_tiles\
	      -taquin --taquin\
	      -tunnel --tunnel\
	      -boxfitting --boxfitting\
	      -cartoon --cartoon\
	      -circlism --circlism\
	      -color_ellipses --color_ellipses\
	      -cubism --cubism\
	      -draw_whirl --draw_whirl\
	      -drawing --drawing\
	      -drop_shadow --drop_shadow\
	      -ellipsionism --ellipsionism\
	      -fire_edges --fire_edges\
	      -fractalize --fractalize\
	      -glow --glow\
	      -halftone --halftone\
	      -hardsketchbw --hardsketchbw\
	      -hearts --hearts\
	      -houghsketchbw --houghsketchbw\
	      -lightrays --lightrays\
	      -light_relief --light_relief\
	      -mosaic --mosaic\
	      -old_photo --old_photo\
	      -pencilbw --pencilbw\
	      -polaroid --polaroid\
	      -polygonize --polygonize\
	      -poster_edges --poster_edges\
	      -poster_hope --poster_hope\
	      -rodilius --rodilius\
	      -stained_glass --stained_glass\
	      -star --star\
	      -stars --stars\
	      -sketchbw --sketchbw\
	      -sponge --sponge\
	      -stencil --stencil\
	      -stencilbw --stencilbw\
	      -tetris --tetris\
	      -warhol --warhol\
	      -weave --weave\
	      -whirls --whirls\
	      -euclidean2polar --euclidean2polar\
	      -deform --deform\
	      -fisheye --fisheye\
	      -flower --flower\
	      -kaleidoscope --kaleidoscope\
	      -map_sphere --map_sphere\
	      -polar2euclidean --polar2euclidean\
	      -raindrops --raindrops\
	      -ripple --ripple\
	      -rotoidoscope --rotoidoscope\
	      -symmetrize --symmetrize\
	      -transform_polar --transform_polar\
	      -twirl --twirl\
	      -warp_perspective --warp_perspective\
	      -water --water\
	      -wave --wave\
	      -wind --wind\
	      -zoom --zoom\
	      -cracks --cracks\
	      -light_patch --light_patch\
	      -noise_hurl --noise_hurl\
	      -pixelize --pixelize\
	      -shade_stripes --shade_stripes\
	      -shadow_patch --shadow_patch\
	      -spread --spread\
	      -stripes_y --stripes_y\
	      -texturize_canvas --texturize_canvas\
	      -texturize_paper --texturize_paper\
	      -vignette --vignette\
	      -watermark_visible --watermark_visible\
	      -blend --blend\
	      -blend_edges --blend_edges\
	      -blend_fade --blend_fade\
	      -blend_median --blend_median\
	      -blend_seamless --blend_seamless\
	      -fade_diamond --fade_diamond\
	      -fade_linear --fade_linear\
	      -fade_radial --fade_radial\
	      -fade_x --fade_x\
	      -fade_y --fade_y\
	      -fade_z --fade_z\
	      -sub_alpha --sub_alpha\
	      -animate --animate\
	      -morph --morph\
	      -register_nonrigid --register_nonrigid\
	      -register_rigid --register_rigid\
	      -transition_plasma --transition_plasma\
	      -transition3d --transition3d\
	      -demo --demo\
	      -x_2048 --x_2048\
	      -x_blobs --x_blobs\
	      -x_bouncing --x_bouncing\
	      -x_fire --x_fire\
	      -x_fireworks --x_fireworks\
	      -x_fisheye --x_fisheye\
	      -x_fourier --x_fourier\
	      -x_histogram --x_histogram\
	      -x_hough --x_hough\
	      -x_jawbreaker --x_jawbreaker\
	      -x_life --x_life\
	      -x_light --x_light\
	      -x_mandelbrot --x_mandelbrot\
	      -x_metaballs3d --x_metaballs3d\
	      -x_minesweeper --x_minesweeper\
	      -x_minimal_path --x_minimal_path\
	      -x_pacman --x_pacman\
	      -x_paint --x_paint\
	      -x_plasma --x_plasma\
	      -x_quantize_rgb --x_quantize_rgb\
	      -x_reflection3d --x_reflection3d\
	      -x_rubber3d --x_rubber3d\
	      -x_shadebobs --x_shadebobs\
	      -x_spline --x_spline\
	      -x_tetris --x_tetris\
	      -x_tictactoe --x_tictactoe\
	      -x_waves --x_waves\
	      -x_whirl --x_whirl\
	      -output_pink3d --output_pink3d\
	      -pink --pink\
	      -pink_grayskel --pink_grayskel\
	      -pink_heightmaxima --pink_heightmaxima\
	      -pink_heightminima --pink_heightminima\
	      -pink_htkern --pink_htkern\
	      -pink_lvkern --pink_lvkern\
	      -pink_reg_minima --pink_reg_minima\
	      -pink_skelcurv --pink_skelcurv\
	      -pink_skelend --pink_skelend\
	      -pink_skeleton --pink_skeleton\
	      -pink_skelpar --pink_skelpar\
	      -pink_wshed --pink_wshed\
	      -alert --alert\
	      -arg --arg\
	      -at --at\
	      -autocrop_coords --autocrop_coords\
	      -average_color --average_color\
	      -basename --basename\
	      -bin --bin\
	      -bin2dec --bin2dec\
	      -dec --dec\
	      -dec2str --dec2str\
	      -dec2bin --dec2bin\
	      -dec2hex --dec2hex\
	      -dec2oct --dec2oct\
	      -fact --fact\
	      -file_mv --file_mv\
	      -file_rand --file_rand\
	      -file_rm --file_rm\
	      -file_slash --file_slash\
	      -filename --filename\
	      -fitratio_wh --fitratio_wh\
	      -fitscreen --fitscreen\
	      -gcd --gcd\
	      -hex --hex\
	      -hex2dec --hex2dec\
	      -hex2str --hex2str\
	      -img2str --img2str\
	      -img2text --img2text\
	      -img82hex --img82hex\
	      -hex2img8 --hex2img8\
	      -is_3d --is_3d\
	      -is_percent --is_percent\
	      -is_windows --is_windows\
	      -mad --mad\
	      -max_w --max_w\
	      -max_h --max_h\
	      -max_d --max_d\
	      -max_s --max_s\
	      -max_wh --max_wh\
	      -max_whd --max_whd\
	      -max_whds --max_whds\
	      -med --med\
	      -color_med --color_med\
	      -min_w --min_w\
	      -min_h --min_h\
	      -min_d --min_d\
	      -min_s --min_s\
	      -min_wh --min_wh\
	      -min_whd --min_whd\
	      -min_whds --min_whds\
	      -normalize_filename --normalize_filename\
	      -oct --oct\
	      -oct2dec --oct2dec\
	      -padint --padint\
	      -path_gimp --path_gimp\
	      -path_tmp --path_tmp\
	      -path_user --path_user\
	      -quote --quote\
	      -region_feature --region_feature\
	      -reset --reset\
	      -RGB --RGB\
	      -RGBA --RGBA\
	      -str --str\
	      -str2hex --str2hex\
	      -stresc --stresc\
	      -strcat --strcat\
	      -strcmp --strcmp\
	      -strlen --strlen\
	      -strreplace --strreplace\
	      -struncase --struncase\
	      -strver --strver\
	      -tic --tic\
	      -toc --toc\
	      -variance_noise --variance_noise\
	     "

	coms="\
	      debug\
	      h\
	      help\
	      version\
	      apply_camera\
	      apply_files\
	      camera\
	      m\
	      command\
	      cupid\
	      cursor\
	      d\
	      display\
	      d0\
	      display0\
	      d3d\
	      display3d\
	      da\
	      display_array\
	      dfft\
	      display_fft\
	      dg\
	      display_graph\
	      dh\
	      display_histogram\
	      display_parametric\
	      dp\
	      display_polar\
	      drgba\
	      display_rgba\
	      dt\
	      display_tensors\
	      dw\
	      display_warp\
	      document_gmic\
	      e\
	      echo\
	      echo_file\
	      echo_stdout\
	      function1d\
	      gmicky\
	      gmicky_wilber\
	      heart\
	      i\
	      input\
	      o\
	      output\
	      output_ggr\
	      on\
	      outputn\
	      op\
	      outputp\
	      ow\
	      outputw\
	      plot\
	      p\
	      print\
	      rainbow_lut\
	      roddy\
	      remove_duplicates\
	      remove_empty\
	      select\
	      sh\
	      shared\
	      srand\
	      testimage2d\
	      text2img\
	      type\
	      uncommand\
	      uniform_distribution\
	      up\
	      update\
	      v\
	      verbose\
	      wait\
	      warn\
	      w\
	      window\
	      k\
	      keep\
	      mv\
	      move\
	      nm\
	      name\
	      rm\
	      remove\
	      rv\
	      reverse\
	      sort_list\
	      sort_str\
	      abs\
	      acos\
	      +\
	      add\
	      and\
	      asin\
	      atan\
	      atan2\
	      <<\
	      bsl\
	      >>\
	      bsr\
	      cos\
	      cosh\
	      /\
	      div\
	      ==\
	      eq\
	      exp\
	      >=\
	      ge\
	      >\
	      gt\
	      <=\
	      le\
	      <\
	      lt\
	      log\
	      log10\
	      log2\
	      max\
	      //\
	      mdiv\
	      min\
	      %\
	      mod\
	      **\
	      mmul\
	      *\
	      mul\
	      !=\
	      neq\
	      or\
	      ^\
	      pow\
	      rol\
	      ror\
	      sign\
	      sin\
	      sinc\
	      sinh\
	      sqr\
	      sqrt\
	      -\
	      sub\
	      tan\
	      tanh\
	      xor\
	      apply_curve\
	      apply_gamma\
	      balance_gamma\
	      complex2polar\
	      compress_rle\
	      cumul\
	      c\
	      cut\
	      discard\
	      eigen2tensor\
	      endian\
	      equalize\
	      f\
	      fill\
	      float2int8\
	      int82float\
	      index\
	      image_integral\
	      map\
	      map_clut\
	      mix_channels\
	      negative\
	      noise\
	      norm\
	      n\
	      normalize\
	      normalize_sum\
	      not\
	      orientation\
	      otsu\
	      polar2complex\
	      quantize\
	      rand\
	      replace\
	      replace_inf\
	      replace_nan\
	      replace_seq\
	      replace_str\
	      round\
	      roundify\
	      =\
	      set\
	      threshold\
	      t2\
	      threshold2\
	      uncompress_rle\
	      unrepeat\
	      vector2tensor\
	      apply_channels\
	      autoindex\
	      bayer2rgb\
	      cmy2rgb\
	      cmyk2rgb\
	      colormap\
	      compose_channels\
	      direction2rgb\
	      ditheredbw\
	      fc\
	      fill_color\
	      gradient2rgb\
	      hsi2rgb\
	      hsi82rgb\
	      hsl2rgb\
	      hsl82rgb\
	      hsv2rgb\
	      hsv82rgb\
	      lab2lch\
	      lab2rgb\
	      lab82rgb\
	      lch2lab\
	      lch2rgb\
	      lch82rgb\
	      luminance\
	      mix_rgb\
	      pseudogray\
	      replace_color\
	      rgb2bayer\
	      rgb2cmy\
	      rgb2cmyk\
	      rgb2hsi\
	      rgb2hsi8\
	      rgb2hsl\
	      rgb2hsl8\
	      rgb2hsv\
	      rgb2hsv8\
	      rgb2lab\
	      rgb2lab8\
	      rgb2lch\
	      rgb2lch8\
	      rgb2luv\
	      rgb2srgb\
	      rgb2xyz\
	      rgb2xyz8\
	      rgb2ycbcr\
	      rgb2yuv\
	      rgb2yuv8\
	      remove_opacity\
	      select_color\
	      sepia\
	      solarize\
	      split_opacity\
	      srgb2rgb\
	      to_a\
	      to_color\
	      to_colormode\
	      to_gray\
	      to_graya\
	      to_pseudogray\
	      to_rgb\
	      to_rgba\
	      transfer_colors\
	      xyz2rgb\
	      xyz82rgb\
	      ycbcr2rgb\
	      yuv2rgb\
	      yuv82rgb\
	      a\
	      append\
	      append_tiles\
	      autocrop\
	      autocrop_components\
	      autocrop_seq\
	      channels\
	      columns\
	      z\
	      crop\
	      diagonal\
	      elevate\
	      expand_x\
	      expand_xy\
	      expand_xyz\
	      expand_y\
	      expand_z\
	      montage\
	      mirror\
	      permute\
	      r\
	      resize\
	      pow2\
	      rr2d\
	      resize_ratio2d\
	      r2dx\
	      resize2dx\
	      r2dy\
	      resize2dy\
	      r3dx\
	      resize3dx\
	      r3dy\
	      resize3dy\
	      r3dz\
	      resize3dz\
	      rotate\
	      rotate_tileable\
	      rows\
	      scale2x\
	      scale3x\
	      seamcarve\
	      shift\
	      shrink_x\
	      shrink_xy\
	      shrink_xyz\
	      shrink_y\
	      shrink_z\
	      slices\
	      sort\
	      s\
	      split\
	      split_tiles\
	      y\
	      unroll\
	      upscale_smart\
	      warp\
	      bandpass\
	      bilateral\
	      b\
	      blur\
	      blur_angular\
	      blur_linear\
	      blur_radial\
	      blur_selective\
	      blur_x\
	      blur_xy\
	      blur_xyz\
	      blur_y\
	      blur_z\
	      bokeh\
	      compose_freq\
	      convolve\
	      convolve_fft\
	      correlate\
	      cross_correlation\
	      curvature\
	      deblur\
	      deblur_goldmeinel\
	      deblur_richardsonlucy\
	      deconvolve_fft\
	      deinterlace\
	      denoise\
	      denoise_haar\
	      deriche\
	      dilate\
	      dilate_circ\
	      dilate_oct\
	      divergence\
	      dog\
	      diffusiontensors\
	      edges\
	      erode\
	      erode_circ\
	      erode_oct\
	      fft\
	      g\
	      gradient\
	      gradient_orientation\
	      gradient_norm\
	      haar\
	      heat_flow\
	      hessian\
	      iee\
	      ifft\
	      ihaar\
	      inn\
	      inpaint\
	      inpaint_flow\
	      inpaint_holes\
	      kuwahara\
	      laplacian\
	      lic\
	      map_tones\
	      map_tones_fast\
	      meancurvature_flow\
	      median\
	      normalize_local\
	      normalized_cross_correlation\
	      phase_correlation\
	      pde_flow\
	      periodize_poisson\
	      red_eye\
	      remove_hotpixels\
	      remove_pixels\
	      sharpen\
	      smooth\
	      split_freq\
	      solidify\
	      solidify_linear\
	      solidify_watershed\
	      solve_poisson\
	      split_details\
	      structuretensors\
	      syntexturize\
	      tv_flow\
	      unsharp\
	      unsharp_octave\
	      vanvliet\
	      watermark_fourier\
	      watershed\
	      area\
	      area_fg\
	      at_line\
	      barycenter\
	      detect_skin\
	      displacement\
	      distance\
	      float2fft8\
	      fft82float\
	      fftpolar\
	      histogram\
	      histogram3\
	      histogram_cumul\
	      histogram_pointwise\
	      hough\
	      ifftpolar\
	      isophotes\
	      label\
	      label_fg\
	      max_patch\
	      min_patch\
	      minimal_path\
	      mse\
	      patches\
	      plot2value\
	      pointcloud\
	      psnr\
	      segment_watershed\
	      skeleton\
	      ssd_patch\
	      thinning\
	      tones\
	      topographic_map\
	      axes\
	      ball\
	      chessboard\
	      cie1931\
	      circle\
	      ellipse\
	      flood\
	      gaussian\
	      graph\
	      grid\
	      j\
	      image\
	      line\
	      mandelbrot\
	      marble\
	      maze\
	      maze_mask\
	      object3d\
	      pack_sprites\
	      piechart\
	      plasma\
	      point\
	      polka_dots\
	      polygon\
	      quiver\
	      rectangle\
	      rorschach\
	      sierpinski\
	      snowflake\
	      spiralbw\
	      spline\
	      t\
	      text\
	      text_outline\
	      triangle_shade\
	      truchet\
	      turbulence\
	      yinyang\
	      dijkstra\
	      eigen\
	      invert\
	      solve\
	      svd\
	      transpose\
	      trisolve\
	      +3d\
	      add3d\
	      animate3d\
	      apply_camera3d\
	      apply_pose3d\
	      arrow3d\
	      axes3d\
	      b3d\
	      background3d\
	      box3d\
	      c3d\
	      center3d\
	      circle3d\
	      circles3d\
	      col3d\
	      color3d\
	      colorcube3d\
	      cone3d\
	      cubes3d\
	      cup3d\
	      cylinder3d\
	      distribution3d\
	      /3d\
	      div3d\
	      db3d\
	      double3d\
	      elevation3d\
	      empty3d\
	      extrude3d\
	      f3d\
	      focale3d\
	      gaussians3d\
	      gmic3d\
	      gyroid3d\
	      histogram3d\
	      image6cube3d\
	      imageblocks3d\
	      imagecube3d\
	      imageplane3d\
	      imagepyramid3d\
	      imagerubik3d\
	      imagesphere3d\
	      isoline3d\
	      isosurface3d\
	      label_points3d\
	      lathe3d\
	      l3d\
	      light3d\
	      line3d\
	      lissajous3d\
	      m3d\
	      mode3d\
	      md3d\
	      moded3d\
	      *3d\
	      mul3d\
	      n3d\
	      normalize3d\
	      o3d\
	      opacity3d\
	      parametric3d\
	      pca_patch3d\
	      plane3d\
	      point3d\
	      pointcloud3d\
	      pose3d\
	      p3d\
	      primitives3d\
	      projections3d\
	      pyramid3d\
	      quadrangle3d\
	      rv3d\
	      reverse3d\
	      r3d\
	      rotate3d\
	      rotation3d\
	      sierpinski3d\
	      skeleton3d\
	      snapshot3d\
	      sl3d\
	      specl3d\
	      ss3d\
	      specs3d\
	      sphere3d\
	      spherical3d\
	      spline3d\
	      s3d\
	      split3d\
	      sprite3d\
	      sprites3d\
	      star3d\
	      streamline3d\
	      -3d\
	      sub3d\
	      superformula3d\
	      text_pointcloud3d\
	      text3d\
	      t3d\
	      texturize3d\
	      torus3d\
	      triangle3d\
	      volume3d\
	      weird3d\
	      apply_parallel\
	      apply_parallel_channels\
	      apply_parallel_overlap\
	      check\
	      check3d\
	      continue\
	      break\
	      do\
	      done\
	      elif\
	      else\
	      endif\
	      endl\
	      endlocal\
	      error\
	      x\
	      exec\
	      if\
	      l\
	      local\
	      mutex\
	      onfail\
	      parallel\
	      progress\
	      q\
	      quit\
	      repeat\
	      return\
	      rprogress\
	      skip\
	      u\
	      status\
	      while\
	      array\
	      array_fade\
	      array_mirror\
	      array_random\
	      frame\
	      frame_blur\
	      frame_cube\
	      frame_fuzzy\
	      frame_painting\
	      frame_pattern\
	      frame_round\
	      frame_x\
	      frame_xy\
	      frame_xyz\
	      frame_y\
	      img2ascii\
	      imagegrid\
	      linearize_tiles\
	      map_sprites\
	      puzzle\
	      quadratize_tiles\
	      rotate_tiles\
	      shift_tiles\
	      taquin\
	      tunnel\
	      boxfitting\
	      cartoon\
	      circlism\
	      color_ellipses\
	      cubism\
	      draw_whirl\
	      drawing\
	      drop_shadow\
	      ellipsionism\
	      fire_edges\
	      fractalize\
	      glow\
	      halftone\
	      hardsketchbw\
	      hearts\
	      houghsketchbw\
	      lightrays\
	      light_relief\
	      mosaic\
	      old_photo\
	      pencilbw\
	      polaroid\
	      polygonize\
	      poster_edges\
	      poster_hope\
	      rodilius\
	      stained_glass\
	      star\
	      stars\
	      sketchbw\
	      sponge\
	      stencil\
	      stencilbw\
	      tetris\
	      warhol\
	      weave\
	      whirls\
	      euclidean2polar\
	      deform\
	      fisheye\
	      flower\
	      kaleidoscope\
	      map_sphere\
	      polar2euclidean\
	      raindrops\
	      ripple\
	      rotoidoscope\
	      symmetrize\
	      transform_polar\
	      twirl\
	      warp_perspective\
	      water\
	      wave\
	      wind\
	      zoom\
	      cracks\
	      light_patch\
	      noise_hurl\
	      pixelize\
	      shade_stripes\
	      shadow_patch\
	      spread\
	      stripes_y\
	      texturize_canvas\
	      texturize_paper\
	      vignette\
	      watermark_visible\
	      blend\
	      blend_edges\
	      blend_fade\
	      blend_median\
	      blend_seamless\
	      fade_diamond\
	      fade_linear\
	      fade_radial\
	      fade_x\
	      fade_y\
	      fade_z\
	      sub_alpha\
	      animate\
	      morph\
	      register_nonrigid\
	      register_rigid\
	      transition_plasma\
	      transition3d\
	      demo\
	      x_2048\
	      x_blobs\
	      x_bouncing\
	      x_fire\
	      x_fireworks\
	      x_fisheye\
	      x_fourier\
	      x_histogram\
	      x_hough\
	      x_jawbreaker\
	      x_life\
	      x_light\
	      x_mandelbrot\
	      x_metaballs3d\
	      x_minesweeper\
	      x_minimal_path\
	      x_pacman\
	      x_paint\
	      x_plasma\
	      x_quantize_rgb\
	      x_reflection3d\
	      x_rubber3d\
	      x_shadebobs\
	      x_spline\
	      x_tetris\
	      x_tictactoe\
	      x_waves\
	      x_whirl\
	      output_pink3d\
	      pink\
	      pink_grayskel\
	      pink_heightmaxima\
	      pink_heightminima\
	      pink_htkern\
	      pink_lvkern\
	      pink_reg_minima\
	      pink_skelcurv\
	      pink_skelend\
	      pink_skeleton\
	      pink_skelpar\
	      pink_wshed\
	      alert\
	      arg\
	      at\
	      autocrop_coords\
	      average_color\
	      basename\
	      bin\
	      bin2dec\
	      dec\
	      dec2str\
	      dec2bin\
	      dec2hex\
	      dec2oct\
	      fact\
	      file_mv\
	      file_rand\
	      file_rm\
	      file_slash\
	      filename\
	      fitratio_wh\
	      fitscreen\
	      gcd\
	      hex\
	      hex2dec\
	      hex2str\
	      img2str\
	      img2text\
	      img82hex\
	      hex2img8\
	      is_3d\
	      is_percent\
	      is_windows\
	      mad\
	      max_w\
	      max_h\
	      max_d\
	      max_s\
	      max_wh\
	      max_whd\
	      max_whds\
	      med\
	      color_med\
	      min_w\
	      min_h\
	      min_d\
	      min_s\
	      min_wh\
	      min_whd\
	      min_whds\
	      normalize_filename\
	      oct\
	      oct2dec\
	      padint\
	      path_gimp\
	      path_tmp\
	      path_user\
	      quote\
	      region_feature\
	      reset\
	      RGB\
	      RGBA\
	      str\
	      str2hex\
	      stresc\
	      strcat\
	      strcmp\
	      strlen\
	      strreplace\
	      struncase\
	      strver\
	      tic\
	      toc\
	      variance_noise\
	     "

	case "${prev}" in
		"-help" | "--help")
		COMPREPLY=( $(compgen -W "$coms" -- "$cur") )
		return 0
		;;
		"-apply_camera" | "--apply_camera")
		COMPREPLY=( $(compgen -W "_command,_camera_index>=0,_skip_frames>=0,_output_filename >") )
		return 0
		;;
		"-apply_files" | "--apply_files")
		COMPREPLY=( $(compgen -W ""command",list_of_filenames,_output_prefix,_output_extension,_view_window={0|1} >") )
		return 0
		;;
		"-camera" | "--camera")
		COMPREPLY=( $(compgen -W "_camera_index>=0,_nb_frames>0,_skip_frames>=0,_capture_width>=0,_capture_height>=0 >") )
		return 0
		;;
		"-cupid" | "--cupid")
		COMPREPLY=( $(compgen -W "_size>0 >") )
		return 0
		;;
		"-cursor" | "--cursor")
		COMPREPLY=( $(compgen -W "_mode={0=hide|1=show} >") )
		return 0
		;;
		"-display" | "--display")
		COMPREPLY=( $(compgen -W "_X,_Y,_Z >") )
		return 0
		;;
		"-display_array" | "--display_array")
		COMPREPLY=( $(compgen -W "_width>0,_height>0 >") )
		return 0
		;;
		"-display_graph" | "--display_graph")
		COMPREPLY=( $(compgen -W "_width>32,_height>32,_plot_type,_vertex_type,_xmin,_xmax,_ymin,_ymax,_xlabel,_ylabel >") )
		return 0
		;;
		"-display_histogram" | "--display_histogram")
		COMPREPLY=( $(compgen -W "_width>0,_height>0,_clusters>0,_min_value[%],_max_value[%],_show_axes={0|1},_expression. >") )
		return 0
		;;
		"-display_parametric" | "--display_parametric")
		COMPREPLY=( $(compgen -W "_width>0,_height>0,_outline_opacity,_vertex_radius>=0,_is_antialiased={0|1},_is_decorated={0|1},_xlabel,_ylabel >") )
		return 0
		;;
		"-display_polar" | "--display_polar")
		COMPREPLY=( $(compgen -W "_width>32,_height>32,_outline_type,_fill_R,_fill_G,_fill_B,_theta_start,_theta_end,_xlabel,_ylabel >") )
		return 0
		;;
		"-display_tensors" | "--display_tensors")
		COMPREPLY=( $(compgen -W "_size_factor>0,_ellipse_factor>=0,_colored_mode={0|1} >") )
		return 0
		;;
		"-display_warp" | "--display_warp")
		COMPREPLY=( $(compgen -W "_cell_size>0 >") )
		return 0
		;;
		"-document_gmic" | "--document_gmic")
		COMPREPLY=( $(compgen -W "_format={ascii|html|latex|xml|bash|images},_image_path,_write_wrapper={0|1} >") )
		return 0
		;;
		"-echo" | "--echo")
		COMPREPLY=( $(compgen -W "message >") )
		return 0
		;;
		"-echo_file" | "--echo_file")
		COMPREPLY=( $(compgen -W "filename,message >") )
		return 0
		;;
		"-echo_stdout" | "--echo_stdout")
		COMPREPLY=( $(compgen -W "message >") )
		return 0
		;;
		"-function1d" | "--function1d")
		COMPREPLY=( $(compgen -W "0<=smoothness<=1,x0>=0,y0,x1>=0,y1,...,xn>=0,yn >") )
		return 0
		;;
		"-heart" | "--heart")
		COMPREPLY=( $(compgen -W "_width>0,_height>0 >") )
		return 0
		;;
		"-output_ggr" | "--output_ggr")
		COMPREPLY=( $(compgen -W "filename,_gradient_name >") )
		return 0
		;;
		"-outputn" | "--outputn")
		COMPREPLY=( $(compgen -W "filename >") )
		return 0
		;;
		"-outputp" | "--outputp")
		COMPREPLY=( $(compgen -W "prefix >") )
		return 0
		;;
		"-plot" | "--plot")
		COMPREPLY=( $(compgen -W "_plot_type,_vertex_type,_xmin,_xmax,_ymin,_ymax 'formula',_resolution>=0,_plot_type,_vertex_type,_xmin,xmax,_ymin,_ymax") )
		return 0
		;;
		"-select" | "--select")
		COMPREPLY=( $(compgen -W "feature_type,_X,_Y,_Z >") )
		return 0
		;;
		"-shared" | "--shared")
		COMPREPLY=( $(compgen -W "x0[%],x1[%],y[%],z[%],v[%] y0[%],y1[%],z[%],v[%] z0[%],z1[%],v[%] v0[%],v1[%] (noargs)") )
		return 0
		;;
		"-srand" | "--srand")
		COMPREPLY=( $(compgen -W "value (noargs)") )
		return 0
		;;
		"-testimage2d" | "--testimage2d")
		COMPREPLY=( $(compgen -W "_width>0,_height>0,_spectrum>0 >") )
		return 0
		;;
		"-text2img" | "--text2img")
		COMPREPLY=( $(compgen -W "text,_line_separator >") )
		return 0
		;;
		"-type" | "--type")
		COMPREPLY=( $(compgen -W "datatype >") )
		return 0
		;;
		"-uncommand" | "--uncommand")
		COMPREPLY=( $(compgen -W "command_name[,_command_name2,...] *") )
		return 0
		;;
		"-uniform_distribution" | "--uniform_distribution")
		COMPREPLY=( $(compgen -W "nb_levels>=1,spectrum>=1 >") )
		return 0
		;;
		"-verbose" | "--verbose")
		COMPREPLY=( $(compgen -W "_level {+|-}") )
		return 0
		;;
		"-wait" | "--wait")
		COMPREPLY=( $(compgen -W "delay (noargs)") )
		return 0
		;;
		"-warn" | "--warn")
		COMPREPLY=( $(compgen -W "message >") )
		return 0
		;;
		"-window" | "--window")
		COMPREPLY=( $(compgen -W "_width[%]>=-1,_height[%]>=-1,_normalization,_fullscreen,_pos_x[%],_pos_y[%],_title >") )
		return 0
		;;
		"-move" | "--move")
		COMPREPLY=( $(compgen -W "position[%] >") )
		return 0
		;;
		"-name" | "--name")
		COMPREPLY=( $(compgen -W "name,_is_modified={0|1} >") )
		return 0
		;;
		"-sort_list" | "--sort_list")
		COMPREPLY=( $(compgen -W "_ordering={+|-},_criterion >") )
		return 0
		;;
		"-add" | "--add")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-and" | "--and")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-atan2" | "--atan2")
		COMPREPLY=( $(compgen -W "[x_argument] >") )
		return 0
		;;
		"-bsl" | "--bsl")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-bsr" | "--bsr")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-div" | "--div")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-eq" | "--eq")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-ge" | "--ge")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-gt" | "--gt")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-le" | "--le")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-lt" | "--lt")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-max" | "--max")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-mdiv" | "--mdiv")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-min" | "--min")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-mod" | "--mod")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-mmul" | "--mmul")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-mul" | "--mul")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-neq" | "--neq")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-or" | "--or")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-pow" | "--pow")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-rol" | "--rol")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-ror" | "--ror")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-sub" | "--sub")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-xor" | "--xor")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-apply_curve" | "--apply_curve")
		COMPREPLY=( $(compgen -W "0<=smoothness<=1,x0,y0,x1,y1,x2,y2,...,xN,yN >") )
		return 0
		;;
		"-apply_gamma" | "--apply_gamma")
		COMPREPLY=( $(compgen -W "gamma>=0 >") )
		return 0
		;;
		"-balance_gamma" | "--balance_gamma")
		COMPREPLY=( $(compgen -W "_ref_color1,... >") )
		return 0
		;;
		"-compress_rle" | "--compress_rle")
		COMPREPLY=( $(compgen -W "_is_binary_data={0|1},_maximum_sequence_length>=0 >") )
		return 0
		;;
		"-cut" | "--cut")
		COMPREPLY=( $(compgen -W "{value0[%]|[image0]},{value1[%]|[image1]} [image] (noargs)") )
		return 0
		;;
		"-discard" | "--discard")
		COMPREPLY=( $(compgen -W "value >") )
		return 0
		;;
		"-endian" | "--endian")
		COMPREPLY=( $(compgen -W "_datatype >") )
		return 0
		;;
		"-equalize" | "--equalize")
		COMPREPLY=( $(compgen -W "_nb_levels>0[%],_value_min[%],_value_max[%] >") )
		return 0
		;;
		"-fill" | "--fill")
		COMPREPLY=( $(compgen -W "value1,_value2,.. [image] 'formula'") )
		return 0
		;;
		"-index" | "--index")
		COMPREPLY=( $(compgen -W "{[palette]|predefined_palette},0<=_dithering<=1,_map_palette={0|1} >") )
		return 0
		;;
		"-map" | "--map")
		COMPREPLY=( $(compgen -W "[palette] predefined_palette") )
		return 0
		;;
		"-mix_channels" | "--mix_channels")
		COMPREPLY=( $(compgen -W "(a00,...,aMN) >") )
		return 0
		;;
		"-noise" | "--noise")
		COMPREPLY=( $(compgen -W "std_variation>=0[%],_noise_type >") )
		return 0
		;;
		"-normalize" | "--normalize")
		COMPREPLY=( $(compgen -W "{value0[%]|[image0]},{value1[%]|[image1]} [image]") )
		return 0
		;;
		"-otsu" | "--otsu")
		COMPREPLY=( $(compgen -W "_nb_levels>0 >") )
		return 0
		;;
		"-quantize" | "--quantize")
		COMPREPLY=( $(compgen -W "nb_levels>=1,_keep_values={0|1},_is_uniform={0|1} >") )
		return 0
		;;
		"-rand" | "--rand")
		COMPREPLY=( $(compgen -W "{value0[%]|[image0]},{value1[%]|[image1]} [image]") )
		return 0
		;;
		"-replace" | "--replace")
		COMPREPLY=( $(compgen -W "value_src,value_dest >") )
		return 0
		;;
		"-replace_inf" | "--replace_inf")
		COMPREPLY=( $(compgen -W "_expression >") )
		return 0
		;;
		"-replace_nan" | "--replace_nan")
		COMPREPLY=( $(compgen -W "_expression >") )
		return 0
		;;
		"-replace_seq" | "--replace_seq")
		COMPREPLY=( $(compgen -W ""search_seq","replace_seq" >") )
		return 0
		;;
		"-replace_str" | "--replace_str")
		COMPREPLY=( $(compgen -W ""search_str","replace_str" >") )
		return 0
		;;
		"-round" | "--round")
		COMPREPLY=( $(compgen -W "rounding_value>=0,_rounding_type (noargs)") )
		return 0
		;;
		"-roundify" | "--roundify")
		COMPREPLY=( $(compgen -W "gamma>=0 >") )
		return 0
		;;
		"-set" | "--set")
		COMPREPLY=( $(compgen -W "value,_x[%],_y[%],_z[%],_c[%] >") )
		return 0
		;;
		"-threshold" | "--threshold")
		COMPREPLY=( $(compgen -W "value[%],_is_soft (noargs)") )
		return 0
		;;
		"-threshold2" | "--threshold2")
		COMPREPLY=( $(compgen -W "min[%],max[%] >") )
		return 0
		;;
		"-apply_channels" | "--apply_channels")
		COMPREPLY=( $(compgen -W ""command",_channels={all=0|rgba=1|rgb=2|y=3|cbcr=4|cb=5|cr=6|l=7|ab=8|a=9|b=10|h=11|s=12|v=13|k=14|cg=15|ch=16|c=17|H=18|r=19|g=20|b=21|alpha==22},_normalize={0=cut|1=normalize} >") )
		return 0
		;;
		"-autoindex" | "--autoindex")
		COMPREPLY=( $(compgen -W "nb_colors>0,0<=_dithering<=1,_method={0=median-cut|1=k-means} >") )
		return 0
		;;
		"-bayer2rgb" | "--bayer2rgb")
		COMPREPLY=( $(compgen -W "_GM_smoothness,_RB_smoothness1,_RB_smoothness2 >") )
		return 0
		;;
		"-colormap" | "--colormap")
		COMPREPLY=( $(compgen -W "nb_levels>=1,_method={0=median-cut|1=k-means},_sort_vectors={0|1} >") )
		return 0
		;;
		"-fill_color" | "--fill_color")
		COMPREPLY=( $(compgen -W "col1,...,colN >") )
		return 0
		;;
		"-gradient2rgb" | "--gradient2rgb")
		COMPREPLY=( $(compgen -W "_is_orientation={0|1} >") )
		return 0
		;;
		"-mix_rgb" | "--mix_rgb")
		COMPREPLY=( $(compgen -W "a11,a12,a13,a21,a22,a23,a31,a32,a33 >") )
		return 0
		;;
		"-pseudogray" | "--pseudogray")
		COMPREPLY=( $(compgen -W "_max_increment>=0,_JND_threshold>=0,_bits_depth>0 >") )
		return 0
		;;
		"-replace_color" | "--replace_color")
		COMPREPLY=( $(compgen -W "tolerance[%]>=0,smoothness[%]>=0,src1,src2,...,dest1,dest2,... >") )
		return 0
		;;
		"-rgb2bayer" | "--rgb2bayer")
		COMPREPLY=( $(compgen -W "_start_pattern=0,_color_grid=0 >") )
		return 0
		;;
		"-select_color" | "--select_color")
		COMPREPLY=( $(compgen -W "tolerance[%]>=0,col1,..,colN >") )
		return 0
		;;
		"-to_colormode" | "--to_colormode")
		COMPREPLY=( $(compgen -W "mode={0=adaptive|1=G|2=GA|3=RGB|4=RGBA} >") )
		return 0
		;;
		"-to_pseudogray" | "--to_pseudogray")
		COMPREPLY=( $(compgen -W "_max_step>=0,_is_perceptual_constraint={0|1},_bits_depth>0 >") )
		return 0
		;;
		"-transfer_colors" | "--transfer_colors")
		COMPREPLY=( $(compgen -W "_transfer_brightness={0|1} >") )
		return 0
		;;
		"-append" | "--append")
		COMPREPLY=( $(compgen -W "[image],axis,_alignment axis,_alignment") )
		return 0
		;;
		"-append_tiles" | "--append_tiles")
		COMPREPLY=( $(compgen -W "_M>=0,_N>=0,0<=_x_alignment<=1,0<=_y_alignment<=1 >") )
		return 0
		;;
		"-autocrop" | "--autocrop")
		COMPREPLY=( $(compgen -W "value1,value2,... (noargs)") )
		return 0
		;;
		"-autocrop_components" | "--autocrop_components")
		COMPREPLY=( $(compgen -W "_threshold[%],_min_area[%]>=0,_is_high_connectivity={0|1},_output_type={0=crop|1=segmentation|2=coordinates} >") )
		return 0
		;;
		"-autocrop_seq" | "--autocrop_seq")
		COMPREPLY=( $(compgen -W "value1,value2,...|auto >") )
		return 0
		;;
		"-channels" | "--channels")
		COMPREPLY=( $(compgen -W "{[image0]|c0[%]},_{[image1]|c1[%]} >") )
		return 0
		;;
		"-columns" | "--columns")
		COMPREPLY=( $(compgen -W "{[image0]|x0[%]},_{[image1]|x1[%]} >") )
		return 0
		;;
		"-crop" | "--crop")
		COMPREPLY=( $(compgen -W "x0[%],x1[%],_boundary x0[%],y0[%],x1[%],y1[%],_boundary x0[%],y0[%],z0[%],x1[%],y1[%],z1[%],_boundary x0[%],y0[%],z0[%],c0[%],x1[%],y1[%],z1[%],c1[%],_boundary (noargs)") )
		return 0
		;;
		"-elevate" | "--elevate")
		COMPREPLY=( $(compgen -W "_depth,_is_plain,_is_colored >") )
		return 0
		;;
		"-expand_x" | "--expand_x")
		COMPREPLY=( $(compgen -W "size_x>=0,_boundary={0=dirichlet|1=neumann|2=periodic} >") )
		return 0
		;;
		"-expand_xy" | "--expand_xy")
		COMPREPLY=( $(compgen -W "size>=0,_boundary={0=dirichlet|1=neumann|2=periodic} >") )
		return 0
		;;
		"-expand_xyz" | "--expand_xyz")
		COMPREPLY=( $(compgen -W "size>=0,_boundary={0=dirichlet|1=neumann|2=periodic} >") )
		return 0
		;;
		"-expand_y" | "--expand_y")
		COMPREPLY=( $(compgen -W "size_y>=0,_boundary={0=dirichlet|1=neumann|2=periodic} >") )
		return 0
		;;
		"-expand_z" | "--expand_z")
		COMPREPLY=( $(compgen -W "size_z>=0,_boundary={0=dirichlet|1=neumann|2=periodic} >") )
		return 0
		;;
		"-montage" | "--montage")
		COMPREPLY=( $(compgen -W ""_layout_code",_montage_mode={0<=alignment<=1|2<=scale+2<=3},_output_mode={0=singlelayer|1=multiplelayers},"_processing_command" >") )
		return 0
		;;
		"-mirror" | "--mirror")
		COMPREPLY=( $(compgen -W "{x|y|z}..{x|y|z} >") )
		return 0
		;;
		"-permute" | "--permute")
		COMPREPLY=( $(compgen -W "permutation_string >") )
		return 0
		;;
		"-resize" | "--resize")
		COMPREPLY=( $(compgen -W "[image],_interpolation,_boundary,_ax,_ay,_az,_ac {[image_w]|width>0[%]},_{[image_h]|height>0[%]},_{[image_d]|depth>0[%]},_{[image_s]|spectrum>0[%]},_interpolation,_boundary,_ax,_ay,_az,_ac (noargs)") )
		return 0
		;;
		"-pow2" | "--pow2")
		COMPREPLY=( $(compgen -W "_interpolation,_boundary,_ax,_ay,_az,_ac >") )
		return 0
		;;
		"-resize_ratio2d" | "--resize_ratio2d")
		COMPREPLY=( $(compgen -W "width>0,height>0,_mode={0=inside|1=outside|2=padded},0=<_interpolation<=6 >") )
		return 0
		;;
		"-resize2dx" | "--resize2dx")
		COMPREPLY=( $(compgen -W "width[%]>0,_interpolation,_boundary,_ax,_ay,_az,_ac >") )
		return 0
		;;
		"-resize2dy" | "--resize2dy")
		COMPREPLY=( $(compgen -W "height[%]>=0,_interpolation,_boundary,_ax,_ay,_az,_ac >") )
		return 0
		;;
		"-resize3dx" | "--resize3dx")
		COMPREPLY=( $(compgen -W "width[%]>0,_interpolation,_boundary,_ax,_ay,_az,_ac >") )
		return 0
		;;
		"-resize3dy" | "--resize3dy")
		COMPREPLY=( $(compgen -W "height[%]>0,_interpolation,_boundary,_ax,_ay,_az,_ac >") )
		return 0
		;;
		"-resize3dz" | "--resize3dz")
		COMPREPLY=( $(compgen -W "depth[%]>0,_interpolation,_boundary,_ax,_ay,_az,_ac >") )
		return 0
		;;
		"-rotate" | "--rotate")
		COMPREPLY=( $(compgen -W "angle,_interpolation,_boundary,_cx[%],_cy[%],_zoom >") )
		return 0
		;;
		"-rotate_tileable" | "--rotate_tileable")
		COMPREPLY=( $(compgen -W "angle,_max_size_factor>=0 >") )
		return 0
		;;
		"-rows" | "--rows")
		COMPREPLY=( $(compgen -W "{[image0]|y0[%]},_{[image1]|y1[%]} >") )
		return 0
		;;
		"-seamcarve" | "--seamcarve")
		COMPREPLY=( $(compgen -W "_width[%]>=0,_height[%]>=0,_is_priority_channel={0|1},_is_antialiasing={0|1},_maximum_seams[%]>=0 >") )
		return 0
		;;
		"-shift" | "--shift")
		COMPREPLY=( $(compgen -W "vx[%],_vy[%],_vz[%],_vc[%],_boundary >") )
		return 0
		;;
		"-shrink_x" | "--shrink_x")
		COMPREPLY=( $(compgen -W "size_x>=0 >") )
		return 0
		;;
		"-shrink_xy" | "--shrink_xy")
		COMPREPLY=( $(compgen -W "size>=0 >") )
		return 0
		;;
		"-shrink_xyz" | "--shrink_xyz")
		COMPREPLY=( $(compgen -W "size>=0 >") )
		return 0
		;;
		"-shrink_y" | "--shrink_y")
		COMPREPLY=( $(compgen -W "size_y>=0 >") )
		return 0
		;;
		"-shrink_z" | "--shrink_z")
		COMPREPLY=( $(compgen -W "size_z>=0 >") )
		return 0
		;;
		"-slices" | "--slices")
		COMPREPLY=( $(compgen -W "{[image0]|z0[%]},_{[image1]|z1[%]} >") )
		return 0
		;;
		"-sort" | "--sort")
		COMPREPLY=( $(compgen -W "_ordering={+|-},_axis={x|y|z|c} >") )
		return 0
		;;
		"-split" | "--split")
		COMPREPLY=( $(compgen -W "{x|y|z|c}..{x|y|z|c},_nb_parts keep_splitting_values={+|-},value1,value2,... (noargs)") )
		return 0
		;;
		"-split_tiles" | "--split_tiles")
		COMPREPLY=( $(compgen -W "M!=0,_N!=0,_is_homogeneous={0|1} >") )
		return 0
		;;
		"-unroll" | "--unroll")
		COMPREPLY=( $(compgen -W "_axis={x|y|z|c} >") )
		return 0
		;;
		"-upscale_smart" | "--upscale_smart")
		COMPREPLY=( $(compgen -W "width,_height,_depth,_smoothness>=0,_anisotropy=[0,1],sharpening>=0 >") )
		return 0
		;;
		"-warp" | "--warp")
		COMPREPLY=( $(compgen -W "[warping_field],_is_relative={0|1},_interpolation,_boundary,_nb_frames>0 >") )
		return 0
		;;
		"-bandpass" | "--bandpass")
		COMPREPLY=( $(compgen -W "_min_freq[%],_max_freq[%] >") )
		return 0
		;;
		"-bilateral" | "--bilateral")
		COMPREPLY=( $(compgen -W "[guide],std_variation_s>0[%],std_variation_r[%]>0 std_variation_s>0[%],std_variation_r[%]>0") )
		return 0
		;;
		"-blur" | "--blur")
		COMPREPLY=( $(compgen -W "std_variation>=0[%],_boundary={0=dirichlet|1=neumann},_kernel={0=quasi-gaussian(faster)|1=gaussian} >") )
		return 0
		;;
		"-blur_angular" | "--blur_angular")
		COMPREPLY=( $(compgen -W "amplitude[%],_cx,_cy >") )
		return 0
		;;
		"-blur_linear" | "--blur_linear")
		COMPREPLY=( $(compgen -W "amplitude1[%],_amplitude2[%],_angle,_boundary={0=dirichlet|1=neumann} >") )
		return 0
		;;
		"-blur_radial" | "--blur_radial")
		COMPREPLY=( $(compgen -W "amplitude[%],_cx,_cy >") )
		return 0
		;;
		"-blur_selective" | "--blur_selective")
		COMPREPLY=( $(compgen -W "sigma>=0,_edges>0,_nb_scales>0 >") )
		return 0
		;;
		"-blur_x" | "--blur_x")
		COMPREPLY=( $(compgen -W "amplitude[%]>=0,_boundary={0=dirichlet|1=neumann} >") )
		return 0
		;;
		"-blur_xy" | "--blur_xy")
		COMPREPLY=( $(compgen -W "amplitude_x[%],amplitude_y[%],_boundary={0=dirichlet|1=neumann} >") )
		return 0
		;;
		"-blur_xyz" | "--blur_xyz")
		COMPREPLY=( $(compgen -W "amplitude_x[%],amplitude_y[%],amplitude_z,_boundary={0=dirichlet|1=neumann} >") )
		return 0
		;;
		"-blur_y" | "--blur_y")
		COMPREPLY=( $(compgen -W "amplitude[%]>=0,_boundary={0=dirichlet|1=neumann} >") )
		return 0
		;;
		"-blur_z" | "--blur_z")
		COMPREPLY=( $(compgen -W "amplitude[%]>=0,_boundary={0=dirichlet|1=neumann} >") )
		return 0
		;;
		"-bokeh" | "--bokeh")
		COMPREPLY=( $(compgen -W "_amplitude>=0,_smoothness>=0,0<=_density<=100,_bokeh_size>0,0<=_bokeh_outline_size<=100,_bokeh_outline_amplitude>=0,_bokeh_smoothness>=0 >") )
		return 0
		;;
		"-convolve" | "--convolve")
		COMPREPLY=( $(compgen -W "[mask],_boundary,_is_normalized={0|1} >") )
		return 0
		;;
		"-correlate" | "--correlate")
		COMPREPLY=( $(compgen -W "[mask],_boundary,_is_normalized={0|1} >") )
		return 0
		;;
		"-deblur" | "--deblur")
		COMPREPLY=( $(compgen -W "amplitude[%]>=0,_nb_iter>=0,_dt>=0,_regul>=0,_regul_type={0=Tikhonov|1=meancurv.|2=TV} >") )
		return 0
		;;
		"-deblur_goldmeinel" | "--deblur_goldmeinel")
		COMPREPLY=( $(compgen -W "sigma>=0,_nb_iter>=0,_acceleration>=0,_kernel_type={0=quasi-gaussian(faster)|1=gaussian}. >") )
		return 0
		;;
		"-deblur_richardsonlucy" | "--deblur_richardsonlucy")
		COMPREPLY=( $(compgen -W "sigma>=0,nb_iter>=0,_kernel_type={0=quasi-gaussian(faster)|1=gaussian}. >") )
		return 0
		;;
		"-deinterlace" | "--deinterlace")
		COMPREPLY=( $(compgen -W "_method={0|1} >") )
		return 0
		;;
		"-denoise" | "--denoise")
		COMPREPLY=( $(compgen -W "std_variation_s>=0,_std_variation_p>=0,_patch_size>=0,_lookup_size>=0,_smoothness,_fast_approx={0|1} >") )
		return 0
		;;
		"-denoise_haar" | "--denoise_haar")
		COMPREPLY=( $(compgen -W "_threshold>=0,_nb_scales>=0,_cycle_spinning>0 >") )
		return 0
		;;
		"-deriche" | "--deriche")
		COMPREPLY=( $(compgen -W "std_variation>=0[%],order={0|1|2},axis={x|y|z|c},_boundary >") )
		return 0
		;;
		"-dilate" | "--dilate")
		COMPREPLY=( $(compgen -W "size>=0 size_x>=0,size_y>=0,size_z>=0 [mask],_boundary,_is_normalized={0|1}") )
		return 0
		;;
		"-dilate_circ" | "--dilate_circ")
		COMPREPLY=( $(compgen -W "_size>=0,_boundary,_is_normalized={0|1} >") )
		return 0
		;;
		"-dilate_oct" | "--dilate_oct")
		COMPREPLY=( $(compgen -W "_size>=0,_boundary,_is_normalized={0|1} >") )
		return 0
		;;
		"-dog" | "--dog")
		COMPREPLY=( $(compgen -W "_sigma1>=0[%],_sigma2>=0[%] >") )
		return 0
		;;
		"-diffusiontensors" | "--diffusiontensors")
		COMPREPLY=( $(compgen -W "_sharpness>=0,0<=_anisotropy<=1,_alpha[%],_sigma[%],is_sqrt={0|1} >") )
		return 0
		;;
		"-edges" | "--edges")
		COMPREPLY=( $(compgen -W "_threshold[%]>=0 >") )
		return 0
		;;
		"-erode" | "--erode")
		COMPREPLY=( $(compgen -W "size>=0 size_x>=0,size_y>=0,_size_z>=0 [mask],_boundary,_is_normalized={0|1}") )
		return 0
		;;
		"-erode_circ" | "--erode_circ")
		COMPREPLY=( $(compgen -W "_size>=0,_boundary,_is_normalized={0|1} >") )
		return 0
		;;
		"-erode_oct" | "--erode_oct")
		COMPREPLY=( $(compgen -W "_size>=0,_boundary,_is_normalized={0|1} >") )
		return 0
		;;
		"-gradient" | "--gradient")
		COMPREPLY=( $(compgen -W "{x|y|z}..{x|y|z},_scheme (noargs)") )
		return 0
		;;
		"-gradient_orientation" | "--gradient_orientation")
		COMPREPLY=( $(compgen -W "_dimension={1,2,3} >") )
		return 0
		;;
		"-haar" | "--haar")
		COMPREPLY=( $(compgen -W "scale>0 >") )
		return 0
		;;
		"-heat_flow" | "--heat_flow")
		COMPREPLY=( $(compgen -W "_nb_iter>=0,_dt,_keep_sequence={0|1} >") )
		return 0
		;;
		"-hessian" | "--hessian")
		COMPREPLY=( $(compgen -W "{xx|xy|xz|yy|yz|zz}..{xx|xy|xz|yy|yz|zz} (noargs)") )
		return 0
		;;
		"-ihaar" | "--ihaar")
		COMPREPLY=( $(compgen -W "scale>0 >") )
		return 0
		;;
		"-inpaint" | "--inpaint")
		COMPREPLY=( $(compgen -W "[mask] [mask],0,_fast_method [mask],_patch_size>=1,_lookup_size>=1,_lookup_factor>=0,_lookup_increment!=0,_blend_size>=0,0<=_blend_threshold<=1,_blend_decay>=0,_blend_scales>=1,_is_blend_outer={0|1}") )
		return 0
		;;
		"-inpaint_flow" | "--inpaint_flow")
		COMPREPLY=( $(compgen -W "_nb_iter1>=0,_nb_iter2>=0,_dt>=0,_alpha,_sigma >") )
		return 0
		;;
		"-inpaint_holes" | "--inpaint_holes")
		COMPREPLY=( $(compgen -W "maximal_area[%]>=0,_tolerance>=0,_is_high_connectivity={0|1} >") )
		return 0
		;;
		"-kuwahara" | "--kuwahara")
		COMPREPLY=( $(compgen -W "size>0 >") )
		return 0
		;;
		"-lic" | "--lic")
		COMPREPLY=( $(compgen -W "_amplitude>0,_channels>0 >") )
		return 0
		;;
		"-map_tones" | "--map_tones")
		COMPREPLY=( $(compgen -W "_threshold>=0,_gamma>=0,_smoothness>=0,nb_iter>=0 >") )
		return 0
		;;
		"-map_tones_fast" | "--map_tones_fast")
		COMPREPLY=( $(compgen -W "_radius[%]>=0,_power>=0 >") )
		return 0
		;;
		"-meancurvature_flow" | "--meancurvature_flow")
		COMPREPLY=( $(compgen -W "_nb_iter>=0,_dt,_sequence_flag={0|1} >") )
		return 0
		;;
		"-median" | "--median")
		COMPREPLY=( $(compgen -W "size>=0 >") )
		return 0
		;;
		"-normalize_local" | "--normalize_local")
		COMPREPLY=( $(compgen -W "_amplitude>=0,_radius>0,_n_smooth>=0[%],_a_smooth>=0[%],_is_cut={0|1},_min=0,_max=255 >") )
		return 0
		;;
		"-pde_flow" | "--pde_flow")
		COMPREPLY=( $(compgen -W "_nb_iter>=0,_dt,_velocity_command,_keep_sequence={0|1} >") )
		return 0
		;;
		"-red_eye" | "--red_eye")
		COMPREPLY=( $(compgen -W "0<=_threshold<=100,_smoothness>=0,0<=attenuation<=1 >") )
		return 0
		;;
		"-remove_hotpixels" | "--remove_hotpixels")
		COMPREPLY=( $(compgen -W "_mask_size>0,_threshold[%]>0 >") )
		return 0
		;;
		"-remove_pixels" | "--remove_pixels")
		COMPREPLY=( $(compgen -W "density>=0,_pixel_sum>=0 >") )
		return 0
		;;
		"-sharpen" | "--sharpen")
		COMPREPLY=( $(compgen -W "amplitude>=0 amplitude>=0,edge>=0,_alpha,_sigma") )
		return 0
		;;
		"-smooth" | "--smooth")
		COMPREPLY=( $(compgen -W "amplitude>=0,_sharpness>=0,_anisotropy,_alpha,_sigma,_dl>0,_da>0,_precision>0,interpolation,_fast_approx={0|1} nb_iterations>=0,_sharpness>=0,_anisotropy,_alpha,_sigma,_dt>0,0 [tensor_field],_amplitude>=0,_dl>0,_da>0,_precision>0,_interpolation,_fast_approx={0|1} [tensor_field],_nb_iters>=0,_dt>0,0") )
		return 0
		;;
		"-split_freq" | "--split_freq")
		COMPREPLY=( $(compgen -W "smoothness>0[%] >") )
		return 0
		;;
		"-solidify_linear" | "--solidify_linear")
		COMPREPLY=( $(compgen -W "_sigma>=1,_dsigma>=1,0<=_precision<=1 >") )
		return 0
		;;
		"-solve_poisson" | "--solve_poisson")
		COMPREPLY=( $(compgen -W ""laplacian_command",_nb_iterations>=0,_time_step>0,_nb_scales>=0 >") )
		return 0
		;;
		"-split_details" | "--split_details")
		COMPREPLY=( $(compgen -W "_nb_scales>0,_base_scale[%]>=0,_detail_scale[%]>=0 >") )
		return 0
		;;
		"-structuretensors" | "--structuretensors")
		COMPREPLY=( $(compgen -W "_scheme >") )
		return 0
		;;
		"-syntexturize" | "--syntexturize")
		COMPREPLY=( $(compgen -W "_width[%]>0,_height[%]>0 >") )
		return 0
		;;
		"-tv_flow" | "--tv_flow")
		COMPREPLY=( $(compgen -W "_nb_iter>=0,_dt,_sequence_flag={0|1} >") )
		return 0
		;;
		"-unsharp" | "--unsharp")
		COMPREPLY=( $(compgen -W "radius[%]>=0,_amount>=0,_threshold[%]>=0 >") )
		return 0
		;;
		"-unsharp_octave" | "--unsharp_octave")
		COMPREPLY=( $(compgen -W "_nb_scales>0,_radius[%]>=0,_amount>=0,threshold[%]>=0 >") )
		return 0
		;;
		"-vanvliet" | "--vanvliet")
		COMPREPLY=( $(compgen -W "std_variation>=0[%],order={0|1|2|3},axis={x|y|z|c},_boundary >") )
		return 0
		;;
		"-watermark_fourier" | "--watermark_fourier")
		COMPREPLY=( $(compgen -W "text,_size>0 >") )
		return 0
		;;
		"-watershed" | "--watershed")
		COMPREPLY=( $(compgen -W "[priority_image],_fill_lines={0|1} >") )
		return 0
		;;
		"-area" | "--area")
		COMPREPLY=( $(compgen -W "tolerance>=0,is_high_connectivity={0|1} >") )
		return 0
		;;
		"-area_fg" | "--area_fg")
		COMPREPLY=( $(compgen -W "tolerance>=0,is_high_connectivity={0|1} >") )
		return 0
		;;
		"-at_line" | "--at_line")
		COMPREPLY=( $(compgen -W "x0[%],y0[%],z0[%],x1[%],y1[%],z1[%] >") )
		return 0
		;;
		"-detect_skin" | "--detect_skin")
		COMPREPLY=( $(compgen -W "0<=tolerance<=1,_skin_x,_skin_y,_skin_radius>=0 >") )
		return 0
		;;
		"-displacement" | "--displacement")
		COMPREPLY=( $(compgen -W "[source_image],_smoothness,_precision>=0,_nb_scales>=0,iteration_max>=0,is_backward={0|1} >") )
		return 0
		;;
		"-distance" | "--distance")
		COMPREPLY=( $(compgen -W "isovalue[%],_metric isovalue[%],[metric],_method") )
		return 0
		;;
		"-histogram" | "--histogram")
		COMPREPLY=( $(compgen -W "_nb_levels>0[%],_value0[%],_value1[%] >") )
		return 0
		;;
		"-histogram3" | "--histogram3")
		COMPREPLY=( $(compgen -W "nb_levels>0[%],_value0[%],_value1[%] >") )
		return 0
		;;
		"-histogram_cumul" | "--histogram_cumul")
		COMPREPLY=( $(compgen -W "_nb_levels>0,_is_normalized={0|1},_val0[%],_val1[%] >") )
		return 0
		;;
		"-histogram_pointwise" | "--histogram_pointwise")
		COMPREPLY=( $(compgen -W "nb_levels>0[%],_value0[%],_value1[%] >") )
		return 0
		;;
		"-hough" | "--hough")
		COMPREPLY=( $(compgen -W "_width>0,_height>0,gradient_norm_voting={0|1} >") )
		return 0
		;;
		"-isophotes" | "--isophotes")
		COMPREPLY=( $(compgen -W "_nb_levels>0 >") )
		return 0
		;;
		"-label" | "--label")
		COMPREPLY=( $(compgen -W "_tolerance>=0,is_high_connectivity={0|1} >") )
		return 0
		;;
		"-label_fg" | "--label_fg")
		COMPREPLY=( $(compgen -W "tolerance>=0,is_high_connectivity={0|1} >") )
		return 0
		;;
		"-max_patch" | "--max_patch")
		COMPREPLY=( $(compgen -W "_patch_size>=1 >") )
		return 0
		;;
		"-min_patch" | "--min_patch")
		COMPREPLY=( $(compgen -W "_patch_size>=1 >") )
		return 0
		;;
		"-minimal_path" | "--minimal_path")
		COMPREPLY=( $(compgen -W "x0[%]>=0,y0[%]>=0,z0[%]>=0,x1[%]>=0,y1[%]>=0,z1[%]>=0,_is_high_connectivity={0|1} >") )
		return 0
		;;
		"-patches" | "--patches")
		COMPREPLY=( $(compgen -W "patch_width>0,patch_height>0,patch_depth>0,x0,y0,z0,_x1,_y1,_z1,...,_xN,_yN,_zN >") )
		return 0
		;;
		"-pointcloud" | "--pointcloud")
		COMPREPLY=( $(compgen -W "_type={-X=-X-opacity|0=binary|1=cumulative|2=label},_width,_height>0,_depth>0 >") )
		return 0
		;;
		"-psnr" | "--psnr")
		COMPREPLY=( $(compgen -W "_max_value >") )
		return 0
		;;
		"-segment_watershed" | "--segment_watershed")
		COMPREPLY=( $(compgen -W "_threshold>=0,_fill_lines={0|1} >") )
		return 0
		;;
		"-skeleton" | "--skeleton")
		COMPREPLY=( $(compgen -W "_smoothness[%]>=0 >") )
		return 0
		;;
		"-ssd_patch" | "--ssd_patch")
		COMPREPLY=( $(compgen -W "_use_fourier={0|1},_boundary_conditions={0=dirichlet|1=neumann} >") )
		return 0
		;;
		"-tones" | "--tones")
		COMPREPLY=( $(compgen -W "N>0 >") )
		return 0
		;;
		"-topographic_map" | "--topographic_map")
		COMPREPLY=( $(compgen -W "_nb_levels>0,_smoothness >") )
		return 0
		;;
		"-axes" | "--axes")
		COMPREPLY=( $(compgen -W "x0,x1,y0,y1,_font_height>=0,_opacity,_pattern,_color1,.. >") )
		return 0
		;;
		"-ball" | "--ball")
		COMPREPLY=( $(compgen -W "_size>0,_R,_G,_B,0<=_specular_light<=8,0<=_specular_size<=8,_shadow>=0 >") )
		return 0
		;;
		"-chessboard" | "--chessboard")
		COMPREPLY=( $(compgen -W "size1>0,_size2>0,_offset1,_offset2,_angle,_opacity,_color1,..,_color2,.. >") )
		return 0
		;;
		"-circle" | "--circle")
		COMPREPLY=( $(compgen -W "x[%],y[%],R[%],_opacity,_pattern,_color1,.. >") )
		return 0
		;;
		"-ellipse" | "--ellipse")
		COMPREPLY=( $(compgen -W "x[%],y[%],R[%],r[%],_angle,_opacity,_pattern,_color1,.. >") )
		return 0
		;;
		"-flood" | "--flood")
		COMPREPLY=( $(compgen -W "x[%],_y[%],_z[%],_tolerance>=0,_is_high_connectivity={0|1},_opacity,_color1,.. >") )
		return 0
		;;
		"-gaussian" | "--gaussian")
		COMPREPLY=( $(compgen -W "_sigma1[%],_sigma2[%],_angle >") )
		return 0
		;;
		"-graph" | "--graph")
		COMPREPLY=( $(compgen -W "[function_image],_plot_type,_vertex_type,_ymin,_ymax,_opacity,_pattern,_color1,.. 'formula',_resolution>=0,_plot_type,_vertex_type,_xmin,xmax,_ymin,_ymax,_opacity,_pattern,_color1,..") )
		return 0
		;;
		"-grid" | "--grid")
		COMPREPLY=( $(compgen -W "size_x[%]>=0,size_y[%]>=0,_offset_x[%],_offset_y[%],_opacity,_pattern,_color1,.. >") )
		return 0
		;;
		"-image" | "--image")
		COMPREPLY=( $(compgen -W "[sprite],_x[%],_y[%],_z[%],_c[%],_opacity,_[sprite_mask],_max_opacity_mask >") )
		return 0
		;;
		"-line" | "--line")
		COMPREPLY=( $(compgen -W "x0[%],y0[%],x1[%],y1[%],_opacity,_pattern,_color1,.. >") )
		return 0
		;;
		"-mandelbrot" | "--mandelbrot")
		COMPREPLY=( $(compgen -W "z0r,z0i,z1r,z1i,_iteration_max>=0,_is_julia={0|1},_c0r,_c0i,_opacity >") )
		return 0
		;;
		"-marble" | "--marble")
		COMPREPLY=( $(compgen -W "_image_weight,_pattern_weight,_angle,_amplitude,_sharpness>=0,_anisotropy>=0,_alpha,_sigma,_cut_low>=0,_cut_high>=0 >") )
		return 0
		;;
		"-maze" | "--maze")
		COMPREPLY=( $(compgen -W "_width>0,_height>0,_cell_size>0 >") )
		return 0
		;;
		"-maze_mask" | "--maze_mask")
		COMPREPLY=( $(compgen -W "_cellsize>0 >") )
		return 0
		;;
		"-object3d" | "--object3d")
		COMPREPLY=( $(compgen -W "[object3d],_x[%],_y[%],_z,_opacity,_rendering_mode,_is_double_sided={0|1},_is_zbuffer={0|1},_focale,_light_x,_light_y,_light_z,_specular_lightness,_specular_shininess >") )
		return 0
		;;
		"-pack_sprites" | "--pack_sprites")
		COMPREPLY=( $(compgen -W "_nb_scales>=0,0<=_min_scale<=100,_allow_rotation={0=0deg.|1=180deg.|2=90deg.|3=any},_spacing,_precision>=0,max_iterations>=0 >") )
		return 0
		;;
		"-piechart" | "--piechart")
		COMPREPLY=( $(compgen -W "label_height>=0,label_R,label_G,label_B,"label1",value1,R1,G1,B1,...,"labelN",valueN,RN,GN,BN >") )
		return 0
		;;
		"-plasma" | "--plasma")
		COMPREPLY=( $(compgen -W "_alpha,__beta,_scale>=0 >") )
		return 0
		;;
		"-point" | "--point")
		COMPREPLY=( $(compgen -W "x[%],y[%],_z[%],_opacity,_color1,.. >") )
		return 0
		;;
		"-polka_dots" | "--polka_dots")
		COMPREPLY=( $(compgen -W "diameter>=0,_density,_offset1,_offset2,_angle,_aliasing,_shading,_opacity,_color,... >") )
		return 0
		;;
		"-polygon" | "--polygon")
		COMPREPLY=( $(compgen -W "N>=1,x1[%],y1[%],..,xN[%],yN[%],_opacity,_pattern,_color1,.. >") )
		return 0
		;;
		"-quiver" | "--quiver")
		COMPREPLY=( $(compgen -W "[function_image],_sampling>0,_factor,_is_arrow={0|1},_opacity,_pattern,_color1,.. >") )
		return 0
		;;
		"-rectangle" | "--rectangle")
		COMPREPLY=( $(compgen -W "x0[%],y0[%],x1[%],y1[%],_opacity,_pattern,_color1,.. >") )
		return 0
		;;
		"-rorschach" | "--rorschach")
		COMPREPLY=( $(compgen -W "'smoothness[%]>=0','mirroring={0=none|1=x|2=y|3=xy} >") )
		return 0
		;;
		"-sierpinski" | "--sierpinski")
		COMPREPLY=( $(compgen -W "recursion_level>=0 >") )
		return 0
		;;
		"-snowflake" | "--snowflake")
		COMPREPLY=( $(compgen -W "_recursion>=0,_x0,_y0,_x1,_y1,_x2,_y2,_opacity,_col1,..._colN >") )
		return 0
		;;
		"-spline" | "--spline")
		COMPREPLY=( $(compgen -W "x0[%],y0[%],u0[%],v0[%],x1[%],y1[%],u1[%],v1[%],_nb_vertices>=2,_opacity,_color1,.. >") )
		return 0
		;;
		"-text" | "--text")
		COMPREPLY=( $(compgen -W "text,_x[%],_y[%],_font_height>=0,_opacity,_color1,.. >") )
		return 0
		;;
		"-text_outline" | "--text_outline")
		COMPREPLY=( $(compgen -W "text,_x[%],_y[%],_font_height>0,_outline>=0,_opacity,_color1,.. >") )
		return 0
		;;
		"-triangle_shade" | "--triangle_shade")
		COMPREPLY=( $(compgen -W "x0,y0,x1,y0,x2,y2,R0,G0,B0,...,R1,G1,B1,...,R2,G2,B2,.... >") )
		return 0
		;;
		"-truchet" | "--truchet")
		COMPREPLY=( $(compgen -W "_scale>0,_radius>=0,_pattern_type={0=straight|1=curved} >") )
		return 0
		;;
		"-turbulence" | "--turbulence")
		COMPREPLY=( $(compgen -W "_radius>0,_octaves={1,2,3...,12},_alpha>0,_difference={-10,10},_mode={0,1,2,3} >") )
		return 0
		;;
		"-dijkstra" | "--dijkstra")
		COMPREPLY=( $(compgen -W "starting_node>=0,ending_node>=0 >") )
		return 0
		;;
		"-solve" | "--solve")
		COMPREPLY=( $(compgen -W "[image] >") )
		return 0
		;;
		"-trisolve" | "--trisolve")
		COMPREPLY=( $(compgen -W "[image] >") )
		return 0
		;;
		"-add3d" | "--add3d")
		COMPREPLY=( $(compgen -W "tx,_ty,_tz [object3d] (noargs)") )
		return 0
		;;
		"-animate3d" | "--animate3d")
		COMPREPLY=( $(compgen -W "_width>0,_height>0,_dx,_dy,_dz,_zoom>=0,_filename >") )
		return 0
		;;
		"-apply_camera3d" | "--apply_camera3d")
		COMPREPLY=( $(compgen -W "pos_x,pos_y,pos_z,target_x,target_y,target_z,up_x,up_y,up_z >") )
		return 0
		;;
		"-apply_pose3d" | "--apply_pose3d")
		COMPREPLY=( $(compgen -W "p1,..,p12 >") )
		return 0
		;;
		"-arrow3d" | "--arrow3d")
		COMPREPLY=( $(compgen -W "x0,y0,z0,x1,y1,z1,_radius[%]>=0,_head_length[%]>=0,_head_radius[%]>=0 >") )
		return 0
		;;
		"-axes3d" | "--axes3d")
		COMPREPLY=( $(compgen -W "_size_x,_size_y,_size_z,_font_size>0,_label_x,_label_y,_label_z >") )
		return 0
		;;
		"-background3d" | "--background3d")
		COMPREPLY=( $(compgen -W "R,_G,_B [image] (noargs)") )
		return 0
		;;
		"-box3d" | "--box3d")
		COMPREPLY=( $(compgen -W "_size_x,_size_y,_size_z >") )
		return 0
		;;
		"-circle3d" | "--circle3d")
		COMPREPLY=( $(compgen -W "_x0,_y0,_z0,_radius>=0 >") )
		return 0
		;;
		"-circles3d" | "--circles3d")
		COMPREPLY=( $(compgen -W "_radius>=0,_is_filled={0|1} >") )
		return 0
		;;
		"-color3d" | "--color3d")
		COMPREPLY=( $(compgen -W "R,_G,_B,_opacity >") )
		return 0
		;;
		"-cone3d" | "--cone3d")
		COMPREPLY=( $(compgen -W "_radius,_height,_nb_subdivisions>0 >") )
		return 0
		;;
		"-cubes3d" | "--cubes3d")
		COMPREPLY=( $(compgen -W "_size>=0 >") )
		return 0
		;;
		"-cup3d" | "--cup3d")
		COMPREPLY=( $(compgen -W "_resolution>0 >") )
		return 0
		;;
		"-cylinder3d" | "--cylinder3d")
		COMPREPLY=( $(compgen -W "_radius,_height,_nb_subdivisions>0 >") )
		return 0
		;;
		"-div3d" | "--div3d")
		COMPREPLY=( $(compgen -W "factor factor_x,factor_y,_factor_z") )
		return 0
		;;
		"-double3d" | "--double3d")
		COMPREPLY=( $(compgen -W "_is_double_sided={0|1} >") )
		return 0
		;;
		"-elevation3d" | "--elevation3d")
		COMPREPLY=( $(compgen -W "z-factor [elevation_map] 'formula' (noargs)") )
		return 0
		;;
		"-extrude3d" | "--extrude3d")
		COMPREPLY=( $(compgen -W "_depth>0,_resolution>0,_smoothness[%]>=0 >") )
		return 0
		;;
		"-focale3d" | "--focale3d")
		COMPREPLY=( $(compgen -W "focale >") )
		return 0
		;;
		"-gaussians3d" | "--gaussians3d")
		COMPREPLY=( $(compgen -W "_size>0,_opacity >") )
		return 0
		;;
		"-gyroid3d" | "--gyroid3d")
		COMPREPLY=( $(compgen -W "_resolution>0,_zoom >") )
		return 0
		;;
		"-imageblocks3d" | "--imageblocks3d")
		COMPREPLY=( $(compgen -W "_maximum_elevation,_smoothness[%]>=0 >") )
		return 0
		;;
		"-imagerubik3d" | "--imagerubik3d")
		COMPREPLY=( $(compgen -W "_xy_tiles>=1,0<=xy_shift<=100,0<=z_shift<=100 >") )
		return 0
		;;
		"-imagesphere3d" | "--imagesphere3d")
		COMPREPLY=( $(compgen -W "_resolution1>=3,_resolution2>=3 >") )
		return 0
		;;
		"-isoline3d" | "--isoline3d")
		COMPREPLY=( $(compgen -W "isovalue[%] 'formula',value,_x0,_y0,_x1,_y1,_size_x>0[%],_size_y>0[%]") )
		return 0
		;;
		"-isosurface3d" | "--isosurface3d")
		COMPREPLY=( $(compgen -W "isovalue[%] 'formula',value,_x0,_y0,_z0,_x1,_y1,_z1,_size_x>0[%],_size_y>0[%],_size_z>0[%]") )
		return 0
		;;
		"-label_points3d" | "--label_points3d")
		COMPREPLY=( $(compgen -W "_label_size>0,_opacity >") )
		return 0
		;;
		"-lathe3d" | "--lathe3d")
		COMPREPLY=( $(compgen -W "_resolution>0,_smoothness[%]>=0,_max_angle>=0 >") )
		return 0
		;;
		"-light3d" | "--light3d")
		COMPREPLY=( $(compgen -W "position_x,position_y,position_z [texture] (noargs)") )
		return 0
		;;
		"-line3d" | "--line3d")
		COMPREPLY=( $(compgen -W "x0,y0,z0,x1,y1,z1 >") )
		return 0
		;;
		"-lissajous3d" | "--lissajous3d")
		COMPREPLY=( $(compgen -W "resolution>1,a,A,b,B,c,C >") )
		return 0
		;;
		"-mode3d" | "--mode3d")
		COMPREPLY=( $(compgen -W "_mode >") )
		return 0
		;;
		"-moded3d" | "--moded3d")
		COMPREPLY=( $(compgen -W "_mode >") )
		return 0
		;;
		"-mul3d" | "--mul3d")
		COMPREPLY=( $(compgen -W "factor factor_x,factor_y,_factor_z") )
		return 0
		;;
		"-opacity3d" | "--opacity3d")
		COMPREPLY=( $(compgen -W "_opacity >") )
		return 0
		;;
		"-parametric3d" | "--parametric3d")
		COMPREPLY=( $(compgen -W "_x(a,b),_y(a,b),_z(a,b),_amin,_amax,_bmin,_bmax,_res_a>0,_res_b>0,_res_x>0,_res_y>0,_res_z>0,_smoothness>=0,_isovalue>=0 >") )
		return 0
		;;
		"-pca_patch3d" | "--pca_patch3d")
		COMPREPLY=( $(compgen -W "_patch_size>0,_M>0,_N>0,_normalize_input={0|1},_normalize_output={0|1},_lambda_xy >") )
		return 0
		;;
		"-plane3d" | "--plane3d")
		COMPREPLY=( $(compgen -W "_size_x,_size_y,_nb_subdivisions_x>0,_nb_subdisivions_y>0 >") )
		return 0
		;;
		"-point3d" | "--point3d")
		COMPREPLY=( $(compgen -W "x0,y0,z0 >") )
		return 0
		;;
		"-pose3d" | "--pose3d")
		COMPREPLY=( $(compgen -W "value1,..,value16 (noargs)") )
		return 0
		;;
		"-primitives3d" | "--primitives3d")
		COMPREPLY=( $(compgen -W "mode >") )
		return 0
		;;
		"-projections3d" | "--projections3d")
		COMPREPLY=( $(compgen -W "_x[%],_y[%],_z[%],_is_bounding_box={0|1} >") )
		return 0
		;;
		"-pyramid3d" | "--pyramid3d")
		COMPREPLY=( $(compgen -W "width,height >") )
		return 0
		;;
		"-quadrangle3d" | "--quadrangle3d")
		COMPREPLY=( $(compgen -W "x0,y0,z0,x1,y1,z1,x2,y2,z2,x3,y3,z3 >") )
		return 0
		;;
		"-rotate3d" | "--rotate3d")
		COMPREPLY=( $(compgen -W "u,v,w,angle >") )
		return 0
		;;
		"-rotation3d" | "--rotation3d")
		COMPREPLY=( $(compgen -W "u,v,w,angle >") )
		return 0
		;;
		"-sierpinski3d" | "--sierpinski3d")
		COMPREPLY=( $(compgen -W "_recursion_level>=0,_width,_height >") )
		return 0
		;;
		"-skeleton3d" | "--skeleton3d")
		COMPREPLY=( $(compgen -W "_metric,_frame_type={0=squares|1=diamonds|2=circles|3=auto},_skeleton_opacity,_frame_opacity >") )
		return 0
		;;
		"-snapshot3d" | "--snapshot3d")
		COMPREPLY=( $(compgen -W "_size>0,_zoom>=0,_backgroundR,_backgroundG,_backgroundB >") )
		return 0
		;;
		"-specl3d" | "--specl3d")
		COMPREPLY=( $(compgen -W "value>=0 >") )
		return 0
		;;
		"-specs3d" | "--specs3d")
		COMPREPLY=( $(compgen -W "value>=0 >") )
		return 0
		;;
		"-sphere3d" | "--sphere3d")
		COMPREPLY=( $(compgen -W "radius,_nb_recursions>=0 >") )
		return 0
		;;
		"-spherical3d" | "--spherical3d")
		COMPREPLY=( $(compgen -W "_nb_azimuth>=3,_nb_zenith>=3,_radius_function(phi,theta) >") )
		return 0
		;;
		"-spline3d" | "--spline3d")
		COMPREPLY=( $(compgen -W "x0[%],y0[%],z0[%],u0[%],v0[%],w0[%],x1[%],y1[%],z1[%],u1[%],v1[%],w1[%],_nb_vertices>=2 >") )
		return 0
		;;
		"-split3d" | "--split3d")
		COMPREPLY=( $(compgen -W "_keep_shared_data={0|1} >") )
		return 0
		;;
		"-star3d" | "--star3d")
		COMPREPLY=( $(compgen -W "_nb_branches>0,0<=_thickness<=1 >") )
		return 0
		;;
		"-streamline3d" | "--streamline3d")
		COMPREPLY=( $(compgen -W "x[%],y[%],z[%],_L>=0,_dl>0,_interpolation,_is_backward={0|1},_is_oriented={0|1} 'formula',x,y,z,_L>=0,_dl>0,_interpolation,_is_backward={0|1},_is_oriented={0|1}") )
		return 0
		;;
		"-sub3d" | "--sub3d")
		COMPREPLY=( $(compgen -W "tx,_ty,_tz >") )
		return 0
		;;
		"-superformula3d" | "--superformula3d")
		COMPREPLY=( $(compgen -W "resolution>1,m>=1,n1,n2,n3 >") )
		return 0
		;;
		"-text_pointcloud3d" | "--text_pointcloud3d")
		COMPREPLY=( $(compgen -W "_"text1",_"text2",_smoothness >") )
		return 0
		;;
		"-text3d" | "--text3d")
		COMPREPLY=( $(compgen -W "text,_font_height>0,_depth>0,_smoothness >") )
		return 0
		;;
		"-texturize3d" | "--texturize3d")
		COMPREPLY=( $(compgen -W "[ind_texture],_[ind_coords] >") )
		return 0
		;;
		"-torus3d" | "--torus3d")
		COMPREPLY=( $(compgen -W "_radius1,_radius2,_nb_subdivisions1>2,_nb_subdivisions2>2 >") )
		return 0
		;;
		"-triangle3d" | "--triangle3d")
		COMPREPLY=( $(compgen -W "x0,y0,z0,x1,y1,z1,x2,y2,z2 >") )
		return 0
		;;
		"-weird3d" | "--weird3d")
		COMPREPLY=( $(compgen -W "_resolution>0 >") )
		return 0
		;;
		"-apply_parallel" | "--apply_parallel")
		COMPREPLY=( $(compgen -W ""command" >") )
		return 0
		;;
		"-apply_parallel_channels" | "--apply_parallel_channels")
		COMPREPLY=( $(compgen -W ""command" >") )
		return 0
		;;
		"-apply_parallel_overlap" | "--apply_parallel_overlap")
		COMPREPLY=( $(compgen -W ""command",overlap[%],nb_threads={0=auto|1|2|4|8|16} >") )
		return 0
		;;
		"-check" | "--check")
		COMPREPLY=( $(compgen -W "expression >") )
		return 0
		;;
		"-check3d" | "--check3d")
		COMPREPLY=( $(compgen -W "_is_full_check={0|1} >") )
		return 0
		;;
		"-elif" | "--elif")
		COMPREPLY=( $(compgen -W "boolean filename") )
		return 0
		;;
		"-error" | "--error")
		COMPREPLY=( $(compgen -W "message >") )
		return 0
		;;
		"-exec" | "--exec")
		COMPREPLY=( $(compgen -W "command >") )
		return 0
		;;
		"-if" | "--if")
		COMPREPLY=( $(compgen -W "boolean filename") )
		return 0
		;;
		"-mutex" | "--mutex")
		COMPREPLY=( $(compgen -W "indice,_action={0=unlock|1=lock} >") )
		return 0
		;;
		"-parallel" | "--parallel")
		COMPREPLY=( $(compgen -W "_wait_threads,"command1","command2",... >") )
		return 0
		;;
		"-progress" | "--progress")
		COMPREPLY=( $(compgen -W "0<=value<=100 -1") )
		return 0
		;;
		"-repeat" | "--repeat")
		COMPREPLY=( $(compgen -W "nb_iterations >") )
		return 0
		;;
		"-rprogress" | "--rprogress")
		COMPREPLY=( $(compgen -W "0<=value<=100|-1|"command",0<=value_min<=100,0<=value_max<=100 >") )
		return 0
		;;
		"-skip" | "--skip")
		COMPREPLY=( $(compgen -W "item >") )
		return 0
		;;
		"-status" | "--status")
		COMPREPLY=( $(compgen -W "value >") )
		return 0
		;;
		"-while" | "--while")
		COMPREPLY=( $(compgen -W "boolean filename") )
		return 0
		;;
		"-array" | "--array")
		COMPREPLY=( $(compgen -W "M>0,_N>0,_expand_type={0=min|1=max|2=all} >") )
		return 0
		;;
		"-array_fade" | "--array_fade")
		COMPREPLY=( $(compgen -W "M>0,_N>0,0<=_fade_start<=100,0<=_fade_end<=100,_expand_type={0=min|1=max|2=all} >") )
		return 0
		;;
		"-array_mirror" | "--array_mirror")
		COMPREPLY=( $(compgen -W "N>=0,_dir={0=x|1=y|2=xy|3=tri-xy},_expand_type={0|1} >") )
		return 0
		;;
		"-array_random" | "--array_random")
		COMPREPLY=( $(compgen -W "Ms>0,_Ns>0,_Md>0,_Nd>0 >") )
		return 0
		;;
		"-frame_blur" | "--frame_blur")
		COMPREPLY=( $(compgen -W "_sharpness>0,_size>=0,_smoothness,_shading,_blur >") )
		return 0
		;;
		"-frame_cube" | "--frame_cube")
		COMPREPLY=( $(compgen -W "_depth>=0,_x_center,_y_center,_left_side={0=normal|1=mirror-x|2=mirror-y|3=mirror-xy},_right_side,_lower_side,_upper_side >") )
		return 0
		;;
		"-frame_fuzzy" | "--frame_fuzzy")
		COMPREPLY=( $(compgen -W "size_x>=0,_size_y>=0,_fuzzyness>=0,_smoothness>=0,_R,_G,_B,_A >") )
		return 0
		;;
		"-frame_painting" | "--frame_painting")
		COMPREPLY=( $(compgen -W "_size[%]>=0,0<=_contrast<=1,_profile_smoothness[%]>=0,_R,_G,_B,_vignette_size[%]>=0,_vignette_contrast>=0,_defects_contrast>=0,0<=_defects_density<=100,_defects_size>=0,_defects_smoothness[%]>=0,_serial_number >") )
		return 0
		;;
		"-frame_pattern" | "--frame_pattern")
		COMPREPLY=( $(compgen -W "M>=3,_pattern={0=firstimage|1=self},_constrain_size={0|1} >") )
		return 0
		;;
		"-frame_round" | "--frame_round")
		COMPREPLY=( $(compgen -W "_sharpness>0,_size>=0,_smoothness,_shading,_R,_G,_B,_A >") )
		return 0
		;;
		"-frame_x" | "--frame_x")
		COMPREPLY=( $(compgen -W "size_x[%]>=0,_col1,...,_colN >") )
		return 0
		;;
		"-frame_xy" | "--frame_xy")
		COMPREPLY=( $(compgen -W "size_x[%]>=0,_size_y[%]>=0,_col1,...,_colN >") )
		return 0
		;;
		"-frame_xyz" | "--frame_xyz")
		COMPREPLY=( $(compgen -W "size_x[%]>=0,_size_y[%]>=0,_size_z[%]>=0_col1,...,_colN >") )
		return 0
		;;
		"-frame_y" | "--frame_y")
		COMPREPLY=( $(compgen -W "size_y[%]>=0,_col1,...,_colN >") )
		return 0
		;;
		"-img2ascii" | "--img2ascii")
		COMPREPLY=( $(compgen -W "_charset,_analysis_scale>0,_analysis_smoothness[%]>=0,_synthesis_scale>0,_output_ascii_filename >") )
		return 0
		;;
		"-imagegrid" | "--imagegrid")
		COMPREPLY=( $(compgen -W "M>0,_N>0 >") )
		return 0
		;;
		"-linearize_tiles" | "--linearize_tiles")
		COMPREPLY=( $(compgen -W "M>0,_N>0 >") )
		return 0
		;;
		"-map_sprites" | "--map_sprites")
		COMPREPLY=( $(compgen -W "_nb_sprites>=1,_allow_rotation={0=none|1=90deg.|2=180deg.} >") )
		return 0
		;;
		"-puzzle" | "--puzzle")
		COMPREPLY=( $(compgen -W "_width>0,_height>0,_M>=1,_N>=1,_curvature,_centering,_connectors_variability,_resolution>=1 >") )
		return 0
		;;
		"-quadratize_tiles" | "--quadratize_tiles")
		COMPREPLY=( $(compgen -W "M>0,_N>0 >") )
		return 0
		;;
		"-rotate_tiles" | "--rotate_tiles")
		COMPREPLY=( $(compgen -W "angle,_M>0,N>0 >") )
		return 0
		;;
		"-shift_tiles" | "--shift_tiles")
		COMPREPLY=( $(compgen -W "M>0,_N>0,_amplitude >") )
		return 0
		;;
		"-taquin" | "--taquin")
		COMPREPLY=( $(compgen -W "M>0,_N>0,_remove_tile={0=none|1=first|2=last|3=random},_relief,_border_thickness[%],_border_outline[%],_outline_color >") )
		return 0
		;;
		"-tunnel" | "--tunnel")
		COMPREPLY=( $(compgen -W "_level>=0,_factor>0,_cx,_cy,_opacity,_angle >") )
		return 0
		;;
		"-boxfitting" | "--boxfitting")
		COMPREPLY=( $(compgen -W "_min_box_size>=1,_max_box_size>=0,_initial_density>=0,_nb_attempts>=1 >") )
		return 0
		;;
		"-cartoon" | "--cartoon")
		COMPREPLY=( $(compgen -W "_smoothness,_sharpening,_threshold>=0,_thickness>=0,_color>=0,quantization>0 >") )
		return 0
		;;
		"-circlism" | "--circlism")
		COMPREPLY=( $(compgen -W "_radius_min>0,_radius_max>0,_smoothness[%]>=0,_radius_linearity>=0,_location_linearity>=0,_shape={0=squares|1=diamonds|2=circle} >") )
		return 0
		;;
		"-color_ellipses" | "--color_ellipses")
		COMPREPLY=( $(compgen -W "_count>0,_radius>=0,_opacity>=0 >") )
		return 0
		;;
		"-cubism" | "--cubism")
		COMPREPLY=( $(compgen -W "_density>=0,0<=_thickness<=50,_max_angle,_opacity,_smoothness>=0 >") )
		return 0
		;;
		"-draw_whirl" | "--draw_whirl")
		COMPREPLY=( $(compgen -W "_amplitude>=0 >") )
		return 0
		;;
		"-drawing" | "--drawing")
		COMPREPLY=( $(compgen -W "_amplitude>=0 >") )
		return 0
		;;
		"-drop_shadow" | "--drop_shadow")
		COMPREPLY=( $(compgen -W "_offset_x[%],_offset_y[%],_smoothness[%]>=0,0<=_curvature<=1,_expand_size={0|1} >") )
		return 0
		;;
		"-ellipsionism" | "--ellipsionism")
		COMPREPLY=( $(compgen -W "_R>0[%],_r>0[%],_smoothness>=0[%],_opacity,_outline>0,_density>0 >") )
		return 0
		;;
		"-fire_edges" | "--fire_edges")
		COMPREPLY=( $(compgen -W "_edges>=0,0<=_attenuation<=1,_smoothness>=0,_threshold>=0,_nb_frames>0,_starting_frame>=0,frame_skip>=0 >") )
		return 0
		;;
		"-fractalize" | "--fractalize")
		COMPREPLY=( $(compgen -W "0<=detail_level<=1 >") )
		return 0
		;;
		"-glow" | "--glow")
		COMPREPLY=( $(compgen -W "_amplitude>=0 >") )
		return 0
		;;
		"-halftone" | "--halftone")
		COMPREPLY=( $(compgen -W "nb_levels>=2,_size_dark>=2,_size_bright>=2,_shape={0=square|1=diamond|2=circle|3=inv-square|4=inv-diamond|5=inv-circle},_smoothness[%]>=0 >") )
		return 0
		;;
		"-hardsketchbw" | "--hardsketchbw")
		COMPREPLY=( $(compgen -W "_amplitude>=0,_density>=0,_opacity,0<=_edge_threshold<=100,_is_fast={0|1} >") )
		return 0
		;;
		"-hearts" | "--hearts")
		COMPREPLY=( $(compgen -W "_density>=0 >") )
		return 0
		;;
		"-houghsketchbw" | "--houghsketchbw")
		COMPREPLY=( $(compgen -W "_density>=0,_radius>0,0<=_threshold<=100,0<=_opacity<=1,_votesize[%]>0 >") )
		return 0
		;;
		"-lightrays" | "--lightrays")
		COMPREPLY=( $(compgen -W "100<=_density<=0,_cx,_cy,_ray_length>=0,_ray_attenuation>=0 >") )
		return 0
		;;
		"-light_relief" | "--light_relief")
		COMPREPLY=( $(compgen -W "_ambient_light,_specular_lightness,_specular_size,_light_smoothness,_darkness,_xl,_yl,_zl,_zscale,_opacity_bumpmap={0|1} >") )
		return 0
		;;
		"-mosaic" | "--mosaic")
		COMPREPLY=( $(compgen -W "_density>=0,_edges={0|1} >") )
		return 0
		;;
		"-pencilbw" | "--pencilbw")
		COMPREPLY=( $(compgen -W "_size>=0,_amplitude>=0 >") )
		return 0
		;;
		"-polaroid" | "--polaroid")
		COMPREPLY=( $(compgen -W "_size1>=0,_size2>=0 >") )
		return 0
		;;
		"-polygonize" | "--polygonize")
		COMPREPLY=( $(compgen -W "_warp_amplitude>=0,_smoothness[%]>=0,_min_area[%]>=0,_resolution_x[%]>0,_resolution_y[%]>0 >") )
		return 0
		;;
		"-poster_edges" | "--poster_edges")
		COMPREPLY=( $(compgen -W "0<=_edge_threshold<=100,0<=_edge_shade<=100,_edge_thickness>=0,_edge_antialiasing>=0,0<=_posterization_level<=15,_posterization_antialiasing>=0 >") )
		return 0
		;;
		"-poster_hope" | "--poster_hope")
		COMPREPLY=( $(compgen -W "_smoothness>=0 >") )
		return 0
		;;
		"-rodilius" | "--rodilius")
		COMPREPLY=( $(compgen -W "0<=_amplitude<=100,_0<=thickness<=100,_sharpness>=0,_nb_orientations>0,_offset,_color_mode={0=darker|1=brighter} >") )
		return 0
		;;
		"-stained_glass" | "--stained_glass")
		COMPREPLY=( $(compgen -W "_edges[%]>=0,shading>=0,is_thin_separators={0|1} >") )
		return 0
		;;
		"-star" | "--star")
		COMPREPLY=( $(compgen -W "_width>0,_height>0,_nb_branches>0,0<=_thickness<=1 >") )
		return 0
		;;
		"-stars" | "--stars")
		COMPREPLY=( $(compgen -W "_density[%]>=0,_depth>=0,_size>0,_nb_branches>=1,0<=_thickness<=1,_smoothness[%]>=0,_R,_G,_B,_opacity >") )
		return 0
		;;
		"-sponge" | "--sponge")
		COMPREPLY=( $(compgen -W "_size>0 >") )
		return 0
		;;
		"-stencil" | "--stencil")
		COMPREPLY=( $(compgen -W "_radius[%]>=0,_smoothness>=0,_iterations>=0 >") )
		return 0
		;;
		"-stencilbw" | "--stencilbw")
		COMPREPLY=( $(compgen -W "_edges>=0,_smoothness>=0 >") )
		return 0
		;;
		"-tetris" | "--tetris")
		COMPREPLY=( $(compgen -W "_scale>0 >") )
		return 0
		;;
		"-warhol" | "--warhol")
		COMPREPLY=( $(compgen -W "_M>0,_N>0,_smoothness>=0,_color>=0 >") )
		return 0
		;;
		"-weave" | "--weave")
		COMPREPLY=( $(compgen -W "_density>=0,0<=_thickness<=100,0<=_shadow<=100,_shading>=0,_fibers_amplitude>=0,_fibers_smoothness>=0,_angle,-1<=_x_curvature<=1,-1<=_y_curvature<=1 >") )
		return 0
		;;
		"-whirls" | "--whirls")
		COMPREPLY=( $(compgen -W "_texture>=0,_smoothness>=0,_darkness>=0,_lightness>=0 >") )
		return 0
		;;
		"-euclidean2polar" | "--euclidean2polar")
		COMPREPLY=( $(compgen -W "_cx,_cy,_n>0,_boundary={0=dirichlet|1=neumann|2=periodic} >") )
		return 0
		;;
		"-deform" | "--deform")
		COMPREPLY=( $(compgen -W "_amplitude>=0 >") )
		return 0
		;;
		"-fisheye" | "--fisheye")
		COMPREPLY=( $(compgen -W "_x,_y,0<=_radius<=100,_amplitude>=0 >") )
		return 0
		;;
		"-flower" | "--flower")
		COMPREPLY=( $(compgen -W "_amplitude,_frequency,_offset_r[%],_angle,_cx,_cy,_boundary={0=dirichlet|1=neumann|2=periodic} >") )
		return 0
		;;
		"-kaleidoscope" | "--kaleidoscope")
		COMPREPLY=( $(compgen -W "_cx,_cy,_radius,_angle,_boundary={0=dirichlet|1=neumann|2=periodic} >") )
		return 0
		;;
		"-map_sphere" | "--map_sphere")
		COMPREPLY=( $(compgen -W "_width>0,_height>0,_radius,_dilation>0,_fading>=0,_fading_power>=0 >") )
		return 0
		;;
		"-polar2euclidean" | "--polar2euclidean")
		COMPREPLY=( $(compgen -W "_cx,_cy,_n>0,_boundary={0=dirichlet|1=neumann|2=periodic} >") )
		return 0
		;;
		"-raindrops" | "--raindrops")
		COMPREPLY=( $(compgen -W "_amplitude,_density>=0,_wavelength>=0,_merging_steps>=0 >") )
		return 0
		;;
		"-ripple" | "--ripple")
		COMPREPLY=( $(compgen -W "_amplitude,_frequency,_shape={0=bloc|1=triangle|2=sine|3=sine+|4=random},_angle,_offset >") )
		return 0
		;;
		"-rotoidoscope" | "--rotoidoscope")
		COMPREPLY=( $(compgen -W "_cx,_cy,_tiles>0,_smoothness[%]>=0,_boundary={0=dirichlet|1=neumann|2=periodic} >") )
		return 0
		;;
		"-symmetrize" | "--symmetrize")
		COMPREPLY=( $(compgen -W "_x[%],_y[%],_angle,_boundary={0=dirichlet|1=neumann|2=periodic},_is_antisymmetry={0|1},_swap_sides={0|1} >") )
		return 0
		;;
		"-transform_polar" | "--transform_polar")
		COMPREPLY=( $(compgen -W ""expr_radius",_"expr_angle",_x_center,_y_center,_boundary={0=dirichlet|1=neumann} >") )
		return 0
		;;
		"-twirl" | "--twirl")
		COMPREPLY=( $(compgen -W "_amplitude,_cx,_cy,_boundary={0=dirichlet|1=neumann|2=periodic} >") )
		return 0
		;;
		"-warp_perspective" | "--warp_perspective")
		COMPREPLY=( $(compgen -W "_x-angle,_y-angle,_zoom>0,_x-center,_y-center,_boundary={0=dirichlet|1=neumann|2=periodic} >") )
		return 0
		;;
		"-water" | "--water")
		COMPREPLY=( $(compgen -W "_amplitude>=0,_smoothness>=0 >") )
		return 0
		;;
		"-wave" | "--wave")
		COMPREPLY=( $(compgen -W "_amplitude>=0,_frequency>=0,_center_x,_center_y >") )
		return 0
		;;
		"-wind" | "--wind")
		COMPREPLY=( $(compgen -W "_amplitude>=0,_angle,0<=_attenuation<=1,_threshold >") )
		return 0
		;;
		"-zoom" | "--zoom")
		COMPREPLY=( $(compgen -W "_factor,_cx,_cy,_cz,_boundary={0=dirichlet|1=neumann|2=periodic} >") )
		return 0
		;;
		"-cracks" | "--cracks")
		COMPREPLY=( $(compgen -W "_density>=0,_amplitude,_relief={0|1} >") )
		return 0
		;;
		"-light_patch" | "--light_patch")
		COMPREPLY=( $(compgen -W "_density>0,_darkness>=0,_lightness>=0 >") )
		return 0
		;;
		"-noise_hurl" | "--noise_hurl")
		COMPREPLY=( $(compgen -W "_amplitude>=0 >") )
		return 0
		;;
		"-pixelize" | "--pixelize")
		COMPREPLY=( $(compgen -W "_scale_x>0,_scale_y>0,_scale_z>0 >") )
		return 0
		;;
		"-shade_stripes" | "--shade_stripes")
		COMPREPLY=( $(compgen -W "_frequency>=0,_direction={0=horizontal|1=vertical},_darkness>=0,_lightness>=0 >") )
		return 0
		;;
		"-shadow_patch" | "--shadow_patch")
		COMPREPLY=( $(compgen -W "_opacity>=0 >") )
		return 0
		;;
		"-spread" | "--spread")
		COMPREPLY=( $(compgen -W "_dx>=0,_dy>=0,_dz>=0 >") )
		return 0
		;;
		"-stripes_y" | "--stripes_y")
		COMPREPLY=( $(compgen -W "_frequency>=0 >") )
		return 0
		;;
		"-texturize_canvas" | "--texturize_canvas")
		COMPREPLY=( $(compgen -W "_amplitude>=0,_fibrousness>=0,_emboss_level>=0 >") )
		return 0
		;;
		"-vignette" | "--vignette")
		COMPREPLY=( $(compgen -W "_strength>=0,0<=_radius_min<=100,0<=_radius_max<=100 >") )
		return 0
		;;
		"-watermark_visible" | "--watermark_visible")
		COMPREPLY=( $(compgen -W "_text,0<_opacity<1,_size>0,_angle,_mode={0=remove|1=add},_smoothness>=0 >") )
		return 0
		;;
		"-blend" | "--blend")
		COMPREPLY=( $(compgen -W "blending_mode,0<=_opacity<=1,_revert_layer_order={0|1} >") )
		return 0
		;;
		"-blend_edges" | "--blend_edges")
		COMPREPLY=( $(compgen -W "smoothness[%]>=0 >") )
		return 0
		;;
		"-blend_seamless" | "--blend_seamless")
		COMPREPLY=( $(compgen -W "_is_mixed_mode={0|1},_inner_fading[%]>=0,_outer_fading[%]>=0 >") )
		return 0
		;;
		"-fade_diamond" | "--fade_diamond")
		COMPREPLY=( $(compgen -W "0<=_start<=100,0<=_end<=100 >") )
		return 0
		;;
		"-fade_linear" | "--fade_linear")
		COMPREPLY=( $(compgen -W "_angle,0<=_start<=100,0<=_end<=100 >") )
		return 0
		;;
		"-fade_radial" | "--fade_radial")
		COMPREPLY=( $(compgen -W "0<=_start<=100,0<=_end<=100 >") )
		return 0
		;;
		"-fade_x" | "--fade_x")
		COMPREPLY=( $(compgen -W "0<=_start<=100,0<=_end<=100 >") )
		return 0
		;;
		"-fade_y" | "--fade_y")
		COMPREPLY=( $(compgen -W "0<=_start<=100,0<=_end<=100 >") )
		return 0
		;;
		"-fade_z" | "--fade_z")
		COMPREPLY=( $(compgen -W "0<=_start<=100,0<=_end<=100 >") )
		return 0
		;;
		"-sub_alpha" | "--sub_alpha")
		COMPREPLY=( $(compgen -W "_opacity_gain>=1 >") )
		return 0
		;;
		"-animate" | "--animate")
		COMPREPLY=( $(compgen -W "filter_name,"param1_start,..,paramN_start","param1_end,..,paramN_end",nb_frames>=0,_output_frames={0|1},_output_filename delay>0") )
		return 0
		;;
		"-morph" | "--morph")
		COMPREPLY=( $(compgen -W "nb_frames>0,_smoothness>=0,_precision>0 >") )
		return 0
		;;
		"-register_nonrigid" | "--register_nonrigid")
		COMPREPLY=( $(compgen -W "_smoothness>=0,_precision>0,_nb_scale>=0 >") )
		return 0
		;;
		"-register_rigid" | "--register_rigid")
		COMPREPLY=( $(compgen -W "_smoothness>=0 >") )
		return 0
		;;
		"-transition_plasma" | "--transition_plasma")
		COMPREPLY=( $(compgen -W "_nb_frames>=2,_scale>=0,_smoothness[%]>=0 >") )
		return 0
		;;
		"-transition3d" | "--transition3d")
		COMPREPLY=( $(compgen -W "_nb_frames>=2,_nb_xtiles>0,_nb_ytiles>0,_axis_x,_axis_y,_axis_z,_is_antialias >") )
		return 0
		;;
		"-demo" | "--demo")
		COMPREPLY=( $(compgen -W "_run_in_parallel={0=no|1=yes|2=auto} >") )
		return 0
		;;
		"-x_jawbreaker" | "--x_jawbreaker")
		COMPREPLY=( $(compgen -W "0<_width<20,0<_height<20,0<_balls<=8 >") )
		return 0
		;;
		"-x_mandelbrot" | "--x_mandelbrot")
		COMPREPLY=( $(compgen -W "_julia={0|1},_c0r,_c0i >") )
		return 0
		;;
		"-x_minesweeper" | "--x_minesweeper")
		COMPREPLY=( $(compgen -W "8<=_width=<20,8<=_height<=20 >") )
		return 0
		;;
		"-x_quantize_rgb" | "--x_quantize_rgb")
		COMPREPLY=( $(compgen -W "_nbcolors>=2 >") )
		return 0
		;;
		"-x_whirl" | "--x_whirl")
		COMPREPLY=( $(compgen -W "_opacity>=0 >") )
		return 0
		;;
		"-output_pink3d" | "--output_pink3d")
		COMPREPLY=( $(compgen -W "filename >") )
		return 0
		;;
		"-pink_grayskel" | "--pink_grayskel")
		COMPREPLY=( $(compgen -W "_connectivity={4|8|6|26},_lambda=0 >") )
		return 0
		;;
		"-pink_heightmaxima" | "--pink_heightmaxima")
		COMPREPLY=( $(compgen -W "_connectivity={4|8|6|26},_height=1 >") )
		return 0
		;;
		"-pink_heightminima" | "--pink_heightminima")
		COMPREPLY=( $(compgen -W "_connectivity={4|8|6|26},_height=1 >") )
		return 0
		;;
		"-pink_htkern" | "--pink_htkern")
		COMPREPLY=( $(compgen -W "_connectivity={4|8|6|26},_type={""|u} >") )
		return 0
		;;
		"-pink_lvkern" | "--pink_lvkern")
		COMPREPLY=( $(compgen -W "_connectivity={4|8|6|26},_type={""|u} >") )
		return 0
		;;
		"-pink_reg_minima" | "--pink_reg_minima")
		COMPREPLY=( $(compgen -W "_connectivity={4|8|6|26} >") )
		return 0
		;;
		"-pink_skelcurv" | "--pink_skelcurv")
		COMPREPLY=( $(compgen -W "_prio={0|1|2|3|4|8|6|26},_connectivity={4|8|6|26},_inhibit={""} >") )
		return 0
		;;
		"-pink_skelend" | "--pink_skelend")
		COMPREPLY=( $(compgen -W "_connectivity={4|8|6|26},_n=0 >") )
		return 0
		;;
		"-pink_skeleton" | "--pink_skeleton")
		COMPREPLY=( $(compgen -W "_prio={0|1|2|3|4|8|6|26},_connectivity={4|8|6|26},_inhibit={""} >") )
		return 0
		;;
		"-pink_skelpar" | "--pink_skelpar")
		COMPREPLY=( $(compgen -W "_algorithm={0..29},_nsteps=_1,_inhibit="" >") )
		return 0
		;;
		"-pink_wshed" | "--pink_wshed")
		COMPREPLY=( $(compgen -W "_connectivity={4|8|6|26},_inverse={0|1},_height=0 >") )
		return 0
		;;
		"-alert" | "--alert")
		COMPREPLY=( $(compgen -W "_title,_message,_label_button1,_label_button2,... >") )
		return 0
		;;
		"-arg" | "--arg")
		COMPREPLY=( $(compgen -W "n,_arg1,...,_argN >") )
		return 0
		;;
		"-at" | "--at")
		COMPREPLY=( $(compgen -W "_x,_y,_z >") )
		return 0
		;;
		"-autocrop_coords" | "--autocrop_coords")
		COMPREPLY=( $(compgen -W "value1,value2,...|auto >") )
		return 0
		;;
		"-basename" | "--basename")
		COMPREPLY=( $(compgen -W "file_path,_variable_name_for_folder >") )
		return 0
		;;
		"-bin" | "--bin")
		COMPREPLY=( $(compgen -W "binary_int1,... >") )
		return 0
		;;
		"-bin2dec" | "--bin2dec")
		COMPREPLY=( $(compgen -W "binary_int1,... >") )
		return 0
		;;
		"-dec" | "--dec")
		COMPREPLY=( $(compgen -W "decimal_int1,... >") )
		return 0
		;;
		"-dec2str" | "--dec2str")
		COMPREPLY=( $(compgen -W "decimal_int1,... >") )
		return 0
		;;
		"-dec2bin" | "--dec2bin")
		COMPREPLY=( $(compgen -W "decimal_int1,... >") )
		return 0
		;;
		"-dec2hex" | "--dec2hex")
		COMPREPLY=( $(compgen -W "decimal_int1,... >") )
		return 0
		;;
		"-dec2oct" | "--dec2oct")
		COMPREPLY=( $(compgen -W "decimal_int1,... >") )
		return 0
		;;
		"-fact" | "--fact")
		COMPREPLY=( $(compgen -W "value >") )
		return 0
		;;
		"-file_mv" | "--file_mv")
		COMPREPLY=( $(compgen -W "filename_src,filename_dest >") )
		return 0
		;;
		"-file_rm" | "--file_rm")
		COMPREPLY=( $(compgen -W "filename >") )
		return 0
		;;
		"-filename" | "--filename")
		COMPREPLY=( $(compgen -W "filename,_number1,_number2,...,_numberN >") )
		return 0
		;;
		"-fitratio_wh" | "--fitratio_wh")
		COMPREPLY=( $(compgen -W "min_width,min_height,ratio_wh >") )
		return 0
		;;
		"-fitscreen" | "--fitscreen")
		COMPREPLY=( $(compgen -W "width,height,_depth >") )
		return 0
		;;
		"-gcd" | "--gcd")
		COMPREPLY=( $(compgen -W "a,b >") )
		return 0
		;;
		"-hex" | "--hex")
		COMPREPLY=( $(compgen -W "hexadecimal_int1,... >") )
		return 0
		;;
		"-hex2dec" | "--hex2dec")
		COMPREPLY=( $(compgen -W "hexadecimal_int1,... >") )
		return 0
		;;
		"-hex2str" | "--hex2str")
		COMPREPLY=( $(compgen -W "hexadecimal_string >") )
		return 0
		;;
		"-img2text" | "--img2text")
		COMPREPLY=( $(compgen -W "_line_separator >") )
		return 0
		;;
		"-is_percent" | "--is_percent")
		COMPREPLY=( $(compgen -W "string >") )
		return 0
		;;
		"-normalize_filename" | "--normalize_filename")
		COMPREPLY=( $(compgen -W "filename >") )
		return 0
		;;
		"-oct" | "--oct")
		COMPREPLY=( $(compgen -W "octal_int1,... >") )
		return 0
		;;
		"-oct2dec" | "--oct2dec")
		COMPREPLY=( $(compgen -W "octal_int1,... >") )
		return 0
		;;
		"-padint" | "--padint")
		COMPREPLY=( $(compgen -W "number,_size>0 >") )
		return 0
		;;
		"-quote" | "--quote")
		COMPREPLY=( $(compgen -W "string >") )
		return 0
		;;
		"-region_feature" | "--region_feature")
		COMPREPLY=( $(compgen -W "region_label,feature,_default_value >") )
		return 0
		;;
		"-str" | "--str")
		COMPREPLY=( $(compgen -W "string >") )
		return 0
		;;
		"-str2hex" | "--str2hex")
		COMPREPLY=( $(compgen -W "string >") )
		return 0
		;;
		"-stresc" | "--stresc")
		COMPREPLY=( $(compgen -W "val1,...,valN >") )
		return 0
		;;
		"-strcat" | "--strcat")
		COMPREPLY=( $(compgen -W "string1,string2,... >") )
		return 0
		;;
		"-strcmp" | "--strcmp")
		COMPREPLY=( $(compgen -W "string1,string2 >") )
		return 0
		;;
		"-strlen" | "--strlen")
		COMPREPLY=( $(compgen -W "string1 >") )
		return 0
		;;
		"-strreplace" | "--strreplace")
		COMPREPLY=( $(compgen -W "string,search,replace >") )
		return 0
		;;
		"-struncase" | "--struncase")
		COMPREPLY=( $(compgen -W "string >") )
		return 0
		;;
		"-h" | "--h")
		COMPREPLY=( $(compgen -W "$coms" -- "$cur") )
		return 0
		;;
		"-d" | "--d")
		COMPREPLY=( $(compgen -W "_X,_Y,_Z >") )
		return 0
		;;
		"-da" | "--da")
		COMPREPLY=( $(compgen -W "_width>0,_height>0 >") )
		return 0
		;;
		"-dg" | "--dg")
		COMPREPLY=( $(compgen -W "_width>32,_height>32,_plot_type,_vertex_type,_xmin,_xmax,_ymin,_ymax,_xlabel,_ylabel >") )
		return 0
		;;
		"-dh" | "--dh")
		COMPREPLY=( $(compgen -W "_width>0,_height>0,_clusters>0,_min_value[%],_max_value[%],_show_axes={0|1},_expression. >") )
		return 0
		;;
		"-dp" | "--dp")
		COMPREPLY=( $(compgen -W "_width>32,_height>32,_outline_type,_fill_R,_fill_G,_fill_B,_theta_start,_theta_end,_xlabel,_ylabel >") )
		return 0
		;;
		"-dt" | "--dt")
		COMPREPLY=( $(compgen -W "_size_factor>0,_ellipse_factor>=0,_colored_mode={0|1} >") )
		return 0
		;;
		"-dw" | "--dw")
		COMPREPLY=( $(compgen -W "_cell_size>0 >") )
		return 0
		;;
		"-e" | "--e")
		COMPREPLY=( $(compgen -W "message >") )
		return 0
		;;
		"-on" | "--on")
		COMPREPLY=( $(compgen -W "filename >") )
		return 0
		;;
		"-op" | "--op")
		COMPREPLY=( $(compgen -W "prefix >") )
		return 0
		;;
		"-sh" | "--sh")
		COMPREPLY=( $(compgen -W "x0[%],x1[%],y[%],z[%],v[%] y0[%],y1[%],z[%],v[%] z0[%],z1[%],v[%] v0[%],v1[%] (noargs)") )
		return 0
		;;
		"-v" | "--v")
		COMPREPLY=( $(compgen -W "_level {+|-}") )
		return 0
		;;
		"-w" | "--w")
		COMPREPLY=( $(compgen -W "_width[%]>=-1,_height[%]>=-1,_normalization,_fullscreen,_pos_x[%],_pos_y[%],_title >") )
		return 0
		;;
		"-mv" | "--mv")
		COMPREPLY=( $(compgen -W "position[%] >") )
		return 0
		;;
		"-nm" | "--nm")
		COMPREPLY=( $(compgen -W "name,_is_modified={0|1} >") )
		return 0
		;;
		"-+" | "--+")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-<<" | "--<<")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"->>" | "-->>")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-/" | "--/")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-==" | "--==")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"->=" | "-->=")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"->" | "-->")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-<=" | "--<=")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-<" | "--<")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-//" | "--//")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-%" | "--%")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-**" | "--**")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-*" | "--*")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-!=" | "--!=")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-^" | "--^")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"--" | "---")
		COMPREPLY=( $(compgen -W "value[%] [image] 'formula' (noargs)") )
		return 0
		;;
		"-c" | "--c")
		COMPREPLY=( $(compgen -W "{value0[%]|[image0]},{value1[%]|[image1]} [image] (noargs)") )
		return 0
		;;
		"-f" | "--f")
		COMPREPLY=( $(compgen -W "value1,_value2,.. [image] 'formula'") )
		return 0
		;;
		"-n" | "--n")
		COMPREPLY=( $(compgen -W "{value0[%]|[image0]},{value1[%]|[image1]} [image]") )
		return 0
		;;
		"-=" | "--=")
		COMPREPLY=( $(compgen -W "value,_x[%],_y[%],_z[%],_c[%] >") )
		return 0
		;;
		"-t2" | "--t2")
		COMPREPLY=( $(compgen -W "min[%],max[%] >") )
		return 0
		;;
		"-fc" | "--fc")
		COMPREPLY=( $(compgen -W "col1,...,colN >") )
		return 0
		;;
		"-a" | "--a")
		COMPREPLY=( $(compgen -W "[image],axis,_alignment axis,_alignment") )
		return 0
		;;
		"-z" | "--z")
		COMPREPLY=( $(compgen -W "x0[%],x1[%],_boundary x0[%],y0[%],x1[%],y1[%],_boundary x0[%],y0[%],z0[%],x1[%],y1[%],z1[%],_boundary x0[%],y0[%],z0[%],c0[%],x1[%],y1[%],z1[%],c1[%],_boundary (noargs)") )
		return 0
		;;
		"-r" | "--r")
		COMPREPLY=( $(compgen -W "[image],_interpolation,_boundary,_ax,_ay,_az,_ac {[image_w]|width>0[%]},_{[image_h]|height>0[%]},_{[image_d]|depth>0[%]},_{[image_s]|spectrum>0[%]},_interpolation,_boundary,_ax,_ay,_az,_ac (noargs)") )
		return 0
		;;
		"-rr2d" | "--rr2d")
		COMPREPLY=( $(compgen -W "width>0,height>0,_mode={0=inside|1=outside|2=padded},0=<_interpolation<=6 >") )
		return 0
		;;
		"-r2dx" | "--r2dx")
		COMPREPLY=( $(compgen -W "width[%]>0,_interpolation,_boundary,_ax,_ay,_az,_ac >") )
		return 0
		;;
		"-r2dy" | "--r2dy")
		COMPREPLY=( $(compgen -W "height[%]>=0,_interpolation,_boundary,_ax,_ay,_az,_ac >") )
		return 0
		;;
		"-r3dx" | "--r3dx")
		COMPREPLY=( $(compgen -W "width[%]>0,_interpolation,_boundary,_ax,_ay,_az,_ac >") )
		return 0
		;;
		"-r3dy" | "--r3dy")
		COMPREPLY=( $(compgen -W "height[%]>0,_interpolation,_boundary,_ax,_ay,_az,_ac >") )
		return 0
		;;
		"-r3dz" | "--r3dz")
		COMPREPLY=( $(compgen -W "depth[%]>0,_interpolation,_boundary,_ax,_ay,_az,_ac >") )
		return 0
		;;
		"-s" | "--s")
		COMPREPLY=( $(compgen -W "{x|y|z|c}..{x|y|z|c},_nb_parts keep_splitting_values={+|-},value1,value2,... (noargs)") )
		return 0
		;;
		"-y" | "--y")
		COMPREPLY=( $(compgen -W "_axis={x|y|z|c} >") )
		return 0
		;;
		"-b" | "--b")
		COMPREPLY=( $(compgen -W "std_variation>=0[%],_boundary={0=dirichlet|1=neumann},_kernel={0=quasi-gaussian(faster)|1=gaussian} >") )
		return 0
		;;
		"-g" | "--g")
		COMPREPLY=( $(compgen -W "{x|y|z}..{x|y|z},_scheme (noargs)") )
		return 0
		;;
		"-j" | "--j")
		COMPREPLY=( $(compgen -W "[sprite],_x[%],_y[%],_z[%],_c[%],_opacity,_[sprite_mask],_max_opacity_mask >") )
		return 0
		;;
		"-t" | "--t")
		COMPREPLY=( $(compgen -W "text,_x[%],_y[%],_font_height>=0,_opacity,_color1,.. >") )
		return 0
		;;
		"-+3d" | "--+3d")
		COMPREPLY=( $(compgen -W "tx,_ty,_tz [object3d] (noargs)") )
		return 0
		;;
		"-b3d" | "--b3d")
		COMPREPLY=( $(compgen -W "R,_G,_B [image] (noargs)") )
		return 0
		;;
		"-col3d" | "--col3d")
		COMPREPLY=( $(compgen -W "R,_G,_B,_opacity >") )
		return 0
		;;
		"-/3d" | "--/3d")
		COMPREPLY=( $(compgen -W "factor factor_x,factor_y,_factor_z") )
		return 0
		;;
		"-db3d" | "--db3d")
		COMPREPLY=( $(compgen -W "_is_double_sided={0|1} >") )
		return 0
		;;
		"-f3d" | "--f3d")
		COMPREPLY=( $(compgen -W "focale >") )
		return 0
		;;
		"-l3d" | "--l3d")
		COMPREPLY=( $(compgen -W "position_x,position_y,position_z [texture] (noargs)") )
		return 0
		;;
		"-m3d" | "--m3d")
		COMPREPLY=( $(compgen -W "_mode >") )
		return 0
		;;
		"-md3d" | "--md3d")
		COMPREPLY=( $(compgen -W "_mode >") )
		return 0
		;;
		"-*3d" | "--*3d")
		COMPREPLY=( $(compgen -W "factor factor_x,factor_y,_factor_z") )
		return 0
		;;
		"-o3d" | "--o3d")
		COMPREPLY=( $(compgen -W "_opacity >") )
		return 0
		;;
		"-p3d" | "--p3d")
		COMPREPLY=( $(compgen -W "mode >") )
		return 0
		;;
		"-r3d" | "--r3d")
		COMPREPLY=( $(compgen -W "u,v,w,angle >") )
		return 0
		;;
		"-sl3d" | "--sl3d")
		COMPREPLY=( $(compgen -W "value>=0 >") )
		return 0
		;;
		"-ss3d" | "--ss3d")
		COMPREPLY=( $(compgen -W "value>=0 >") )
		return 0
		;;
		"-s3d" | "--s3d")
		COMPREPLY=( $(compgen -W "_keep_shared_data={0|1} >") )
		return 0
		;;
		"--3d" | "---3d")
		COMPREPLY=( $(compgen -W "tx,_ty,_tz >") )
		return 0
		;;
		"-t3d" | "--t3d")
		COMPREPLY=( $(compgen -W "[ind_texture],_[ind_coords] >") )
		return 0
		;;
		"-x" | "--x")
		COMPREPLY=( $(compgen -W "command >") )
		return 0
		;;
		"-u" | "--u")
		COMPREPLY=( $(compgen -W "value >") )
		return 0
		;;
		"-frame" | "--frame")
		COMPREPLY=( $(compgen -W "size_x[%]>=0,_size_y[%]>=0,_col1,...,_colN >") )
		return 0
		;;
	esac

	if [[ "$cur" == -* ]] ; then 
		COMPREPLY=( $(compgen -W "$opts" -- "$cur") ) 
	else 
		_filedir 
	fi 
 
} 
 complete -F _gmic_def -o filenames gmic