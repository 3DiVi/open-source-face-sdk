#include <tdv/modules/MatcherModule.h>
#include <tdv/data/ContextUtils.h>
#include <tdv/utils/rassert/RAssert.h>

#ifdef __SSE2__
#include <mmintrin.h>
#include <xmmintrin.h>
#include <emmintrin.h>
#include <smmintrin.h>  // sse 4.1 actually
#endif


namespace{

float l2_distance(
	const float* v1,  // must be aligned for SSE
	const float* v2,  // must be aligned for SSE
	const int n)
{
	#ifdef __SSE2__

	__m128 sse_d = _mm_set1_ps(0);

	for(int i = 0; i < n; (i += 4), (v1 += 4), (v2 += 4))
	{
		const __m128 r = _mm_sub_ps( _mm_load_ps(v1), _mm_load_ps(v2) );

		sse_d = _mm_add_ps( sse_d, _mm_mul_ps(r, r) );
	}

	float d_buf[4];
	_mm_storeu_ps(d_buf, sse_d);

	const float d = d_buf[0] + d_buf[1] + d_buf[2] + d_buf[3];

	#else

	// #error need to build with SSE !!!

	float d = 0;

	for(int i = 0; i < n; (++i), (++v1), (++v2))
	{
		const float r = *v1 - *v2;

		d += r * r;
	}
	#endif

	return d;

}

double distance(const tdv::data::Context& a, const tdv::data::Context& b){
	return l2_distance(
		a["template"].get<std::vector<float>>().data(),
		b["template"].get<std::vector<float>>().data(),
		a["template_size"].get<long>()
	);
}

}


namespace tdv {

namespace modules {

MatcherModule::MatcherModule(const tdv::data::Context& config):
	threshold(config.get<double>("threshold", 1.175))
{}

void MatcherModule::operator ()(tdv::data::Context& data)
{
	if(data.contains("verification"))
	{
		verifyMatch(data["verification"]);
	}
	else
	{
		RHAssert2(0x0219d2b3, true, "No input data for verification");
	}
}

void MatcherModule::verifyMatch(tdv::data::Context& data)
{
	RCAssert(0x4c0d78cd, data["objects"].size() == 2);

	double result = distance(data["objects"][0], data["objects"][1]);

	data["result"]["distance"] = result;
	data["result"]["verdict"] = result < threshold ? true : false;
}

}
}
