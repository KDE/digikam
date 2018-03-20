/*****************************************************************************/
// Copyright 2006-2007 Adobe Systems Incorporated
// All Rights Reserved.
//
// NOTICE:  Adobe permits you to use, modify, and distribute this file in
// accordance with the terms of the Adobe license agreement accompanying it.
/*****************************************************************************/

/* $Id: //mondo/dng_sdk_1_3/dng_sdk/source/dng_render.cpp#1 $ */
/* $DateTime: 2009/06/22 05:04:49 $ */
/* $Change: 578634 $ */
/* $Author: tknoll $ */

/*****************************************************************************/

#include "dng_render.h"

#include "dng_1d_table.h"
#include "dng_bottlenecks.h"
#include "dng_camera_profile.h"
#include "dng_color_space.h"
#include "dng_color_spec.h"
#include "dng_filter_task.h"
#include "dng_host.h"
#include "dng_image.h"
#include "dng_negative.h"
#include "dng_resample.h"
#include "dng_utils.h"

/*****************************************************************************/

dng_function_exposure_ramp::dng_function_exposure_ramp (real64 white,
														real64 black,
														real64 minBlack)

	:	fSlope (1.0 / (white - black))
	,	fBlack (black)

	,	fRadius (0.0)
	,	fQScale (0.0)

	{

	const real64 kMaxCurveX = 0.5;			// Fraction of minBlack.

	const real64 kMaxCurveY = 1.0 / 16.0;	// Fraction of white.

	fRadius = Min_real64 (kMaxCurveX * minBlack,
						  kMaxCurveY / fSlope);

	if (fRadius > 0.0)
		fQScale= fSlope / (4.0 * fRadius);
	else
		fQScale = 0.0;

	}

/*****************************************************************************/

real64 dng_function_exposure_ramp::Evaluate (real64 x) const
	{

	if (x <= fBlack - fRadius)
		return 0.0;

	if (x >= fBlack + fRadius)
		return Min_real64 ((x - fBlack) * fSlope, 1.0);

	real64 y = x - (fBlack - fRadius);

	return fQScale * y * y;

	}

/*****************************************************************************/

dng_function_exposure_tone::dng_function_exposure_tone (real64 exposure)

	:	fIsNOP (exposure >= 0.0)

	,	fSlope (0.0)

	,	a (0.0)
	,	b (0.0)
	,	c (0.0)

	{

	if (!fIsNOP)
		{

		// Find slope to use for the all except the highest two f-stops.

		fSlope = pow (2.0, exposure);

		// Find quadradic parameters that match this darking at the crossover
		// point, yet still map pure white to pure white.

		a = 16.0 / 9.0 * (1.0 - fSlope);

		b = fSlope - 0.5 * a;

		c = 1.0 - a - b;

		}

	}

/*****************************************************************************/

real64 dng_function_exposure_tone::Evaluate (real64 x) const
	{

	if (!fIsNOP)
		{

		if (x <= 0.25)
			x = x * fSlope;

		else
			x = (a * x + b) * x + c;

		}

	return x;

	}

/*****************************************************************************/

