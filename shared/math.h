﻿// HASHIMOTO, Yasuhiro (E-mail: hy @ sys.t.u-tokyo.ac.jp)
// update: 2008/11/28

/* Copyright (c) 2005-2009, HASHIMOTO, Yasuhiro, All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *   - Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *
 *   - Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *
 *   - Neither the name of the University of Tokyo nor the names of its contributors
 *     may be used to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA,
 * OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef INCLUDE_GUARD_SHARED_MATH_H
#define INCLUDE_GUARD_SHARED_MATH_H

#include <cmath>

namespace hashimoto_ut {

  namespace {

    template <typename T>
    inline T sqr(T value) { return value*value; }

#undef M_PI
    double const M_PI    = 3.141592653589793238462643383280;

#undef M_PI2
    double const M_PI2   = 1.570796326794896619231321691640;

#undef M_PI3
    double const M_PI3   = M_PI/3.0;

#undef M_PI4
    double const M_PI4   = M_PI/4.0;

#undef M_PI6
    double const M_PI6   = M_PI/6.0;

#undef M_2PI
    double const M_2PI   = 6.283185307179586476925286766559;

#undef M_SQRT2
    double const M_SQRT2 = 1.414213562373095048801688724210;

#undef M_SQRT3
    double const M_SQRT3 = 1.732050807568877293527446341506;

#undef M_E
    double const M_E     = 2.718281828459045235360287471353;

    float const COS360[] = {
      1.000000f,0.999848f,0.999391f,0.998630f,0.997564f,
      0.996195f,0.994522f,0.992546f,0.990268f,0.987688f,
      0.984808f,0.981627f,0.978148f,0.974370f,0.970296f,
      0.965926f,0.961262f,0.956305f,0.951057f,0.945519f,
      0.939693f,0.933580f,0.927184f,0.920505f,0.913545f,
      0.906308f,0.898794f,0.891007f,0.882948f,0.874620f,
      0.866025f,0.857167f,0.848048f,0.838671f,0.829038f,
      0.819152f,0.809017f,0.798636f,0.788011f,0.777146f,
      0.766044f,0.754710f,0.743145f,0.731354f,0.719340f,
      0.707107f,0.694658f,0.681998f,0.669131f,0.656059f,
      0.642788f,0.629320f,0.615661f,0.601815f,0.587785f,
      0.573576f,0.559193f,0.544639f,0.529919f,0.515038f,
      0.500000f,0.484810f,0.469472f,0.453990f,0.438371f,
      0.422618f,0.406737f,0.390731f,0.374607f,0.358368f,
      0.342020f,0.325568f,0.309017f,0.292372f,0.275637f,
      0.258819f,0.241922f,0.224951f,0.207912f,0.190809f,
      0.173648f,0.156434f,0.139173f,0.121869f,0.104528f,
      0.087156f,0.069756f,0.052336f,0.034899f,0.017452f,
      0.000000f,-0.017452f,-0.034899f,-0.052336f,-0.069756f,
      -0.087156f,-0.104528f,-0.121869f,-0.139173f,-0.156434f,
      -0.173648f,-0.190809f,-0.207912f,-0.224951f,-0.241922f,
      -0.258819f,-0.275637f,-0.292372f,-0.309017f,-0.325568f,
      -0.342020f,-0.358368f,-0.374607f,-0.390731f,-0.406737f,
      -0.422618f,-0.438371f,-0.453990f,-0.469472f,-0.484810f,
      -0.500000f,-0.515038f,-0.529919f,-0.544639f,-0.559193f,
      -0.573576f,-0.587785f,-0.601815f,-0.615661f,-0.629320f,
      -0.642788f,-0.656059f,-0.669131f,-0.681998f,-0.694658f,
      -0.707107f,-0.719340f,-0.731354f,-0.743145f,-0.754710f,
      -0.766044f,-0.777146f,-0.788011f,-0.798636f,-0.809017f,
      -0.819152f,-0.829038f,-0.838671f,-0.848048f,-0.857167f,
      -0.866025f,-0.874620f,-0.882948f,-0.891007f,-0.898794f,
      -0.906308f,-0.913545f,-0.920505f,-0.927184f,-0.933580f,
      -0.939693f,-0.945519f,-0.951057f,-0.956305f,-0.961262f,
      -0.965926f,-0.970296f,-0.974370f,-0.978148f,-0.981627f,
      -0.984808f,-0.987688f,-0.990268f,-0.992546f,-0.994522f,
      -0.996195f,-0.997564f,-0.998630f,-0.999391f,-0.999848f,
      -1.000000f,-0.999848f,-0.999391f,-0.998630f,-0.997564f,
      -0.996195f,-0.994522f,-0.992546f,-0.990268f,-0.987688f,
      -0.984808f,-0.981627f,-0.978148f,-0.974370f,-0.970296f,
      -0.965926f,-0.961262f,-0.956305f,-0.951057f,-0.945519f,
      -0.939693f,-0.933580f,-0.927184f,-0.920505f,-0.913545f,
      -0.906308f,-0.898794f,-0.891007f,-0.882948f,-0.874620f,
      -0.866025f,-0.857167f,-0.848048f,-0.838671f,-0.829038f,
      -0.819152f,-0.809017f,-0.798636f,-0.788011f,-0.777146f,
      -0.766044f,-0.754710f,-0.743145f,-0.731354f,-0.719340f,
      -0.707107f,-0.694658f,-0.681998f,-0.669131f,-0.656059f,
      -0.642788f,-0.629320f,-0.615661f,-0.601815f,-0.587785f,
      -0.573576f,-0.559193f,-0.544639f,-0.529919f,-0.515038f,
      -0.500000f,-0.484810f,-0.469472f,-0.453990f,-0.438371f,
      -0.422618f,-0.406737f,-0.390731f,-0.374607f,-0.358368f,
      -0.342020f,-0.325568f,-0.309017f,-0.292372f,-0.275637f,
      -0.258819f,-0.241922f,-0.224951f,-0.207912f,-0.190809f,
      -0.173648f,-0.156434f,-0.139173f,-0.121869f,-0.104528f,
      -0.087156f,-0.069756f,-0.052336f,-0.034899f,-0.017452f,
      -0.000000f,0.017452f,0.034899f,0.052336f,0.069756f,
      0.087156f,0.104528f,0.121869f,0.139173f,0.156434f,
      0.173648f,0.190809f,0.207912f,0.224951f,0.241922f,
      0.258819f,0.275637f,0.292372f,0.309017f,0.325568f,
      0.342020f,0.358368f,0.374607f,0.390731f,0.406737f,
      0.422618f,0.438371f,0.453990f,0.469472f,0.484810f,
      0.500000f,0.515038f,0.529919f,0.544639f,0.559193f,
      0.573576f,0.587785f,0.601815f,0.615661f,0.629320f,
      0.642788f,0.656059f,0.669131f,0.681998f,0.694658f,
      0.707107f,0.719340f,0.731354f,0.743145f,0.754710f,
      0.766044f,0.777146f,0.788011f,0.798636f,0.809017f,
      0.819152f,0.829038f,0.838671f,0.848048f,0.857167f,
      0.866025f,0.874620f,0.882948f,0.891007f,0.898794f,
      0.906308f,0.913545f,0.920505f,0.927184f,0.933580f,
      0.939693f,0.945519f,0.951057f,0.956305f,0.961262f,
      0.965926f,0.970296f,0.974370f,0.978148f,0.981627f,
      0.984808f,0.987688f,0.990268f,0.992546f,0.994522f,
      0.996195f,0.997564f,0.998630f,0.999391f,0.999848f
      };

    float const SIN360[] = {
      0.000000f,0.017452f,0.034899f,0.052336f,0.069756f,
      0.087156f,0.104528f,0.121869f,0.139173f,0.156434f,
      0.173648f,0.190809f,0.207912f,0.224951f,0.241922f,
      0.258819f,0.275637f,0.292372f,0.309017f,0.325568f,
      0.342020f,0.358368f,0.374607f,0.390731f,0.406737f,
      0.422618f,0.438371f,0.453990f,0.469472f,0.484810f,
      0.500000f,0.515038f,0.529919f,0.544639f,0.559193f,
      0.573576f,0.587785f,0.601815f,0.615661f,0.629320f,
      0.642788f,0.656059f,0.669131f,0.681998f,0.694658f,
      0.707107f,0.719340f,0.731354f,0.743145f,0.754710f,
      0.766044f,0.777146f,0.788011f,0.798636f,0.809017f,
      0.819152f,0.829038f,0.838671f,0.848048f,0.857167f,
      0.866025f,0.874620f,0.882948f,0.891007f,0.898794f,
      0.906308f,0.913545f,0.920505f,0.927184f,0.933580f,
      0.939693f,0.945519f,0.951057f,0.956305f,0.961262f,
      0.965926f,0.970296f,0.974370f,0.978148f,0.981627f,
      0.984808f,0.987688f,0.990268f,0.992546f,0.994522f,
      0.996195f,0.997564f,0.998630f,0.999391f,0.999848f,
      1.000000f,0.999848f,0.999391f,0.998630f,0.997564f,
      0.996195f,0.994522f,0.992546f,0.990268f,0.987688f,
      0.984808f,0.981627f,0.978148f,0.974370f,0.970296f,
      0.965926f,0.961262f,0.956305f,0.951057f,0.945519f,
      0.939693f,0.933580f,0.927184f,0.920505f,0.913545f,
      0.906308f,0.898794f,0.891007f,0.882948f,0.874620f,
      0.866025f,0.857167f,0.848048f,0.838671f,0.829038f,
      0.819152f,0.809017f,0.798636f,0.788011f,0.777146f,
      0.766044f,0.754710f,0.743145f,0.731354f,0.719340f,
      0.707107f,0.694658f,0.681998f,0.669131f,0.656059f,
      0.642788f,0.629320f,0.615661f,0.601815f,0.587785f,
      0.573576f,0.559193f,0.544639f,0.529919f,0.515038f,
      0.500000f,0.484810f,0.469472f,0.453990f,0.438371f,
      0.422618f,0.406737f,0.390731f,0.374607f,0.358368f,
      0.342020f,0.325568f,0.309017f,0.292372f,0.275637f,
      0.258819f,0.241922f,0.224951f,0.207912f,0.190809f,
      0.173648f,0.156434f,0.139173f,0.121869f,0.104528f,
      0.087156f,0.069756f,0.052336f,0.034899f,0.017452f,
      0.000000f,-0.017452f,-0.034899f,-0.052336f,-0.069756f,
      -0.087156f,-0.104528f,-0.121869f,-0.139173f,-0.156434f,
      -0.173648f,-0.190809f,-0.207912f,-0.224951f,-0.241922f,
      -0.258819f,-0.275637f,-0.292372f,-0.309017f,-0.325568f,
      -0.342020f,-0.358368f,-0.374607f,-0.390731f,-0.406737f,
      -0.422618f,-0.438371f,-0.453990f,-0.469472f,-0.484810f,
      -0.500000f,-0.515038f,-0.529919f,-0.544639f,-0.559193f,
      -0.573576f,-0.587785f,-0.601815f,-0.615661f,-0.629320f,
      -0.642788f,-0.656059f,-0.669131f,-0.681998f,-0.694658f,
      -0.707107f,-0.719340f,-0.731354f,-0.743145f,-0.754710f,
      -0.766044f,-0.777146f,-0.788011f,-0.798636f,-0.809017f,
      -0.819152f,-0.829038f,-0.838671f,-0.848048f,-0.857167f,
      -0.866025f,-0.874620f,-0.882948f,-0.891007f,-0.898794f,
      -0.906308f,-0.913545f,-0.920505f,-0.927184f,-0.933580f,
      -0.939693f,-0.945519f,-0.951057f,-0.956305f,-0.961262f,
      -0.965926f,-0.970296f,-0.974370f,-0.978148f,-0.981627f,
      -0.984808f,-0.987688f,-0.990268f,-0.992546f,-0.994522f,
      -0.996195f,-0.997564f,-0.998630f,-0.999391f,-0.999848f,
      -1.000000f,-0.999848f,-0.999391f,-0.998630f,-0.997564f,
      -0.996195f,-0.994522f,-0.992546f,-0.990268f,-0.987688f,
      -0.984808f,-0.981627f,-0.978148f,-0.974370f,-0.970296f,
      -0.965926f,-0.961262f,-0.956305f,-0.951057f,-0.945519f,
      -0.939693f,-0.933580f,-0.927184f,-0.920505f,-0.913545f,
      -0.906308f,-0.898794f,-0.891007f,-0.882948f,-0.874620f,
      -0.866025f,-0.857167f,-0.848048f,-0.838671f,-0.829038f,
      -0.819152f,-0.809017f,-0.798636f,-0.788011f,-0.777146f,
      -0.766044f,-0.754710f,-0.743145f,-0.731354f,-0.719340f,
      -0.707107f,-0.694658f,-0.681998f,-0.669131f,-0.656059f,
      -0.642788f,-0.629320f,-0.615661f,-0.601815f,-0.587785f,
      -0.573576f,-0.559193f,-0.544639f,-0.529919f,-0.515038f,
      -0.500000f,-0.484810f,-0.469472f,-0.453990f,-0.438371f,
      -0.422618f,-0.406737f,-0.390731f,-0.374607f,-0.358368f,
      -0.342020f,-0.325568f,-0.309017f,-0.292372f,-0.275637f,
      -0.258819f,-0.241922f,-0.224951f,-0.207912f,-0.190809f,
      -0.173648f,-0.156434f,-0.139173f,-0.121869f,-0.104528f,
      -0.087156f,-0.069756f,-0.052336f,-0.034899f,-0.017452f
      };
  } // The end of the anonymous namespace
} // The end of the namespace "hashimoto_ut"

#endif // INCLUDE_GUARD_SHARED_MATH_H