real64 dng_tone_curve_acr3_default::Evaluate (real64 x) const
	{

	static const real32 kTable [] =
		{
		0.00000f, 0.00078f, 0.00160f, 0.00242f,
		0.00314f, 0.00385f, 0.00460f, 0.00539f,
		0.00623f, 0.00712f, 0.00806f, 0.00906f,
		0.01012f, 0.01122f, 0.01238f, 0.01359f,
		0.01485f, 0.01616f, 0.01751f, 0.01890f,
		0.02033f, 0.02180f, 0.02331f, 0.02485f,
		0.02643f, 0.02804f, 0.02967f, 0.03134f,
		0.03303f, 0.03475f, 0.03648f, 0.03824f,
		0.04002f, 0.04181f, 0.04362f, 0.04545f,
		0.04730f, 0.04916f, 0.05103f, 0.05292f,
		0.05483f, 0.05675f, 0.05868f, 0.06063f,
		0.06259f, 0.06457f, 0.06655f, 0.06856f,
		0.07057f, 0.07259f, 0.07463f, 0.07668f,
		0.07874f, 0.08081f, 0.08290f, 0.08499f,
		0.08710f, 0.08921f, 0.09134f, 0.09348f,
		0.09563f, 0.09779f, 0.09996f, 0.10214f,
		0.10433f, 0.10652f, 0.10873f, 0.11095f,
		0.11318f, 0.11541f, 0.11766f, 0.11991f,
		0.12218f, 0.12445f, 0.12673f, 0.12902f,
		0.13132f, 0.13363f, 0.13595f, 0.13827f,
		0.14061f, 0.14295f, 0.14530f, 0.14765f,
		0.15002f, 0.15239f, 0.15477f, 0.15716f,
		0.15956f, 0.16197f, 0.16438f, 0.16680f,
		0.16923f, 0.17166f, 0.17410f, 0.17655f,
		0.17901f, 0.18148f, 0.18395f, 0.18643f,
		0.18891f, 0.19141f, 0.19391f, 0.19641f,
		0.19893f, 0.20145f, 0.20398f, 0.20651f,
		0.20905f, 0.21160f, 0.21416f, 0.21672f,
		0.21929f, 0.22185f, 0.22440f, 0.22696f,
		0.22950f, 0.23204f, 0.23458f, 0.23711f,
		0.23963f, 0.24215f, 0.24466f, 0.24717f,
		0.24967f, 0.25216f, 0.25465f, 0.25713f,
		0.25961f, 0.26208f, 0.26454f, 0.26700f,
		0.26945f, 0.27189f, 0.27433f, 0.27676f,
		0.27918f, 0.28160f, 0.28401f, 0.28641f,
		0.28881f, 0.29120f, 0.29358f, 0.29596f,
		0.29833f, 0.30069f, 0.30305f, 0.30540f,
		0.30774f, 0.31008f, 0.31241f, 0.31473f,
		0.31704f, 0.31935f, 0.32165f, 0.32395f,
		0.32623f, 0.32851f, 0.33079f, 0.33305f,
		0.33531f, 0.33756f, 0.33981f, 0.34205f,
		0.34428f, 0.34650f, 0.34872f, 0.35093f,
		0.35313f, 0.35532f, 0.35751f, 0.35969f,
		0.36187f, 0.36404f, 0.36620f, 0.36835f,
		0.37050f, 0.37264f, 0.37477f, 0.37689f,
		0.37901f, 0.38112f, 0.38323f, 0.38533f,
		0.38742f, 0.38950f, 0.39158f, 0.39365f,
		0.39571f, 0.39777f, 0.39982f, 0.40186f,
		0.40389f, 0.40592f, 0.40794f, 0.40996f,
		0.41197f, 0.41397f, 0.41596f, 0.41795f,
		0.41993f, 0.42191f, 0.42388f, 0.42584f,
		0.42779f, 0.42974f, 0.43168f, 0.43362f,
		0.43554f, 0.43747f, 0.43938f, 0.44129f,
		0.44319f, 0.44509f, 0.44698f, 0.44886f,
		0.45073f, 0.45260f, 0.45447f, 0.45632f,
		0.45817f, 0.46002f, 0.46186f, 0.46369f,
		0.46551f, 0.46733f, 0.46914f, 0.47095f,
		0.47275f, 0.47454f, 0.47633f, 0.47811f,
		0.47989f, 0.48166f, 0.48342f, 0.48518f,
		0.48693f, 0.48867f, 0.49041f, 0.49214f,
		0.49387f, 0.49559f, 0.49730f, 0.49901f,
		0.50072f, 0.50241f, 0.50410f, 0.50579f,
		0.50747f, 0.50914f, 0.51081f, 0.51247f,
		0.51413f, 0.51578f, 0.51742f, 0.51906f,
		0.52069f, 0.52232f, 0.52394f, 0.52556f,
		0.52717f, 0.52878f, 0.53038f, 0.53197f,
		0.53356f, 0.53514f, 0.53672f, 0.53829f,
		0.53986f, 0.54142f, 0.54297f, 0.54452f,
		0.54607f, 0.54761f, 0.54914f, 0.55067f,
		0.55220f, 0.55371f, 0.55523f, 0.55673f,
		0.55824f, 0.55973f, 0.56123f, 0.56271f,
		0.56420f, 0.56567f, 0.56715f, 0.56861f,
		0.57007f, 0.57153f, 0.57298f, 0.57443f,
		0.57587f, 0.57731f, 0.57874f, 0.58017f,
		0.58159f, 0.58301f, 0.58443f, 0.58583f,
		0.58724f, 0.58864f, 0.59003f, 0.59142f,
		0.59281f, 0.59419f, 0.59556f, 0.59694f,
		0.59830f, 0.59966f, 0.60102f, 0.60238f,
		0.60373f, 0.60507f, 0.60641f, 0.60775f,
		0.60908f, 0.61040f, 0.61173f, 0.61305f,
		0.61436f, 0.61567f, 0.61698f, 0.61828f,
		0.61957f, 0.62087f, 0.62216f, 0.62344f,
		0.62472f, 0.62600f, 0.62727f, 0.62854f,
		0.62980f, 0.63106f, 0.63232f, 0.63357f,
		0.63482f, 0.63606f, 0.63730f, 0.63854f,
		0.63977f, 0.64100f, 0.64222f, 0.64344f,
		0.64466f, 0.64587f, 0.64708f, 0.64829f,
		0.64949f, 0.65069f, 0.65188f, 0.65307f,
		0.65426f, 0.65544f, 0.65662f, 0.65779f,
		0.65897f, 0.66013f, 0.66130f, 0.66246f,
		0.66362f, 0.66477f, 0.66592f, 0.66707f,
		0.66821f, 0.66935f, 0.67048f, 0.67162f,
		0.67275f, 0.67387f, 0.67499f, 0.67611f,
		0.67723f, 0.67834f, 0.67945f, 0.68055f,
		0.68165f, 0.68275f, 0.68385f, 0.68494f,
		0.68603f, 0.68711f, 0.68819f, 0.68927f,
		0.69035f, 0.69142f, 0.69249f, 0.69355f,
		0.69461f, 0.69567f, 0.69673f, 0.69778f,
		0.69883f, 0.69988f, 0.70092f, 0.70196f,
		0.70300f, 0.70403f, 0.70506f, 0.70609f,
		0.70711f, 0.70813f, 0.70915f, 0.71017f,
		0.71118f, 0.71219f, 0.71319f, 0.71420f,
		0.71520f, 0.71620f, 0.71719f, 0.71818f,
		0.71917f, 0.72016f, 0.72114f, 0.72212f,
		0.72309f, 0.72407f, 0.72504f, 0.72601f,
		0.72697f, 0.72794f, 0.72890f, 0.72985f,
		0.73081f, 0.73176f, 0.73271f, 0.73365f,
		0.73460f, 0.73554f, 0.73647f, 0.73741f,
		0.73834f, 0.73927f, 0.74020f, 0.74112f,
		0.74204f, 0.74296f, 0.74388f, 0.74479f,
		0.74570f, 0.74661f, 0.74751f, 0.74842f,
		0.74932f, 0.75021f, 0.75111f, 0.75200f,
		0.75289f, 0.75378f, 0.75466f, 0.75555f,
		0.75643f, 0.75730f, 0.75818f, 0.75905f,
		0.75992f, 0.76079f, 0.76165f, 0.76251f,
		0.76337f, 0.76423f, 0.76508f, 0.76594f,
		0.76679f, 0.76763f, 0.76848f, 0.76932f,
		0.77016f, 0.77100f, 0.77183f, 0.77267f,
		0.77350f, 0.77432f, 0.77515f, 0.77597f,
		0.77680f, 0.77761f, 0.77843f, 0.77924f,
		0.78006f, 0.78087f, 0.78167f, 0.78248f,
		0.78328f, 0.78408f, 0.78488f, 0.78568f,
		0.78647f, 0.78726f, 0.78805f, 0.78884f,
		0.78962f, 0.79040f, 0.79118f, 0.79196f,
		0.79274f, 0.79351f, 0.79428f, 0.79505f,
		0.79582f, 0.79658f, 0.79735f, 0.79811f,
		0.79887f, 0.79962f, 0.80038f, 0.80113f,
		0.80188f, 0.80263f, 0.80337f, 0.80412f,
		0.80486f, 0.80560f, 0.80634f, 0.80707f,
		0.80780f, 0.80854f, 0.80926f, 0.80999f,
		0.81072f, 0.81144f, 0.81216f, 0.81288f,
		0.81360f, 0.81431f, 0.81503f, 0.81574f,
		0.81645f, 0.81715f, 0.81786f, 0.81856f,
		0.81926f, 0.81996f, 0.82066f, 0.82135f,
		0.82205f, 0.82274f, 0.82343f, 0.82412f,
		0.82480f, 0.82549f, 0.82617f, 0.82685f,
		0.82753f, 0.82820f, 0.82888f, 0.82955f,
		0.83022f, 0.83089f, 0.83155f, 0.83222f,
		0.83288f, 0.83354f, 0.83420f, 0.83486f,
		0.83552f, 0.83617f, 0.83682f, 0.83747f,
		0.83812f, 0.83877f, 0.83941f, 0.84005f,
		0.84069f, 0.84133f, 0.84197f, 0.84261f,
		0.84324f, 0.84387f, 0.84450f, 0.84513f,
		0.84576f, 0.84639f, 0.84701f, 0.84763f,
		0.84825f, 0.84887f, 0.84949f, 0.85010f,
		0.85071f, 0.85132f, 0.85193f, 0.85254f,
		0.85315f, 0.85375f, 0.85436f, 0.85496f,
		0.85556f, 0.85615f, 0.85675f, 0.85735f,
		0.85794f, 0.85853f, 0.85912f, 0.85971f,
		0.86029f, 0.86088f, 0.86146f, 0.86204f,
		0.86262f, 0.86320f, 0.86378f, 0.86435f,
		0.86493f, 0.86550f, 0.86607f, 0.86664f,
		0.86720f, 0.86777f, 0.86833f, 0.86889f,
		0.86945f, 0.87001f, 0.87057f, 0.87113f,
		0.87168f, 0.87223f, 0.87278f, 0.87333f,
		0.87388f, 0.87443f, 0.87497f, 0.87552f,
		0.87606f, 0.87660f, 0.87714f, 0.87768f,
		0.87821f, 0.87875f, 0.87928f, 0.87981f,
		0.88034f, 0.88087f, 0.88140f, 0.88192f,
		0.88244f, 0.88297f, 0.88349f, 0.88401f,
		0.88453f, 0.88504f, 0.88556f, 0.88607f,
		0.88658f, 0.88709f, 0.88760f, 0.88811f,
		0.88862f, 0.88912f, 0.88963f, 0.89013f,
		0.89063f, 0.89113f, 0.89163f, 0.89212f,
		0.89262f, 0.89311f, 0.89360f, 0.89409f,
		0.89458f, 0.89507f, 0.89556f, 0.89604f,
		0.89653f, 0.89701f, 0.89749f, 0.89797f,
		0.89845f, 0.89892f, 0.89940f, 0.89987f,
		0.90035f, 0.90082f, 0.90129f, 0.90176f,
		0.90222f, 0.90269f, 0.90316f, 0.90362f,
		0.90408f, 0.90454f, 0.90500f, 0.90546f,
		0.90592f, 0.90637f, 0.90683f, 0.90728f,
		0.90773f, 0.90818f, 0.90863f, 0.90908f,
		0.90952f, 0.90997f, 0.91041f, 0.91085f,
		0.91130f, 0.91173f, 0.91217f, 0.91261f,
		0.91305f, 0.91348f, 0.91392f, 0.91435f,
		0.91478f, 0.91521f, 0.91564f, 0.91606f,
		0.91649f, 0.91691f, 0.91734f, 0.91776f,
		0.91818f, 0.91860f, 0.91902f, 0.91944f,
		0.91985f, 0.92027f, 0.92068f, 0.92109f,
		0.92150f, 0.92191f, 0.92232f, 0.92273f,
		0.92314f, 0.92354f, 0.92395f, 0.92435f,
		0.92475f, 0.92515f, 0.92555f, 0.92595f,
		0.92634f, 0.92674f, 0.92713f, 0.92753f,
		0.92792f, 0.92831f, 0.92870f, 0.92909f,
		0.92947f, 0.92986f, 0.93025f, 0.93063f,
		0.93101f, 0.93139f, 0.93177f, 0.93215f,
		0.93253f, 0.93291f, 0.93328f, 0.93366f,
		0.93403f, 0.93440f, 0.93478f, 0.93515f,
		0.93551f, 0.93588f, 0.93625f, 0.93661f,
		0.93698f, 0.93734f, 0.93770f, 0.93807f,
		0.93843f, 0.93878f, 0.93914f, 0.93950f,
		0.93986f, 0.94021f, 0.94056f, 0.94092f,
		0.94127f, 0.94162f, 0.94197f, 0.94231f,
		0.94266f, 0.94301f, 0.94335f, 0.94369f,
		0.94404f, 0.94438f, 0.94472f, 0.94506f,
		0.94540f, 0.94573f, 0.94607f, 0.94641f,
		0.94674f, 0.94707f, 0.94740f, 0.94774f,
		0.94807f, 0.94839f, 0.94872f, 0.94905f,
		0.94937f, 0.94970f, 0.95002f, 0.95035f,
		0.95067f, 0.95099f, 0.95131f, 0.95163f,
		0.95194f, 0.95226f, 0.95257f, 0.95289f,
		0.95320f, 0.95351f, 0.95383f, 0.95414f,
		0.95445f, 0.95475f, 0.95506f, 0.95537f,
		0.95567f, 0.95598f, 0.95628f, 0.95658f,
		0.95688f, 0.95718f, 0.95748f, 0.95778f,
		0.95808f, 0.95838f, 0.95867f, 0.95897f,
		0.95926f, 0.95955f, 0.95984f, 0.96013f,
		0.96042f, 0.96071f, 0.96100f, 0.96129f,
		0.96157f, 0.96186f, 0.96214f, 0.96242f,
		0.96271f, 0.96299f, 0.96327f, 0.96355f,
		0.96382f, 0.96410f, 0.96438f, 0.96465f,
		0.96493f, 0.96520f, 0.96547f, 0.96574f,
		0.96602f, 0.96629f, 0.96655f, 0.96682f,
		0.96709f, 0.96735f, 0.96762f, 0.96788f,
		0.96815f, 0.96841f, 0.96867f, 0.96893f,
		0.96919f, 0.96945f, 0.96971f, 0.96996f,
		0.97022f, 0.97047f, 0.97073f, 0.97098f,
		0.97123f, 0.97149f, 0.97174f, 0.97199f,
		0.97223f, 0.97248f, 0.97273f, 0.97297f,
		0.97322f, 0.97346f, 0.97371f, 0.97395f,
		0.97419f, 0.97443f, 0.97467f, 0.97491f,
		0.97515f, 0.97539f, 0.97562f, 0.97586f,
		0.97609f, 0.97633f, 0.97656f, 0.97679f,
		0.97702f, 0.97725f, 0.97748f, 0.97771f,
		0.97794f, 0.97817f, 0.97839f, 0.97862f,
		0.97884f, 0.97907f, 0.97929f, 0.97951f,
		0.97973f, 0.97995f, 0.98017f, 0.98039f,
		0.98061f, 0.98082f, 0.98104f, 0.98125f,
		0.98147f, 0.98168f, 0.98189f, 0.98211f,
		0.98232f, 0.98253f, 0.98274f, 0.98295f,
		0.98315f, 0.98336f, 0.98357f, 0.98377f,
		0.98398f, 0.98418f, 0.98438f, 0.98458f,
		0.98478f, 0.98498f, 0.98518f, 0.98538f,
		0.98558f, 0.98578f, 0.98597f, 0.98617f,
		0.98636f, 0.98656f, 0.98675f, 0.98694f,
		0.98714f, 0.98733f, 0.98752f, 0.98771f,
		0.98789f, 0.98808f, 0.98827f, 0.98845f,
		0.98864f, 0.98882f, 0.98901f, 0.98919f,
		0.98937f, 0.98955f, 0.98973f, 0.98991f,
		0.99009f, 0.99027f, 0.99045f, 0.99063f,
		0.99080f, 0.99098f, 0.99115f, 0.99133f,
		0.99150f, 0.99167f, 0.99184f, 0.99201f,
		0.99218f, 0.99235f, 0.99252f, 0.99269f,
		0.99285f, 0.99302f, 0.99319f, 0.99335f,
		0.99351f, 0.99368f, 0.99384f, 0.99400f,
		0.99416f, 0.99432f, 0.99448f, 0.99464f,
		0.99480f, 0.99495f, 0.99511f, 0.99527f,
		0.99542f, 0.99558f, 0.99573f, 0.99588f,
		0.99603f, 0.99619f, 0.99634f, 0.99649f,
		0.99664f, 0.99678f, 0.99693f, 0.99708f,
		0.99722f, 0.99737f, 0.99751f, 0.99766f,
		0.99780f, 0.99794f, 0.99809f, 0.99823f,
		0.99837f, 0.99851f, 0.99865f, 0.99879f,
		0.99892f, 0.99906f, 0.99920f, 0.99933f,
		0.99947f, 0.99960f, 0.99974f, 0.99987f,
		1.00000f
		};

	const uint32 kTableSize = sizeof (kTable    ) /
							  sizeof (kTable [0]);

	real32 y = (real32) x * (real32) (kTableSize - 1);

	int32 index = Pin_int32 (0, (int32) y, kTableSize - 2);

	real32 fract = y - (real32) index;

	return kTable [index    ] * (1.0f - fract) +
		   kTable [index + 1] * (       fract);

	}

/*****************************************************************************/

real64 dng_tone_curve_acr3_default::EvaluateInverse (real64 x) const
	{

	static const real32 kTable [] =
		{
		0.00000f, 0.00121f, 0.00237f, 0.00362f,
		0.00496f, 0.00621f, 0.00738f, 0.00848f,
		0.00951f, 0.01048f, 0.01139f, 0.01227f,
		0.01312f, 0.01393f, 0.01471f, 0.01547f,
		0.01620f, 0.01692f, 0.01763f, 0.01831f,
		0.01899f, 0.01965f, 0.02030f, 0.02094f,
		0.02157f, 0.02218f, 0.02280f, 0.02340f,
		0.02399f, 0.02458f, 0.02517f, 0.02574f,
		0.02631f, 0.02688f, 0.02744f, 0.02800f,
		0.02855f, 0.02910f, 0.02965f, 0.03019f,
		0.03072f, 0.03126f, 0.03179f, 0.03232f,
		0.03285f, 0.03338f, 0.03390f, 0.03442f,
		0.03493f, 0.03545f, 0.03596f, 0.03647f,
		0.03698f, 0.03749f, 0.03799f, 0.03849f,
		0.03899f, 0.03949f, 0.03998f, 0.04048f,
		0.04097f, 0.04146f, 0.04195f, 0.04244f,
		0.04292f, 0.04341f, 0.04389f, 0.04437f,
		0.04485f, 0.04533f, 0.04580f, 0.04628f,
		0.04675f, 0.04722f, 0.04769f, 0.04816f,
		0.04863f, 0.04910f, 0.04956f, 0.05003f,
		0.05049f, 0.05095f, 0.05141f, 0.05187f,
		0.05233f, 0.05278f, 0.05324f, 0.05370f,
		0.05415f, 0.05460f, 0.05505f, 0.05551f,
		0.05595f, 0.05640f, 0.05685f, 0.05729f,
		0.05774f, 0.05818f, 0.05863f, 0.05907f,
		0.05951f, 0.05995f, 0.06039f, 0.06083f,
		0.06126f, 0.06170f, 0.06214f, 0.06257f,
		0.06301f, 0.06344f, 0.06388f, 0.06431f,
		0.06474f, 0.06517f, 0.06560f, 0.06602f,
		0.06645f, 0.06688f, 0.06731f, 0.06773f,
		0.06815f, 0.06858f, 0.06900f, 0.06943f,
		0.06985f, 0.07027f, 0.07069f, 0.07111f,
		0.07152f, 0.07194f, 0.07236f, 0.07278f,
		0.07319f, 0.07361f, 0.07402f, 0.07444f,
		0.07485f, 0.07526f, 0.07567f, 0.07608f,
		0.07650f, 0.07691f, 0.07732f, 0.07772f,
		0.07813f, 0.07854f, 0.07895f, 0.07935f,
		0.07976f, 0.08016f, 0.08057f, 0.08098f,
		0.08138f, 0.08178f, 0.08218f, 0.08259f,
		0.08299f, 0.08339f, 0.08379f, 0.08419f,
		0.08459f, 0.08499f, 0.08539f, 0.08578f,
		0.08618f, 0.08657f, 0.08697f, 0.08737f,
		0.08776f, 0.08816f, 0.08855f, 0.08894f,
		0.08934f, 0.08973f, 0.09012f, 0.09051f,
		0.09091f, 0.09130f, 0.09169f, 0.09208f,
		0.09247f, 0.09286f, 0.09324f, 0.09363f,
		0.09402f, 0.09440f, 0.09479f, 0.09518f,
		0.09556f, 0.09595f, 0.09633f, 0.09672f,
		0.09710f, 0.09749f, 0.09787f, 0.09825f,
		0.09863f, 0.09901f, 0.09939f, 0.09978f,
		0.10016f, 0.10054f, 0.10092f, 0.10130f,
		0.10167f, 0.10205f, 0.10243f, 0.10281f,
		0.10319f, 0.10356f, 0.10394f, 0.10432f,
		0.10469f, 0.10507f, 0.10544f, 0.10582f,
		0.10619f, 0.10657f, 0.10694f, 0.10731f,
		0.10768f, 0.10806f, 0.10843f, 0.10880f,
		0.10917f, 0.10954f, 0.10991f, 0.11029f,
		0.11066f, 0.11103f, 0.11141f, 0.11178f,
		0.11215f, 0.11253f, 0.11290f, 0.11328f,
		0.11365f, 0.11403f, 0.11440f, 0.11478f,
		0.11516f, 0.11553f, 0.11591f, 0.11629f,
		0.11666f, 0.11704f, 0.11742f, 0.11780f,
		0.11818f, 0.11856f, 0.11894f, 0.11932f,
		0.11970f, 0.12008f, 0.12046f, 0.12084f,
		0.12122f, 0.12161f, 0.12199f, 0.12237f,
		0.12276f, 0.12314f, 0.12352f, 0.12391f,
		0.12429f, 0.12468f, 0.12506f, 0.12545f,
		0.12583f, 0.12622f, 0.12661f, 0.12700f,
		0.12738f, 0.12777f, 0.12816f, 0.12855f,
		0.12894f, 0.12933f, 0.12972f, 0.13011f,
		0.13050f, 0.13089f, 0.13129f, 0.13168f,
		0.13207f, 0.13247f, 0.13286f, 0.13325f,
		0.13365f, 0.13404f, 0.13444f, 0.13483f,
		0.13523f, 0.13563f, 0.13603f, 0.13642f,
		0.13682f, 0.13722f, 0.13762f, 0.13802f,
		0.13842f, 0.13882f, 0.13922f, 0.13962f,
		0.14003f, 0.14043f, 0.14083f, 0.14124f,
		0.14164f, 0.14204f, 0.14245f, 0.14285f,
		0.14326f, 0.14366f, 0.14407f, 0.14448f,
		0.14489f, 0.14530f, 0.14570f, 0.14611f,
		0.14652f, 0.14693f, 0.14734f, 0.14776f,
		0.14817f, 0.14858f, 0.14900f, 0.14941f,
		0.14982f, 0.15024f, 0.15065f, 0.15107f,
		0.15148f, 0.15190f, 0.15232f, 0.15274f,
		0.15316f, 0.15357f, 0.15399f, 0.15441f,
		0.15483f, 0.15526f, 0.15568f, 0.15610f,
		0.15652f, 0.15695f, 0.15737f, 0.15779f,
		0.15822f, 0.15864f, 0.15907f, 0.15950f,
		0.15992f, 0.16035f, 0.16078f, 0.16121f,
		0.16164f, 0.16207f, 0.16250f, 0.16293f,
		0.16337f, 0.16380f, 0.16423f, 0.16467f,
		0.16511f, 0.16554f, 0.16598f, 0.16641f,
		0.16685f, 0.16729f, 0.16773f, 0.16816f,
		0.16860f, 0.16904f, 0.16949f, 0.16993f,
		0.17037f, 0.17081f, 0.17126f, 0.17170f,
		0.17215f, 0.17259f, 0.17304f, 0.17349f,
		0.17393f, 0.17438f, 0.17483f, 0.17528f,
		0.17573f, 0.17619f, 0.17664f, 0.17709f,
		0.17754f, 0.17799f, 0.17845f, 0.17890f,
		0.17936f, 0.17982f, 0.18028f, 0.18073f,
		0.18119f, 0.18165f, 0.18211f, 0.18257f,
		0.18303f, 0.18350f, 0.18396f, 0.18442f,
		0.18489f, 0.18535f, 0.18582f, 0.18629f,
		0.18676f, 0.18723f, 0.18770f, 0.18817f,
		0.18864f, 0.18911f, 0.18958f, 0.19005f,
		0.19053f, 0.19100f, 0.19147f, 0.19195f,
		0.19243f, 0.19291f, 0.19339f, 0.19387f,
		0.19435f, 0.19483f, 0.19531f, 0.19579f,
		0.19627f, 0.19676f, 0.19724f, 0.19773f,
		0.19821f, 0.19870f, 0.19919f, 0.19968f,
		0.20017f, 0.20066f, 0.20115f, 0.20164f,
		0.20214f, 0.20263f, 0.20313f, 0.20362f,
		0.20412f, 0.20462f, 0.20512f, 0.20561f,
		0.20611f, 0.20662f, 0.20712f, 0.20762f,
		0.20812f, 0.20863f, 0.20913f, 0.20964f,
		0.21015f, 0.21066f, 0.21117f, 0.21168f,
		0.21219f, 0.21270f, 0.21321f, 0.21373f,
		0.21424f, 0.21476f, 0.21527f, 0.21579f,
		0.21631f, 0.21683f, 0.21735f, 0.21787f,
		0.21839f, 0.21892f, 0.21944f, 0.21997f,
		0.22049f, 0.22102f, 0.22155f, 0.22208f,
		0.22261f, 0.22314f, 0.22367f, 0.22420f,
		0.22474f, 0.22527f, 0.22581f, 0.22634f,
		0.22688f, 0.22742f, 0.22796f, 0.22850f,
		0.22905f, 0.22959f, 0.23013f, 0.23068f,
		0.23123f, 0.23178f, 0.23232f, 0.23287f,
		0.23343f, 0.23398f, 0.23453f, 0.23508f,
		0.23564f, 0.23620f, 0.23675f, 0.23731f,
		0.23787f, 0.23843f, 0.23899f, 0.23956f,
		0.24012f, 0.24069f, 0.24125f, 0.24182f,
		0.24239f, 0.24296f, 0.24353f, 0.24410f,
		0.24468f, 0.24525f, 0.24582f, 0.24640f,
		0.24698f, 0.24756f, 0.24814f, 0.24872f,
		0.24931f, 0.24989f, 0.25048f, 0.25106f,
		0.25165f, 0.25224f, 0.25283f, 0.25342f,
		0.25401f, 0.25460f, 0.25520f, 0.25579f,
		0.25639f, 0.25699f, 0.25759f, 0.25820f,
		0.25880f, 0.25940f, 0.26001f, 0.26062f,
		0.26122f, 0.26183f, 0.26244f, 0.26306f,
		0.26367f, 0.26429f, 0.26490f, 0.26552f,
		0.26614f, 0.26676f, 0.26738f, 0.26800f,
		0.26863f, 0.26925f, 0.26988f, 0.27051f,
		0.27114f, 0.27177f, 0.27240f, 0.27303f,
		0.27367f, 0.27431f, 0.27495f, 0.27558f,
		0.27623f, 0.27687f, 0.27751f, 0.27816f,
		0.27881f, 0.27945f, 0.28011f, 0.28076f,
		0.28141f, 0.28207f, 0.28272f, 0.28338f,
		0.28404f, 0.28470f, 0.28536f, 0.28602f,
		0.28669f, 0.28736f, 0.28802f, 0.28869f,
		0.28937f, 0.29004f, 0.29071f, 0.29139f,
		0.29207f, 0.29274f, 0.29342f, 0.29410f,
		0.29479f, 0.29548f, 0.29616f, 0.29685f,
		0.29754f, 0.29823f, 0.29893f, 0.29962f,
		0.30032f, 0.30102f, 0.30172f, 0.30242f,
		0.30312f, 0.30383f, 0.30453f, 0.30524f,
		0.30595f, 0.30667f, 0.30738f, 0.30809f,
		0.30881f, 0.30953f, 0.31025f, 0.31097f,
		0.31170f, 0.31242f, 0.31315f, 0.31388f,
		0.31461f, 0.31534f, 0.31608f, 0.31682f,
		0.31755f, 0.31829f, 0.31904f, 0.31978f,
		0.32053f, 0.32127f, 0.32202f, 0.32277f,
		0.32353f, 0.32428f, 0.32504f, 0.32580f,
		0.32656f, 0.32732f, 0.32808f, 0.32885f,
		0.32962f, 0.33039f, 0.33116f, 0.33193f,
		0.33271f, 0.33349f, 0.33427f, 0.33505f,
		0.33583f, 0.33662f, 0.33741f, 0.33820f,
		0.33899f, 0.33978f, 0.34058f, 0.34138f,
		0.34218f, 0.34298f, 0.34378f, 0.34459f,
		0.34540f, 0.34621f, 0.34702f, 0.34783f,
		0.34865f, 0.34947f, 0.35029f, 0.35111f,
		0.35194f, 0.35277f, 0.35360f, 0.35443f,
		0.35526f, 0.35610f, 0.35694f, 0.35778f,
		0.35862f, 0.35946f, 0.36032f, 0.36117f,
		0.36202f, 0.36287f, 0.36372f, 0.36458f,
		0.36545f, 0.36631f, 0.36718f, 0.36805f,
		0.36891f, 0.36979f, 0.37066f, 0.37154f,
		0.37242f, 0.37331f, 0.37419f, 0.37507f,
		0.37596f, 0.37686f, 0.37775f, 0.37865f,
		0.37955f, 0.38045f, 0.38136f, 0.38227f,
		0.38317f, 0.38409f, 0.38500f, 0.38592f,
		0.38684f, 0.38776f, 0.38869f, 0.38961f,
		0.39055f, 0.39148f, 0.39242f, 0.39335f,
		0.39430f, 0.39524f, 0.39619f, 0.39714f,
		0.39809f, 0.39904f, 0.40000f, 0.40097f,
		0.40193f, 0.40289f, 0.40386f, 0.40483f,
		0.40581f, 0.40679f, 0.40777f, 0.40875f,
		0.40974f, 0.41073f, 0.41172f, 0.41272f,
		0.41372f, 0.41472f, 0.41572f, 0.41673f,
		0.41774f, 0.41875f, 0.41977f, 0.42079f,
		0.42181f, 0.42284f, 0.42386f, 0.42490f,
		0.42594f, 0.42697f, 0.42801f, 0.42906f,
		0.43011f, 0.43116f, 0.43222f, 0.43327f,
		0.43434f, 0.43540f, 0.43647f, 0.43754f,
		0.43862f, 0.43970f, 0.44077f, 0.44186f,
		0.44295f, 0.44404f, 0.44514f, 0.44624f,
		0.44734f, 0.44845f, 0.44956f, 0.45068f,
		0.45179f, 0.45291f, 0.45404f, 0.45516f,
		0.45630f, 0.45744f, 0.45858f, 0.45972f,
		0.46086f, 0.46202f, 0.46318f, 0.46433f,
		0.46550f, 0.46667f, 0.46784f, 0.46901f,
		0.47019f, 0.47137f, 0.47256f, 0.47375f,
		0.47495f, 0.47615f, 0.47735f, 0.47856f,
		0.47977f, 0.48099f, 0.48222f, 0.48344f,
		0.48467f, 0.48590f, 0.48714f, 0.48838f,
		0.48963f, 0.49088f, 0.49213f, 0.49340f,
		0.49466f, 0.49593f, 0.49721f, 0.49849f,
		0.49977f, 0.50106f, 0.50236f, 0.50366f,
		0.50496f, 0.50627f, 0.50758f, 0.50890f,
		0.51023f, 0.51155f, 0.51289f, 0.51422f,
		0.51556f, 0.51692f, 0.51827f, 0.51964f,
		0.52100f, 0.52237f, 0.52374f, 0.52512f,
		0.52651f, 0.52790f, 0.52930f, 0.53070f,
		0.53212f, 0.53353f, 0.53495f, 0.53638f,
		0.53781f, 0.53925f, 0.54070f, 0.54214f,
		0.54360f, 0.54506f, 0.54653f, 0.54800f,
		0.54949f, 0.55098f, 0.55247f, 0.55396f,
		0.55548f, 0.55699f, 0.55851f, 0.56003f,
		0.56156f, 0.56310f, 0.56464f, 0.56621f,
		0.56777f, 0.56933f, 0.57091f, 0.57248f,
		0.57407f, 0.57568f, 0.57727f, 0.57888f,
		0.58050f, 0.58213f, 0.58376f, 0.58541f,
		0.58705f, 0.58871f, 0.59037f, 0.59204f,
		0.59373f, 0.59541f, 0.59712f, 0.59882f,
		0.60052f, 0.60226f, 0.60399f, 0.60572f,
		0.60748f, 0.60922f, 0.61099f, 0.61276f,
		0.61455f, 0.61635f, 0.61814f, 0.61996f,
		0.62178f, 0.62361f, 0.62545f, 0.62730f,
		0.62917f, 0.63104f, 0.63291f, 0.63480f,
		0.63671f, 0.63862f, 0.64054f, 0.64249f,
		0.64443f, 0.64638f, 0.64835f, 0.65033f,
		0.65232f, 0.65433f, 0.65633f, 0.65836f,
		0.66041f, 0.66245f, 0.66452f, 0.66660f,
		0.66868f, 0.67078f, 0.67290f, 0.67503f,
		0.67717f, 0.67932f, 0.68151f, 0.68368f,
		0.68587f, 0.68809f, 0.69033f, 0.69257f,
		0.69482f, 0.69709f, 0.69939f, 0.70169f,
		0.70402f, 0.70634f, 0.70869f, 0.71107f,
		0.71346f, 0.71587f, 0.71829f, 0.72073f,
		0.72320f, 0.72567f, 0.72818f, 0.73069f,
		0.73323f, 0.73579f, 0.73838f, 0.74098f,
		0.74360f, 0.74622f, 0.74890f, 0.75159f,
		0.75429f, 0.75704f, 0.75979f, 0.76257f,
		0.76537f, 0.76821f, 0.77109f, 0.77396f,
		0.77688f, 0.77982f, 0.78278f, 0.78579f,
		0.78883f, 0.79187f, 0.79498f, 0.79809f,
		0.80127f, 0.80445f, 0.80767f, 0.81095f,
		0.81424f, 0.81757f, 0.82094f, 0.82438f,
		0.82782f, 0.83133f, 0.83488f, 0.83847f,
		0.84210f, 0.84577f, 0.84951f, 0.85328f,
		0.85713f, 0.86103f, 0.86499f, 0.86900f,
		0.87306f, 0.87720f, 0.88139f, 0.88566f,
		0.89000f, 0.89442f, 0.89891f, 0.90350f,
		0.90818f, 0.91295f, 0.91780f, 0.92272f,
		0.92780f, 0.93299f, 0.93828f, 0.94369f,
		0.94926f, 0.95493f, 0.96082f, 0.96684f,
		0.97305f, 0.97943f, 0.98605f, 0.99291f,
		1.00000f
		};

	const uint32 kTableSize = sizeof (kTable    ) /
							  sizeof (kTable [0]);

	real32 y = (real32) x * (real32) (kTableSize - 1);

	int32 index = Pin_int32 (0, (int32) y, kTableSize - 2);

	real32 fract = y - (real32) index;

	return kTable [index    ] * (1.0f - fract) +
		   kTable [index + 1] * (       fract);

	}

/*****************************************************************************/

const dng_1d_function & dng_tone_curve_acr3_default::Get ()
	{

	static dng_tone_curve_acr3_default static_dng_tone_curve_acr3_default;

	return static_dng_tone_curve_acr3_default;

	}

/*****************************************************************************/

class dng_render_task: public dng_filter_task
	{

	protected:

		const dng_negative &fNegative;

		const dng_render &fParams;

		dng_point fSrcOffset;

		dng_vector fCameraWhite;
		dng_matrix fCameraToRGB;

		AutoPtr<dng_hue_sat_map> fHueSatMap;

		dng_1d_table fExposureRamp;

		AutoPtr<dng_hue_sat_map> fLookTable;

		dng_1d_table fToneCurve;

		dng_matrix fRGBtoFinal;

		dng_1d_table fEncodeGamma;

		AutoPtr<dng_memory_block> fTempBuffer [kMaxMPThreads];

	public:

		dng_render_task (const dng_image &srcImage,
						 dng_image &dstImage,
						 const dng_negative &negative,
						 const dng_render &params,
						 const dng_point &srcOffset);

		virtual dng_rect SrcArea (const dng_rect &dstArea);

		virtual void Start (uint32 threadCount,
							const dng_point &tileSize,
							dng_memory_allocator *allocator,
							dng_abort_sniffer *sniffer);

		virtual void ProcessArea (uint32 threadIndex,
								  dng_pixel_buffer &srcBuffer,
								  dng_pixel_buffer &dstBuffer);

	};

/*****************************************************************************/

dng_render_task::dng_render_task (const dng_image &srcImage,
								  dng_image &dstImage,
								  const dng_negative &negative,
								  const dng_render &params,
								  const dng_point &srcOffset)

	:	dng_filter_task (srcImage,
						 dstImage)

	,	fNegative  (negative )
	,	fParams    (params   )
	,	fSrcOffset (srcOffset)

	,	fCameraWhite ()
	,	fCameraToRGB ()

	,	fHueSatMap ()

	,	fExposureRamp ()

	,	fLookTable ()

	,	fToneCurve ()

	,	fRGBtoFinal ()

	,	fEncodeGamma ()

	{

	fSrcPixelType = ttFloat;
	fDstPixelType = ttFloat;

	}

/*****************************************************************************/

dng_rect dng_render_task::SrcArea (const dng_rect &dstArea)
	{

	return dstArea + fSrcOffset;

	}

/*****************************************************************************/

void dng_render_task::Start (uint32 threadCount,
							 const dng_point &tileSize,
							 dng_memory_allocator *allocator,
							 dng_abort_sniffer *sniffer)
	{

	dng_filter_task::Start (threadCount,
							tileSize,
							allocator,
							sniffer);

	// Compute camera space to linear ProPhoto RGB parameters.

	if (!fNegative.IsMonochrome ())
		{

		dng_camera_profile_id profileID;	// Default profile ID.

		AutoPtr<dng_color_spec> spec (fNegative.MakeColorSpec (profileID));

		if (fParams.WhiteXY ().IsValid ())
			{

			spec->SetWhiteXY (fParams.WhiteXY ());

			}

		else if (fNegative.HasCameraNeutral ())
			{

			spec->SetWhiteXY (spec->NeutralToXY (fNegative.CameraNeutral ()));

			}

		else if (fNegative.HasCameraWhiteXY ())
			{

			spec->SetWhiteXY (fNegative.CameraWhiteXY ());

			}

		else
			{

			spec->SetWhiteXY (D55_xy_coord ());

			}

		fCameraWhite = spec->CameraWhite ();

		fCameraToRGB = dng_space_ProPhoto::Get ().MatrixFromPCS () *
					   spec->CameraToPCS ();

		// Find Hue/Sat table, if any.

		const dng_camera_profile *profile = fNegative.ProfileByID (profileID);

		if (profile)
			{

			fHueSatMap.Reset (profile->HueSatMapForWhite (spec->WhiteXY ()));

			if (profile->HasLookTable ())
				{

				fLookTable.Reset (new dng_hue_sat_map (profile->LookTable ()));

				}

			}

		}

	// Compute exposure/shadows ramp.

	real64 exposure = fParams.Exposure () +
					  fNegative.BaselineExposure () -
					  (log (fNegative.Stage3Gain ()) / log (2.0));

		{

		real64 white = 1.0 / pow (2.0, Max_real64 (0.0, exposure));

		real64 black = fParams.Shadows () *
					   fNegative.ShadowScale () *
					   fNegative.Stage3Gain () *
					   0.001;

		black = Min_real64 (black, 0.99 * white);

		dng_function_exposure_ramp rampFunction (white,
												 black,
												 black);

		fExposureRamp.Initialize (*allocator, rampFunction);

		}

	// Compute tone curve.

		{

		// If there is any negative exposure compenation to perform
		// (beyond what the camera provides for with its baseline exposure),
		// we fake this by darkening the tone curve.

		dng_function_exposure_tone exposureTone (exposure);

		dng_1d_concatenate totalTone (exposureTone,
									  fParams.ToneCurve ());

		fToneCurve.Initialize (*allocator, totalTone);

		}

	// Compute linear ProPhoto RGB to final space parameters.

		{

		const dng_color_space &finalSpace = fParams.FinalSpace ();

		fRGBtoFinal = finalSpace.MatrixFromPCS () *
					  dng_space_ProPhoto::Get ().MatrixToPCS ();

		fEncodeGamma.Initialize (*allocator, finalSpace.GammaFunction ());

		}

	// Allocate temp buffer to hold one row of RGB data.

	uint32 tempBufferSize = tileSize.h * sizeof (real32) * 3;

	for (uint32 threadIndex = 0; threadIndex < threadCount; threadIndex++)
		{

		fTempBuffer [threadIndex] . Reset (allocator->Allocate (tempBufferSize));

		}

	}

/*****************************************************************************/

void dng_render_task::ProcessArea (uint32 threadIndex,
								   dng_pixel_buffer &srcBuffer,
								   dng_pixel_buffer &dstBuffer)
	{

	dng_rect srcArea = srcBuffer.fArea;
	dng_rect dstArea = dstBuffer.fArea;

	uint32 srcCols = srcArea.W ();

	real32 *tPtrR = fTempBuffer [threadIndex]->Buffer_real32 ();

	real32 *tPtrG = tPtrR + srcCols;
	real32 *tPtrB = tPtrG + srcCols;

	for (int32 srcRow = srcArea.t; srcRow < srcArea.b; srcRow++)
		{

		// First convert from camera native space to linear PhotoRGB,
		// applying the white balance and camera profile.

			{

			const real32 *sPtrA = (const real32 *)
								  srcBuffer.ConstPixel (srcRow,
													    srcArea.l,
													    0);

			if (fSrcPlanes == 1)
				{

				// For monochrome cameras, this just requires copying
				// the data into all three color channels.

				DoCopyBytes (sPtrA, tPtrR, srcCols * sizeof (real32));
				DoCopyBytes (sPtrA, tPtrG, srcCols * sizeof (real32));
				DoCopyBytes (sPtrA, tPtrB, srcCols * sizeof (real32));

				}

			else
				{

				const real32 *sPtrB = sPtrA + srcBuffer.fPlaneStep;
				const real32 *sPtrC = sPtrB + srcBuffer.fPlaneStep;

				if (fSrcPlanes == 3)
					{

					DoBaselineABCtoRGB (sPtrA,
									    sPtrB,
									    sPtrC,
									    tPtrR,
									    tPtrG,
									    tPtrB,
									    srcCols,
									    fCameraWhite,
									    fCameraToRGB);

					}

				else
					{

					const real32 *sPtrD = sPtrC + srcBuffer.fPlaneStep;

					DoBaselineABCDtoRGB (sPtrA,
									     sPtrB,
									     sPtrC,
									     sPtrD,
									     tPtrR,
									     tPtrG,
									     tPtrB,
									     srcCols,
									     fCameraWhite,
									     fCameraToRGB);

					}

				// Apply Hue/Sat map, if any.

				if (fHueSatMap.Get ())
					{

					DoBaselineHueSatMap (tPtrR,
										 tPtrG,
										 tPtrB,
										 tPtrR,
										 tPtrG,
										 tPtrB,
										 srcCols,
										 *fHueSatMap.Get ());

					}

				}

			}

		// Apply exposure curve.

		DoBaseline1DTable (tPtrR,
						   tPtrR,
						   srcCols,
						   fExposureRamp);

		DoBaseline1DTable (tPtrG,
						   tPtrG,
						   srcCols,
						   fExposureRamp);

		DoBaseline1DTable (tPtrB,
						   tPtrB,
						   srcCols,
						   fExposureRamp);

		// Apply Hue/Sat map, if any.

		if (fLookTable.Get ())
			{

			DoBaselineHueSatMap (tPtrR,
								 tPtrG,
								 tPtrB,
								 tPtrR,
								 tPtrG,
								 tPtrB,
								 srcCols,
								 *fLookTable.Get ());

			}

		// Apply baseline tone curve.

		DoBaselineRGBTone (tPtrR,
					       tPtrG,
						   tPtrB,
						   tPtrR,
					       tPtrG,
						   tPtrB,
						   srcCols,
						   fToneCurve);

		// Convert to final color space.

		int32 dstRow = srcRow + (dstArea.t - srcArea.t);

		if (fDstPlanes == 1)
			{

			real32 *dPtrG = dstBuffer.DirtyPixel_real32 (dstRow,
														 dstArea.l,
														 0);

			DoBaselineRGBtoGray (tPtrR,
								 tPtrG,
								 tPtrB,
								 dPtrG,
								 srcCols,
								 fRGBtoFinal);

			DoBaseline1DTable (dPtrG,
							   dPtrG,
							   srcCols,
							   fEncodeGamma);

			}

		else
			{

			real32 *dPtrR = dstBuffer.DirtyPixel_real32 (dstRow,
														 dstArea.l,
														 0);

			real32 *dPtrG = dPtrR + dstBuffer.fPlaneStep;
			real32 *dPtrB = dPtrG + dstBuffer.fPlaneStep;

			DoBaselineRGBtoRGB (tPtrR,
								tPtrG,
								tPtrB,
								dPtrR,
								dPtrG,
								dPtrB,
								srcCols,
								fRGBtoFinal);

			DoBaseline1DTable (dPtrR,
							   dPtrR,
							   srcCols,
							   fEncodeGamma);

			DoBaseline1DTable (dPtrG,
							   dPtrG,
							   srcCols,
							   fEncodeGamma);

			DoBaseline1DTable (dPtrB,
							   dPtrB,
							   srcCols,
							   fEncodeGamma);

			}

		}

	}

/*****************************************************************************/

dng_render::dng_render (dng_host &host,
						const dng_negative &negative)

	:	fHost			(host)
	,	fNegative		(negative)

	,	fWhiteXY		()

	,	fExposure		(0.0)
	,	fShadows		(5.0)

	,	fToneCurve		(&dng_tone_curve_acr3_default::Get ())

	,	fFinalSpace		(&dng_space_sRGB::Get ())
	,	fFinalPixelType (ttByte)

	,	fMaximumSize	(0)

	,	fProfileToneCurve ()

	{

	// Switch to NOP default parameters for non-scence referred data.

	if (fNegative.ColorimetricReference () != crSceneReferred)
		{

		fShadows = 0.0;

		fToneCurve = &dng_1d_identity::Get ();

		}

	// Use default tone curve from profile if any.

	const dng_camera_profile *profile = fNegative.ProfileByID (dng_camera_profile_id ());

	if (profile && profile->ToneCurve ().IsValid ())
		{

		fProfileToneCurve.Reset (new dng_spline_solver);

		profile->ToneCurve ().Solve (*fProfileToneCurve.Get ());

		fToneCurve = fProfileToneCurve.Get ();

		}

	}

/*****************************************************************************/

dng_image * dng_render::Render ()
	{

	const dng_image *srcImage = fNegative.Stage3Image ();

	dng_rect srcBounds = fNegative.DefaultCropArea ();

	dng_point dstSize;

	dstSize.h =	fNegative.DefaultFinalWidth  ();
	dstSize.v = fNegative.DefaultFinalHeight ();

	if (MaximumSize ())
		{

		if (Max_uint32 (dstSize.h, dstSize.v) > MaximumSize ())
			{

			real64 ratio = fNegative.AspectRatio ();

			if (ratio >= 1.0)
				{
				dstSize.h = MaximumSize ();
				dstSize.v = Max_uint32 (1, Round_uint32 (dstSize.h / ratio));
				}

			else
				{
				dstSize.v = MaximumSize ();
				dstSize.h = Max_uint32 (1, Round_uint32 (dstSize.v * ratio));
				}

			}

		}

	AutoPtr<dng_image> tempImage;

	if (srcBounds.Size () != dstSize)
		{

		tempImage.Reset (fHost.Make_dng_image (dstSize,
											   srcImage->Planes    (),
											   srcImage->PixelType ()));

		ResampleImage (fHost,
					   *srcImage,
					   *tempImage.Get (),
					   srcBounds,
					   tempImage->Bounds (),
					   dng_resample_bicubic::Get ());

		srcImage = tempImage.Get ();

		srcBounds = tempImage->Bounds ();

		}

	uint32 dstPlanes = FinalSpace ().IsMonochrome () ? 1 : 3;

	AutoPtr<dng_image> dstImage (fHost.Make_dng_image (srcBounds.Size (),
													   dstPlanes,
													   FinalPixelType ()));

	dng_render_task task (*srcImage,
						  *dstImage.Get (),
						  fNegative,
						  *this,
						  srcBounds.TL ());

	fHost.PerformAreaTask (task,
						   dstImage->Bounds ());

	return dstImage.Release ();

	}

/*****************************************************************************/
